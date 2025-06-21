/**
 * Asthra Programming Language - Concurrency Integration Common Definitions
 * 
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common definitions and types for concurrency integration testing
 */

#ifndef CONCURRENCY_INTEGRATION_COMMON_H
#define CONCURRENCY_INTEGRATION_COMMON_H

#include "../../runtime/asthra_runtime.h"
#include "../../runtime/asthra_ffi_memory.h"
#include "../framework/test_framework.h"
#include "../performance/performance_validation.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>

// =============================================================================
// COMMON DATA STRUCTURES
// =============================================================================

// Thread management context
typedef struct {
    pthread_t thread_id;
    bool is_registered;
    bool is_active;
    const char* thread_name;
    void* thread_data;
    atomic_int reference_count;
    struct timespec creation_time;
} ConcurrencyTestThread;

// Lock-free data structure for testing
typedef struct LockFreeNode {
    atomic_uintptr_t next;
    atomic_int data;
    atomic_bool is_valid;
} LockFreeNode;

typedef struct {
    atomic_uintptr_t head;
    atomic_size_t size;
    atomic_size_t operations_count;
} LockFreeQueue;

// Synchronization primitives testing
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_rwlock_t rwlock;
    atomic_bool barrier_ready;
    atomic_int barrier_count;
    int max_threads;
} SynchronizationPrimitives;

// Concurrency test context
typedef struct {
    ConcurrencyTestThread* threads;
    size_t thread_count;
    size_t max_threads;
    
    LockFreeQueue* test_queue;
    SynchronizationPrimitives* sync_primitives;
    
    // Performance tracking
    atomic_size_t total_operations;
    atomic_size_t successful_operations;
    atomic_size_t failed_operations;
    double test_duration_ms;
    
    // Error tracking
    atomic_size_t race_conditions_detected;
    atomic_size_t deadlocks_detected;
    atomic_size_t memory_leaks_detected;
    
    bool test_active;
    struct timespec start_time;
} ConcurrencyTestContext;

// Test thread data structure for stress testing
typedef struct {
    int thread_id;
    int iterations;
    int operations_per_iteration;
    bool test_result;
} ConcurrencyStressTestData;

// =============================================================================
// GLOBAL CONTEXT
// =============================================================================

extern ConcurrencyTestContext* g_concurrency_context;

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

static inline double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static inline uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static inline void sleep_ms(int milliseconds) {
    struct timespec sleep_time = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (milliseconds % 1000) * 1000000
    };
    nanosleep(&sleep_time, NULL);
}

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Lock-free queue operations
LockFreeQueue* lockfree_queue_create(void);
bool lockfree_queue_enqueue(LockFreeQueue* queue, int data);
bool lockfree_queue_dequeue(LockFreeQueue* queue, int* data);
size_t lockfree_queue_size(LockFreeQueue* queue);
void lockfree_queue_destroy(LockFreeQueue* queue);

// Thread management
bool concurrency_register_test_thread(const char* thread_name);
bool concurrency_unregister_test_thread(void);

// Synchronization primitives
SynchronizationPrimitives* sync_primitives_create(int max_threads);
bool sync_primitives_barrier_wait(SynchronizationPrimitives* sync);
bool sync_primitives_test_mutex(SynchronizationPrimitives* sync, int iterations);
bool sync_primitives_test_rwlock_read(SynchronizationPrimitives* sync, int iterations);
bool sync_primitives_test_rwlock_write(SynchronizationPrimitives* sync, int iterations);
void sync_primitives_destroy(SynchronizationPrimitives* sync);

// Test context management
ConcurrencyTestContext* concurrency_test_context_create(size_t max_threads);
bool concurrency_test_initialize(size_t max_threads);
bool concurrency_test_start(void);
bool concurrency_test_stop(void);
void concurrency_test_get_statistics(size_t* total_ops, size_t* successful_ops, 
                                    size_t* failed_ops, double* duration_ms,
                                    size_t* race_conditions, size_t* deadlocks);
void concurrency_test_cleanup(void);

// Test thread functions
void* concurrency_producer_thread(void* arg);
void* concurrency_consumer_thread(void* arg);
void* concurrency_sync_stress_thread(void* arg);

// Test execution functions
bool run_concurrency_stress_test(int num_threads, int iterations_per_thread);
bool test_concurrency_performance_integration(void);
bool run_concurrency_integration_tests(void);

#endif // CONCURRENCY_INTEGRATION_COMMON_H