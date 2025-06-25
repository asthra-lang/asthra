/**
 * Asthra Programming Language v1.2 Pattern Matching Test Helpers Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of helper functions for pattern matching tests.
 */

#include "test_pattern_matching_helpers.h"

// =============================================================================
// HELPER FUNCTION IMPLEMENTATIONS
// =============================================================================

TestResult test_result_ok(int value) {
    return (TestResult){.is_ok = true, .data.ok_value = value};
}

TestResult test_result_err(const char *error) {
    return (TestResult){.is_ok = false, .data.error_message = error};
}

// Mock FFI function that can return errors
TestResult mock_ffi_function(int input) {
    if (input < 0) {
        return test_result_err("Negative input not allowed");
    }
    if (input > 1000) {
        return test_result_err("Input too large");
    }
    return test_result_ok(input * 2);
}

// Recursive function for deep call stack testing
TestResult recursive_impl(int depth, int max_depth) {
    if (depth >= max_depth) {
        if (depth % 2 == 0) {
            return test_result_ok(depth);
        } else {
            return test_result_err("Max depth reached");
        }
    }

    // Recursive call
    TestResult result = recursive_impl(depth + 1, max_depth);

    // Propagate errors up the call stack
    if (!result.is_ok) {
        return result;
    }

    // Transform the result
    return test_result_ok(result.data.ok_value + 1);
}
