#include "hir.h"

#include <stdio.h>
#include <string.h>

#include "../common.h"
#include "../diagnostics.h"

typedef struct
{
  hir_program *prog;
  hir_sub *sub;
  hir *tail;
  uint32_t next_tmp;
  uint32_t next_label;
  char *break_label;
  char *continue_label;
} lower_ctx;

static hir_operand
op_none (void)
{
  hir_operand o;
  o.kind = HO_NONE;
  return o;
}

static hir_operand
op_var (const char *name, type_t *ty)
{
  hir_operand o;
  o.kind = HO_VAR;
  o.var.name = strdup (name);
  o.var.owner = 0;
  o.var.ty = ty;
  return o;
}

static hir_operand
op_temp (lower_ctx *ctx, type_t *ty)
{
  hir_operand o;
  o.kind = HO_TEMP;
  o.tmp.id = ctx->next_tmp++;
  o.tmp.ty = ty;
  return o;
}

static hir_operand
op_lit_int (int v)
{
  hir_operand o;
  o.kind = HO_LIT;
  o.lit.type = H_INT;
  o.lit.size = sizeof (int);
  o.lit.val.i = v;
  return o;
}

static hir_operand
op_lit_float (float v)
{
  hir_operand o;
  o.kind = HO_LIT;
  o.lit.type = H_FLOAT;
  o.lit.size = sizeof (float);
  o.lit.val.f = v;
  return o;
}

static hir_operand
op_label (char *name)
{
  hir_operand o;
  o.kind = HO_LABEL;
  o.label.name = name;
  return o;
}

static char *
new_label (lower_ctx *ctx)
{
  char buf[16];
  snprintf (buf, sizeof (buf), "L%u", ctx->next_label++);
  return strdup (buf);
}

static hir *
emit (lower_ctx *ctx, h_opcode op, hir_operand dst, hir_operand src1,
      hir_operand src2)
{
  hir *ins = IALLOC (hir);
  ins->op = op;
  ins->dst = dst;
  ins->src1 = src1;
  ins->src2 = src2;
  ins->prev = ctx->tail;
  ins->next = NULL;
  if (ctx->tail)
    ctx->tail->next = ins;
  else
    ctx->sub->entry = ins;
  ctx->tail = ins;
  return ins;
}

static type_t *
find_var_type (lower_ctx *ctx, const char *name)
{
  hir_var *it;
  if (ctx->sub)
    for (it = cvector_begin (ctx->sub->locals); it != cvector_end (ctx->sub->locals); it++)
      if (strcmp (it->name, name) == 0)
        return it->ty;
  if (ctx->prog)
    for (it = cvector_begin (ctx->prog->globals); it != cvector_end (ctx->prog->globals); it++)
      if (strcmp (it->name, name) == 0)
        return it->ty;
  return NULL;
}

static type_t *
cls_to_type (classical_type *ct)
{
  if (!ct)
    return new_simple_type (SIZE_INT, TY_INT);
  switch (ct->kind)
    {
    case TYPE_INT:
      return new_simple_type (ct->int_type->size->as.literal.data.i / 8, TY_INT);
    case TYPE_UINT:
      return new_simple_type (ct->uint_type->size->as.literal.data.i / 8, TY_UINT);
    case TYPE_FLOAT:
      return new_simple_type (ct->float_type->size->as.literal.data.i / 8, TY_FLOAT);
    case TYPE_BOOL:
      return new_simple_type (1, TY_BOOL);
    default:
      return new_simple_type (SIZE_INT, TY_INT);
    }
}

static int
is_compound_assign (operator op)
{
  switch (op)
    {
    case OP_PLUS_ASSIGN: case OP_MINUS_ASSIGN: case OP_MUL_ASSIGN:
    case OP_DIV_ASSIGN: case OP_MOD_ASSIGN: case OP_LSHIFT_ASSIGN:
    case OP_RSHIFT_ASSIGN: case OP_AND_ASSIGN: case OP_XOR_ASSIGN:
    case OP_OR_ASSIGN:
      return 1;
    default:
      return 0;
    }
}

