/**
 * Asthra Programming Language v1.2 Concurrency Tests - Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common definitions, structures, and utilities shared across concurrency tests.
 */

#ifndef ASTHRA_CONCURRENCY_TEST_COMMON_H
#define ASTHRA_CONCURRENCY_TEST_COMMON_H

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Basic test framework macros - use unique names to avoid conflicts
#define CONCURRENCY_TEST_PASS 1
#define CONCURRENCY_TEST_FAIL 0

typedef int ConcurrencyTestResult;

// Benchmark context structure
typedef struct {
    double throughput_ops_per_sec;
} BenchmarkContext;

typedef struct {
    const char *name;
    size_t tests_run;
    size_t tests_passed;
} ConcurrencyTestContext;

typedef struct {
    const char *name;
    size_t tests_run;
    size_t tests_passed;
    BenchmarkContext benchmark;
} AsthraV12TestContext;

#define CONCURRENCY_TEST_ASSERT(ctx, condition, message, ...)                                      \
    ((ctx)->tests_run++,                                                                           \
     (!(condition)) ? (printf("ASSERTION FAILED: " message "\n", ##__VA_ARGS__), false) : true)

// Mock task structure for testing
typedef struct {
    int task_id;
    void *(*function)(void *);
    void *arg;
    void *result;
    pthread_t thread;
    _Atomic(int) status; // 0=created, 1=running, 2=completed, 3=error
    _Atomic(bool) should_cancel;
    struct timespec start_time;
    struct timespec end_time;
} TestTask;

// Task status constants
#define TASK_STATUS_CREATED 0
#define TASK_STATUS_RUNNING 1
#define TASK_STATUS_COMPLETED 2
#define TASK_STATUS_ERROR 3

// Mock Result type for concurrent operations
typedef struct {
    bool is_ok;
    union {
        void *ok_value;
        const char *error_message;
    } data;
} ConcurrentResult;

// Mock future/promise structure for testing foundation
typedef struct {
    _Atomic(bool) is_ready;
    void *value;
    const char *error;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} TestFuture;

// Mock task handle structure
typedef struct {
    TestTask *task;
    TestFuture *future;
    int handle_id;
} TestTaskHandle;

// Test function prototypes
ConcurrencyTestResult test_concurrency_spawn_basic(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_spawn_with_c_functions(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_c_threads_calling_asthra(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_task_creation(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_task_execution(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_task_cleanup(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_result_propagation(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_error_across_tasks(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_error_handling_stress(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_await_foundation(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_task_handles(AsthraV12TestContext *ctx);
ConcurrencyTestResult test_concurrency_future_compatibility(AsthraV12TestContext *ctx);

#endif // ASTHRA_CONCURRENCY_TEST_COMMON_H
