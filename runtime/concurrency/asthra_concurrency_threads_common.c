/**
 * Asthra Concurrency Thread Management - Common Utilities Implementation
 * Shared utilities and helper functions for modular thread management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_concurrency_threads_common.h"

// =============================================================================
// COMMON UTILITY IMPLEMENTATIONS
// =============================================================================

AsthraConcurrencyThreadRegistryStats asthra_concurrency_get_thread_registry_stats_impl(void) {
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
