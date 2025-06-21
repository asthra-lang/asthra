/**
 * Asthra Programming Language v1.2 Concurrency Tests - Utilities Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Header file for common utility functions used across concurrency tests.
 */

#ifndef ASTHRA_CONCURRENCY_TEST_UTILS_H
#define ASTHRA_CONCURRENCY_TEST_UTILS_H

#include "concurrency_test_common.h"

// Utility function declarations
uint64_t asthra_test_get_time_ns(void);

// Benchmark function declarations (used in error handling tests)
void asthra_benchmark_start(AsthraV12TestContext *ctx);
void asthra_benchmark_end(AsthraV12TestContext *ctx);
void asthra_benchmark_iteration(AsthraV12TestContext *ctx);

// Concurrency-specific benchmark stubs (renamed to avoid conflicts)
void concurrency_benchmark_start_stub(AsthraV12TestContext *ctx);
void concurrency_benchmark_iteration_stub(AsthraV12TestContext *ctx);
void concurrency_benchmark_end_stub(AsthraV12TestContext *ctx);

// ConcurrentResult helper functions
ConcurrentResult concurrent_result_ok(void *value);
ConcurrentResult concurrent_result_err(const char *error);

// Task management functions
void* task_wrapper(void *arg);
TestTask* test_spawn(void *(*function)(void*), void *arg);
void test_task_join(TestTask *task);
void test_task_destroy(TestTask *task);

// Future management functions
TestFuture* test_future_create(void);
void test_future_complete(TestFuture *future, void *value);
void test_future_error(TestFuture *future, const char *error);
void* test_future_await(TestFuture *future);
void test_future_destroy(TestFuture *future);

// Task handle management functions
TestTaskHandle* test_task_handle_create(TestTask *task);
void test_task_handle_destroy(TestTaskHandle *handle);

// Test context wrapper functions (for concurrency test compatibility)
void test_context_init(AsthraV12TestContext *ctx);
void test_context_cleanup(AsthraV12TestContext *ctx);

#endif // ASTHRA_CONCURRENCY_TEST_UTILS_H 
