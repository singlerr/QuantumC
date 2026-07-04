#include "ast.h"
#include "ast_sem.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>

/* ---- IR allocation helpers ---- */

identifier *
new_identifier (char *name)
{
  identifier *id = IALLOC (identifier);
  id->name = name ? strdup (name) : strdup ("");
  return id;
}

static expression *
mk_int_lit (int n)
{
  expression *e = IALLOC (expression);
  e->kind = EXPR_LITERAL;
  e->as.literal.literal_kind = LIT_DEC_INT;
  e->as.literal.data.i = n;
  return e;
}

static statement_list *
append_stmt (statement_list *head, statement *s)
{
  statement_list *node = IALLOC (statement_list);
  node->value = s;
  node->next = NULL;
  node->prev = NULL;
  if (!head)
    return node;
  statement_list *it = head;
  while (it->next)
    it = it->next;
  it->next = node;
  node->prev = it;
  return head;
}

static expression_list *
append_expr (expression_list *head, expression *e)
{
  expression_list *node = IALLOC (expression_list);
  node->value = e;
  node->next = NULL;
  node->prev = NULL;
  if (!head)
    return node;
  expression_list *it = head;
  while (it->next)
    it = it->next;
  it->next = node;
  node->prev = it;
  return head;
}

static qubit_list *
append_qubit (qubit_list *head, qubit *q)
{
  qubit_list *node = IALLOC (qubit_list);
  node->value = q;
  node->next = NULL;
  node->prev = NULL;
  if (!head)
    return node;
  qubit_list *it = head;
  while (it->next)
    it = it->next;
  it->next = node;
  node->prev = it;
  return head;
}

/* ---- type conversion ---- */

static classical_type *
mk_cls (type_kind kind, const char *name, int bits)
{
  classical_type *ct = IALLOC (classical_type);
  ct->kind = kind;
  ct->type_name = strdup (name);
  switch (kind)
    {
    case TYPE_INT:
      ct->int_type = IALLOC (int_type);
      ct->int_type->size = mk_int_lit (bits);
      break;
    case TYPE_UINT:
      ct->uint_type = IALLOC (uint_type);
      ct->uint_type->size = mk_int_lit (bits);
      break;
    case TYPE_FLOAT:
      ct->float_type = IALLOC (float_type);
      ct->float_type->size = mk_int_lit (bits);
      break;
    case TYPE_BOOL:
      ct->bool_type = IALLOC (bool_type);
      ct->bool_type->size = mk_int_lit (1);
      break;
    default:
      break;
    }
  return ct;
}

static classical_type *
type_to_cls (type_t *ty)
{
  if (!ty)
    return mk_cls (TYPE_INT, "int", 32);
  switch (ty->tag)
    {
    case TY_INT:    return mk_cls (TYPE_INT,   "int",   ty->size * 8);
    case TY_UINT:   return mk_cls (TYPE_UINT,  "uint",  ty->size * 8);
    case TY_FLOAT:
    case TY_DOUBLE: return mk_cls (TYPE_FLOAT, "float", ty->size * 8);
    case TY_BOOL:   return mk_cls (TYPE_BOOL,  "bool",  1);
    case TY_CHAR:   return mk_cls (TYPE_INT,   "int",   8);
    case TY_SHORT:  return mk_cls (TYPE_INT,   "int",   16);
    case TY_LONG:   return mk_cls (TYPE_INT,   "int",   64);
    default:        return mk_cls (TYPE_INT,   "int",   32);
    }
}

/* ---- operator mapping ---- */

static operator
ast_to_op (ast_tag_t tag)
{
  switch (tag)
    {
    case AST_ADD: case AST_ASSIGN_ADD: return OP_PLUS;
    case AST_SUB: case AST_ASSIGN_SUB: return OP_MINUS;
    case AST_MUL: case AST_ASSIGN_MUL: return OP_ASTERISK;
    case AST_DIV: case AST_ASSIGN_DIV: return OP_SLASH;
    case AST_MOD: case AST_ASSIGN_MOD: return OP_PERCENT;
    case AST_LSHIFT: case AST_ASSIGN_LSHIFT: return OP_LSHIFT;
    case AST_RSHIFT: case AST_ASSIGN_RSHIFT: return OP_RSHIFT;
    case AST_AND: case AST_ASSIGN_AND: return OP_AMP;
    case AST_OR:  case AST_ASSIGN_OR:  return OP_PIPE;
    case AST_XOR: case AST_ASSIGN_XOR: return OP_CARET;
    case AST_LAND: return OP_DOUBLE_AMP;
    case AST_LOR:  return OP_DOUBLE_PIPE;
    case AST_EQ:   return OP_EQ;
    case AST_NEQ:  return OP_NEQ;
    case AST_LT:   return OP_LT;
    case AST_GT:   return OP_GT;
    case AST_LEQ:  return OP_LEQ;
    case AST_GEQ:  return OP_GEQ;
    case AST_ASSIGN: return OP_ASSIGN;
    case AST_UNARY_MINUS: return OP_MINUS;
    case AST_UNARY_NOT:   return OP_TILDE;
    case AST_UNARY_LNOT:  return OP_EXCLAMATION_POINT;
    default: return OP_PLUS;
    }
}