static h_opcode
arith_opcode (operator op)
{
  switch (op)
    {
    case OP_PLUS: case OP_PLUS_ASSIGN: return H_ADD;
    case OP_MINUS: case OP_MINUS_ASSIGN: return H_SUB;
    case OP_ASTERISK: case OP_MUL_ASSIGN: return H_MUL;
    case OP_SLASH: case OP_DIV_ASSIGN: return H_DIV;
    case OP_PERCENT: case OP_MOD_ASSIGN: return H_MOD;
    case OP_LSHIFT: case OP_LSHIFT_ASSIGN: return H_SHL;
    case OP_RSHIFT: case OP_RSHIFT_ASSIGN: return H_SHR;
    case OP_AMP: case OP_AND_ASSIGN: case OP_DOUBLE_AMP: return H_AND;
    case OP_PIPE: case OP_OR_ASSIGN: case OP_DOUBLE_PIPE: return H_OR;
    case OP_CARET: case OP_XOR_ASSIGN: return H_XOR;
    case OP_LT: return H_LT;
    case OP_GT: return H_GT;
    case OP_GEQ: return H_GE;
    case OP_LEQ: return H_LE;
    case OP_EQ: return H_EQ;
    case OP_NEQ: return H_NE;
    default: return H_ADD;
    }
}

static hir_operand lower_expr (lower_ctx *ctx, expression *e);

static hir_operand
index_operand (lower_ctx *ctx, expression *e)
{
  if (e->as.index.list && e->as.index.list->value && e->as.index.list->value->expr)
    return lower_expr (ctx, e->as.index.list->value->expr);
  LOG_WARN ("unsupported index expression");
  return op_lit_int (0);
}

static void
store_to_lvalue (lower_ctx *ctx, expression *lhs, hir_operand value)
{
  if (lhs->kind == EXPR_IDENTIFIER)
    {
      hir_operand dst = op_var (lhs->as.identifier->name,
                                 find_var_type (ctx, lhs->as.identifier->name));
      emit (ctx, H_COPY, dst, value, op_none ());
      return;
    }
  if (lhs->kind == EXPR_INDEX)
    {
      hir_operand base = lower_expr (ctx, lhs->as.index.collection);
      hir_operand index = index_operand (ctx, lhs);
      emit (ctx, H_SET, base, index, value);
      return;
    }
  LOG_WARN ("unsupported assignment target kind %d", lhs->kind);
}

