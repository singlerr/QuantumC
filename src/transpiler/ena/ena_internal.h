#ifndef ENA_INTERNAL_H
#define ENA_INTERNAL_H

#include "ena.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum ena_undo_kind
{
  ENA_UNDO_NEW_ELEM = 0,
  ENA_UNDO_SET_ELEM = 1
} ena_undo_kind_t;

typedef struct ena_undo_entry
{
  ena_undo_kind_t kind;
  size_t index;
  uint32_t old_parent;
  uint32_t old_rank;
  void *old_value;
} ena_undo_entry_t;

typedef struct ena_snapshot_meta
{
  uint32_t id;
  size_t undo_len;
  size_t value_count;
} ena_snapshot_meta_t;

typedef struct ena_undo_log
{
  ena_undo_entry_t *entries;
  size_t len;
  size_t cap;

  ena_snapshot_meta_t *snapshots;
  size_t snapshots_len;
  size_t snapshots_cap;
  uint32_t next_snapshot_id;
} ena_undo_log_t;

typedef struct ena_var
{
  uint32_t parent;
  uint32_t rank;
  void *value;
} ena_var_t;

typedef struct ena_var_store
{
  ena_var_t *vars;
  size_t len;
  size_t cap;
} ena_var_store_t;

struct ena_table
{
  ena_config_t cfg;
  ena_var_store_t store;
  ena_undo_log_t undo;
};

typedef void (*ena_apply_undo_fn) (void *ctx, const ena_undo_entry_t *entry);

ena_status_t ena_undo_log_init (ena_undo_log_t *log);
void ena_undo_log_destroy (ena_undo_log_t *log,
                           const ena_value_vtable_t *value_vtable,
                           void *userdata);

bool ena_undo_log_in_snapshot (const ena_undo_log_t *log);

ena_status_t ena_undo_log_push_new (ena_undo_log_t *log, size_t index);
ena_status_t ena_undo_log_push_set (ena_undo_log_t *log, size_t index,
                                    uint32_t old_parent, uint32_t old_rank,
                                    const void *old_value,
                                    const ena_value_vtable_t *value_vtable,
                                    void *userdata);

ena_status_t ena_undo_log_start_snapshot (ena_undo_log_t *log,
                                          size_t value_count,
                                          ena_snapshot_t *out_snapshot);
ena_status_t ena_undo_log_rollback_to (ena_undo_log_t *log,
                                       ena_snapshot_t snapshot,
                                       ena_apply_undo_fn apply, void *ctx);
ena_status_t ena_undo_log_commit (ena_undo_log_t *log, ena_snapshot_t snapshot,
                                  const ena_value_vtable_t *value_vtable,
                                  void *userdata);

ena_status_t ena_var_store_init (ena_var_store_t *store);
void ena_var_store_destroy (ena_var_store_t *store,
                            const ena_value_vtable_t *value_vtable,
                            void *userdata);
ena_status_t ena_var_store_reserve (ena_var_store_t *store, size_t additional);
ena_status_t ena_var_store_push_new (ena_var_store_t *store,
                                     const void *value_blob,
                                     const ena_value_vtable_t *value_vtable,
                                     void *userdata, bool in_snapshot,
                                     ena_undo_log_t *undo_log,
                                     size_t *out_index);
ena_status_t ena_var_store_record_set (ena_var_store_t *store, size_t index,
                                       bool in_snapshot,
                                       ena_undo_log_t *undo_log,
                                       const ena_value_vtable_t *value_vtable,
                                       void *userdata);
void ena_var_store_apply_undo (ena_var_store_t *store,
                               const ena_undo_entry_t *entry,
                               const ena_value_vtable_t *value_vtable,
                               void *userdata);

#endif
