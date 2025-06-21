/**
 * Asthra Programming Language v1.2 Pattern Matching Test Helpers
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common data structures and helper functions for pattern matching tests.
 */

#ifndef ASTHRA_TEST_PATTERN_MATCHING_HELPERS_H
#define ASTHRA_TEST_PATTERN_MATCHING_HELPERS_H

#include "test_comprehensive.h"
#include "../../runtime/types/asthra_runtime_result.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

// =============================================================================
// TEST DATA STRUCTURES
// =============================================================================

// Mock Result<T,E> implementation for testing
typedef struct {
    bool is_ok;
    union {
        int ok_value;
        const char *error_message;
    } data;
} TestResult;

// Nested Result type for complex pattern matching tests
typedef struct {
    bool outer_is_ok;
    union {
        TestResult inner_result;
        const char *outer_error;
    } outer_data;
} NestedResult;

// Task result structure for boundary testing
typedef struct {
    int task_id;
    TestResult result;
} TaskResult;

// =============================================================================
// HELPER FUNCTION DECLARATIONS
// =============================================================================

/**
 * Create a successful TestResult with the given value
 */
TestResult test_result_ok(int value);

/**
 * Create a failed TestResult with the given error message
 */
TestResult test_result_err(const char *error);

/**
 * Mock FFI function that can return errors based on input validation
 */
TestResult mock_ffi_function(int input);

/**
 * Recursive function for testing deep call stack error propagation
 */
TestResult recursive_impl(int depth, int max_depth);

#endif // ASTHRA_TEST_PATTERN_MATCHING_HELPERS_H 