static hir_operand
lower_expr (lower_ctx *ctx, expression *e)
{
  if (!e)
    return op_lit_int (0);

  switch (e->kind)
    {
    case EXPR_IDENTIFIER:
      return op_var (e->as.identifier->name, find_var_type (ctx, e->as.identifier->name));

    case EXPR_LITERAL:
      switch (e->as.literal.literal_kind)
        {
        case LIT_BIN_INT: case LIT_OCT_INT: case LIT_DEC_INT: case LIT_HEX_INT:
          return op_lit_int (e->as.literal.data.i);
        case LIT_FLOAT:
          return op_lit_float (e->as.literal.data.f);
        case LIT_BOOL:
          return op_lit_int (e->as.literal.data.b);
        default:
          LOG_WARN ("unsupported literal kind %d", e->as.literal.literal_kind);
          return op_lit_int (0);
        }

    case EXPR_BINARY:
      {
        operator op = e->as.binary.op;
        if (op == OP_ASSIGN)
          {
            hir_operand rhs = lower_expr (ctx, e->as.binary.rhs);
            store_to_lvalue (ctx, e->as.binary.lhs, rhs);
            return rhs;
          }
        if (is_compound_assign (op))
          {
            hir_operand cur = lower_expr (ctx, e->as.binary.lhs);
            hir_operand rhs = lower_expr (ctx, e->as.binary.rhs);
            hir_operand result = op_temp (ctx, NULL);
            emit (ctx, arith_opcode (op), result, cur, rhs);
            store_to_lvalue (ctx, e->as.binary.lhs, result);
            return result;
          }
        hir_operand lhs = lower_expr (ctx, e->as.binary.lhs);
        hir_operand rhs = lower_expr (ctx, e->as.binary.rhs);
        hir_operand result = op_temp (ctx, NULL);
        emit (ctx, arith_opcode (op), result, lhs, rhs);
        return result;
      }

    case EXPR_UNARY:
      {
        hir_operand v = lower_expr (ctx, e->as.unary.expr);
        h_opcode op;
        switch (e->as.unary.op)
          {
          case OP_MINUS: op = H_NEG; break;
          case OP_TILDE: op = H_COMP; break;
          case OP_EXCLAMATION_POINT: op = H_NOT; break;
          default: return v;
          }
        hir_operand result = op_temp (ctx, NULL);
        emit (ctx, op, result, v, op_none ());
        return result;
      }

    case EXPR_CAST:
      return lower_expr (ctx, e->as.cast.argument);

    case EXPR_INDEX:
      {
        hir_operand base = lower_expr (ctx, e->as.index.collection);
        hir_operand index = index_operand (ctx, e);
        hir_operand result = op_temp (ctx, NULL);
        emit (ctx, H_GET, result, base, index);
        return result;
      }

    case EXPR_FUNC_CALL:
      {
        expression_list *arg;
        for (arg = e->as.function_call.arguments; arg; arg = arg->next)
          {
            hir_operand a = lower_expr (ctx, arg->value);
            emit (ctx, H_PARAM, op_none (), a, op_none ());
          }
        hir_operand result = op_temp (ctx, NULL);
        hir_operand callee = op_label (strdup (e->as.function_call.name->name));
        emit (ctx, H_CALL, result, callee, op_none ());
        return result;
      }

    default:
      LOG_WARN ("unsupported expression kind %d (quantum lowering not implemented yet)", e->kind);
      return op_lit_int (0);
    }
}

static void lower_stmt (lower_ctx *ctx, statement *s);

static void
lower_block (lower_ctx *ctx, statement_list *stmts)
{
  statement_list *it;
  for (it = stmts; it; it = it->next)
    lower_stmt (ctx, it->value);
}

