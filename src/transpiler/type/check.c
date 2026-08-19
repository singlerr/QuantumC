#include "check.h"
#include "../ast.h"
#include "../symtab.h"
#include "deco.h"
#include "tytab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- delta helpers ---- */

static binding_t *
delta_find (binding_t *d, const char *name)
{
  for (; d; d = d->next)
    if (!strncmp (d->name, name, SYM_MAXLEN))
      return d;
  return NULL;
}

static binding_t *
delta_ensure (check_ctx_t *ctx, const char *name)
{
  binding_t *b = delta_find (ctx->delta, name);
  if (!b)
    {
      b = malloc (sizeof (binding_t));
      strncpy (b->name, name, SYM_MAXLEN);
      b->consumed = 0;
      b->next = ctx->delta;
      ctx->delta = b;
    }
  return b;
}

static binding_t *
delta_clone (binding_t *d)
{
  binding_t *head = NULL, *tail = NULL;
  for (; d; d = d->next)
    {
      binding_t *c = malloc (sizeof (binding_t));
      *c = *d;
      c->next = NULL;
      if (!tail)
        head = tail = c;
      else
        {
          tail->next = c;
          tail = c;
        }
    }
  return head;
}

static void
delta_free (binding_t *d)
{
  while (d)
    {
      binding_t *n = d->next;
      free (d);
      d = n;
    }
}

/* ---- type helpers ---- */

static int
is_scalar_qubit (type_t *ty)
{
  return ty && ty->tag == TY_QUBIT;
}

/* Usual arithmetic conversions: return the wider type. */
static type_t *
usual_arith (type_t *a, type_t *b)
{
  if (!a)
    return b;
  if (!b)
    return a;
  return (a->size >= b->size) ? a : b;
}

/* ---- qubit arg extraction ---- */

/* Returns the variable name if arg is a scalar qubit AST_VAR, else NULL. */
static const char *
scalar_qubit_name (ast_t *a)
{
  if (!a || a->tag != AST_VAR)
    return NULL;
  type_t *ty = a->ty ? a->ty->ty : NULL;
  if (!is_scalar_qubit (ty))
    return NULL;
  return a->var.name;
}

/* ---- gate/measure enforcement ---- */

static void
check_gate_args (check_ctx_t *ctx, ast_t *e)
{
  /* pairwise distinct check for scalar qubit args; mark consumed for measure
   */
  ast_t **arg;
  int is_measure = (e->app.fun && e->app.fun->tag == AST_VAR
                    && strcmp (e->app.fun->var.name, "measure") == 0);

  const char *seen[32];
  int nseen = 0;

  for (arg = cvector_begin (e->app.args); arg != cvector_end (e->app.args);
       arg++)
    {
      const char *name = scalar_qubit_name (*arg);
      if (!name)
        continue;

      /* aliasing check (gates only — measure takes one qubit anyway) */
      if (!is_measure)
        {
          int i;
          for (i = 0; i < nseen; i++)
            if (!strcmp (seen[i], name))
              {
                fprintf (stderr, "error: qubit '%s' aliased in gate call\n",
                         name);
                exit (1);
              }
          if (nseen < 32)
            seen[nseen++] = name;
        }

      binding_t *b = delta_ensure (ctx, name);
      if (b->consumed)
        {
          fprintf (stderr, "error: qubit '%s' used after measure\n", name);
          exit (1);
        }
      if (is_measure)
        b->consumed = 1;
    }
}

/* ---- branch/loop helpers ---- */

/* Report if any qubit newly consumed in 'post' that wasn't in 'pre'. */
static void
check_no_new_consumption (binding_t *pre, binding_t *post, const char *ctx_msg)
{
  binding_t *b;
  for (b = post; b; b = b->next)
    {
      if (!b->consumed)
        continue;
      binding_t *bpre = delta_find (pre, b->name);
      if (!bpre || !bpre->consumed)
        {
          fprintf (stderr, "error: qubit '%s' consumed in %s\n", b->name,
                   ctx_msg);
          exit (1);
        }
    }
}