/* ---- forward declarations ---- */

static expression *conv_expr (ast_t *a);
static statement *conv_stmt (ast_t *a);
static statement_list *conv_compound (ast_t *a);

/* ---- qubit arg extraction ---- */

static qubit *
ast_to_qubit (ast_t *a)
{
  if (!a)
    return NULL;
  qubit *q = IALLOC (qubit);
  if (a->tag == AST_ARRAY_ACCESS && a->arr_access.array
      && a->arr_access.array->tag == AST_VAR)
    {
      identifier *arr_id = new_identifier (a->arr_access.array->var.name);
      expr_or_range *eor = IALLOC (expr_or_range);
      eor->expr = conv_expr (a->arr_access.index);
      index_element *elem = IALLOC (index_element);
      elem->kind = EXPR_EXPRESSION;
      elem->index.expr_or_range = eor;
      indexed_identifier *iid = IALLOC (indexed_identifier);
      iid->name = arr_id;
      iid->index = elem;
      q->kind = ID_INDEXED_IDENTIFIER;
      q->value.indexed_identifier = iid;
    }
  else
    {
      q->kind = ID_IDENTIFIER;
      q->value.identifier
          = new_identifier (a->tag == AST_VAR ? a->var.name : "q");
    }
  return q;
}

static qubit_list *
flatten_qubit_args (ast_t *a)
{
  if (!a)
    return NULL;
  if (a->tag == AST_LIST)
    {
      qubit_list *prev = flatten_qubit_args (a->expr_list.prev);
      qubit *q = ast_to_qubit (a->expr_list.value);
      return append_qubit (prev, q);
    }
  return append_qubit (NULL, ast_to_qubit (a));
}

/* ---- expression conversion ---- */

static expression_list *
flatten_arg_list (ast_t *a)
{
  if (!a)
    return NULL;
  if (a->tag == AST_LIST)
    {
      expression_list *prev = flatten_arg_list (a->expr_list.prev);
      expression *val = conv_expr (a->expr_list.value);
      return append_expr (prev, val);
    }
  return append_expr (NULL, conv_expr (a));
}

