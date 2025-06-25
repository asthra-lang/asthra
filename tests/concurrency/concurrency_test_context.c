/**
 * Asthra Programming Language - Concurrency Test Context Management
 *
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test context management for concurrency integration testing
 */

#include "concurrency_integration_common.h"

// Global test context
ConcurrencyIntegrationTestContext *g_concurrency_context = NULL;

// =============================================================================
// CONCURRENCY TEST CONTEXT MANAGEMENT
// =============================================================================

/**
 * Create concurrency test context
 */
ConcurrencyIntegrationTestContext *concurrency_test_context_create(size_t max_threads) {
    ConcurrencyIntegrationTestContext *context =
        calloc(1, sizeof(ConcurrencyIntegrationTestContext));
    if (!context)
        return NULL;

    context->max_threads = max_threads;
    context->threads = calloc(max_threads, sizeof(ConcurrencyTestThread));
    if (!context->threads) {
        free(context);
        return NULL;
    }

    context->test_queue = lockfree_queue_create();
    if (!context->test_queue) {
        free(context->threads);
        free(context);
        return NULL;
    }

    context->sync_primitives = sync_primitives_create((int)max_threads);
    if (!context->sync_primitives) {
        lockfree_queue_destroy(context->test_queue);
        free(context->threads);
        free(context);
        return NULL;
    }

    // Initialize atomic counters
    atomic_store(&context->total_operations, 0);
    atomic_store(&context->successful_operations, 0);
    atomic_store(&context->failed_operations, 0);
    atomic_store(&context->race_conditions_detected, 0);
    atomic_store(&context->deadlocks_detected, 0);
    atomic_store(&context->memory_leaks_detected, 0);

    context->test_active = false;
    context->thread_count = 0;

    return context;
}

/**
 * Initialize global concurrency testing
 */
bool concurrency_test_initialize(size_t max_threads) {
    if (g_concurrency_context) {
        return false; // Already initialized
    }

    g_concurrency_context = concurrency_test_context_create(max_threads);
    return g_concurrency_context != NULL;
}

/**
 * Start concurrency test
 */
bool concurrency_test_start(void) {
    if (!g_concurrency_context || g_concurrency_context->test_active) {
        return false;
    }

    clock_gettime(CLOCK_MONOTONIC, &g_concurrency_context->start_time);
    g_concurrency_context->test_active = true;

    return true;
}

/**
 * Stop concurrency test
 */
bool concurrency_test_stop(void) {
    if (!g_concurrency_context || !g_concurrency_context->test_active) {
        return false;
    }

    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double start_ms = (double)g_concurrency_context->start_time.tv_sec * 1000.0 +
                      (double)g_concurrency_context->start_time.tv_nsec / 1000000.0;
    double end_ms = (double)end_time.tv_sec * 1000.0 + (double)end_time.tv_nsec / 1000000.0;

    g_concurrency_context->test_duration_ms = end_ms - start_ms;
    g_concurrency_context->test_active = false;

    return true;
}

/**
 * Get concurrency test statistics
 */
void concurrency_test_get_statistics(size_t *total_ops, size_t *successful_ops, size_t *failed_ops,
                                     double *duration_ms, size_t *race_conditions,
                                     size_t *deadlocks) {
    if (!g_concurrency_context)
        return;

    if (total_ops)
        *total_ops = atomic_load(&g_concurrency_context->total_operations);
    if (successful_ops)
        *successful_ops = atomic_load(&g_concurrency_context->successful_operations);
    if (failed_ops)
        *failed_ops = atomic_load(&g_concurrency_context->failed_operations);
    if (duration_ms)
        *duration_ms = g_concurrency_context->test_duration_ms;
    if (race_conditions)
        *race_conditions = atomic_load(&g_concurrency_context->race_conditions_detected);
    if (deadlocks)
        *deadlocks = atomic_load(&g_concurrency_context->deadlocks_detected);
}

/**
 * Destroy concurrency test context
 */
void concurrency_test_cleanup(void) {
    if (!g_concurrency_context)
        return;

    // Wait for all threads to finish
    for (size_t i = 0; i < g_concurrency_context->max_threads; i++) {
        ConcurrencyTestThread *thread = &g_concurrency_context->threads[i];
        if (thread->is_active) {
            thread->is_active = false;
        }
        if (thread->thread_name) {
            free((void *)thread->thread_name);
        }
    }

    if (g_concurrency_context->sync_primitives) {
        sync_primitives_destroy(g_concurrency_context->sync_primitives);
    }

    if (g_concurrency_context->test_queue) {
        lockfree_queue_destroy(g_concurrency_context->test_queue);
    }

    free(g_concurrency_context->threads);
    free(g_concurrency_context);
    g_concurrency_context = NULL;
}