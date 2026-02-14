#ifndef _BASE_H_
#define _BASE_H_

#include "data/btree/btree.h"
#include "data/hamt/include/hamt.h"
#include "ena/ena.h"
#include <stdint.h>

typedef struct var
{
  uint32_t size;
} var_t;

typedef struct typed_var
{
  var_t var;
  struct type type;
} typed_var_t;

typedef struct type_var
{
  uint32_t id;
} type_var_t;

typedef enum type_tag
{
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_VAR,
  TYPE_FUN
} type_tag_t;

typedef struct type
{
  type_tag_t tag;
  union
  {
    struct
    {
      type_var_t value;
    } var_t;

    struct
    {
      struct type *arg;
      struct type *ret;
    } fun_t;
  };
} type_t;

typedef enum provenance_tag
{
  UNEXPECTED_FUN,
  APP_EXPECTED_FUN,
  EXPECTED_UNIFY
} provenance_tag_t;

typedef struct provenance
{
  provenance_tag_t tag;
  union
  {
    struct
    {
      node_id_t *id;
    } unexpected_fun;
    struct
    {
      node_id_t *id;
    } app_expected_fun;

    struct
    {
      node_id_t *id;
    } expected_unify;
  };

} provenance_t;

typedef enum constraint_tag
{
  CONSTR_TYPE_EQUAL
} constraint_tag_t;

typedef struct constraint
{
  constraint_tag_t tag;
  union
  {
    struct
    {
      provenance_t *provenance;
      type_t *type_1;
      type_t *type_2;
    } type_equal;
  };

} constraint_t;

typedef enum type_error_tag
{
  INFINITE_TYPE,
  UNEXPECTED_FUN,
  APP_EXPECTED_FUN,
  EXPECTED_UNIFY
} type_error_tag_t;

typedef struct type_error
{
  type_error_tag_t tag;
  union
  {
    struct
    {
      type_var_t *type_var;
      type_t *ty;
    } infinite_type;
    struct
    {
      type_t *expected_ty;
      type_t *fun_ty;
    } unexpected_fun;

    struct
    {
      type_t *inferred_ty;
      type_t *expected_fun_ty;
    } app_expected_fun;
  };

} type_error_t;

typedef struct type_scheme
{
  struct btree *unbound;
  type_t *ty;
} type_scheme_t;

typedef struct type_inference
{
  ena_table_t *unification_table;
  struct hamt *errors;
} type_inference_t;

typedef struct type_infer_out
{
  // TODO: Ast<TypeVar>
  type_scheme_t scheme;
  struct hamt *errors;
} type_infer_out_t;

node_id_t *provenance_id (provenance_t *self);

type_t *type_fun (type_t *arg, type_t *ret);
type_t *type_occurs_check (type_t *self, type_var_t *var, type_t **err_out);

#endif