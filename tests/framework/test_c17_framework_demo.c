/**
 * Asthra Programming Language
 * C17 Modernized Test Framework Demonstration
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.1: Test Framework Modernization Demo
 * This file demonstrates the new C17 features in the test framework:
 * - Static assertions for compile-time validation
 * - _Generic for polymorphic test operations
 * - Atomic operations for thread-safe statistics
 * - Enhanced test configuration with designated initializers
 */

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include "test_framework.h"
#include "../runtime/asthra_runtime.h"

// =============================================================================
// C17 STATIC ASSERTIONS FOR TEST VALIDATION
// =============================================================================

// Compile-time validation of test assumptions
_Static_assert(sizeof(int) == 4, "Tests assume 32-bit integers");
_Static_assert(sizeof(size_t) >= 4, "Tests require at least 32-bit size_t");
_Static_assert(sizeof(void*) >= 4, "Tests require at least 32-bit pointers");

// Test framework compatibility validation
_Static_assert(ASTHRA_TEST_RESULT_COUNT == 5, "Test framework expects exactly 5 result types");

// =============================================================================
// TEST IMPLEMENTATIONS USING C17 FEATURES
// =============================================================================

// Test basic assertions with _Generic dispatch
ASTHRA_TEST_DEFINE(test_generic_assertions, ASTHRA_TEST_SEVERITY_HIGH) {
    // Test boolean assertion
    if (!ASTHRA_TEST_ASSERT(context, true, "Boolean assertion should pass")) {
        ASTHRA_TEST_FAIL(context, "Boolean assertion failed");
    }

    // Test integer assertion (non-zero is true)
    if (!ASTHRA_TEST_ASSERT(context, 42, "Integer assertion should pass")) {
        ASTHRA_TEST_FAIL(context, "Integer assertion failed");
    }

    // Test size_t assertion
    size_t test_size = 1024;
    if (!ASTHRA_TEST_ASSERT(context, test_size, "Size assertion should pass")) {
        ASTHRA_TEST_FAIL(context, "Size assertion failed");
    }

    // Test string assertion
    const char *test_string = "Hello, Asthra!";
    if (!ASTHRA_TEST_ASSERT(context, test_string, "String assertion should pass")) {
        ASTHRA_TEST_FAIL(context, "String assertion failed");
    }

    // Test pointer assertion
    void *test_ptr = malloc(64);
    if (!ASTHRA_TEST_ASSERT(context, test_ptr, "Pointer assertion should pass")) {
        free(test_ptr);
        ASTHRA_TEST_FAIL(context, "Pointer assertion failed");
    }
    free(test_ptr);

    ASTHRA_TEST_PASS(context);
}

// Test equality assertions with _Generic dispatch
ASTHRA_TEST_DEFINE(test_generic_equality, ASTHRA_TEST_SEVERITY_HIGH) {
    // Test integer equality
    int expected_int = 42;
    int actual_int = 42;
    if (!ASTHRA_TEST_ASSERT_EQ(context, actual_int, expected_int, "Integer equality should pass")) {
        ASTHRA_TEST_FAIL(context, "Integer equality failed");
    }

    // Test size_t equality
    size_t expected_size = 1024;
    size_t actual_size = 1024;
    if (!ASTHRA_TEST_ASSERT_EQ(context, actual_size, expected_size, "Size equality should pass")) {
        ASTHRA_TEST_FAIL(context, "Size equality failed");
    }

    // Test string equality
    const char *expected_str = "Asthra";
    const char *actual_str = "Asthra";
    if (!ASTHRA_TEST_ASSERT_EQ(context, actual_str, expected_str, "String equality should pass")) {
        ASTHRA_TEST_FAIL(context, "String equality failed");
    }

    // Test pointer equality
    void *test_ptr = malloc(32);
    if (!ASTHRA_TEST_ASSERT_EQ(context, test_ptr, test_ptr, "Pointer equality should pass")) {
        free(test_ptr);
        ASTHRA_TEST_FAIL(context, "Pointer equality failed");
    }
    free(test_ptr);

    ASTHRA_TEST_PASS(context);
}

// Test range assertions with _Generic dispatch
ASTHRA_TEST_DEFINE(test_generic_ranges, ASTHRA_TEST_SEVERITY_MEDIUM) {
    // Test integer range
    int test_value = 50;
    if (!ASTHRA_TEST_ASSERT_RANGE(context, test_value, 1, 100, "Integer should be in range [1, 100]")) {
        ASTHRA_TEST_FAIL(context, "Integer range assertion failed");
    }

    // Test size_t range
    size_t test_size = 512;
    if (!ASTHRA_TEST_ASSERT_RANGE(context, test_size, (size_t)256, (size_t)1024, "Size should be in range [256, 1024]")) {
        ASTHRA_TEST_FAIL(context, "Size range assertion failed");
    }

    // Test double range
    double test_double = 3.14159;
    if (!ASTHRA_TEST_ASSERT_RANGE(context, test_double, 3.0, 4.0, "Double should be in range [3.0, 4.0]")) {
        ASTHRA_TEST_FAIL(context, "Double range assertion failed");
    }

    ASTHRA_TEST_PASS(context);
}

