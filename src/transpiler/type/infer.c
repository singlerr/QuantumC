#include "base.h"
#include <stdio.h>
#include <stdlib.h>

static const uint32_t key_none = -1;
#define NONE (&key_none)

typedef struct type_table_ctx
{
  int force_order_roots;
} type_table_ctx_t;

static ena_index_t
type_table_index (void *userdata, const void *key_blob)
{
  const type_var_t *key = (const type_var_t *)key_blob;
  return key->id;
}

static bool
type_table_order_roots (void *userdata, const void *a_key_blob,
                        const void *a_value_blob, const void *b_key_blob,
                        const void *b_value_blob, void *out_new_root_key_blob,
                        void *out_redirected_key_blob)
{
  type_table_ctx_t *ctx = (type_table_ctx_t *)userdata;
  if (!ctx->force_order_roots)
    {
      return false;
    }

  const type_var_t *a = (const type_var_t *)a_key_blob;
  const type_var_t *b = (const type_var_t *)b_key_blob;
  const int32_t *av = (const int32_t *)a_value_blob;
  const int32_t *bv = (const int32_t *)b_value_blob;

  if (*av >= *bv)
    {
      memcpy (out_new_root_key_blob, a, sizeof (*a));
      memcpy (out_redirected_key_blob, b, sizeof (*b));
    }
  else
    {
      memcpy (out_new_root_key_blob, b, sizeof (*b));
      memcpy (out_redirected_key_blob, a, sizeof (*a));
    }
  return true;
}

static ena_status_t
type_table_key_from_index (void *userdata, ena_index_t idx, void *out_key_blob)
{
  type_var_t *k = (type_var_t *)out_key_blob;
  k->id = idx;
  return ENA_OK;
}

static const char *
type_table_key_tag (void *userdata)
{
  return "TypeVar";
}

static ena_status_t
type_table_value_clone (void *userdata, const void *src_value_blob,
                        void *dst_value_blob)
{
  *(int32_t *)dst_value_blob = *(const int32_t *)src_value_blob;
  return ENA_OK;
}

static void
type_table_value_drop (void *userdata, void *value_blob)
{
}

static ena_status_t
type_table_value_merge (void *userdata, void *dst_in_out_value_blob,
                        const void *other_value_blob)
{
  int32_t *dst = (int32_t *)dst_in_out_value_blob;
  const int32_t *other = (const int32_t *)other_value_blob;
  if (*dst == -1)
    {
      *dst = *other;
      return ENA_OK;
    }
  if (*other == -1 || *dst == *other)
    {
      return ENA_OK;
    }
  return ENA_ERR_MERGE_CONFLICT;
}

static ena_config_t
type_table_config (type_table_ctx_t *ctx)
{
  ena_config_t cfg;
  cfg.userdata = ctx;
  cfg.key.key_size = sizeof (type_var_t);
  cfg.key.index = type_table_index;
  cfg.key.from_index = type_table_key_from_index;
  cfg.key.tag = type_table_key_tag;
  cfg.key.order_roots = type_table_order_roots;
  cfg.value.value_size = sizeof (int32_t);
  cfg.value.clone = type_table_value_clone;
  cfg.value.drop = type_table_value_drop;
  cfg.value.merge = type_table_value_merge;
  return cfg;
}

node_id_t *
provenance_id (provenance_t *self)
{
  switch (self->tag)
    {
    case UNEXPECTED_FUN:
      return self->unexpected_fun.id;
    case APP_EXPECTED_FUN:
      return self->app_expected_fun.id;
    case EXPECTED_UNIFY:
      return self->expected_unify.id;
    default:
      return NULL;
    }
}

type_t *
type_fun (type_t *arg, type_t *ret)
{
}
type_t *
type_occurs_check (type_t *self, type_var_t *var, type_t **err_out)
{
}

type_var_t *
type_infer_fresh_ty_var (type_inference_t *self)
{
  type_var_t *key = (type_var_t *)malloc (sizeof (type_var_t));
  ena_new_key (self->unification_table, NONE, key);
  return key;
}
void
type_infer_infer (type_inference_t *self, struct hamt *env, ast_t *ast,
                  gen_out_t *gen_out, type_t *type_out)
{
  gen_out_t gen;
  switch (ast->tag)
    {
    case AST_INT:
      gen.constraints = NULL;
      gen.typed_ast;
      break;
    case AST_FLOAT:
    case AST_SHORT:
    case AST_QUBIT:
    case AST_ANGLE:
    case AST_DURATION:
      break;
    case AST_VAR:
      break;

    default:
      break;
    }
}
gen_out_t *
type_infer_check (type_inference_t *self, struct hamt *env, ast_t *ast,
                  type_t *type)
{
}
void
type_infer_unification (type_inference_t *self, constraint_vec constraints)
{
}
type_t *
type_infer_normalize_ty (type_inference_t *self, type_t *ty)
{
}
int
type_infer_unify_ty_ty (type_inference_t *self, type_t *unnorm_left,
                        type_t *unnorm_right, unification_error_t *error_out)
{
}
void
type_infer_substitute (type_inference_t *self, type_t *ty,
                       struct btree **unbound_out, type_t **type_out)
{
}
void
type_infer_substitute (type_inference_t *self, ast_t *ast,
                       struct btree **unbound, ast_t **ast_out)
{
}

void
type_infer (ast_t *ast, type_infer_out_t *out)
{
}