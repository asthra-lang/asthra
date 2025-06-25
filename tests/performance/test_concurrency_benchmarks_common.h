/*
 * Concurrency Performance Benchmarks - Common Definitions
 * Shared structures, macros, and utilities for performance testing
 *
 * Phase 8: Testing and Validation
 * Focus: Performance validation, scalability, and tier comparison
 */

#ifndef TEST_CONCURRENCY_BENCHMARKS_COMMON_H
#define TEST_CONCURRENCY_BENCHMARKS_COMMON_H

#include "../../runtime/stdlib_concurrency_support.h"
#include "../framework/test_framework.h"
#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

// ============================================================================
// Global Test Counters
// ============================================================================

extern size_t tests_run;
extern size_t tests_passed;

// ============================================================================
// Test Framework Macros
// ============================================================================

#define ASSERT_TRUE(condition)                                                                     \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("  ✅ PASS: %s\n", #condition);                                                 \
        } else {                                                                                   \
            printf("  ❌ FAIL: %s:%d - %s\n", __FILE__, __LINE__, #condition);                     \
        }                                                                                          \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#define ASSERT_PERFORMANCE(actual_ms, expected_ms, tolerance_percent)                              \
    do {                                                                                           \
        double tolerance = (expected_ms) * (tolerance_percent) / 100.0;                            \
        ASSERT_TRUE(fabs((actual_ms) - (expected_ms)) <= tolerance);                               \
    } while (0)

// ============================================================================
// Timing Utilities
// ============================================================================

typedef struct {
    struct timeval start;
    struct timeval end;
} Timer;

void timer_start(Timer *timer);
double timer_stop(Timer *timer);

// ============================================================================
// Test Function Declarations
// ============================================================================

// Channel performance tests
void test_channel_throughput(void);
void test_channel_latency(void);
void test_channel_scalability(void);

// Select performance tests
void test_select_performance(void);

// Barrier performance tests
void test_barrier_performance(void);

// Semaphore performance tests
void test_semaphore_performance(void);

// Memory performance tests
void test_memory_efficiency(void);

// Tier comparison tests
void test_tier1_vs_tier2_performance(void);

// Stress tests
void test_stress_many_channels(void);

// Runtime initialization
void initialize_benchmark_runtime(void);
void cleanup_benchmark_runtime(void);
void print_benchmark_results(void);

#endif // TEST_CONCURRENCY_BENCHMARKS_COMMON_H
