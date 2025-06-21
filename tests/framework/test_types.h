/**
 * Asthra Programming Language
 * Test Framework - Common Types and Enums
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common types used across the test framework
 */

#ifndef ASTHRA_TEST_TYPES_H
#define ASTHRA_TEST_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 STATIC ASSERTIONS FOR TEST FRAMEWORK VALIDATION
// =============================================================================

// Test framework assumptions validation
_Static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t must be at least 32-bit for test framework");
_Static_assert(sizeof(void*) >= 4, "Pointer size must be at least 32-bit for test framework");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for test framework modernization");
_Static_assert(sizeof(clock_t) > 0, "clock_t must be available for timing measurements");

// Test result type validation
_Static_assert(sizeof(int) >= 4, "int must be at least 32 bits for test results");

// =============================================================================
// TEST FRAMEWORK TYPES AND ENUMS
// =============================================================================

// Test result types with C17 enum validation
typedef enum {
    ASTHRA_TEST_PASS = 0,
    ASTHRA_TEST_FAIL = 1,
    ASTHRA_TEST_SKIP = 2,
    ASTHRA_TEST_ERROR = 3,
    ASTHRA_TEST_TIMEOUT = 4,
    ASTHRA_TEST_RESULT_COUNT = 5
} AsthraTestResult;

// Compile-time validation of test result enum
_Static_assert(ASTHRA_TEST_RESULT_COUNT <= 8, "Test result types must fit in 3 bits");

// Test severity levels
typedef enum {
    ASTHRA_TEST_SEVERITY_LOW = 0,
    ASTHRA_TEST_SEVERITY_MEDIUM = 1,
    ASTHRA_TEST_SEVERITY_HIGH = 2,
    ASTHRA_TEST_SEVERITY_CRITICAL = 3
} AsthraTestSeverity;

// Test case metadata
typedef struct {
    const char *name;
    const char *file;
    int line;
    const char *function;
    AsthraTestSeverity severity;
    uint64_t timeout_ns;
    bool skip;
    const char *skip_reason;
} AsthraTestMetadata;

// Forward declarations
typedef struct AsthraTestContext AsthraTestContext;

// Test function signature
typedef AsthraTestResult (*AsthraTestFunction)(AsthraTestContext *context);

// =============================================================================
// TIMING UTILITIES
// =============================================================================

// High-resolution timing for test duration measurement
static inline uint64_t asthra_test_get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_TYPES_H
