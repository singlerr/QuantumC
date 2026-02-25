#include "ena_internal.h"

#include <stdlib.h>
#include <string.h>

static bool cfg_valid(const ena_config_t *cfg) {
    if (cfg == NULL) {
        return false;
    }
    if (cfg->key.key_size == 0 || cfg->key.index == NULL || cfg->key.from_index == NULL) {
        return false;
    }
    if (cfg->value.value_size == 0 || cfg->value.clone == NULL || cfg->value.drop == NULL || cfg->value.merge == NULL) {
        return false;
    }
    return true;
}

static ena_status_t key_blob_to_index(ena_table_t *table, const void *key_blob, size_t *out_index) {
    if (table == NULL || key_blob == NULL || out_index == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    ena_index_t idx = table->cfg.key.index(table->cfg.userdata, key_blob);
    if ((size_t)idx >= table->store.len) {
        return ENA_ERR_INVALID_ARG;
    }
    *out_index = (size_t)idx;
    return ENA_OK;
}

static ena_status_t index_to_key_blob(ena_table_t *table, size_t idx, void *out_key_blob) {
    if (table == NULL || out_key_blob == NULL || idx >= table->store.len) {
        return ENA_ERR_INVALID_ARG;
    }
    return table->cfg.key.from_index(table->cfg.userdata, (ena_index_t)idx, out_key_blob);
}

static ena_status_t get_root_index(ena_table_t *table, size_t index, size_t *out_root) {
    if (table == NULL || out_root == NULL || index >= table->store.len) {
        return ENA_ERR_INVALID_ARG;
    }

    size_t i = index;
    while ((size_t)table->store.vars[i].parent != i) {
        i = (size_t)table->store.vars[i].parent;
    }
    size_t root = i;

    i = index;
    while ((size_t)table->store.vars[i].parent != i) {
        size_t next = (size_t)table->store.vars[i].parent;
        ena_status_t st = ena_var_store_record_set(
            &table->store,
            i,
            ena_undo_log_in_snapshot(&table->undo),
            &table->undo,
            &table->cfg.value,
            table->cfg.userdata);
        if (st != ENA_OK) {
            return st;
        }
        table->store.vars[i].parent = (uint32_t)root;
        i = next;
    }

    *out_root = root;
    return ENA_OK;
}

static ena_status_t set_parent(ena_table_t *table, size_t index, size_t parent) {
    ena_status_t st = ena_var_store_record_set(
        &table->store,
        index,
        ena_undo_log_in_snapshot(&table->undo),
        &table->undo,
        &table->cfg.value,
        table->cfg.userdata);
    if (st != ENA_OK) {
        return st;
    }
    table->store.vars[index].parent = (uint32_t)parent;
    return ENA_OK;
}

static ena_status_t set_root_rank_and_value(ena_table_t *table, size_t index, uint32_t rank, const void *value_blob) {
    ena_status_t st = ena_var_store_record_set(
        &table->store,
        index,
        ena_undo_log_in_snapshot(&table->undo),
        &table->undo,
        &table->cfg.value,
        table->cfg.userdata);
    if (st != ENA_OK) {
        return st;
    }

    void *replacement = malloc(table->cfg.value.value_size);
    if (replacement == NULL) {
        return ENA_ERR_OOM;
    }
    st = table->cfg.value.clone(table->cfg.userdata, value_blob, replacement);
    if (st != ENA_OK) {
        free(replacement);
        return st;
    }

    table->cfg.value.drop(table->cfg.userdata, table->store.vars[index].value);
    free(table->store.vars[index].value);
    table->store.vars[index].value = replacement;
    table->store.vars[index].rank = rank;
    return ENA_OK;
}

static ena_status_t choose_order_roots(
    ena_table_t *table,
    size_t a,
    size_t b,
    bool *out_override,
    size_t *out_new_root,
    size_t *out_redirected) {
    *out_override = false;
    *out_new_root = a;
    *out_redirected = b;

    if (table->cfg.key.order_roots == NULL) {
        return ENA_OK;
    }

    void *a_key = malloc(table->cfg.key.key_size);
    void *b_key = malloc(table->cfg.key.key_size);
    void *new_root_key = malloc(table->cfg.key.key_size);
    void *redirected_key = malloc(table->cfg.key.key_size);
    if (a_key == NULL || b_key == NULL || new_root_key == NULL || redirected_key == NULL) {
        free(a_key);
        free(b_key);
        free(new_root_key);
        free(redirected_key);
        return ENA_ERR_OOM;
    }

    ena_status_t st = index_to_key_blob(table, a, a_key);
    if (st == ENA_OK) {
        st = index_to_key_blob(table, b, b_key);
    }
    if (st != ENA_OK) {
        free(a_key);
        free(b_key);
        free(new_root_key);
        free(redirected_key);
        return st;
    }

    bool override = table->cfg.key.order_roots(
        table->cfg.userdata,
        a_key,
        table->store.vars[a].value,
        b_key,
        table->store.vars[b].value,
        new_root_key,
        redirected_key);
    if (override) {
        ena_index_t new_root_idx = table->cfg.key.index(table->cfg.userdata, new_root_key);
        ena_index_t redirected_idx = table->cfg.key.index(table->cfg.userdata, redirected_key);
        if (!((new_root_idx == (ena_index_t)a && redirected_idx == (ena_index_t)b) ||
              (new_root_idx == (ena_index_t)b && redirected_idx == (ena_index_t)a))) {
            st = ENA_ERR_INVALID_ARG;
        } else {
            *out_override = true;
            *out_new_root = (size_t)new_root_idx;
            *out_redirected = (size_t)redirected_idx;
            st = ENA_OK;
        }
    }

    free(a_key);
    free(b_key);
    free(new_root_key);
    free(redirected_key);
    return st;
}

static ena_status_t unify_roots_with_value(ena_table_t *table, size_t a, size_t b, const void *combined_value) {
    uint32_t rank_a = table->store.vars[a].rank;
    uint32_t rank_b = table->store.vars[b].rank;

    bool override = false;
    size_t new_root = a;
    size_t redirected = b;
    ena_status_t st = choose_order_roots(table, a, b, &override, &new_root, &redirected);
    if (st != ENA_OK) {
        return st;
    }

    uint32_t new_rank;
    if (override) {
        if (new_root == a) {
            new_rank = (rank_a > rank_b) ? rank_a : (rank_b + 1);
        } else {
            new_rank = (rank_b > rank_a) ? rank_b : (rank_a + 1);
        }
    } else if (rank_a > rank_b) {
        new_root = a;
        redirected = b;
        new_rank = rank_a;
    } else if (rank_a < rank_b) {
        new_root = b;
        redirected = a;
        new_rank = rank_b;
    } else {
        new_root = b;
        redirected = a;
        new_rank = rank_b + 1;
    }

    st = set_parent(table, redirected, new_root);
    if (st != ENA_OK) {
        return st;
    }
    return set_root_rank_and_value(table, new_root, new_rank, combined_value);
}

static void rollback_apply(void *ctx, const ena_undo_entry_t *entry) {
    ena_table_t *table = (ena_table_t *)ctx;
    ena_var_store_apply_undo(&table->store, entry, &table->cfg.value, table->cfg.userdata);
}

ena_status_t ena_table_init(ena_table_t **out_table, const ena_config_t *cfg) {
    if (out_table == NULL || !cfg_valid(cfg)) {
        return ENA_ERR_INVALID_ARG;
    }
    *out_table = NULL;

    ena_table_t *table = (ena_table_t *)calloc(1, sizeof(ena_table_t));
    if (table == NULL) {
        return ENA_ERR_OOM;
    }
    table->cfg = *cfg;

    ena_status_t st = ena_var_store_init(&table->store);
    if (st != ENA_OK) {
        free(table);
        return st;
    }
    st = ena_undo_log_init(&table->undo);
    if (st != ENA_OK) {
        ena_var_store_destroy(&table->store, &table->cfg.value, table->cfg.userdata);
        free(table);
        return st;
    }

    *out_table = table;
    return ENA_OK;
}

void ena_table_destroy(ena_table_t *table) {
    if (table == NULL) {
        return;
    }
    ena_undo_log_destroy(&table->undo, &table->cfg.value, table->cfg.userdata);
    ena_var_store_destroy(&table->store, &table->cfg.value, table->cfg.userdata);
    free(table);
}

ena_status_t ena_new_key(ena_table_t *table, const void *value_blob, void *out_key_blob) {
    if (table == NULL || value_blob == NULL || out_key_blob == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    size_t index = 0;
    ena_status_t st = ena_var_store_push_new(
        &table->store,
        value_blob,
        &table->cfg.value,
        table->cfg.userdata,
        ena_undo_log_in_snapshot(&table->undo),
        &table->undo,
        &index);
    if (st != ENA_OK) {
        return st;
    }
    return index_to_key_blob(table, index, out_key_blob);
}

ena_status_t ena_reserve(ena_table_t *table, size_t additional) {
    if (table == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    return ena_var_store_reserve(&table->store, additional);
}

size_t ena_len(const ena_table_t *table) {
    return table == NULL ? 0 : table->store.len;
}

ena_status_t ena_find(ena_table_t *table, const void *key_blob, void *out_root_key_blob) {
    if (table == NULL || key_blob == NULL || out_root_key_blob == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    size_t index = 0;
    ena_status_t st = key_blob_to_index(table, key_blob, &index);
    if (st != ENA_OK) {
        return st;
    }
    size_t root = 0;
    st = get_root_index(table, index, &root);
    if (st != ENA_OK) {
        return st;
    }
    return index_to_key_blob(table, root, out_root_key_blob);
}

ena_status_t ena_unioned(ena_table_t *table, const void *a_key_blob, const void *b_key_blob, bool *out_unioned) {
    if (table == NULL || a_key_blob == NULL || b_key_blob == NULL || out_unioned == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    size_t a = 0;
    size_t b = 0;
    ena_status_t st = key_blob_to_index(table, a_key_blob, &a);
    if (st != ENA_OK) {
        return st;
    }
    st = key_blob_to_index(table, b_key_blob, &b);
    if (st != ENA_OK) {
        return st;
    }

    size_t ra = 0;
    size_t rb = 0;
    st = get_root_index(table, a, &ra);
    if (st != ENA_OK) {
        return st;
    }
    st = get_root_index(table, b, &rb);
    if (st != ENA_OK) {
        return st;
    }
    *out_unioned = (ra == rb);
    return ENA_OK;
}

ena_status_t ena_union(ena_table_t *table, const void *a_key_blob, const void *b_key_blob) {
    if (table == NULL || a_key_blob == NULL || b_key_blob == NULL) {
        return ENA_ERR_INVALID_ARG;
    }

    size_t a = 0;
    size_t b = 0;
    ena_status_t st = key_blob_to_index(table, a_key_blob, &a);
    if (st != ENA_OK) {
        return st;
    }
    st = key_blob_to_index(table, b_key_blob, &b);
    if (st != ENA_OK) {
        return st;
    }

    size_t root_a = 0;
    size_t root_b = 0;
    st = get_root_index(table, a, &root_a);
    if (st != ENA_OK) {
        return st;
    }
    st = get_root_index(table, b, &root_b);
    if (st != ENA_OK) {
        return st;
    }
    if (root_a == root_b) {
        return ENA_OK;
    }

    void *combined = malloc(table->cfg.value.value_size);
    if (combined == NULL) {
        return ENA_ERR_OOM;
    }
    st = table->cfg.value.clone(table->cfg.userdata, table->store.vars[root_a].value, combined);
    if (st == ENA_OK) {
        st = table->cfg.value.merge(table->cfg.userdata, combined, table->store.vars[root_b].value);
    }
    if (st != ENA_OK) {
        table->cfg.value.drop(table->cfg.userdata, combined);
        free(combined);
        return st;
    }

    st = unify_roots_with_value(table, root_a, root_b, combined);
    table->cfg.value.drop(table->cfg.userdata, combined);
    free(combined);
    return st;
}

ena_status_t ena_union_value(ena_table_t *table, const void *key_blob, const void *value_blob) {
    if (table == NULL || key_blob == NULL || value_blob == NULL) {
        return ENA_ERR_INVALID_ARG;
    }

    size_t index = 0;
    ena_status_t st = key_blob_to_index(table, key_blob, &index);
    if (st != ENA_OK) {
        return st;
    }

    size_t root = 0;
    st = get_root_index(table, index, &root);
    if (st != ENA_OK) {
        return st;
    }

    void *merged = malloc(table->cfg.value.value_size);
    if (merged == NULL) {
        return ENA_ERR_OOM;
    }
    st = table->cfg.value.clone(table->cfg.userdata, table->store.vars[root].value, merged);
    if (st == ENA_OK) {
        st = table->cfg.value.merge(table->cfg.userdata, merged, value_blob);
    }
    if (st != ENA_OK) {
        table->cfg.value.drop(table->cfg.userdata, merged);
        free(merged);
        return st;
    }

    st = set_root_rank_and_value(table, root, table->store.vars[root].rank, merged);
    table->cfg.value.drop(table->cfg.userdata, merged);
    free(merged);
    return st;
}

ena_status_t ena_probe_value(ena_table_t *table, const void *key_blob, void *out_value_blob) {
    if (table == NULL || key_blob == NULL || out_value_blob == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    size_t index = 0;
    ena_status_t st = key_blob_to_index(table, key_blob, &index);
    if (st != ENA_OK) {
        return st;
    }

    size_t root = 0;
    st = get_root_index(table, index, &root);
    if (st != ENA_OK) {
        return st;
    }
    return table->cfg.value.clone(table->cfg.userdata, table->store.vars[root].value, out_value_blob);
}

ena_status_t ena_snapshot(ena_table_t *table, ena_snapshot_t *out_snapshot) {
    if (table == NULL || out_snapshot == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    return ena_undo_log_start_snapshot(&table->undo, table->store.len, out_snapshot);
}

ena_status_t ena_rollback_to(ena_table_t *table, ena_snapshot_t snapshot) {
    if (table == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    return ena_undo_log_rollback_to(&table->undo, snapshot, rollback_apply, table);
}

ena_status_t ena_commit(ena_table_t *table, ena_snapshot_t snapshot) {
    if (table == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    return ena_undo_log_commit(&table->undo, snapshot, &table->cfg.value, table->cfg.userdata);
}
