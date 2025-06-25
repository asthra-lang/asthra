/**
 * Asthra Concurrency Bridge FFI - Callback System
 * Callback queue management and processing functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_ffi_common.h"

// =============================================================================
// CALLBACK SYSTEM
// =============================================================================

AsthraResult Asthra_enqueue_callback(AsthraConcurrencyCallbackFunction func, void *data,
                                     size_t data_size, void *context, uint32_t priority) {
    if (!func || !asthra_concurrency_is_initialized()) {
        return create_error("Invalid callback or bridge not initialized");
    }

    pthread_mutex_lock(&bridge_state.mutex);

    // Check queue limit
    if (bridge_state.callback_queue_count >= bridge_state.queue_size) {
        atomic_fetch_add(&bridge_state.stats.callbacks_dropped, 1);
        pthread_mutex_unlock(&bridge_state.mutex);
        return create_error("Callback queue full");
    }

    // Create callback entry
    CallbackEntry *entry = malloc(sizeof(CallbackEntry));
    if (!entry) {
        pthread_mutex_unlock(&bridge_state.mutex);
        return create_error("Memory allocation failed");
    }

    entry->func = func;
    entry->context = context;
    entry->priority = priority;
    entry->next = NULL;
    clock_gettime(CLOCK_MONOTONIC, &entry->enqueue_time);

    // Copy data if provided
    if (data_size > 0 && data) {
        entry->data = malloc(data_size);
        if (!entry->data) {
            free(entry);
            pthread_mutex_unlock(&bridge_state.mutex);
            return create_error("Memory allocation failed");
        }
        memcpy(entry->data, data, data_size);
        entry->data_size = data_size;
    } else {
        entry->data = NULL;
        entry->data_size = 0;
    }

    // Insert based on priority (higher priority first)
    if (!bridge_state.callback_queue_head ||
        bridge_state.callback_queue_head->priority < priority) {
        // Insert at head
        entry->next = bridge_state.callback_queue_head;
        bridge_state.callback_queue_head = entry;
        if (!bridge_state.callback_queue_tail) {
            bridge_state.callback_queue_tail = entry;
        }
    } else {
        // Find insertion point
        CallbackEntry *prev = bridge_state.callback_queue_head;
        while (prev->next && prev->next->priority >= priority) {
            prev = prev->next;
        }
        entry->next = prev->next;
        prev->next = entry;
        if (!entry->next) {
            bridge_state.callback_queue_tail = entry;
        }
    }

    bridge_state.callback_queue_count++;
    atomic_fetch_add(&bridge_state.stats.callbacks_enqueued, 1);

    pthread_cond_signal(&bridge_state.callback_available);
    pthread_mutex_unlock(&bridge_state.mutex);

    return create_ok();
}

size_t Asthra_process_callbacks(size_t max_callbacks) {
    if (!asthra_concurrency_is_initialized()) {
        return 0;
    }

    size_t processed = 0;

    while (processed < max_callbacks) {
        pthread_mutex_lock(&bridge_state.mutex);

        if (!bridge_state.callback_queue_head) {
            pthread_mutex_unlock(&bridge_state.mutex);
            break;
        }

        // Dequeue callback
        CallbackEntry *entry = bridge_state.callback_queue_head;
        bridge_state.callback_queue_head = entry->next;
        if (!bridge_state.callback_queue_head) {
            bridge_state.callback_queue_tail = NULL;
        }
        bridge_state.callback_queue_count--;

        pthread_mutex_unlock(&bridge_state.mutex);

        // Execute callback
        AsthraResult result = entry->func(entry->data, entry->data_size, entry->context);
        (void)result; // Ignore result for now

        // Cleanup
        if (entry->data) {
            free(entry->data);
        }
        free(entry);

        processed++;
        atomic_fetch_add(&bridge_state.stats.callbacks_processed, 1);
    }

    return processed;
}