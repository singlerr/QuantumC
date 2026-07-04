#include "ena_internal.h"

#include <stdlib.h>
#include <string.h>

static ena_status_t ensure_entry_cap(ena_undo_log_t *log, size_t need) {
    if (need <= log->cap) {
        return ENA_OK;
    }
    size_t next = (log->cap == 0) ? 16 : (log->cap * 2);
    while (next < need) {
        next *= 2;
    }
    ena_undo_entry_t *new_entries = (ena_undo_entry_t *)realloc(log->entries, next * sizeof(ena_undo_entry_t));
    if (new_entries == NULL) {
        return ENA_ERR_OOM;
    }
    log->entries = new_entries;
    log->cap = next;
    return ENA_OK;
}

static ena_status_t ensure_snapshot_cap(ena_undo_log_t *log, size_t need) {
    if (need <= log->snapshots_cap) {
        return ENA_OK;
    }
    size_t next = (log->snapshots_cap == 0) ? 8 : (log->snapshots_cap * 2);
    while (next < need) {
        next *= 2;
    }
    ena_snapshot_meta_t *new_snapshots =
        (ena_snapshot_meta_t *)realloc(log->snapshots, next * sizeof(ena_snapshot_meta_t));
    if (new_snapshots == NULL) {
        return ENA_ERR_OOM;
    }
    log->snapshots = new_snapshots;
    log->snapshots_cap = next;
    return ENA_OK;
}

ena_status_t ena_undo_log_init(ena_undo_log_t *log) {
    if (log == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    memset(log, 0, sizeof(*log));
    log->next_snapshot_id = 1;
    return ENA_OK;
}

void ena_undo_log_destroy(ena_undo_log_t *log, const ena_value_vtable_t *value_vtable, void *userdata) {
    if (log == NULL) {
        return;
    }
    if (value_vtable != NULL && value_vtable->drop != NULL) {
        for (size_t i = 0; i < log->len; i++) {
            if (log->entries[i].kind == ENA_UNDO_SET_ELEM && log->entries[i].old_value != NULL) {
                value_vtable->drop(userdata, log->entries[i].old_value);
            }
        }
    }
    for (size_t i = 0; i < log->len; i++) {
        free(log->entries[i].old_value);
    }
    free(log->entries);
    free(log->snapshots);
    memset(log, 0, sizeof(*log));
}

bool ena_undo_log_in_snapshot(const ena_undo_log_t *log) {
    return log != NULL && log->snapshots_len > 0;
}

ena_status_t ena_undo_log_push_new(ena_undo_log_t *log, size_t index) {
    if (log == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    ena_status_t st = ensure_entry_cap(log, log->len + 1);
    if (st != ENA_OK) {
        return st;
    }
    ena_undo_entry_t *entry = &log->entries[log->len++];
    entry->kind = ENA_UNDO_NEW_ELEM;
    entry->index = index;
    entry->old_parent = 0;
    entry->old_rank = 0;
    entry->old_value = NULL;
    return ENA_OK;
}

ena_status_t ena_undo_log_push_set(
    ena_undo_log_t *log,
    size_t index,
    uint32_t old_parent,
    uint32_t old_rank,
    const void *old_value,
    const ena_value_vtable_t *value_vtable,
    void *userdata) {
    if (log == NULL || value_vtable == NULL || value_vtable->clone == NULL || old_value == NULL) {
        return ENA_ERR_INVALID_ARG;
    }

    ena_status_t st = ensure_entry_cap(log, log->len + 1);
    if (st != ENA_OK) {
        return st;
    }

    void *old_value_copy = malloc(value_vtable->value_size);
    if (old_value_copy == NULL) {
        return ENA_ERR_OOM;
    }
    st = value_vtable->clone(userdata, old_value, old_value_copy);
    if (st != ENA_OK) {
        free(old_value_copy);
        return st;
    }

    ena_undo_entry_t *entry = &log->entries[log->len++];
    entry->kind = ENA_UNDO_SET_ELEM;
    entry->index = index;
    entry->old_parent = old_parent;
    entry->old_rank = old_rank;
    entry->old_value = old_value_copy;
    return ENA_OK;
}

ena_status_t ena_undo_log_start_snapshot(ena_undo_log_t *log, size_t value_count, ena_snapshot_t *out_snapshot) {
    if (log == NULL || out_snapshot == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    ena_status_t st = ensure_snapshot_cap(log, log->snapshots_len + 1);
    if (st != ENA_OK) {
        return st;
    }

    uint32_t id = log->next_snapshot_id++;
    ena_snapshot_meta_t *meta = &log->snapshots[log->snapshots_len++];
    meta->id = id;
    meta->undo_len = log->len;
    meta->value_count = value_count;

    out_snapshot->id = id;
    return ENA_OK;
}

ena_status_t ena_undo_log_rollback_to(ena_undo_log_t *log, ena_snapshot_t snapshot, ena_apply_undo_fn apply, void *ctx) {
    if (log == NULL || apply == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    if (log->snapshots_len == 0) {
        return ENA_ERR_SNAPSHOT_MISMATCH;
    }

    const ena_snapshot_meta_t *meta = &log->snapshots[log->snapshots_len - 1];
    if (meta->id != snapshot.id) {
        return ENA_ERR_SNAPSHOT_MISMATCH;
    }

    while (log->len > meta->undo_len) {
        ena_undo_entry_t entry = log->entries[--log->len];
        apply(ctx, &entry);
        if (entry.kind != ENA_UNDO_SET_ELEM) {
            free(entry.old_value);
        }
    }

    log->snapshots_len--;
    return ENA_OK;
}

ena_status_t ena_undo_log_commit(ena_undo_log_t *log, ena_snapshot_t snapshot, const ena_value_vtable_t *value_vtable, void *userdata) {
    if (log == NULL) {
        return ENA_ERR_INVALID_ARG;
    }
    if (log->snapshots_len == 0) {
        return ENA_ERR_SNAPSHOT_MISMATCH;
    }

    const ena_snapshot_meta_t *meta = &log->snapshots[log->snapshots_len - 1];
    if (meta->id != snapshot.id) {
        return ENA_ERR_SNAPSHOT_MISMATCH;
    }

    if (log->snapshots_len == 1) {
        if (value_vtable != NULL && value_vtable->drop != NULL) {
            for (size_t i = 0; i < log->len; i++) {
                if (log->entries[i].kind == ENA_UNDO_SET_ELEM && log->entries[i].old_value != NULL) {
                    value_vtable->drop(userdata, log->entries[i].old_value);
                }
            }
        }
        for (size_t i = 0; i < log->len; i++) {
            free(log->entries[i].old_value);
            log->entries[i].old_value = NULL;
        }
        log->len = 0;
    }

    log->snapshots_len--;
    return ENA_OK;
}