static expression *
conv_expr (ast_t *a)
{
  if (!a)
    return mk_int_lit (0);
  expression *e;
  switch (a->tag)
    {
    case AST_VAR:
      e = IALLOC (expression);
      e->kind = EXPR_IDENTIFIER;
      e->as.identifier = new_identifier (a->var.name);
      return e;

    case AST_INT:
      return mk_int_lit (a->literal.i);

    case AST_FLOAT:
      e = IALLOC (expression);
      e->kind = EXPR_LITERAL;
      e->as.literal.literal_kind = LIT_FLOAT;
      e->as.literal.data.f = a->literal.f;
      return e;

    case AST_ARRAY_ACCESS:
      {
        expr_or_range *eor = IALLOC (expr_or_range);
        eor->expr = conv_expr (a->arr_access.index);
        expr_or_range_list *il = IALLOC (expr_or_range_list);
        il->value = eor;
        il->next = NULL;
        il->prev = NULL;
        e = IALLOC (expression);
        e->kind = EXPR_INDEX;
        e->as.index.collection = conv_expr (a->arr_access.array);
        e->as.index.index_kind = SINGLE_EXPRESSION;
        e->as.index.list = il;
        return e;
      }

    case AST_POST_INC: case AST_PRE_INC:
      e = IALLOC (expression);
      e->kind = EXPR_BINARY;
      e->as.binary.op = OP_PLUS_ASSIGN;
      e->as.binary.lhs = conv_expr (a->expr_unary.value);
      e->as.binary.rhs = mk_int_lit (1);
      return e;

    case AST_POST_DEC: case AST_PRE_DEC:
      e = IALLOC (expression);
      e->kind = EXPR_BINARY;
      e->as.binary.op = OP_MINUS_ASSIGN;
      e->as.binary.lhs = conv_expr (a->expr_unary.value);
      e->as.binary.rhs = mk_int_lit (1);
      return e;

    case AST_APP:
      {
        ast_t *fun = a->app.fun;
        ast_t *arg = cvector_size (a->app.args) > 0 ? a->app.args[0] : NULL;
        if (fun && fun->tag == AST_VAR)
          {
            const char *fname = fun->var.name;
            if (strncmp (fname, "apply_", 6) == 0)
              {
                e = IALLOC (expression);
                e->kind = EXPR_QUANTUM_GATE;
                e->as.quantum.quantum_gate.name = new_identifier ((char *)(fname + 6));
                e->as.quantum.quantum_gate.modifiers = NULL;
                e->as.quantum.quantum_gate.arguments = NULL;
                e->as.quantum.quantum_gate.designator = NULL;
                e->as.quantum.quantum_gate.qubits = flatten_qubit_args (arg);
                return e;
              }
            if (strcmp (fname, "measure") == 0)
              {
                e = IALLOC (expression);
                e->kind = EXPR_QUANTUM_MEASUREMENT;
                e->as.quantum_measurement.measure.qubit = ast_to_qubit (arg);
                return e;
              }
          }
        e = IALLOC (expression);
        e->kind = EXPR_FUNC_CALL;
        e->as.function_call.name
            = new_identifier (fun && fun->tag == AST_VAR ? fun->var.name : "");
        e->as.function_call.arguments = arg ? flatten_arg_list (arg) : NULL;
        return e;
      }

    case AST_CAST:
      e = IALLOC (expression);
      e->kind = EXPR_CAST;
      e->as.cast.argument = conv_expr (a->expr_cast.value);
      e->as.cast.type
          = type_to_cls (a->expr_cast.ty_caster ? a->expr_cast.ty_caster->ty : NULL);
      return e;

    default:
      if (is_binary_operator (a->tag) || is_assignment_operator (a->tag))
        {
          e = IALLOC (expression);
          e->kind = EXPR_BINARY;
          e->as.binary.op = ast_to_op (a->tag);
          e->as.binary.lhs = conv_expr (a->expr_binary.lhs);
          e->as.binary.rhs = conv_expr (a->expr_binary.rhs);
          return e;
        }
      switch (a->tag)
        {
        case AST_UNARY_MINUS: case AST_UNARY_NOT: case AST_UNARY_LNOT:
        case AST_UNARY_PLUS:  case AST_UNARY_REF: case AST_UNARY_DEREF:
          e = IALLOC (expression);
          e->kind = EXPR_UNARY;
          e->as.unary.op = ast_to_op (a->tag);
          e->as.unary.expr = conv_expr (a->expr_unary.value);
          return e;
        default:
          return mk_int_lit (0);
        }
    }
}

/* ---- declaration list → statement_list ---- */

static statement_list *
conv_decl_list (decl_list_t list)
{
  statement_list *head = NULL;
  decl_t *it;
  for (it = cvector_begin (list); it != cvector_end (list); it++)
    {
      if (!it->has_name)
        continue;
      if (it->type && (it->type->constr & CONSTR_TYPEDEF))
        continue;
      type_t *ty = it->type ? it->type->ty : NULL;
      int is_qubit = 0;
      int qubit_size = 1;
      if (ty)
        {
          if (ty->tag == TY_QUBIT)
            {
              is_qubit = 1;
            }
          else if (ty->tag == TY_ARRAY && ty->ty.ty_array.ref
                   && ty->ty.ty_array.ref->ty
                   && ty->ty.ty_array.ref->ty->tag == TY_QUBIT)
            {
              is_qubit = 1;
              qubit_size = ty->ty.ty_array.size > 0 ? ty->ty.ty_array.size : 1;
            }
        }
      statement *s = IALLOC (statement);
      if (is_qubit)
        {
          s->kind = STMT_QUANTUM_DECLARATION;
          s->classical.qubit_declaration.qubit = new_identifier (it->name);
          s->classical.qubit_declaration.size = mk_int_lit (qubit_size);
        }
      else
        {
          s->kind = STMT_CLASSICAL_DECLARATION;
          s->classical.declaration.type = type_to_cls (ty);
          s->classical.declaration.identifier = new_identifier (it->name);
          if (it->init)
            {
              s->classical.declaration.init_expression_kind = EXPR_EXPRESSION;
              s->classical.declaration.init_expression.expr
                  = conv_expr (it->init);
            }
          else
            {
              s->classical.declaration.init_expression_kind = EXPR_NONE;
            }
        }
      head = append_stmt (head, s);
    }
  return head;
}

/* ---- compound body → statement_list ---- */

static statement_list *
conv_compound (ast_t *a)
{
  if (!a)
    return NULL;
  if (a->tag == AST_COMPOUND)
    {
      statement_list *head = NULL;
      ast_t **it;
      for (it = cvector_begin (a->stmt_compound.ast);
           it != cvector_end (a->stmt_compound.ast); it++)
        {
          statement *s = conv_stmt (*it);
          if (s)
            head = append_stmt (head, s);
        }
      return head;
    }
  /* single statement */
  statement *s = conv_stmt (a);
  return s ? append_stmt (NULL, s) : NULL;
}

/* ---- statement conversion ---- */