static void
lower_stmt (lower_ctx *ctx, statement *s)
{
  if (!s)
    return;

  switch (s->kind)
    {
    case STMT_COMPOUND:
      lower_block (ctx, s->classical.compound.statements);
      break;

    case STMT_EXPRESSION:
      lower_expr (ctx, s->classical.expression.expr);
      break;

    case STMT_CLASSICAL_DECLARATION:
      {
        hir_var v;
        v.name = strdup (s->classical.declaration.identifier->name);
        v.owner = 0;
        v.ty = cls_to_type (s->classical.declaration.type);
        cvector_push_back (ctx->sub->locals, v);
        if (s->classical.declaration.init_expression_kind == EXPR_EXPRESSION
            && s->classical.declaration.init_expression.expr)
          {
            hir_operand val = lower_expr (ctx, s->classical.declaration.init_expression.expr);
            emit (ctx, H_COPY, op_var (v.name, v.ty), val, op_none ());
          }
        break;
      }

    case STMT_QUANTUM_DECLARATION:
      LOG_WARN ("qubit declaration '%s' skipped (quantum lowering not implemented yet)",
                s->classical.qubit_declaration.qubit->name);
      break;

    case STMT_IF:
      {
        hir_operand cond = lower_expr (ctx, s->classical.branching.condition);
        if (!s->classical.branching.else_block)
          {
            char *end = new_label (ctx);
            emit (ctx, H_CJUMP, op_none (), cond, op_label (end));
            lower_block (ctx, s->classical.branching.if_block);
            emit (ctx, H_LABEL, op_label (end), op_none (), op_none ());
          }
        else
          {
            char *else_l = new_label (ctx);
            char *end_l = new_label (ctx);
            emit (ctx, H_CJUMP, op_none (), cond, op_label (else_l));
            lower_block (ctx, s->classical.branching.if_block);
            emit (ctx, H_JUMP, op_none (), op_label (end_l), op_none ());
            emit (ctx, H_LABEL, op_label (else_l), op_none (), op_none ());
            lower_block (ctx, s->classical.branching.else_block);
            emit (ctx, H_LABEL, op_label (end_l), op_none (), op_none ());
          }
        break;
      }

    case STMT_WHILE:
      {
        char *start = new_label (ctx);
        char *end = new_label (ctx);
        char *prev_break = ctx->break_label;
        char *prev_cont = ctx->continue_label;
        ctx->break_label = end;
        ctx->continue_label = start;

        emit (ctx, H_LABEL, op_label (start), op_none (), op_none ());
        hir_operand cond = lower_expr (ctx, s->classical.while_loop.condition);
        emit (ctx, H_CJUMP, op_none (), cond, op_label (end));
        lower_block (ctx, s->classical.while_loop.block);
        emit (ctx, H_JUMP, op_none (), op_label (start), op_none ());
        emit (ctx, H_LABEL, op_label (end), op_none (), op_none ());

        ctx->break_label = prev_break;
        ctx->continue_label = prev_cont;
        break;
      }

    case STMT_RETURN:
      {
        hir_operand v = op_none ();
        if (s->classical.retrn.kind == EXPR_EXPRESSION && s->classical.retrn.expr.expr)
          v = lower_expr (ctx, s->classical.retrn.expr.expr);
        emit (ctx, H_RETURN, op_none (), v, op_none ());
        break;
      }

    case STMT_BREAK:
      if (ctx->break_label)
        emit (ctx, H_JUMP, op_none (), op_label (ctx->break_label), op_none ());
      else
        LOG_WARN ("break outside of loop");
      break;

    case STMT_CONTINUE:
      if (ctx->continue_label)
        emit (ctx, H_JUMP, op_none (), op_label (ctx->continue_label), op_none ());
      else
        LOG_WARN ("continue outside of loop");
      break;

    default:
      LOG_WARN ("unsupported statement kind %d", s->kind);
      break;
    }
}

void
hir_from_program (program *prog, hir_program *out)
{
  out->globals = NULL;
  out->subs = NULL;
  if (!prog)
    return;

  statement_list *it;
  for (it = prog->stmts; it; it = it->next)
    {
      statement *s = it->value;
      if (!s)
        continue;

      if (s->kind == STMT_DEF)
        {
          hir_sub sub;
          sub.name = strdup (s->classical.subroutine_definition.name->name);
          sub.ty_fun = NULL;
          sub.params = NULL;
          sub.locals = NULL;
          sub.entry = NULL;

          lower_ctx ctx;
          ctx.prog = out;
          ctx.sub = &sub;
          ctx.tail = NULL;
          ctx.next_tmp = 0;
          ctx.next_label = 0;
          ctx.break_label = NULL;
          ctx.continue_label = NULL;

          lower_block (&ctx, s->classical.subroutine_definition.body);

          cvector_push_back (out->subs, sub);
        }
      else if (s->kind == STMT_CLASSICAL_DECLARATION)
        {
          hir_var v;
          v.name = strdup (s->classical.declaration.identifier->name);
          v.owner = 0;
          v.ty = cls_to_type (s->classical.declaration.type);
          if (s->classical.declaration.init_expression_kind == EXPR_EXPRESSION)
            LOG_WARN ("global initializer for '%s' ignored", v.name);
          cvector_push_back (out->globals, v);
        }
      else
        {
          LOG_WARN ("unsupported top-level statement kind %d", s->kind);
        }
    }
}

void
hir_cleanup (hir **h)
{
  if (!h || !*h)
    return;
  hir *it = *h;
  while (it)
    {
      hir *next = it->next;
      free (it);
      it = next;
    }
  *h = NULL;
}
