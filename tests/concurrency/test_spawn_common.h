/**
 * Asthra Programming Language v1.2 Concurrency Tests - Spawn Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common definitions, types, and utilities for spawn functionality tests
 */

#ifndef TEST_SPAWN_COMMON_H
#define TEST_SPAWN_COMMON_H

#include "concurrency_test_utils.h"
#include <time.h>
#include <unistd.h>

// =============================================================================
// SHARED TASK FUNCTION DEFINITIONS
// =============================================================================

// Simple task function for basic testing (exported for use by other modules)
void *simple_task_function(void *arg);

// Task with arguments structure
struct TaskArgs {
    int a;
    int b;
    char operation;
};

// Task function that accepts TaskArgs
void *task_with_arguments(void *arg);

// Multi-statement task function
void *multi_statement_task(void *arg);

// Handle task data structure
typedef struct {
    int task_id;
    int data;
} HandleTaskData;

// Handle task function
void *handle_task_function(void *arg);

// Timeout task function
void *timeout_task_function(void *arg);

// Deterministic task function
void *deterministic_task(void *arg);

// Performance task function
void *performance_task(void *arg);

// Error case task functions
void *error_task_null_return(void *arg);
void *error_task_exception(void *arg);

// C integration task functions
void *c_function_for_asthra(void *arg);
void *c_thread_calling_asthra(void *arg);

// Mock Asthra function callable from C
ConcurrentResult asthra_function_from_c(int value);

// =============================================================================
// SHARED UTILITY FUNCTIONS
// =============================================================================

// Timing utilities for performance testing
static inline double timespec_to_ms(struct timespec *start, struct timespec *end) {
    long time_taken = (end->tv_sec - start->tv_sec) * 1000000000L + (end->tv_nsec - start->tv_nsec);
    return time_taken / 1000000.0;
}

// Task cleanup utility
static inline void cleanup_tasks(TestTask **tasks, int count) {
    for (int i = 0; i < count; i++) {
        if (tasks[i]) {
            if (tasks[i]->result) {
                free(tasks[i]->result);
            }
            test_task_destroy(tasks[i]);
        }
    }
}

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

// Basic spawn functionality tests
ConcurrencyTestResult test_concurrency_spawn_basic(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_spawn_deterministic(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_spawn_multiple(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_spawn_with_arguments(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_spawn_multiple_statements(AsthraV12TestContext *ctx);

// Handle operations and await expressions
ConcurrencyTestResult test_concurrency_spawn_with_handle_operations(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_await_expressions(AsthraV12TestContext *ctx);

// Performance and behavior tests
ConcurrencyTestResult test_concurrency_deterministic_behavior(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_performance_scalability(AsthraV12TestContext *ctx);

// Error handling tests
ConcurrencyTestResult test_concurrency_error_cases(AsthraV12TestContext *ctx);

// C integration tests
ConcurrencyTestResult test_concurrency_spawn_with_c_functions(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_c_threads_calling_asthra(AsthraV12TestContext *ctx);

#endif // TEST_SPAWN_COMMON_H
