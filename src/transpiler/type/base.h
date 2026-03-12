#ifndef _BASE_H_
#define _BASE_H_

#include "data/vec/cvector.h"
#include "deco.h"
#include "param.h"

#define FOREACH_TYPE(TYPE)                                                    \
  TYPE (TY_ARRAY)                                                             \
  TYPE (TY_FUN)                                                               \
  TYPE (TY_VOID)                                                              \
  TYPE (TY_CHAR)                                                              \
  TYPE (TY_INT)                                                               \
  TYPE (TY_UINT)                                                              \
  TYPE (TY_FLOAT)                                                             \
  TYPE (TY_SHORT)                                                             \
  TYPE (TY_LONG)                                                              \
  TYPE (TY_DOUBLE)                                                            \
  TYPE (TY_COMPLEX)                                                           \
  TYPE (TY_IMAGINARY)                                                         \
  TYPE (TY_BOOL)                                                              \
  TYPE (TY_QUBIT)                                                             \
  TYPE (TY_DURATION)                                                          \
  TYPE (TY_ANGLE)                                                             \
  TYPE (TY_STRUCT)                                                            \
  TYPE (TY_UNION)                                                             \
  TYPE (TY_ENUM)                                                              \
  TYPE (TY_POINTER)

#define ENUM_GEN(ENUM) ENUM,

typedef enum type_tag
{
  FOREACH_TYPE (ENUM_GEN) TY_MAX
} type_tag_t;

#undef ENUM_GEN

typedef struct ty_pointer
{
  ty_deco_t *ref;
} ty_pointer_t;

typedef struct arg
{
  int has_name;
  char name[SYM_MAXLEN];
  ty_deco_t *ty;
} arg_t;

typedef cvector_vector_type (arg_t) arg_list_t;

typedef struct args
{
  int is_variadic;
  arg_list_t args;
} args_t;

typedef struct ty_array
{
  ty_deco_t *ref;
  int size;
} ty_array_t;

typedef struct ty_fun
{
  int has_name;
  ty_deco_t *ret;
  args_t args;
} ty_fun_t;

typedef struct ty_qubit
{
  uint32_t size;
} ty_qubit_t;

typedef struct ty_complex
{
  struct type *ty;
} ty_complex_t;

typedef struct struct_field
{
  char name[SYM_MAXLEN];
  ty_deco_t *ty;
} struct_field_t;

typedef cvector_vector_type (struct_field_t) struct_fields_t;

typedef struct ty_struct
{
  type_tag_t union_or_struct;
  int has_name;
  char name[SYM_MAXLEN];
  struct_fields_t fields;
} ty_struct_t;

typedef struct type
{
  int size;
  type_tag_t tag;
  union
  {
    ty_struct_t ty_struct;
    ty_fun_t ty_fun;
    ty_pointer_t ty_pointer;
    ty_qubit_t ty_qubit;
  } ty;

} type_t;

const char *type_to_str (type_tag_t tag);
type_t *new_simple_type (int size, type_tag_t tag);
ty_pointer_t new_pointer (ty_deco_t *ty);
ty_deco_t *new_ty_pointer (ty_pointer_t ptr, int constr);
ty_deco_t *new_ty_array (ty_array_t arr, int constr);
ty_deco_t *new_ty_fun (ty_fun_t fun, int constr);
ty_deco_t *append_ty (ty_deco_t *base, ty_deco_t *tail);

args_t *new_args (arg_list_t args, int variadic);
ty_struct_t *begin_struct ();
ty_struct_t *begin_union ();
ty_struct_t *set_struct_name (ty_struct_t *inst, const char *name);
int type_sizeof (type_t *ty);

#define EmptyPointer() new_ty_pointer (new_pointer (NULL), CONSTR_EMPTY)
#define Pointer(ty_deco, constr) new_ty_pointer (new_pointer (ty_deco), constr)

#endif