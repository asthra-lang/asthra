/**
 * Asthra Concurrency Bridge FFI - Thread Registration
 * Thread registration and management functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_ffi_common.h"

// =============================================================================
// THREAD REGISTRATION
// =============================================================================

AsthraResult Asthra_register_thread(const char *name) {
    if (!asthra_concurrency_is_initialized()) {
        return create_error("Bridge not initialized");
    }

    pthread_mutex_lock(&bridge_state.mutex);

    pthread_t current_thread = pthread_self();

    // Check if already registered
    ThreadRegistryEntry *entry = bridge_state.thread_registry;
    while (entry) {
        if (pthread_equal(entry->thread_id, current_thread)) {
            pthread_mutex_unlock(&bridge_state.mutex);
            return create_ok(); // Already registered
        }
        entry = entry->next;
    }

    // Create new entry
    entry = malloc(sizeof(ThreadRegistryEntry));
    if (!entry) {
        pthread_mutex_unlock(&bridge_state.mutex);
        return create_error("Memory allocation failed");
    }

    entry->thread_id = current_thread;
    entry->is_registered = true;
    if (name) {
        strncpy(entry->name, name, sizeof(entry->name) - 1);
        entry->name[sizeof(entry->name) - 1] = '\0';
    } else {
        snprintf(entry->name, sizeof(entry->name), "thread_%lu", (unsigned long)current_thread);
    }

    entry->next = bridge_state.thread_registry;
    bridge_state.thread_registry = entry;

    atomic_fetch_add(&bridge_state.stats.threads_registered, 1);

    pthread_mutex_unlock(&bridge_state.mutex);
    return create_ok();
}

void Asthra_unregister_thread(void) {
    if (!asthra_concurrency_is_initialized()) {
        return;
    }

    pthread_mutex_lock(&bridge_state.mutex);

    pthread_t current_thread = pthread_self();
    ThreadRegistryEntry **prev_ptr = &bridge_state.thread_registry;
    ThreadRegistryEntry *entry = bridge_state.thread_registry;

    while (entry) {
        if (pthread_equal(entry->thread_id, current_thread)) {
            *prev_ptr = entry->next;
            free(entry);
            atomic_fetch_sub(&bridge_state.stats.threads_registered, 1);
            break;
        }
        prev_ptr = &entry->next;
        entry = entry->next;
    }

    pthread_mutex_unlock(&bridge_state.mutex);
}

AsthraResult Asthra_register_c_thread(void) {
    return Asthra_register_thread("c_thread");
}

void Asthra_unregister_c_thread(void) {
    Asthra_unregister_thread();
}

bool Asthra_is_thread_registered(void) {
    if (!asthra_concurrency_is_initialized()) {
        return false;
    }

    pthread_mutex_lock(&bridge_state.mutex);

    pthread_t current_thread = pthread_self();
    ThreadRegistryEntry *entry = bridge_state.thread_registry;
    bool found = false;

    while (entry) {
        if (pthread_equal(entry->thread_id, current_thread)) {
            found = true;
            break;
        }
        entry = entry->next;
    }

    pthread_mutex_unlock(&bridge_state.mutex);
    return found;
}