/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Test Suite - Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.2: Static Analysis Integration Tests - Common Header
 */

#ifndef TEST_STATIC_ANALYSIS_COMMON_H
#define TEST_STATIC_ANALYSIS_COMMON_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../framework/test_framework.h"
#include "static_analysis.h"

// C17 static assertions for test assumptions
ASTHRA_STATIC_ASSERT(sizeof(void *) == 8, "Tests assume 64-bit architecture");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= 4, "Tests assume size_t is at least 32-bit");

// =============================================================================
// COMMON TEST HELPER FUNCTIONS
// =============================================================================

// Test helper for string comparison
static inline bool strings_equal(const char *str1, const char *str2) {
    if (str1 == NULL && str2 == NULL)
        return true;
    if (str1 == NULL || str2 == NULL)
        return false;
    return strcmp(str1, str2) == 0;
}

// Test helper for memory comparison
static inline bool memory_equal(const void *ptr1, const void *ptr2, size_t size) {
    if (ptr1 == NULL && ptr2 == NULL)
        return true;
    if (ptr1 == NULL || ptr2 == NULL)
        return false;
    return memcmp(ptr1, ptr2, size) == 0;
}

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

// Static Analysis Attribute Tests
AsthraTestResult test_static_analysis_attributes(AsthraTestContext *context);

// Const Correctness Tests
AsthraTestResult test_const_correct_string_operations(AsthraTestContext *context);
AsthraTestResult test_const_correct_memory_operations(AsthraTestContext *context);

// Restrict Pointer Tests
AsthraTestResult test_restrict_pointer_operations(AsthraTestContext *context);

// Buffer Operations Tests
AsthraTestResult test_buffer_operations(AsthraTestContext *context);

// String Formatting Tests
AsthraTestResult test_string_formatting(AsthraTestContext *context);

// Static Analysis Configuration Tests
AsthraTestResult test_static_analysis_configuration(AsthraTestContext *context);

#endif // TEST_STATIC_ANALYSIS_COMMON_H
