#include "ena_internal.h"

#include <stdlib.h>
#include <string.h>

static ena_status_t ensure_var_cap(ena_var_store_t *store, size_t need) {
    if (need <= store->cap) {
        return ENA_OK;
    }
    size_t next = (store->cap == 0) ? 16 : (store->cap * 2);
    while (next < need) {
        next *= 2;
    }
    ena_var_t *new_vars = (ena_var_t *)realloc(store->vars, next * sizeof(ena_var_t));
    if (new_vars == NULL) {
        return ENA_ERR_OOM;
    }
    store->vars = new_vars;
    store->cap = next;
    return ENA_OK;
}

ena_status_t ena_var_store_init(ena_var_store_t *store) {
    if (store == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    memset(store, 0, sizeof(*store));
    return ENA_OK;
}

void ena_var_store_destroy(ena_var_store_t *store, const ena_value_vtable_t *value_vtable, void *userdata) {
    if (store == NULL) {
        return;
    }
    if (value_vtable != NULL && value_vtable->drop != NULL) {
        for (size_t i = 0; i < store->len; i++) {
            if (store->vars[i].value != NULL) {
                value_vtable->drop(userdata, store->vars[i].value);
            }
        }
    }
    for (size_t i = 0; i < store->len; i++) {
        free(store->vars[i].value);
    }
    free(store->vars);
    memset(store, 0, sizeof(*store));
}

ena_status_t ena_var_store_reserve(ena_var_store_t *store, size_t additional) {
    if (store == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    return ensure_var_cap(store, store->len + additional);
}

ena_status_t ena_var_store_push_new(
    ena_var_store_t *store,
    const void *value_blob,
    const ena_value_vtable_t *value_vtable,
    void *userdata,
    bool in_snapshot,
    ena_undo_log_t *undo_log,
    size_t *out_index) {
    if (store == NULL || value_blob == NULL || value_vtable == NULL || value_vtable->clone == NULL || out_index == NULL) {
        return ENA_ERR_INVALID_ARG;
    }

    ena_status_t st = ensure_var_cap(store, store->len + 1);
    if (st != ENA_OK) {
        return st;
    }

    size_t index = store->len;
    void *value = malloc(value_vtable->value_size);
    if (value == NULL) {
        return ENA_ERR_OOM;
    }
    st = value_vtable->clone(userdata, value_blob, value);
    if (st != ENA_OK) {
        free(value);
        return st;
    }

    store->vars[index].parent = (uint32_t)index;
    store->vars[index].rank = 0;
    store->vars[index].value = value;
    store->len++;

    if (in_snapshot) {
        st = ena_undo_log_push_new(undo_log, index);
        if (st != ENA_OK) {
            store->len--;
            if (value_vtable->drop != NULL) {
                value_vtable->drop(userdata, value);
            }
            free(value);
            return st;
        }
    }

    *out_index = index;
    return ENA_OK;
}

ena_status_t ena_var_store_record_set(
    ena_var_store_t *store,
    size_t index,
    bool in_snapshot,
    ena_undo_log_t *undo_log,
    const ena_value_vtable_t *value_vtable,
    void *userdata) {
    if (!in_snapshot) {
        return ENA_OK;
    }
    if (store == NULL || undo_log == NULL || value_vtable == NULL || index >= store->len) {
        return ENA_ERR_INVALID_ARG;
    }
    ena_var_t *var = &store->vars[index];
    return ena_undo_log_push_set(undo_log, index, var->parent, var->rank, var->value, value_vtable, userdata);
}

void ena_var_store_apply_undo(
    ena_var_store_t *store,
    const ena_undo_entry_t *entry,
    const ena_value_vtable_t *value_vtable,
    void *userdata) {
    if (store == NULL || entry == NULL) {
        return;
    }

    if (entry->kind == ENA_UNDO_NEW_ELEM) {
        if (store->len == 0) {
            return;
        }
        size_t idx = store->len - 1;
        if (value_vtable != NULL && value_vtable->drop != NULL && store->vars[idx].value != NULL) {
            value_vtable->drop(userdata, store->vars[idx].value);
        }
        free(store->vars[idx].value);
        store->vars[idx].value = NULL;
        store->len--;
        return;
    }

    if (entry->kind == ENA_UNDO_SET_ELEM) {
        if (entry->index >= store->len) {
            return;
        }
        ena_var_t *var = &store->vars[entry->index];
        if (value_vtable != NULL && value_vtable->drop != NULL && var->value != NULL) {
            value_vtable->drop(userdata, var->value);
        }
        free(var->value);
        var->value = entry->old_value;
        var->parent = entry->old_parent;
        var->rank = entry->old_rank;
        return;
    }
}
