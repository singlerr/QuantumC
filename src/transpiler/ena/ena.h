#ifndef ENA_ENA_H
#define ENA_ENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum ena_status {
    ENA_OK = 0,
    ENA_ERR_INVALID_ARG,
    ENA_ERR_OOM,
    ENA_ERR_MERGE_CONFLICT,
    ENA_ERR_SNAPSHOT_MISMATCH,
    ENA_ERR_INTERNAL
} ena_status_t;

typedef uint32_t ena_index_t;

typedef struct ena_table ena_table_t;

typedef struct ena_snapshot {
    uint32_t id;
} ena_snapshot_t;

typedef struct ena_key_vtable {
    size_t key_size;
    ena_index_t (*index)(void *userdata, const void *key_blob);
    ena_status_t (*from_index)(void *userdata, ena_index_t idx, void *out_key_blob);
    const char *(*tag)(void *userdata);
    bool (*order_roots)(
        void *userdata,
        const void *a_key_blob,
        const void *a_value_blob,
        const void *b_key_blob,
        const void *b_value_blob,
        void *out_new_root_key_blob,
        void *out_redirected_key_blob);
} ena_key_vtable_t;

typedef struct ena_value_vtable {
    size_t value_size;
    ena_status_t (*clone)(void *userdata, const void *src_value_blob, void *dst_value_blob);
    void (*drop)(void *userdata, void *value_blob);
    ena_status_t (*merge)(void *userdata, void *dst_in_out_value_blob, const void *other_value_blob);
} ena_value_vtable_t;

typedef struct ena_config {
    void *userdata;
    ena_key_vtable_t key;
    ena_value_vtable_t value;
} ena_config_t;

ena_status_t ena_table_init(ena_table_t **out_table, const ena_config_t *cfg);
void ena_table_destroy(ena_table_t *table);

ena_status_t ena_new_key(ena_table_t *table, const void *value_blob, void *out_key_blob);
ena_status_t ena_reserve(ena_table_t *table, size_t additional);
size_t ena_len(const ena_table_t *table);

ena_status_t ena_find(ena_table_t *table, const void *key_blob, void *out_root_key_blob);
ena_status_t ena_unioned(ena_table_t *table, const void *a_key_blob, const void *b_key_blob, bool *out_unioned);
ena_status_t ena_union(ena_table_t *table, const void *a_key_blob, const void *b_key_blob);
ena_status_t ena_union_value(ena_table_t *table, const void *key_blob, const void *value_blob);
ena_status_t ena_probe_value(ena_table_t *table, const void *key_blob, void *out_value_blob);

ena_status_t ena_snapshot(ena_table_t *table, ena_snapshot_t *out_snapshot);
ena_status_t ena_rollback_to(ena_table_t *table, ena_snapshot_t snapshot);
ena_status_t ena_commit(ena_table_t *table, ena_snapshot_t snapshot);

#endif
