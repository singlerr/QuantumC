#ifndef _DECO_H_
#define _DECO_H_

struct type;

typedef struct type_deco
{
  int constr;
  struct type *ty;
} ty_deco_t;

ty_deco_t *begin_deco_ty (struct type *ty);
ty_deco_t *begin_deco_constr (int constr);
ty_deco_t *deco_constr (ty_deco_t *builder, int contr);
ty_deco_t *deco_type (ty_deco_t *builder, struct type *ty);
ty_deco_t *end_deco (ty_deco_t *builder);
ty_deco_t *new_deco (struct type *ty, int constr);

#define Deco(ty, constr) new_deco (ty, constr)

#endif