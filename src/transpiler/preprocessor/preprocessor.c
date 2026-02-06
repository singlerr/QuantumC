#include "preprocessor.h"
#include "stringbuilder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef EXEC_OP
#define WRAP(val) ((val))
#define CONV(val, type) ((type)val)
#define EXEC_OP(__op, __l, __r, __val)                                        \
  do                                                                          \
    {                                                                         \
      if (__l->kind == __r->kind)                                             \
        {                                                                     \
          if (__l->kind == OP_INTEGER)                                        \
            __val = WRAP (__l->value.i) __op WRAP (__r->value.i);             \
          else if (__l->kind == OP_FLOAT)                                     \
            __val = WRAP (__l->value.i) __op WRAP (__r->value.i);             \
        }                                                                     \
      else if (__l->kind == OP_INTEGER && __r->kind == OP_FLOAT)              \
        {                                                                     \
          __val = CONV (__l->value.i, float) __op WRAP (__r->value.f);        \
        }                                                                     \
      else if (__l->kind == OP_FLOAT && __r->kind == OP_INTEGER)              \
        {                                                                     \
          __val = WRAP (__l->value.f) __op CONV (__r->value.i, float);        \
        }                                                                     \
    }                                                                         \
  while (0);
#endif

struct if_stack *if_stack = NULL;
struct directive *directives = NULL;
static struct placeholder *
append_placeholder (struct placeholder *__dest, struct placeholder *__new)
{

  if (!__dest)
    {
      return __new;
    }

  __new->prev = __dest;
  __dest->next = __new;

  return __new;
}

static struct macro_args *
new_macro_arg (const char *name)
{
  struct macro_args *inst
      = (struct macro_args *)malloc (sizeof (struct macro_args));
  strncpy (inst->name, name, NAMELEN);
  inst->next = NULL;
  inst->prev = NULL;
  return inst;
}

static struct placeholder *
new_placeholder (enum placeholder_kind kind, const char *name)
{
  struct placeholder *inst
      = (struct placeholder *)malloc (sizeof (struct placeholder));
  inst->kind = kind;
  strncpy (inst->name, name, NAMELEN);
  inst->next = NULL;
  inst->prev = NULL;

  return inst;
}

static void
free_args (struct macro_args *args)
{
  struct macro_args *arg = args;
  struct macro_args *temp;
  while (arg)
    {
      temp = arg->next;
      free (args);
      arg = temp;
    }
}

static void
free_define (struct dir_define *define)
{
  free_args (define->args);
  free (define->content);
  free (define);
}

static void
free_directive (struct directive *dir)
{
  switch (dir->kind)
    {
    case DIR_DEFINE:
      free_define (dir->value.define);
      break;
    default:
      break;
    }

  free (dir);
}

static struct directive *
push_directive (struct directive *__dest, struct directive *__new)
{
  if (!__dest)
    {
      return __new;
    }

  __new->prev = __dest;

  return __new;
}

int
validate_expr (enum if_op op, struct operand *l, struct operand *r)
{
  int result;
  switch (op)
    {
    case IF_L:
      EXEC_OP (<, l, r, result);
      break;
    case IF_G:
      EXEC_OP (>, l, r, result);
      break;
    case IF_LE:
      EXEC_OP (<=, l, r, result);
      break;
    case IF_GE:
      EXEC_OP (>=, l, r, result);
      break;
    case IF_NE:
      EXEC_OP (!=, l, r, result);
      break;
    case IF_EQ:
      EXEC_OP (==, l, r, result);
      break;
    default:
      perror ("Unknown operator");
      result = 0;
    }

  free (l);
  free (r);
  return result;
}

int
expand_placeholder (char **out, struct placeholder *body)
{
  struct placeholder *ph = body;
  struct string_builder sb;
  init_str_builder (&sb);
  char *buf;
  while (ph)
    {
      switch (ph->kind)
        {
        case PH_TEXT:
          str_append (&sb, ph->name);
          break;
        case PH_PLACEHOLDER:
          str_append (&sb, ph->name);
          break;
        case PH_STRINGIFIED:
          buf = (char *)malloc (strlen (ph->name) + 2 + 1);
          snprintf (buf, sizeof (buf), "\"%s\"", ph->name);
          str_append (&sb, buf);
          free (buf);
          break;
        }
      ph = ph->next;
    }

  *out = end_str_builder (&sb);
  return 1;
}