static statement *
conv_stmt (ast_t *a)
{
  if (!a)
    return NULL;
  statement *s = IALLOC (statement);
  switch (a->tag)
    {
    case AST_COMPOUND:
      s->kind = STMT_COMPOUND;
      s->classical.compound.statements = conv_compound (a);
      break;

    case AST_IF:
      s->kind = STMT_IF;
      s->classical.branching.condition = conv_expr (a->stmt_if.condition);
      s->classical.branching.if_block = conv_compound (a->stmt_if.body);
      s->classical.branching.else_block = NULL;
      break;

    case AST_IF_ELSE:
      s->kind = STMT_IF;
      s->classical.branching.condition = conv_expr (a->stmt_if_else.condition);
      s->classical.branching.if_block = conv_compound (a->stmt_if_else.body);
      s->classical.branching.else_block = conv_compound (a->stmt_if_else.else_body);
      break;

    case AST_WHILE:
    case AST_DO_WHILE:
      s->kind = STMT_WHILE;
      s->classical.while_loop.condition = conv_expr (a->stmt_while.condition);
      s->classical.while_loop.block = conv_compound (a->stmt_while.body);
      break;

    case AST_FOR:
      {
        /* lower C for-loop to { init; while (cond) { body; inc; } } */
        statement_list *stmts = NULL;
        if (a->stmt_for.lhs)
          {
            if (a->stmt_for.lhs->tag == AST_DECL)
              {
                statement_list *decls = conv_decl_list (a->stmt_for.lhs->decl_list);
                statement_list *d;
                for (d = decls; d; d = d->next)
                  stmts = append_stmt (stmts, d->value);
              }
            else
              {
                statement *init = IALLOC (statement);
                init->kind = STMT_EXPRESSION;
                init->classical.expression.expr = conv_expr (a->stmt_for.lhs);
                stmts = append_stmt (stmts, init);
              }
          }
        statement *whl = IALLOC (statement);
        whl->kind = STMT_WHILE;
        whl->classical.while_loop.condition
            = a->stmt_for.mhs ? conv_expr (a->stmt_for.mhs) : mk_int_lit (1);
        statement_list *body = conv_compound (a->stmt_for.body);
        if (a->stmt_for.rhs)
          {
            statement *inc = IALLOC (statement);
            inc->kind = STMT_EXPRESSION;
            inc->classical.expression.expr = conv_expr (a->stmt_for.rhs);
            body = append_stmt (body, inc);
          }
        whl->classical.while_loop.block = body;
        stmts = append_stmt (stmts, whl);
        s->kind = STMT_COMPOUND;
        s->classical.compound.statements = stmts;
        break;
      }

    case AST_RETURN:
      s->kind = STMT_RETURN;
      s->classical.retrn.kind = EXPR_EXPRESSION;
      s->classical.retrn.expr.expr
          = a->expr_unary.value ? conv_expr (a->expr_unary.value) : NULL;
      break;

    case AST_BREAK:
      s->kind = STMT_BREAK;
      break;

    case AST_CONTINUE:
      s->kind = STMT_CONTINUE;
      break;

    case AST_DECL:
      {
        statement_list *decls = conv_decl_list (a->decl_list);
        if (!decls)
          {
            free (s);
            return NULL;
          }
        if (!decls->next)
          {
            /* single decl — unwrap the list node */
            statement *single = decls->value;
            free (decls);
            free (s);
            return single;
          }
        s->kind = STMT_COMPOUND;
        s->classical.compound.statements = decls;
        break;
      }

    case AST_FUN:
      s->kind = STMT_DEF;
      s->classical.subroutine_definition.name
          = new_identifier (a->fun.name[0] ? a->fun.name : "unnamed");
      s->classical.subroutine_definition.arguments = NULL;
      s->classical.subroutine_definition.body = conv_compound (a->fun.body);
      s->classical.subroutine_definition.return_type
          = type_to_cls (a->fun.ty_fun);
      break;

    default:
      /* expression statement */
      s->kind = STMT_EXPRESSION;
      s->classical.expression.expr = conv_expr (a);
      break;
    }
  return s;
}

/* ---- top-level walk (AST_LIST chain from translation_unit) ---- */

static statement_list *
conv_top_level (ast_t *root)
{
  if (!root)
    return NULL;
  if (root->tag == AST_LIST)
    {
      statement_list *prev = conv_top_level (root->expr_list.prev);
      statement *s = conv_stmt (root->expr_list.value);
      return s ? append_stmt (prev, s) : prev;
    }
  statement *s = conv_stmt (root);
  return s ? append_stmt (NULL, s) : NULL;
}

/* ---- public API ---- */

void
convert_program (ast_t *root, program **out)
{
  *out = IALLOC (program);
  (*out)->stmts = conv_top_level (root);
}