// Test atomic statistics functionality
ASTHRA_TEST_DEFINE(test_atomic_statistics, ASTHRA_TEST_SEVERITY_HIGH) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!ASTHRA_TEST_ASSERT(context, stats, "Statistics creation should succeed")) {
        ASTHRA_TEST_FAIL(context, "Failed to create test statistics");
    }

    // Test atomic increment operations
    asthra_test_increment_stat(&stats->tests_run, 5);
    asthra_test_increment_stat(&stats->tests_passed, 3);
    asthra_test_increment_stat(&stats->tests_failed, 2);

    // Test atomic read operations
    uint64_t tests_run = asthra_test_get_stat(&stats->tests_run);
    uint64_t tests_passed = asthra_test_get_stat(&stats->tests_passed);
    uint64_t tests_failed = asthra_test_get_stat(&stats->tests_failed);

    if (!asthra_test_assert_int_eq(context, (int)tests_run, 5, "Tests run should be 5")) {
        asthra_test_statistics_destroy(stats);
        ASTHRA_TEST_FAIL(context, "Atomic tests_run counter failed");
    }

    if (!asthra_test_assert_int_eq(context, (int)tests_passed, 3, "Tests passed should be 3")) {
        asthra_test_statistics_destroy(stats);
        ASTHRA_TEST_FAIL(context, "Atomic tests_passed counter failed");
    }

    if (!asthra_test_assert_int_eq(context, (int)tests_failed, 2, "Tests failed should be 2")) {
        asthra_test_statistics_destroy(stats);
        ASTHRA_TEST_FAIL(context, "Atomic tests_failed counter failed");
    }

    // Test compare-and-swap operation
    uint64_t expected = 0;
    uint64_t new_value = 100;
    bool cas_result = asthra_test_compare_and_swap_stat(&stats->max_duration_ns, &expected, new_value);
    if (!ASTHRA_TEST_ASSERT(context, cas_result, "Compare-and-swap should succeed")) {
        asthra_test_statistics_destroy(stats);
        ASTHRA_TEST_FAIL(context, "Atomic compare-and-swap failed");
    }

    uint64_t max_duration = asthra_test_get_stat(&stats->max_duration_ns);
    if (!asthra_test_assert_int_eq(context, (int)max_duration, 100, "Max duration should be 100")) {
        asthra_test_statistics_destroy(stats);
        ASTHRA_TEST_FAIL(context, "Compare-and-swap value incorrect");
    }

    asthra_test_statistics_destroy(stats);
    ASTHRA_TEST_PASS(context);
}

// Test C17 designated initializers in test configuration
ASTHRA_TEST_DEFINE(test_designated_initializers, ASTHRA_TEST_SEVERITY_MEDIUM) {
    // Test default configuration creation
    AsthraTestSuiteConfig default_config = asthra_test_suite_config_default();

    if (!ASTHRA_TEST_ASSERT(context, default_config.name, "Default config should have a name")) {
        ASTHRA_TEST_FAIL(context, "Default configuration name is NULL");
    }

    if (!ASTHRA_TEST_ASSERT_EQ(context, (bool)default_config.parallel_execution, (bool)false, "Default should not be parallel")) {
        ASTHRA_TEST_FAIL(context, "Default parallel execution setting incorrect");
    }

    if (!ASTHRA_TEST_ASSERT_EQ(context, (int)default_config.default_timeout_ns, (int)30000000000ULL, "Default timeout should be 30 seconds")) {
        ASTHRA_TEST_FAIL(context, "Default timeout setting incorrect");
    }

    // Test custom configuration creation with C17 designated initializers
    AsthraTestSuiteConfig custom_config = {
        .name = "Custom Test Suite",
        .description = "Test suite with custom settings",
        .parallel_execution = true,
        .stop_on_failure = true,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 60000000000ULL, // 60 seconds
        .max_parallel_tests = 8,
        .statistics = NULL
    };

    if (!ASTHRA_TEST_ASSERT_EQ(context, (bool)custom_config.parallel_execution, (bool)true, "Custom config should be parallel")) {
        ASTHRA_TEST_FAIL(context, "Custom parallel execution setting incorrect");
    }

    if (!ASTHRA_TEST_ASSERT_EQ(context, (bool)custom_config.stop_on_failure, (bool)true, "Custom config should stop on failure")) {
        ASTHRA_TEST_FAIL(context, "Custom stop on failure setting incorrect");
    }

    if (!asthra_test_assert_int_eq(context, (int)custom_config.max_parallel_tests, 8, "Custom config should allow 8 parallel tests")) {
        ASTHRA_TEST_FAIL(context, "Custom max parallel tests setting incorrect");
    }

    ASTHRA_TEST_PASS(context);
}

