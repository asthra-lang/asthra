/**
 * Asthra Programming Language v1.2 Concurrency Tests - Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common utility functions and implementations for concurrency tests.
 */

#include "concurrency_test_utils.h"
#include "../../runtime/asthra_integration_stubs.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// =============================================================================
// UTILITY FUNCTION IMPLEMENTATIONS
// =============================================================================

// Helper functions for benchmark support (concurrency-specific implementations)
void concurrency_benchmark_start_stub(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    // Initialize benchmark data with the single field we have
    ctx->benchmark.throughput_ops_per_sec = 0.0;
}

void concurrency_benchmark_iteration_stub(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    // Simple iteration tracking - just update throughput estimation
    static int iteration_count = 0;
    iteration_count++;
}

void concurrency_benchmark_end_stub(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    // Simple throughput calculation based on test count
    if (ctx->tests_run > 0) {
        ctx->benchmark.throughput_ops_per_sec = (double)ctx->tests_run;
    }
}

// Benchmark function implementations (used in error handling tests)
void asthra_benchmark_start(AsthraV12TestContext *ctx) {
    concurrency_benchmark_start_stub(ctx);
}

void asthra_benchmark_iteration(AsthraV12TestContext *ctx) {
    concurrency_benchmark_iteration_stub(ctx);
}

void asthra_benchmark_end(AsthraV12TestContext *ctx) {
    concurrency_benchmark_end_stub(ctx);
}

uint64_t asthra_test_get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// ConcurrentResult helper functions
ConcurrentResult concurrent_result_ok(void *value) {
    return (ConcurrentResult){.is_ok = true, .data.ok_value = value};
}

ConcurrentResult concurrent_result_err(const char *error) {
    return (ConcurrentResult){.is_ok = false, .data.error_message = error};
}

// =============================================================================
// TASK MANAGEMENT IMPLEMENTATIONS
// =============================================================================

// Task execution wrapper
void *task_wrapper(void *arg) {
    TestTask *task = (TestTask *)arg;

    clock_gettime(CLOCK_MONOTONIC, &task->start_time);
    atomic_store(&task->status, TASK_STATUS_RUNNING);

    // Check for cancellation
    if (atomic_load(&task->should_cancel)) {
        atomic_store(&task->status, TASK_STATUS_ERROR);
        return NULL;
    }

    // Execute the actual task function
    task->result = task->function(task->arg);

    clock_gettime(CLOCK_MONOTONIC, &task->end_time);
    atomic_store(&task->status, TASK_STATUS_COMPLETED);

    return task->result;
}

// Mock spawn function
TestTask *test_spawn(void *(*function)(void *), void *arg) {
    static int next_task_id = 1;

    TestTask *task = malloc(sizeof(TestTask));
    if (!task)
        return NULL;

    task->task_id = next_task_id++;
    task->function = function;
    task->arg = arg;
    task->result = NULL;
    atomic_store(&task->status, TASK_STATUS_CREATED);
    atomic_store(&task->should_cancel, false);

    // Create thread
    int result = pthread_create(&task->thread, NULL, task_wrapper, task);
    if (result != 0) {
        free(task);
        return NULL;
    }

    return task;
}

void test_task_join(TestTask *task) {
    if (!task)
        return;
    pthread_join(task->thread, NULL);
}

void test_task_destroy(TestTask *task) {
    if (!task)
        return;
    free(task);
}

// =============================================================================
// FUTURE IMPLEMENTATIONS
// =============================================================================

TestFuture *test_future_create(void) {
    TestFuture *future = malloc(sizeof(TestFuture));
    if (!future)
        return NULL;

    atomic_store(&future->is_ready, false);
    future->value = NULL;
    future->error = NULL;
    pthread_mutex_init(&future->mutex, NULL);
    pthread_cond_init(&future->condition, NULL);

    return future;
}

void test_future_complete(TestFuture *future, void *value) {
    if (!future)
        return;

    pthread_mutex_lock(&future->mutex);
    future->value = value;
    atomic_store(&future->is_ready, true);
    pthread_cond_broadcast(&future->condition);
    pthread_mutex_unlock(&future->mutex);
}

void test_future_error(TestFuture *future, const char *error) {
    if (!future)
        return;

    pthread_mutex_lock(&future->mutex);
    future->error = error;
    atomic_store(&future->is_ready, true);
    pthread_cond_broadcast(&future->condition);
    pthread_mutex_unlock(&future->mutex);
}

void *test_future_await(TestFuture *future) {
    if (!future)
        return NULL;

    pthread_mutex_lock(&future->mutex);
    while (!atomic_load(&future->is_ready)) {
        pthread_cond_wait(&future->condition, &future->mutex);
    }
    void *result = future->value;
    pthread_mutex_unlock(&future->mutex);

    return result;
}

void test_future_destroy(TestFuture *future) {
    if (!future)
        return;

    pthread_mutex_destroy(&future->mutex);
    pthread_cond_destroy(&future->condition);
    free(future);
}

// =============================================================================
// TASK HANDLE IMPLEMENTATIONS
// =============================================================================

TestTaskHandle *test_task_handle_create(TestTask *task) {
    static int next_handle_id = 1;

    TestTaskHandle *handle = malloc(sizeof(TestTaskHandle));
    if (!handle)
        return NULL;

    handle->task = task;
    handle->future = test_future_create();
    handle->handle_id = next_handle_id++;

    if (!handle->future) {
        free(handle);
        return NULL;
    }

    return handle;
}

void test_task_handle_destroy(TestTaskHandle *handle) {
    if (!handle)
        return;

    if (handle->future) {
        test_future_destroy(handle->future);
    }
    free(handle);
}

// =============================================================================
// TEST CONTEXT WRAPPER FUNCTIONS
// =============================================================================

/**
 * Wrapper for test_context_init to match expected interface
 */
void test_context_init(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    // Map AsthraV12TestContext to AsthraTestContext
    // For now, we'll just initialize basic fields
    memset(ctx, 0, sizeof(AsthraV12TestContext));

    // Initialize basic context fields
    ctx->name = "concurrency_test";
    ctx->tests_run = 0;
    ctx->tests_passed = 0;

    // Initialize benchmark context
    ctx->benchmark.throughput_ops_per_sec = 0.0;
}

/**
 * Wrapper for test_context_cleanup to match expected interface
 */
void test_context_cleanup(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    // Basic cleanup - no dynamic memory to free in this simple case
    (void)ctx; // Suppress unused parameter warning
}
