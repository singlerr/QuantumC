#include "deco.h"
#include <stdlib.h>
#include <string.h>

ty_deco_t *
new_deco (struct type *ty, int constr)
{
  ty_deco_t *d = calloc (1, sizeof (ty_deco_t));
  d->is_cast = 0;
  d->ty = ty;
  d->constr = constr;
  return d;
}

ty_deco_t *
new_deco_cast (ty_deco_t *lhs, ty_deco_t *rhs)
{
  ty_deco_t *d = calloc (1, sizeof (ty_deco_t));
  d->is_cast = 1;
  d->cast.lhs = lhs;
  d->cast.rhs = rhs;
  return d;
}

ty_deco_t *
begin_deco_ty (struct type *ty)
{
  return new_deco (ty, 0);
}

ty_deco_t *
begin_deco_constr (int constr)
{
  return new_deco (NULL, constr);
}

ty_deco_t *
deco_constr (ty_deco_t *builder, int constr)
{
  builder->constr |= constr;
  return builder;
}

ty_deco_t *
deco_type (ty_deco_t *builder, struct type *ty)
{
  builder->ty = ty;
  return builder;
}

ty_deco_t *
end_deco (ty_deco_t *builder)
{
  return builder;
}