// Test runtime integration with new framework
ASTHRA_TEST_DEFINE(test_runtime_integration, ASTHRA_TEST_SEVERITY_HIGH) {
    // Test runtime initialization
    AsthraGCConfig gc_config = {
        .initial_heap_size = 1024 * 1024,
        .max_heap_size = 16 * 1024 * 1024,
        .gc_threshold = 0.8,
        .conservative_mode = true,
        .concurrent_gc = false
    };

    int result = asthra_runtime_init(&gc_config);
    if (!asthra_test_assert_int_eq(context, result, 0, "Runtime initialization should succeed")) {
        ASTHRA_TEST_FAIL(context, "Runtime initialization failed");
    }

    // Test memory allocation with new assertion framework
    void *test_ptr = asthra_ffi_alloc(1024, ASTHRA_TRANSFER_FULL);
    if (!ASTHRA_TEST_ASSERT(context, test_ptr, "Memory allocation should succeed")) {
        asthra_runtime_cleanup();
        ASTHRA_TEST_FAIL(context, "Memory allocation failed");
    }

    // Test string operations
    AsthraString str1 = asthra_string_new("Hello, ");
    AsthraString str2 = asthra_string_new("C17!");
    AsthraString concat_result = asthra_string_concat(str1, str2);

    if (!ASTHRA_TEST_ASSERT(context, concat_result.data, "String concatenation should succeed")) {
        asthra_string_free(str1);
        asthra_string_free(str2);
        asthra_runtime_cleanup();
        ASTHRA_TEST_FAIL(context, "String concatenation failed");
    }

    size_t expected_len = 9; // "Hello, C17!"
    size_t actual_len = asthra_string_len(concat_result);
    if (!ASTHRA_TEST_ASSERT_EQ(context, actual_len, expected_len, "Concatenated string length should be correct")) {
        asthra_string_free(str1);
        asthra_string_free(str2);
        asthra_string_free(concat_result);
        asthra_runtime_cleanup();
        ASTHRA_TEST_FAIL(context, "String length incorrect");
    }

    // Cleanup
    asthra_string_free(str1);
    asthra_string_free(str2);
    asthra_string_free(concat_result);
    asthra_runtime_cleanup();

    ASTHRA_TEST_PASS(context);
}

// Test that demonstrates skipping functionality
ASTHRA_TEST_DEFINE(test_skip_demonstration, ASTHRA_TEST_SEVERITY_LOW) {
    // This test demonstrates the skip functionality
    ASTHRA_TEST_SKIP(context, "This test is intentionally skipped to demonstrate skip functionality");
}

// Test that demonstrates failure handling
ASTHRA_TEST_DEFINE(test_failure_demonstration, ASTHRA_TEST_SEVERITY_LOW) {
    // This test demonstrates failure handling
    int expected = 42;
    int actual = 24;

    if (!ASTHRA_TEST_ASSERT_EQ(context, actual, expected, "This assertion is designed to fail")) {
        ASTHRA_TEST_FAIL(context, "Demonstration of test failure with detailed error message");
    }

    ASTHRA_TEST_PASS(context);
}

// =============================================================================
// TEST SUITE EXECUTION
// =============================================================================

int main(void) {
    printf("=== Asthra C17 Test Framework Demonstration ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Create test suite configuration with C17 designated initializers
    AsthraTestSuiteConfig config = {
        .name = "C17 Framework Demo Suite",
        .description = "Demonstration of C17 modernized test framework features",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 30000000000ULL,
        .max_parallel_tests = 1,
        .statistics = stats
    };

    // Define test array with metadata
    AsthraTestFunction tests[] = {
        test_generic_assertions,
        test_generic_equality,
        test_generic_ranges,
        test_atomic_statistics,
        test_designated_initializers,
        test_runtime_integration,
        test_skip_demonstration,
        test_failure_demonstration
    };

    AsthraTestMetadata metadata[] = {
        test_generic_assertions_metadata,
        test_generic_equality_metadata,
        test_generic_ranges_metadata,
        test_atomic_statistics_metadata,
        test_designated_initializers_metadata,
        test_runtime_integration_metadata,
        test_skip_demonstration_metadata,
        test_failure_demonstration_metadata
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    printf("Running %zu tests...\n\n", test_count);

    // Run the test suite
    AsthraTestResult suite_result = asthra_test_run_suite(tests, metadata, test_count, &config);

    // Print final statistics
    printf("\n");
    asthra_test_statistics_print(stats, false);

    // Cleanup
    asthra_test_statistics_destroy(stats);

    // Return appropriate exit code
    return (suite_result == ASTHRA_TEST_PASS) ? 0 : 1;
}