/* Require both post_then and post_else consumed the same qubit set. */
static void
check_branch_join (binding_t *post_then, binding_t *post_else)
{
  binding_t *b;
  for (b = post_then; b; b = b->next)
    {
      if (!b->consumed)
        continue;
      binding_t *be = delta_find (post_else, b->name);
      if (!be || !be->consumed)
        {
          fprintf (stderr, "error: qubit '%s' consumed in then but not else\n",
                   b->name);
          exit (1);
        }
    }
  for (b = post_else; b; b = b->next)
    {
      if (!b->consumed)
        continue;
      binding_t *bt = delta_find (post_then, b->name);
      if (!bt || !bt->consumed)
        {
          fprintf (stderr, "error: qubit '%s' consumed in else but not then\n",
                   b->name);
          exit (1);
        }
    }
}

/* ---- synth_expr ---- */

type_t *
synth_expr (check_ctx_t *ctx, ast_t *e)
{
  if (!e)
    return NULL;
  if (e->ty && !e->ty->is_cast)
    return e->ty->ty;

  switch (e->tag)
    {
    case AST_INT:
      e->ty = Deco (Type (SIZE_INT, TY_INT), CONSTR_EMPTY);
      break;
    case AST_FLOAT:
      e->ty = Deco (Type (SIZE_FLOAT, TY_FLOAT), CONSTR_EMPTY);
      break;
    case AST_VAR:
      {
        ty_deco_t *d = search_symbol_type (e->var.name, 0);
        if (d)
          e->ty = d;
        break;
      }
    case AST_CAST:
      if (e->expr_cast.ty_caster)
        e->ty = e->expr_cast.ty_caster;
      break;

    case AST_ADD:
    case AST_SUB:
    case AST_MUL:
    case AST_DIV:
    case AST_MOD:
    case AST_AND:
    case AST_OR:
    case AST_XOR:
    case AST_LSHIFT:
    case AST_RSHIFT:
      {
        type_t *lt = synth_expr (ctx, e->expr_binary.lhs);
        type_t *rt = synth_expr (ctx, e->expr_binary.rhs);
        type_t *res = usual_arith (lt, rt);
        if (res)
          e->ty = Deco (res, CONSTR_EMPTY);
        break;
      }
    case AST_LT:
    case AST_GT:
    case AST_LEQ:
    case AST_GEQ:
    case AST_EQ:
    case AST_NEQ:
    case AST_LAND:
    case AST_LOR:
      e->ty = Deco (Type (SIZE_INT, TY_INT), CONSTR_EMPTY);
      synth_expr (ctx, e->expr_binary.lhs);
      synth_expr (ctx, e->expr_binary.rhs);
      break;

    case AST_ASSIGN:
    case AST_ASSIGN_ADD:
    case AST_ASSIGN_SUB:
    case AST_ASSIGN_MUL:
    case AST_ASSIGN_DIV:
    case AST_ASSIGN_MOD:
    case AST_ASSIGN_AND:
    case AST_ASSIGN_OR:
    case AST_ASSIGN_XOR:
    case AST_ASSIGN_LSHIFT:
    case AST_ASSIGN_RSHIFT:
      {
        type_t *lt = synth_expr (ctx, e->expr_binary.lhs);
        synth_expr (ctx, e->expr_binary.rhs); /* also recurse into rhs */
        if (lt)
          e->ty = Deco (lt, CONSTR_EMPTY);
        break;
      }

    case AST_UNARY_PLUS:
    case AST_UNARY_MINUS:
    case AST_UNARY_NOT:
    case AST_UNARY_LNOT:
    case AST_UNARY_REF:
    case AST_UNARY_DEREF:
    case AST_POST_INC:
    case AST_POST_DEC:
    case AST_PRE_INC:
    case AST_PRE_DEC:
    case AST_RETURN:
      synth_expr (ctx, e->expr_unary.value);
      break;

    case AST_APP:
      {
        /* gate / measure affine enforcement */
        if (e->app.fun && e->app.fun->tag == AST_VAR)
          {
            const char *fname = e->app.fun->var.name;
            if (strncmp (fname, "apply_", 6) == 0
                || strcmp (fname, "measure") == 0)
              check_gate_args (ctx, e);
          }
        /* recurse into args */
        ast_t **arg;
        for (arg = cvector_begin (e->app.args);
             arg != cvector_end (e->app.args); arg++)
          synth_expr (ctx, *arg);
        break;
      }

    case AST_LIST:
      synth_expr (ctx, e->expr_list.prev);
      synth_expr (ctx, e->expr_list.value);
      break;

    case AST_COMPOUND:
      {
        ast_t **it;
        for (it = cvector_begin (e->stmt_compound.ast);
             it != cvector_end (e->stmt_compound.ast); it++)
          synth_expr (ctx, *it);
        break;
      }

    case AST_FUN:
      {
        check_ctx_t fn = *ctx;
        fn.delta = NULL; /* fresh Δ per function */
        synth_expr (&fn, e->fun.body);
        delta_free (fn.delta);
        break;
      }

    case AST_IF:
      {
        synth_expr (ctx, e->stmt_if.condition);
        binding_t *pre = delta_clone (ctx->delta);
        synth_expr (ctx, e->stmt_if.body);
        check_no_new_consumption (pre, ctx->delta, "if without else");
        delta_free (ctx->delta);
        ctx->delta = pre;
        break;
      }

    case AST_IF_ELSE:
      {
        synth_expr (ctx, e->stmt_if_else.condition);
        binding_t *pre = delta_clone (ctx->delta);

        synth_expr (ctx, e->stmt_if_else.body);
        binding_t *post_then = ctx->delta; /* take ownership */

        ctx->delta = delta_clone (pre);
        synth_expr (ctx, e->stmt_if_else.else_body);
        /* ctx->delta is now post_else */

        check_branch_join (post_then, ctx->delta);
        delta_free (post_then);
        delta_free (pre);
        /* leave ctx->delta = post_else as merged result */
        break;
      }

    case AST_WHILE:
      {
        synth_expr (ctx, e->stmt_while.condition);
        binding_t *pre = delta_clone (ctx->delta);
        synth_expr (ctx, e->stmt_while.body);
        check_no_new_consumption (pre, ctx->delta, "loop body");
        delta_free (ctx->delta);
        ctx->delta = pre;
        break;
      }

    case AST_DO_WHILE:
      {
        binding_t *pre = delta_clone (ctx->delta);
        synth_expr (ctx, e->stmt_while.body);
        check_no_new_consumption (pre, ctx->delta, "loop body");
        delta_free (ctx->delta);
        ctx->delta = pre;
        synth_expr (ctx, e->stmt_while.condition);
        break;
      }

    case AST_FOR:
      {
        synth_expr (ctx, e->stmt_for.lhs);
        synth_expr (ctx, e->stmt_for.mhs);
        binding_t *pre = delta_clone (ctx->delta);
        synth_expr (ctx, e->stmt_for.body);
        check_no_new_consumption (pre, ctx->delta, "loop body");
        delta_free (ctx->delta);
        ctx->delta = pre;
        synth_expr (ctx, e->stmt_for.rhs);
        break;
      }

    case AST_ARRAY_ACCESS:
      synth_expr (ctx, e->arr_access.array);
      synth_expr (ctx, e->arr_access.index);
      break;

    case AST_MEMBER_ACCESS:
      synth_expr (ctx, e->member_access.aggregate);
      synth_expr (ctx, e->member_access.member);
      break;

    case AST_COND:
      synth_expr (ctx, e->expr_ternary.lhs);
      synth_expr (ctx, e->expr_ternary.mhs);
      synth_expr (ctx, e->expr_ternary.rhs);
      break;

    default:
      break;
    }

  return e->ty ? e->ty->ty : NULL;
}

int
check_expr (check_ctx_t *ctx, ast_t *e, type_t *expected)
{
  type_t *got = synth_expr (ctx, e);
  if (!got || !expected)
    return 1; /* ponytail: missing type → optimistic */
  if (type_equals (got, expected))
    return 1;
  if (got->size != expected->size)
    {
      ast_t *inner = malloc (sizeof (ast_t));
      *inner = *e;
      e->tag = AST_CAST;
      memset (&e->expr_cast, 0, sizeof (e->expr_cast));
      e->expr_cast.ty_caster = Deco (expected, CONSTR_EMPTY);
      e->expr_cast.value = inner;
      e->ty = Deco (expected, CONSTR_EMPTY);
    }
  return 1;
}
