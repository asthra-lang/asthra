/**
 * Asthra Programming Language String Operations Test Helpers
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common helpers, structures, and utilities for string operation tests.
 */

#ifndef ASTHRA_TEST_STRING_HELPERS_H
#define ASTHRA_TEST_STRING_HELPERS_H

#include "test_comprehensive.h"
#include "test_comprehensive_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST DATA STRUCTURES
// =============================================================================

// Mock string structure for testing
typedef struct {
    char *data;
    size_t length;
    size_t capacity;
    bool is_gc_managed;
} TestString;

// =============================================================================
// HELPER FUNCTION DECLARATIONS
// =============================================================================

// String creation and management
TestString *test_string_create(const char *str);
void test_string_destroy(TestString *ts);
TestString *test_string_concat(const TestString *a, const TestString *b);

// Test utility functions
uint64_t asthra_test_get_time_ns(void);

// Benchmark utility functions
void asthra_benchmark_start(AsthraExtendedTestContext *ctx);
void asthra_benchmark_end(AsthraExtendedTestContext *ctx);
void asthra_benchmark_iteration(AsthraExtendedTestContext *ctx);

// Backward compatibility aliases
// Note: asthra_benchmark_* functions are used directly (no aliases needed)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_STRING_HELPERS_H
