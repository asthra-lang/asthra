/**
 * Asthra Programming Language
 * Performance Test Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Performance measurement and memory tracking functions for testing
 */

#ifndef ASTHRA_PERFORMANCE_TEST_UTILS_H
#define ASTHRA_PERFORMANCE_TEST_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PERFORMANCE TESTING UTILITIES
// =============================================================================

/**
 * Performance measurement structure
 */
typedef struct {
    uint64_t start_time;
    uint64_t end_time;
    size_t memory_used;
    size_t peak_memory;
    size_t allocations_count;
    size_t deallocations_count;
} PerformanceMetrics;

/**
 * Start performance measurement
 * @return Performance metrics structure
 */
PerformanceMetrics *start_performance_measurement(void);

/**
 * End performance measurement
 * @param metrics The metrics to update
 */
void end_performance_measurement(PerformanceMetrics *metrics);

/**
 * Report performance metrics
 * @param metrics The metrics to report
 * @param test_name The name of the test
 */
void report_performance(const PerformanceMetrics *metrics, const char *test_name);

/**
 * Track memory allocation
 * @param size Size of allocation
 */
void track_memory_allocation(size_t size);

/**
 * Track memory deallocation
 * @param size Size of deallocation
 */
void track_memory_deallocation(size_t size);

/**
 * Get current memory usage
 * @return Current memory usage in bytes
 */
size_t get_current_memory_usage(void);

/**
 * Reset memory tracking
 */
void reset_memory_tracking(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PERFORMANCE_TEST_UTILS_H
