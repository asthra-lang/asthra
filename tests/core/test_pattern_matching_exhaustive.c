/**
 * Asthra Programming Language v1.2 Pattern Matching Exhaustive Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for exhaustive pattern matching, ensuring all Result<T,E> cases
 * are handled and complex pattern scenarios work correctly.
 */

#include "test_pattern_matching_helpers.h"

// =============================================================================
// EXHAUSTIVE MATCHING TESTS
// =============================================================================

AsthraTestResult test_result_exhaustive_matching(AsthraV12TestContext *ctx) {
    // Test that all Result<T,E> cases are handled in pattern matching

    // Test Ok case
    TestResult ok_result = test_result_ok(42);
    bool ok_handled = false;
    bool err_handled = false;

    // Simulate pattern matching
    if (ok_result.is_ok) {
        ok_handled = true;
        if (!ASTHRA_TEST_ASSERT(&ctx->base, ok_result.data.ok_value == 42,
                               "Ok value should be 42, got %d", ok_result.data.ok_value)) {
            return ASTHRA_TEST_FAIL;
        }
    } else {
        err_handled = true;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, ok_handled && !err_handled,
                           "Ok case should be handled, not error case")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test Err case
    TestResult err_result = test_result_err("Test error");
    ok_handled = false;
    err_handled = false;

    if (err_result.is_ok) {
        ok_handled = true;
    } else {
        err_handled = true;
        if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(err_result.data.error_message, "Test error") == 0,
                               "Error message should be 'Test error', got '%s'", err_result.data.error_message)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !ok_handled && err_handled,
                           "Error case should be handled, not ok case")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_result_all_cases_handled(AsthraV12TestContext *ctx) {
    // Test that compiler ensures all cases are handled

    // Array of test results to check
    TestResult test_cases[] = {
        test_result_ok(1),
        test_result_ok(100),
        test_result_err("Error 1"),
        test_result_err("Error 2"),
        test_result_ok(0)
    };

    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);
    int ok_count = 0;
    int err_count = 0;

    for (size_t i = 0; i < test_count; i++) {
        if (test_cases[i].is_ok) {
            ok_count++;
        } else {
            err_count++;
        }
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, ok_count == 3,
                           "Expected 3 Ok cases, got %d", ok_count)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, err_count == 2,
                           "Expected 2 Err cases, got %d", err_count)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_result_nested_matching(AsthraV12TestContext *ctx) {
    // Test nested Result types and complex pattern matching

    // Create test cases
    NestedResult nested_ok_ok = {
        .outer_is_ok = true,
        .outer_data.inner_result = test_result_ok(42)
    };

    NestedResult nested_ok_err = {
        .outer_is_ok = true,
        .outer_data.inner_result = test_result_err("Inner error")
    };

    NestedResult nested_err = {
        .outer_is_ok = false,
        .outer_data.outer_error = "Outer error"
    };

    // Test nested_ok_ok case
    if (nested_ok_ok.outer_is_ok) {
        if (nested_ok_ok.outer_data.inner_result.is_ok) {
            if (!ASTHRA_TEST_ASSERT(&ctx->base, nested_ok_ok.outer_data.inner_result.data.ok_value == 42,
                                   "Nested Ok(Ok(42)) should have value 42")) {
                return ASTHRA_TEST_FAIL;
            }
        } else {
            return ASTHRA_TEST_FAIL; // Should not reach here
        }
    } else {
        return ASTHRA_TEST_FAIL; // Should not reach here
    }

    // Test nested_ok_err case
    if (nested_ok_err.outer_is_ok) {
        if (!nested_ok_err.outer_data.inner_result.is_ok) {
            if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(nested_ok_err.outer_data.inner_result.data.error_message, "Inner error") == 0,
                                   "Nested Ok(Err) should have inner error message")) {
                return ASTHRA_TEST_FAIL;
            }
        } else {
            return ASTHRA_TEST_FAIL; // Should not reach here
        }
    } else {
        return ASTHRA_TEST_FAIL; // Should not reach here
    }

    // Test nested_err case
    if (!nested_err.outer_is_ok) {
        if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(nested_err.outer_data.outer_error, "Outer error") == 0,
                               "Nested Err should have outer error message")) {
            return ASTHRA_TEST_FAIL;
        }
    } else {
        return ASTHRA_TEST_FAIL; // Should not reach here
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_result_complex_patterns(AsthraV12TestContext *ctx) {
    // Test complex pattern matching scenarios

    // Test pattern matching with guards (simulated)
    TestResult results[] = {
        test_result_ok(-5),   // Should be handled specially
        test_result_ok(0),    // Zero case
        test_result_ok(42),   // Normal positive
        test_result_ok(1000), // Large value
        test_result_err("Network error"),
        test_result_err("Parse error")
    };

    size_t result_count = sizeof(results) / sizeof(results[0]);

    for (size_t i = 0; i < result_count; i++) {
        TestResult result = results[i];

        if (result.is_ok) {
            int value = result.data.ok_value;

            // Simulate pattern matching with guards
            if (value < 0) {
                // Handle negative values
                if (!ASTHRA_TEST_ASSERT(&ctx->base, value == -5,
                                       "Expected negative value -5, got %d", value)) {
                    return ASTHRA_TEST_FAIL;
                }
            } else if (value == 0) {
                // Handle zero
                if (!ASTHRA_TEST_ASSERT(&ctx->base, value == 0,
                                       "Expected zero value")) {
                    return ASTHRA_TEST_FAIL;
                }
            } else if (value > 100) {
                // Handle large values
                if (!ASTHRA_TEST_ASSERT(&ctx->base, value >= 100,
                                       "Expected large value >= 100, got %d", value)) {
                    return ASTHRA_TEST_FAIL;
                }
            } else {
                // Handle normal positive values
                if (!ASTHRA_TEST_ASSERT(&ctx->base, value > 0 && value <= 100,
                                       "Expected normal positive value, got %d", value)) {
                    return ASTHRA_TEST_FAIL;
                }
            }
        } else {
            // Handle error cases
            const char *error = result.data.error_message;
            if (strstr(error, "Network") != NULL) {
                // Network error handling
                if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(error, "Network error") == 0,
                                       "Expected 'Network error', got '%s'", error)) {
                    return ASTHRA_TEST_FAIL;
                }
            } else if (strstr(error, "Parse") != NULL) {
                // Parse error handling
                if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(error, "Parse error") == 0,
                                       "Expected 'Parse error', got '%s'", error)) {
                    return ASTHRA_TEST_FAIL;
                }
            } else {
                // Unknown error
                return ASTHRA_TEST_FAIL;
            }
        }
    }

    return ASTHRA_TEST_PASS;
} 
