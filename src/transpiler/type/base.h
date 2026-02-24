#ifndef _BASE_H_
#define _BASE_H_

#include "ast.h"
#include "data/btree/btree.h"
#include "data/hamt/include/hamt.h"
#include "ena/ena.h"
#include <stdint.h>

typedef struct constraint *constraint_vec;
typedef struct type *type_vec;
typedef struct type_var
{
  ena_index_t id;
} type_var_t;

typedef enum type_tag
{
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_VAR,
  TYPE_FUN,
  TYPE_STRUCT,
  TYPE_UNION,
  TYPE_QUBIT,
  TYPE_ANGLE,
  TYPE_DURATION,
  TYPE_ENUM
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
      type_vec arg;
      struct type *ret;
    } fun_t;

    struct
    {
      type_vec field;
    } struct_t;

    struct
    {
      type_vec field;
    } union_t;
  } data;
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
  ast_t *ast;
  type_scheme_t scheme;
  struct hamt *errors;
} type_infer_out_t;

typedef struct gen_out
{
  constraint_vec constraints;
  ast_t *typed_ast;
} gen_out_t;

typedef enum unification_error_tag
{
  TYPE_NOT_EQUAL,
  INFINITE_TYPE
} unification_error_tag_t;

typedef struct unification_error
{
  unification_error_tag_t tag;
  union
  {
    struct
    {
      type_t *type_1;
      type_t *type_2;
    } type_not_equal;

    struct
    {
      type_var_t *type_var;
      type_t *type;
    } infinite_type;
  } data;
} unification_error_t;

node_id_t *provenance_id (provenance_t *self);

type_t *type_fun (type_t *arg, type_t *ret);
type_t *type_occurs_check (type_t *self, type_var_t *var, type_t **err_out);

type_var_t *type_infer_fresh_ty_var (type_inference_t *self);
void type_infer_infer (type_inference_t *self, struct hamt *env, ast_t *ast,
                       gen_out_t *gen_out, type_t *type_out);
gen_out_t *type_infer_check (type_inference_t *self, struct hamt *env,
                             ast_t *ast, type_t *type);
void type_infer_unification (type_inference_t *self,
                             constraint_vec constraints);
type_t *type_infer_normalize_ty (type_inference_t *self, type_t *ty);
int type_infer_unify_ty_ty (type_inference_t *self, type_t *unnorm_left,
                            type_t *unnorm_right,
                            unification_error_t *error_out);
void type_infer_substitute (type_inference_t *self, type_t *ty,
                            struct btree **unbound_out, type_t **type_out);
void type_infer_substitute (type_inference_t *self, ast_t *ast,
                            struct btree **unbound, ast_t **ast_out);

void type_infer (ast_t *ast, type_infer_out_t *out);
#endif