struct dir_define *
find_macro (const char *name)
{
  struct directive *dir;
  for (dir = directives; dir; dir = dir->prev)
    {
      if (dir->kind == DIR_DEFINE)
        {
          if (!strcmp (dir->value.define->name, name))
            {
              return dir->value.define;
            }
        }
    }

  return NULL;
}

struct dir_define *
new_define (const char *name)
{
  struct dir_define *inst
      = (struct dir_define *)malloc (sizeof (struct dir_define));
  inst->content = NULL;
  inst->args = NULL;
  strncpy (inst->name, name, NAMELEN);

  return inst;
}

void
push_define (struct dir_define *define)
{
  struct directive *inst
      = (struct directive *)malloc (sizeof (struct directive));
  inst->kind = DIR_DEFINE;
  inst->value.define = define;

  directives = push_directive (directives, inst);
}

void
pop_define ()
{
  struct directive *dir;
  struct directive *former = NULL;
  struct directive *temp;
  for (dir = directives; dir; dir = dir->prev)
    {
      if (dir->kind == DIR_DEFINE)
        {
          if (former)
            {
              former->prev = dir->prev;
            }
          temp = dir->prev;
          free_directive (dir);
          dir = temp;
          break;
        }

      former = dir;
    }
}

struct macro_args *
args_builder_end (struct macro_args *chain, const char *name)
{
  if (!chain)
    {
      return NULL;
    }
  struct macro_args *cur = new_macro_arg (name);
  cur->prev = chain;
  chain->next = cur;

  // go to head
  for (; cur; cur = cur->prev)
    {
    }
  return cur;
}

struct macro_args *
args_builder_append (struct macro_args *chain, const char *name)
{
  if (!chain)
    {
      return NULL;
    }

  struct macro_args *cur = new_macro_arg (name);
  cur->prev = chain;
  chain->next = cur;
  return cur;
}

struct macro_args *
args_builder_begin (const char *name)
{
  struct macro_args *cur = new_macro_arg (name);
  return cur;
}

struct placeholder *
ph_builder_append (struct placeholder *chain, enum placeholder_kind kind,
                   const char *name)
{
  if (!chain)
    {
      return NULL;
    }
  struct placeholder *cur = new_placeholder (kind, name);
  cur->prev = chain;
  chain->next = cur;

  return cur;
}

struct placeholder *
ph_builder_concat (struct placeholder *chain, struct placeholder *item)
{
  if (!chain)
    {
      return NULL;
    }
  item->prev = chain;
  chain->next = item;

  return item;
}

struct placeholder *
ph_builder_begin (enum placeholder_kind kind, const char *name)
{
  struct placeholder *cur = new_placeholder (kind, name);
  return cur;
}

struct placeholder *
ph_builder_end (struct placeholder *chain, enum placeholder_kind kind,
                const char *name)
{
  if (!chain)
    {
      return NULL;
    }
  struct placeholder *cur = new_placeholder (kind, name);
  cur->prev = chain;
  chain->next = cur;

  // go to head
  for (; cur; cur = cur->prev)
    {
    }
  return cur;
}

struct dir_openqasm *
openqasm_new (int version)
{
  struct dir_openqasm *inst
      = (struct dir_openqasm *)malloc (sizeof (struct dir_openqasm));
  inst->version = version;
  return inst;
}

struct dir_define *
top_define ()
{
  struct directive *dir;
  for (dir = directives; dir; dir = dir->prev)
    {
      if (dir->kind == DIR_DEFINE)
        {
          return dir->value.define;
        }
    }

  return NULL;
}

int
is_define_arg (struct macro_args *arg_list, const char *name)
{
  for (; arg_list; arg_list = arg_list->next)
    {
      if (!strncmp (arg_list->name, name, NAMELEN))
        {
          return 1;
        }
    }

  return 0;
}

struct if_stack *
push_if ()
{
  struct if_stack *inst = (struct if_stack *)malloc (sizeof (struct if_stack));
  inst->enabled = !should_skip ();
  inst->prev = if_stack;
  if_stack = inst;

  return inst;
}
struct if_stack *
pop_if ()
{
  if (!if_stack)
    {
      return NULL;
    }

  struct if_stack *inst = if_stack;
  if_stack = inst->prev;
  return inst;
}

struct if_stack *
top_if ()
{
  return if_stack;
}

int
should_skip ()
{
  struct if_stack *top = top_if ();
  if (!top)
    {
      return 0;
    }

  return top->enabled;
}
