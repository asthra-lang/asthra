/**
 * Asthra Programming Language - Thread Management Utilities
 *
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Thread registration and management for concurrency testing
 */

#include "concurrency_integration_common.h"

// Global test context reference
extern ConcurrencyIntegrationTestContext *g_concurrency_context;

// =============================================================================
// THREAD MANAGEMENT
// =============================================================================

/**
 * Register thread for concurrency testing
 */
bool concurrency_register_test_thread(const char *thread_name) {
    if (!g_concurrency_context || !thread_name)
        return false;

    // Find available slot
    for (size_t i = 0; i < g_concurrency_context->max_threads; i++) {
        ConcurrencyTestThread *thread = &g_concurrency_context->threads[i];
        if (!thread->is_registered) {
            thread->thread_id = pthread_self();
            thread->is_registered = true;
            thread->is_active = true;
            thread->thread_name = strdup(thread_name);
            atomic_store(&thread->reference_count, 1);
            clock_gettime(CLOCK_MONOTONIC, &thread->creation_time);
            g_concurrency_context->thread_count++;
            return true;
        }
    }

    return false; // No available slots
}

/**
 * Unregister thread
 */
bool concurrency_unregister_test_thread(void) {
    if (!g_concurrency_context)
        return false;

    pthread_t current_thread = pthread_self();

    for (size_t i = 0; i < g_concurrency_context->max_threads; i++) {
        ConcurrencyTestThread *thread = &g_concurrency_context->threads[i];
        if (thread->is_registered && pthread_equal(thread->thread_id, current_thread)) {
            thread->is_active = false;
            thread->is_registered = false;
            free((void *)thread->thread_name);
            thread->thread_name = NULL;
            atomic_store(&thread->reference_count, 0);
            g_concurrency_context->thread_count--;
            return true;
        }
    }

    return false;
}