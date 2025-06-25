/**
 * Asthra Concurrency Thread Management - Thread Registry Implementation
 * Thread registration and management for GC integration
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_concurrency_thread_registry.h"

// =============================================================================
// THREAD REGISTRATION IMPLEMENTATION
// =============================================================================

AsthraResult Asthra_register_c_thread(void) {
    if (!asthra_concurrency_is_initialized()) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED,
                                                      "Concurrency bridge not initialized");
    }

    if (asthra_concurrency_get_thread_data()) {
        // Already registered
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    }

    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();

    // Allocate thread data
    AsthraConcurrencyThreadData *thread_data = malloc(sizeof(AsthraConcurrencyThreadData));
    if (!thread_data) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Failed to allocate thread data");
    }

    // Initialize thread data
    thread_data->thread_id = pthread_self();
    atomic_store_explicit(&thread_data->is_registered, true, ASTHRA_MEMORY_ORDER_RELAXED);
    thread_data->gc_roots = NULL;
    atomic_store_explicit(&thread_data->gc_root_count, 0, ASTHRA_MEMORY_ORDER_RELAXED);
    thread_data->gc_root_capacity = 0;
    // Note: callback_queue removed - not part of thread data structure
    asthra_concurrency_atomic_store_counter(&thread_data->registration_time,
                                            asthra_concurrency_get_timestamp_ms());
    thread_data->next = NULL;

    // Add to registry
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->thread_registry.registry_mutex);
    AsthraConcurrencyThreadData *old_head =
        atomic_load_explicit(&bridge->thread_registry.threads, ASTHRA_MEMORY_ORDER_ACQUIRE);
    thread_data->next = old_head;
    atomic_store_explicit(&bridge->thread_registry.threads, thread_data,
                          ASTHRA_MEMORY_ORDER_RELEASE);
    asthra_concurrency_atomic_increment_size(&bridge->thread_registry.thread_count);
    atomic_fetch_add_explicit(&bridge->thread_registry.total_registered, 1,
                              ASTHRA_MEMORY_ORDER_RELAXED);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->thread_registry.registry_mutex);

    // Set thread-local data
    asthra_concurrency_set_thread_data(thread_data);

    // Register with runtime GC
    asthra_gc_register_root(&thread_data);

    asthra_concurrency_atomic_increment_counter(&bridge->stats.threads_registered);

    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

void Asthra_unregister_c_thread(void) {
    AsthraConcurrencyThreadData *thread_data = asthra_concurrency_get_thread_data();
    if (!thread_data) {
        return;
    }

    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();

    // Unregister all GC roots
    size_t root_count = asthra_concurrency_atomic_load_size(&thread_data->gc_root_count);
    for (size_t i = 0; i < root_count; i++) {
        asthra_gc_unregister_root(thread_data->gc_roots[i]);
    }

    // Remove from registry using atomic operations
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->thread_registry.registry_mutex);
    AsthraConcurrencyThreadData *current =
        atomic_load_explicit(&bridge->thread_registry.threads, ASTHRA_MEMORY_ORDER_ACQUIRE);
    AsthraConcurrencyThreadData *prev = NULL;

    while (current) {
        if (current == thread_data) {
            if (prev) {
                prev->next = current->next;
            } else {
                atomic_store_explicit(&bridge->thread_registry.threads, current->next,
                                      ASTHRA_MEMORY_ORDER_RELEASE);
            }
            asthra_concurrency_atomic_decrement_size(&bridge->thread_registry.thread_count);
            break;
        }
        prev = current;
        current = current->next;
    }
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->thread_registry.registry_mutex);

    // Cleanup thread data
    if (thread_data->gc_roots) {
        free(thread_data->gc_roots);
    }

    // Unregister from runtime GC
    asthra_gc_unregister_root(&thread_data);

    free(thread_data);
    asthra_concurrency_set_thread_data(NULL);
}

bool Asthra_thread_is_registered(void) {
    AsthraConcurrencyThreadData *thread_data = asthra_concurrency_get_thread_data();
    return thread_data != NULL && thread_data->is_registered;
}

AsthraConcurrencyThreadRegistryStats Asthra_get_thread_registry_stats(void) {
    AsthraConcurrencyThreadRegistryStats stats = {0};

    if (!asthra_concurrency_is_initialized()) {
        return stats;
    }

    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();

    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->thread_registry.registry_mutex);

    stats.active_threads =
        asthra_concurrency_atomic_load_size(&bridge->thread_registry.thread_count);
    stats.total_registered = atomic_load_explicit(&bridge->thread_registry.total_registered,
                                                  ASTHRA_MEMORY_ORDER_ACQUIRE);
    stats.is_shutdown =
        atomic_load_explicit(&bridge->thread_registry.shutdown, ASTHRA_MEMORY_ORDER_ACQUIRE);

    // Count total GC roots across all threads
    AsthraConcurrencyThreadData *current =
        atomic_load_explicit(&bridge->thread_registry.threads, ASTHRA_MEMORY_ORDER_ACQUIRE);
    while (current) {
        stats.total_gc_roots += asthra_concurrency_atomic_load_size(&current->gc_root_count);
        current = current->next;
    }

    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->thread_registry.registry_mutex);

    return stats;
}
