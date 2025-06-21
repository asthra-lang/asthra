/**
 * Asthra Programming Language
 * Test Framework - Statistics Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Thread-safe test statistics with atomic operations
 * Enhanced for Testing Framework Standardization Plan Phase 1
 */

#ifndef ASTHRA_TEST_STATISTICS_H
#define ASTHRA_TEST_STATISTICS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPES AND STRUCTURES
// =============================================================================

// Thread-safe test statistics with C17 atomic operations
typedef struct {
    _Atomic(uint64_t) tests_run;
    _Atomic(uint64_t) tests_passed;
    _Atomic(uint64_t) tests_failed;
    _Atomic(uint64_t) tests_skipped;
    _Atomic(uint64_t) tests_error;
    _Atomic(uint64_t) tests_timeout;
    _Atomic(uint64_t) total_duration_ns;
    _Atomic(uint64_t) max_duration_ns;
    _Atomic(uint64_t) min_duration_ns;
    _Atomic(uint64_t) assertions_checked;
    _Atomic(uint64_t) assertions_failed;
    
    // Compatibility fields for simple access (NEW for Phase 1)
    // These provide non-atomic access for simple test suites
    union {
        struct {
            size_t total_tests;
            size_t passed_tests;
            size_t failed_tests;
            size_t skipped_tests;
            size_t error_tests;
        };
        // Padding to ensure alignment
        char _padding[40];
    };
} AsthraTestStatistics;

// =============================================================================
// ATOMIC OPERATIONS FOR THREAD-SAFE STATISTICS
// =============================================================================

// Atomic statistics operations with explicit memory ordering
static inline void asthra_test_increment_stat(_Atomic(uint64_t) *counter, uint64_t value) {
    atomic_fetch_add_explicit(counter, value, memory_order_relaxed);
}

static inline uint64_t asthra_test_get_stat(const _Atomic(uint64_t) *counter) {
    return atomic_load_explicit(counter, memory_order_acquire);
}

static inline void asthra_test_set_stat(_Atomic(uint64_t) *counter, uint64_t value) {
    atomic_store_explicit(counter, value, memory_order_release);
}

static inline bool asthra_test_compare_and_swap_stat(_Atomic(uint64_t) *counter,
                                                    uint64_t *expected,
                                                    uint64_t desired) {
    return atomic_compare_exchange_weak_explicit(counter, expected, desired,
                                               memory_order_acq_rel, memory_order_acquire);
}

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Test statistics management
AsthraTestStatistics* asthra_test_statistics_create(void);
void asthra_test_statistics_destroy(AsthraTestStatistics *stats);
void asthra_test_statistics_reset(AsthraTestStatistics *stats);
void asthra_test_statistics_print(const AsthraTestStatistics *stats, bool json_format);

// NEW: Sync compatibility fields with atomic counters (Phase 1)
void asthra_test_statistics_sync_compat_fields(AsthraTestStatistics *stats);

// =============================================================================
// TIMING UTILITIES
// =============================================================================

// Convert nanoseconds to human-readable format
static inline double asthra_test_ns_to_ms(uint64_t ns) {
    return (double)ns / 1000000.0;
}

static inline double asthra_test_ns_to_seconds(uint64_t ns) {
    return (double)ns / 1000000000.0;
}

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_STATISTICS_H
