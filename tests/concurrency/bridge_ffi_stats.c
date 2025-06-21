/**
 * Asthra Concurrency Bridge FFI - Statistics and Monitoring
 * Statistics collection and state monitoring functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_ffi_common.h"

// =============================================================================
// STATISTICS AND MONITORING
// =============================================================================

AsthraConcurrencyStats Asthra_get_concurrency_stats(void) {
    pthread_mutex_lock(&bridge_state.mutex);
    AsthraConcurrencyStats stats = bridge_state.stats;
    pthread_mutex_unlock(&bridge_state.mutex);
    return stats;
}

void Asthra_dump_concurrency_state(FILE* output) {
    if (!output || !asthra_concurrency_is_initialized()) {
        return;
    }
    
    pthread_mutex_lock(&bridge_state.mutex);
    
    fprintf(output, "=== Asthra Concurrency Bridge State ===\n");
    fprintf(output, "Initialized: %s\n", bridge_state.initialized ? "Yes" : "No");
    fprintf(output, "Max Tasks: %zu\n", bridge_state.task_pool_size);
    fprintf(output, "Max Callbacks: %zu\n", bridge_state.queue_size);
    fprintf(output, "Next Task ID: %llu\n", (unsigned long long)atomic_load(&bridge_state.next_task_id));
    fprintf(output, "\n");
    
    fprintf(output, "=== Statistics ===\n");
    fprintf(output, "Tasks Spawned: %llu\n", (unsigned long long)atomic_load(&bridge_state.stats.tasks_spawned));
    fprintf(output, "Tasks Completed: %llu\n", (unsigned long long)atomic_load(&bridge_state.stats.tasks_completed));
    fprintf(output, "Tasks Failed: %llu\n", (unsigned long long)atomic_load(&bridge_state.stats.tasks_failed));
    fprintf(output, "Callbacks Enqueued: %llu\n", (unsigned long long)atomic_load(&bridge_state.stats.callbacks_enqueued));
    fprintf(output, "Callbacks Processed: %llu\n", (unsigned long long)atomic_load(&bridge_state.stats.callbacks_processed));
    fprintf(output, "Threads Registered: %llu\n", (unsigned long long)atomic_load(&bridge_state.stats.threads_registered));
    fprintf(output, "Mutex Contentions: %llu\n", (unsigned long long)atomic_load(&bridge_state.stats.mutex_contentions));
    fprintf(output, "\n");
    
    fprintf(output, "=== Callback Queue ===\n");
    fprintf(output, "Queue Size: %zu\n", bridge_state.callback_queue_count);
    fprintf(output, "Shutdown: %s\n", "No");
    fprintf(output, "\n");
    
    fprintf(output, "=== Thread Registry ===\n");
    ThreadRegistryEntry *thread_entry = bridge_state.thread_registry;
    int thread_count = 0;
    while (thread_entry) {
        thread_count++;
        thread_entry = thread_entry->next;
    }
    fprintf(output, "Registered Threads: %d\n", thread_count);
    fprintf(output, "Registry Shutdown: %s\n", "No");
    
    pthread_mutex_unlock(&bridge_state.mutex);
}

// =============================================================================
// HELPER RESULT FUNCTIONS
// =============================================================================

// Result functions are implemented in the runtime library