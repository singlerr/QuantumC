#include "base.h"
#include "../data/vec/cvector.h"
#include "deco.h"
#include "intern.h"
#include <stdlib.h>
#include <string.h>

type_t *
new_simple_type (int size, type_tag_t tag)
{
  type_t *t = calloc (1, sizeof (type_t));
  t->size = size;
  t->tag = tag;
  return intern_type (t);
}

type_t *
new_struct_type (ty_struct_t *s)
{
  type_t *t = calloc (1, sizeof (type_t));
  t->size = -1;
  t->tag = s->union_or_struct;
  t->ty.ty_struct = *s;
  return intern_type (t);
}

ty_pointer_t
new_pointer (ty_deco_t *ty)
{
  return (ty_pointer_t){ .ref = ty };
}

ty_deco_t *
new_ty_pointer (ty_pointer_t ptr, int constr)
{
  type_t *t = calloc (1, sizeof (type_t));
  t->size = 8;
  t->tag = TY_POINTER;
  t->ty.ty_pointer = ptr;
  if (ptr.ref)
    t = intern_type (t);
  return new_deco (t, constr);
}

ty_deco_t *
new_ty_array (ty_array_t arr, int constr)
{
  type_t *t = calloc (1, sizeof (type_t));
  t->size = -1;
  t->tag = TY_ARRAY;
  t->ty.ty_array = arr;
  if (arr.ref)
    t = intern_type (t);
  return new_deco (t, constr);
}

ty_deco_t *
new_ty_fun (ty_fun_t fun, int constr)
{
  type_t *t = calloc (1, sizeof (type_t));
  t->size = 0;
  t->tag = TY_FUN;
  t->ty.ty_fun = fun;
  return new_deco (t, constr);
}

ty_deco_t *
append_ty (ty_deco_t *base, ty_deco_t *tail)
{
  if (!base)
    return tail;

  ty_deco_t *cur = base;
  for (;;)
    {
      if (!cur->ty)
        {
          cur->ty = tail->ty;
          return base;
        }
      type_t *t = cur->ty;
      switch (t->tag)
        {
        case TY_POINTER:
          if (!t->ty.ty_pointer.ref)
            {
              t->ty.ty_pointer.ref = tail;
              return base;
            }
          cur = t->ty.ty_pointer.ref;
          break;
        case TY_ARRAY:
          if (!t->ty.ty_array.ref)
            {
              t->ty.ty_array.ref = tail;
              return base;
            }
          cur = t->ty.ty_array.ref;
          break;
        case TY_FUN:
          if (!t->ty.ty_fun.ret)
            {
              t->ty.ty_fun.ret = tail;
              return base;
            }
          cur = t->ty.ty_fun.ret;
          break;
        default:
          return base;
        }
    }
}

args_t *
new_args (arg_list_t args, int variadic)
{
  args_t *a = calloc (1, sizeof (args_t));
  a->is_variadic = variadic;
  a->args = args;
  return a;
}

ty_struct_t *
begin_struct ()
{
  ty_struct_t *s = calloc (1, sizeof (ty_struct_t));
  s->union_or_struct = TY_STRUCT;
  s->has_name = 0;
  s->fields = NULL;
  return s;
}

ty_struct_t *
begin_union ()
{
  ty_struct_t *s = calloc (1, sizeof (ty_struct_t));
  s->union_or_struct = TY_UNION;
  s->has_name = 0;
  s->fields = NULL;
  return s;
}

ty_struct_t *
set_struct_name (ty_struct_t *inst, const char *name)
{
  inst->has_name = 1;
  strncpy (inst->name, name, SYM_MAXLEN);
  return inst;
}

ty_deco_t *
clone_ty_deco (ty_deco_t *ty)
{
  if (!ty)
    return NULL;
  ty_deco_t *c = malloc (sizeof (ty_deco_t));
  *c = *ty;
  return c;
}

type_t *
clone_type (type_t *ty)
{
  if (!ty)
    return NULL;
  type_t *c = malloc (sizeof (type_t));
  *c = *ty;
  return c;
}
