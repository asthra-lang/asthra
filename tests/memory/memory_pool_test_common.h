#ifndef MEMORY_POOL_TEST_COMMON_H
#define MEMORY_POOL_TEST_COMMON_H

#include "memory_pool.h"
#include "benchmark.h"
#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// =============================================================================
// COMMON TEST DATA STRUCTURES
// =============================================================================

// Test data structure for pool allocation
typedef struct {
    uint64_t id;
    char data[64];
    double value;
} TestObject;

// Thread test context
typedef struct {
    AsthraFixedSizePool *pool;
    int thread_id;
    int iterations;
    void **allocated_ptrs;
    int allocation_count;
    bool success;
} ThreadTestContext;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Fixed-size pool tests
AsthraTestResult test_fixed_pool_creation(AsthraTestContext *context);
AsthraTestResult test_fixed_pool_allocation(AsthraTestContext *context);
AsthraTestResult test_fixed_pool_statistics(AsthraTestContext *context);
AsthraTestResult test_fixed_pool_thread_safety(AsthraTestContext *context);

// Stack allocator tests
AsthraTestResult test_stack_allocator_basic(AsthraTestContext *context);
AsthraTestResult test_stack_allocator_alignment(AsthraTestContext *context);

// Ring buffer tests
AsthraTestResult test_ring_buffer_basic(AsthraTestContext *context);
AsthraTestResult test_ring_buffer_wrap_around(AsthraTestContext *context);

// Performance tests
AsthraTestResult test_memory_pool_performance(AsthraTestContext *context);

// Thread worker function
void* thread_pool_test_worker(void *arg);

#endif // MEMORY_POOL_TEST_COMMON_H 
