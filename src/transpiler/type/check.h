#ifndef _CHECK_H_
#define _CHECK_H_

#include "base.h"
#include "intern.h"

/* affine binding: tracks scalar qubit variables in Δ */
typedef struct binding
{
  char name[SYM_MAXLEN];
  int consumed;          /* set true after measure */
  struct binding *next;
} binding_t;

typedef struct check_ctx
{
  type_t *fn_ret;        /* expected return type of enclosing function */
  binding_t *delta;      /* Δ: scalar qubit bindings, affine */
} check_ctx_t;

struct ast;

type_t *synth_expr (check_ctx_t *ctx, struct ast *e);
int check_expr (check_ctx_t *ctx, struct ast *e, type_t *expected);

#endif
