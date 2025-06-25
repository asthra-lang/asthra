/**
 * Asthra Programming Language
 * Test Framework - Assertions Main Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test for assertion functionality
 */

#include "test_assertions.h"
#include "test_context.h"
#include "test_statistics.h"
#include "test_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test function implementations
static AsthraTestResult test_basic_assertions(AsthraTestContext *context);
static AsthraTestResult test_equality_assertions(AsthraTestContext *context);
static AsthraTestResult test_range_assertions(AsthraTestContext *context);

// Test implementations
static AsthraTestResult test_basic_assertions(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test boolean assertion
    if (!ASTHRA_TEST_ASSERT(context, true, "Boolean assertion should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test integer assertion (non-zero is true)
    if (!ASTHRA_TEST_ASSERT(context, 42, "Integer assertion should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test size_t assertion
    size_t test_size = 1024;
    if (!ASTHRA_TEST_ASSERT(context, test_size, "Size assertion should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test string assertion
    const char *test_string = "Hello, Asthra!";
    if (!ASTHRA_TEST_ASSERT(context, test_string, "String assertion should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test pointer assertion
    void *test_ptr = malloc(64);
    if (!ASTHRA_TEST_ASSERT(context, test_ptr, "Pointer assertion should pass")) {
        free(test_ptr);
        return ASTHRA_TEST_FAIL;
    }
    free(test_ptr);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_equality_assertions(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test boolean equality
    if (!ASTHRA_TEST_ASSERT_EQ(context, true, true, "Boolean equality should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test integer equality
    if (!asthra_test_assert_int_eq(context, 42, 42, "Integer equality should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test string equality
    if (!ASTHRA_TEST_ASSERT_EQ(context, "test", "test", "String equality should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_range_assertions(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test integer range
    if (!ASTHRA_TEST_ASSERT_RANGE(context, 50, 0, 100, "Integer range should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test size_t range
    size_t test_size = 500;
    if (!ASTHRA_TEST_ASSERT_RANGE(context, test_size, (size_t)0, (size_t)1000,
                                  "Size range should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("=== Asthra Test Framework Assertions Test ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Define test metadata
    AsthraTestMetadata metadata1 = {.name = "test_basic_assertions",
                                    .file = __FILE__,
                                    .line = __LINE__,
                                    .function = "test_basic_assertions",
                                    .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                    .timeout_ns = 30000000000ULL,
                                    .skip = false,
                                    .skip_reason = NULL};

    AsthraTestMetadata metadata2 = {.name = "test_equality_assertions",
                                    .file = __FILE__,
                                    .line = __LINE__,
                                    .function = "test_equality_assertions",
                                    .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                    .timeout_ns = 30000000000ULL,
                                    .skip = false,
                                    .skip_reason = NULL};

    AsthraTestMetadata metadata3 = {.name = "test_range_assertions",
                                    .file = __FILE__,
                                    .line = __LINE__,
                                    .function = "test_range_assertions",
                                    .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                    .timeout_ns = 30000000000ULL,
                                    .skip = false,
                                    .skip_reason = NULL};

    // Run tests
    AsthraTestContext *context1 = asthra_test_context_create(&metadata1, stats);
    AsthraTestContext *context2 = asthra_test_context_create(&metadata2, stats);
    AsthraTestContext *context3 = asthra_test_context_create(&metadata3, stats);

    if (!context1 || !context2 || !context3) {
        fprintf(stderr, "Failed to create test contexts\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    // Run the tests
    AsthraTestResult result1 = test_basic_assertions(context1);
    AsthraTestResult result2 = test_equality_assertions(context2);
    AsthraTestResult result3 = test_range_assertions(context3);

    // Print results
    printf("Basic assertions: %s\n", result1 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");
    printf("Equality assertions: %s\n", result2 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");
    printf("Range assertions: %s\n", result3 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");

    // Print final statistics
    printf("\n");
    asthra_test_statistics_print(stats, false);

    // Cleanup
    asthra_test_context_destroy(context1);
    asthra_test_context_destroy(context2);
    asthra_test_context_destroy(context3);
    asthra_test_statistics_destroy(stats);

    // Return appropriate exit code
    if (result1 == ASTHRA_TEST_PASS && result2 == ASTHRA_TEST_PASS && result3 == ASTHRA_TEST_PASS) {
        printf("All assertion tests passed!\n");
        return 0;
    } else {
        printf("Some assertion tests failed!\n");
        return 1;
    }
}
