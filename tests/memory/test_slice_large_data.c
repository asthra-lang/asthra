/**
 * Asthra Programming Language v1.2 Slice Large Data Handling Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for handling large data sets including PQC-sized data, performance, and memory safety.
 */

#include "slice_test_common.h"

// =============================================================================
// LARGE DATA HANDLING TESTS
// =============================================================================

AsthraTestResult test_slice_pqc_sized_data(AsthraV12TestContext *ctx) {
    // Test handling of Post-Quantum Cryptography sized data (large)
    
    const size_t pqc_size = 1024 * 1024; // 1MB
    const size_t element_size = 1024; // 1KB elements
    const size_t element_count = pqc_size / element_size;
    
    TestSliceHeader *large_slice = test_slice_create(element_size, element_count);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, large_slice != NULL,
                           "Failed to create large slice for PQC data")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Fill with pattern data
    uint8_t *pattern_buffer = malloc(element_size);
    if (!pattern_buffer) {
        test_slice_destroy(large_slice);
        return ASTHRA_TEST_ERROR;
    }
    
    for (size_t i = 0; i < element_count; i++) {
        // Create unique pattern for each element
        for (size_t j = 0; j < element_size; j++) {
            pattern_buffer[j] = (uint8_t)((i + j) % 256);
        }
        
        bool success = test_slice_push(large_slice, pattern_buffer);
        if (!ASTHRA_TEST_ASSERT(&ctx->base, success,
                               "Failed to push large element %zu", i)) {
            free(pattern_buffer);
            test_slice_destroy(large_slice);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Verify data integrity
    for (size_t i = 0; i < element_count; i++) {
        uint8_t *element = (uint8_t*)test_slice_get(large_slice, i);
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, element != NULL,
                               "Failed to get large element %zu", i)) {
            free(pattern_buffer);
            test_slice_destroy(large_slice);
            return ASTHRA_TEST_FAIL;
        }
        
        // Verify pattern
        for (size_t j = 0; j < element_size; j++) {
            uint8_t expected = (uint8_t)((i + j) % 256);
            if (!ASTHRA_TEST_ASSERT(&ctx->base, element[j] == expected,
                                   "Data corruption at element %zu, byte %zu: expected %u, got %u", 
                                   i, j, expected, element[j])) {
                free(pattern_buffer);
                test_slice_destroy(large_slice);
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    free(pattern_buffer);
    test_slice_destroy(large_slice);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_large_data_performance(AsthraV12TestContext *ctx) {
    // Test performance with large data sets
    
    asthra_benchmark_start(ctx);
    
    const size_t large_capacity = 100000; // 100K elements
    TestSliceHeader *perf_slice = test_slice_create(sizeof(uint64_t), large_capacity);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, perf_slice != NULL,
                           "Failed to create large performance slice")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Benchmark insertion
    uint64_t insert_start = asthra_test_get_time_ns();
    
    for (size_t i = 0; i < large_capacity; i++) {
        uint64_t value = i * 12345;
        test_slice_push(perf_slice, &value);
        
        if (i % 10000 == 0) {
            asthra_benchmark_iteration(ctx);
        }
    }
    
    uint64_t insert_end = asthra_test_get_time_ns();
    uint64_t insert_duration = insert_end - insert_start;
    
    // Benchmark random access
    uint64_t access_start = asthra_test_get_time_ns();
    volatile uint64_t sum = 0;
    
    for (int i = 0; i < 100000; i++) {
        size_t index = (size_t)(i * 7919) % large_capacity; // Pseudo-random access
        uint64_t *value_ptr = (uint64_t*)test_slice_get(perf_slice, index);
        if (value_ptr) {
            sum += *value_ptr;
        }
    }
    
    uint64_t access_end = asthra_test_get_time_ns();
    uint64_t access_duration = access_end - access_start;
    
    asthra_benchmark_end(ctx);
    
    // Performance should be reasonable
    double insert_rate = (double)large_capacity / ((double)insert_duration / 1e9);
    double access_rate = 100000.0 / ((double)access_duration / 1e9);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, insert_rate > 1000000.0, // > 1M inserts/sec
                           "Large data insertion too slow: %.0f ops/sec", insert_rate)) {
        test_slice_destroy(perf_slice);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, access_rate > 10000000.0, // > 10M accesses/sec
                           "Large data access too slow: %.0f ops/sec", access_rate)) {
        test_slice_destroy(perf_slice);
        return ASTHRA_TEST_FAIL;
    }
    
    // Record performance metrics
    ctx->benchmark.throughput_ops_per_sec = access_rate;
    ctx->benchmark.min_duration_ns = access_duration;
    ctx->benchmark.max_duration_ns = insert_duration;
    
    test_slice_destroy(perf_slice);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_large_data_memory_safety(AsthraV12TestContext *ctx) {
    // Test memory safety with large data operations
    
    const size_t stress_capacity = 50000;
    TestSliceHeader *safety_slice = test_slice_create(sizeof(void*), stress_capacity);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, safety_slice != NULL,
                           "Failed to create memory safety test slice")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Allocate many small objects and store pointers
    void **allocated_ptrs = malloc(stress_capacity * sizeof(void*));
    if (!allocated_ptrs) {
        test_slice_destroy(safety_slice);
        return ASTHRA_TEST_ERROR;
    }
    
    // Fill slice with allocated pointers
    for (size_t i = 0; i < stress_capacity; i++) {
        void *ptr = malloc(64); // Small allocation
        if (!ptr) {
            // Cleanup on allocation failure
            for (size_t j = 0; j < i; j++) {
                free(allocated_ptrs[j]);
            }
            free(allocated_ptrs);
            test_slice_destroy(safety_slice);
            return ASTHRA_TEST_ERROR;
        }
        
        allocated_ptrs[i] = ptr;
        test_slice_push(safety_slice, &ptr);
    }
    
    // Verify all pointers are accessible and valid
    for (size_t i = 0; i < stress_capacity; i++) {
        void **ptr_ptr = (void**)test_slice_get(safety_slice, i);
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, ptr_ptr != NULL,
                               "Failed to access pointer at index %zu", i)) {
            // Cleanup
            for (size_t j = 0; j < stress_capacity; j++) {
                free(allocated_ptrs[j]);
            }
            free(allocated_ptrs);
            test_slice_destroy(safety_slice);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, *ptr_ptr == allocated_ptrs[i],
                               "Pointer mismatch at index %zu", i)) {
            // Cleanup
            for (size_t j = 0; j < stress_capacity; j++) {
                free(allocated_ptrs[j]);
            }
            free(allocated_ptrs);
            test_slice_destroy(safety_slice);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Test bounds checking still works with large data
    bool valid = test_slice_bounds_check(safety_slice, stress_capacity);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !valid,
                           "Bounds check should fail for index at capacity")) {
        // Cleanup
        for (size_t j = 0; j < stress_capacity; j++) {
            free(allocated_ptrs[j]);
        }
        free(allocated_ptrs);
        test_slice_destroy(safety_slice);
        return ASTHRA_TEST_FAIL;
    }
    
    // Cleanup all allocations
    for (size_t i = 0; i < stress_capacity; i++) {
        free(allocated_ptrs[i]);
    }
    free(allocated_ptrs);
    test_slice_destroy(safety_slice);
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Asthra Slice Large Data Tests ===\n\n");
    
    AsthraV12TestContext ctx = {0};
    
    // Run tests
    AsthraTestResult results[] = {
        test_slice_pqc_sized_data(&ctx),
        test_slice_large_data_performance(&ctx),
        test_slice_large_data_memory_safety(&ctx)
    };
    
    // Print results
    const char* test_names[] = {
        "PQC-Sized Data",
        "Large Data Performance", 
        "Memory Safety"
    };
    
    size_t test_count = sizeof(results) / sizeof(results[0]);
    size_t passed = 0;
    
    printf("\n=== Test Results ===\n");
    for (size_t i = 0; i < test_count; i++) {
        printf("[%s] %s\n", 
               results[i] == ASTHRA_TEST_PASS ? "PASS" : "FAIL",
               test_names[i]);
        if (results[i] == ASTHRA_TEST_PASS) {
            passed++;
        }
    }
    
    printf("\n=== Summary ===\n");
    printf("Tests run: %zu\n", test_count);
    printf("Passed: %zu\n", passed);
    printf("Failed: %zu\n", test_count - passed);
    printf("Pass rate: %.1f%%\n", (double)passed / (double)test_count * 100.0);
    
    return (passed == test_count) ? 0 : 1;
} 
