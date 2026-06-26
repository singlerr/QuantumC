#include "affine.h"
#include "../ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- global affine state ---- */

static binding_t *g_delta = NULL;

#define STACK_MAX 64
static binding_t *snap_stack[STACK_MAX];
static int snap_top = 0;
static binding_t *then_stack[STACK_MAX];
static int then_top = 0;

/* ---- binding helpers (duplicate-free; mirror check.c's local ones) ---- */

static void
g_delta_free (binding_t *d)
{
  while (d)
    {
      binding_t *n = d->next;
      free (d);
      d = n;
    }
}

static binding_t *
g_delta_clone (binding_t *d)
{
  binding_t *head = NULL, *tail = NULL;
  for (; d; d = d->next)
    {
      binding_t *c = malloc (sizeof (binding_t));
      *c = *d;
      c->next = NULL;
      if (!tail) head = tail = c;
      else { tail->next = c; tail = c; }
    }
  return head;
}

static binding_t *
g_delta_find (const char *name)
{
  binding_t *b;
  for (b = g_delta; b; b = b->next)
    if (!strncmp (b->name, name, SYM_MAXLEN))
      return b;
  return NULL;
}

static binding_t *
g_delta_ensure (const char *name)
{
  binding_t *b = g_delta_find (name);
  if (!b)
    {
      b = malloc (sizeof (binding_t));
      strncpy (b->name, name, SYM_MAXLEN);
      b->consumed = 0;
      b->next = g_delta;
      g_delta = b;
    }
  return b;
}

/* ---- loop/if checks ---- */

static void
check_no_new (binding_t *pre, binding_t *post, const char *ctx)
{
  binding_t *b;
  for (b = post; b; b = b->next)
    {
      if (!b->consumed) continue;
      binding_t *p = NULL;
      binding_t *it;
      for (it = pre; it; it = it->next)
        if (!strncmp (it->name, b->name, SYM_MAXLEN)) { p = it; break; }
      if (!p || !p->consumed)
        {
          fprintf (stderr, "error: qubit '%s' consumed in %s\n", b->name, ctx);
          exit (1);
        }
    }
}

static void
check_join (binding_t *post_then, binding_t *post_else)
{
  binding_t *b;
  for (b = post_then; b; b = b->next)
    {
      if (!b->consumed) continue;
      binding_t *be = NULL;
      binding_t *it;
      for (it = post_else; it; it = it->next)
        if (!strncmp (it->name, b->name, SYM_MAXLEN)) { be = it; break; }
      if (!be || !be->consumed)
        {
          fprintf (stderr, "error: qubit '%s' consumed in then but not else\n", b->name);
          exit (1);
        }
    }
  for (b = post_else; b; b = b->next)
    {
      if (!b->consumed) continue;
      binding_t *bt = NULL;
      binding_t *it;
      for (it = post_then; it; it = it->next)
        if (!strncmp (it->name, b->name, SYM_MAXLEN)) { bt = it; break; }
      if (!bt || !bt->consumed)
        {
          fprintf (stderr, "error: qubit '%s' consumed in else but not then\n", b->name);
          exit (1);
        }
    }
}

/* ---- public API ---- */

void
affine_fn_enter (void)
{
  g_delta_free (g_delta);
  g_delta = NULL;
}

void
affine_fn_exit (void)
{
  g_delta_free (g_delta);
  g_delta = NULL;
}

void
affine_snap_push (void)
{
  if (snap_top >= STACK_MAX) { fprintf (stderr, "affine stack overflow\n"); exit (1); }
  snap_stack[snap_top++] = g_delta_clone (g_delta);
}

void
affine_if_noelse (void)
{
  if (snap_top <= 0) return;
  binding_t *pre = snap_stack[--snap_top];
  check_no_new (pre, g_delta, "if without else");
  g_delta_free (g_delta);
  g_delta = pre;
}

void
affine_prep_else (void)
{
  /* save post-then and restore pre for else branch */
  if (snap_top <= 0) return;
  binding_t *pre = snap_stack[--snap_top];
  if (then_top >= STACK_MAX) { fprintf (stderr, "affine then-stack overflow\n"); exit (1); }
  then_stack[then_top++] = g_delta; /* take ownership */
  g_delta = g_delta_clone (pre);
  g_delta_free (pre);
}

void
affine_join (void)
{
  if (then_top <= 0) return;
  binding_t *post_then = then_stack[--then_top];
  check_join (post_then, g_delta);
  g_delta_free (post_then);
  /* g_delta stays as post_else (the merged result) */
}

void
affine_loop_push (void)
{
  affine_snap_push ();
}

void
affine_loop_pop (void)
{
  if (snap_top <= 0) return;
  binding_t *pre = snap_stack[--snap_top];
  check_no_new (pre, g_delta, "loop body");
  g_delta_free (g_delta);
  g_delta = pre;
}

/* ---- gate/measure call check ---- */

static void
check_arg_flat (ast_t *a, int is_gate, int is_measure,
                const char **seen, int *nseen)
{
  if (!a) return;
  if (a->tag == AST_LIST)
    {
      check_arg_flat (a->expr_list.prev, is_gate, is_measure, seen, nseen);
      check_arg_flat (a->expr_list.value, is_gate, is_measure, seen, nseen);
      return;
    }
  if (a->tag != AST_VAR) return;
  type_t *ty = a->ty ? a->ty->ty : NULL;
  if (!ty || ty->tag != TY_QUBIT) return; /* only scalar qubits tracked */

  const char *name = a->var.name;

  if (is_gate)
    {
      int i;
      for (i = 0; i < *nseen; i++)
        if (!strcmp (seen[i], name))
          {
            fprintf (stderr, "error: qubit '%s' aliased in gate call\n", name);
            exit (1);
          }
      if (*nseen < 32) seen[(*nseen)++] = name;
    }

  binding_t *b = g_delta_ensure (name);
  if (b->consumed)
    {
      fprintf (stderr, "error: qubit '%s' used after measure\n", name);
      exit (1);
    }
  if (is_measure)
    b->consumed = 1;
}

void
affine_check_call (const char *fname, ast_t *arglist)
{
  if (!fname || !arglist) return;
  int is_gate = strncmp (fname, "apply_", 6) == 0;
  int is_measure = strcmp (fname, "measure") == 0;
  if (!is_gate && !is_measure) return;

  const char *seen[32];
  int nseen = 0;
  check_arg_flat (arglist, is_gate, is_measure, seen, &nseen);
}
