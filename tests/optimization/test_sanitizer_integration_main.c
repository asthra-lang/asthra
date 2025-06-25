/**
 * Asthra Programming Language
 * Sanitizer Integration Test Suite - Main Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file orchestrates all sanitizer integration tests.
 */

#include "test_sanitizer_common.h"

int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return EXIT_FAILURE;
    }

    // Configure test suite
    AsthraTestSuiteConfig config = asthra_test_suite_config_default();
    config.name = "Sanitizer Integration Test Suite";
    config.description = "Tests sanitizer detection capabilities with controlled test cases";
    config.verbose_output = true;
    config.stop_on_failure = false;
    config.default_timeout_ns = 60 * 1000000000ULL; // 60 seconds
    config.statistics = stats;

    printf("=============================================================================\n");
    printf("Asthra Sanitizer Integration Test Suite\n");
    printf("Testing: ASan, UBSan, TSan, MSan detection capabilities\n");
    printf("=============================================================================\n\n");

    // Define test cases
    AsthraTestFunction tests[] = {
        // AddressSanitizer Tests
        test_asan_buffer_overflow_detection, test_asan_use_after_free_detection,
        test_asan_memory_leak_detection, test_asan_double_free_detection,

        // UndefinedBehaviorSanitizer Tests
        test_ubsan_integer_overflow_detection, test_ubsan_null_pointer_dereference_detection,
        test_ubsan_array_bounds_detection, test_ubsan_division_by_zero_detection,

        // ThreadSanitizer Tests
        test_tsan_race_condition_detection, test_tsan_data_race_infrastructure,

        // MemorySanitizer Tests
        test_msan_uninitialized_memory_detection, test_msan_uninitialized_variable_detection,

        // Integration Tests
        test_sanitizer_environment_setup, test_sanitizer_symbol_availability};

    AsthraTestMetadata metadata[] = {
        // AddressSanitizer Tests
        {"ASan Buffer Overflow Detection", __FILE__, __LINE__,
         "test_asan_buffer_overflow_detection", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"ASan Use-After-Free Detection", __FILE__, __LINE__, "test_asan_use_after_free_detection",
         ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"ASan Memory Leak Detection", __FILE__, __LINE__, "test_asan_memory_leak_detection",
         ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {"ASan Double-Free Detection", __FILE__, __LINE__, "test_asan_double_free_detection",
         ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},

        // UndefinedBehaviorSanitizer Tests
        {"UBSan Integer Overflow Detection", __FILE__, __LINE__,
         "test_ubsan_integer_overflow_detection", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"UBSan Null Pointer Dereference Detection", __FILE__, __LINE__,
         "test_ubsan_null_pointer_dereference_detection", ASTHRA_TEST_SEVERITY_HIGH, 0, false,
         NULL},
        {"UBSan Array Bounds Detection", __FILE__, __LINE__, "test_ubsan_array_bounds_detection",
         ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"UBSan Division by Zero Detection", __FILE__, __LINE__,
         "test_ubsan_division_by_zero_detection", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},

        // ThreadSanitizer Tests
        {"TSan Race Condition Detection", __FILE__, __LINE__, "test_tsan_race_condition_detection",
         ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {"TSan Data Race Infrastructure", __FILE__, __LINE__, "test_tsan_data_race_infrastructure",
         ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},

        // MemorySanitizer Tests
        {"MSan Uninitialized Memory Detection", __FILE__, __LINE__,
         "test_msan_uninitialized_memory_detection", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {"MSan Uninitialized Variable Detection", __FILE__, __LINE__,
         "test_msan_uninitialized_variable_detection", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},

        // Integration Tests
        {"Sanitizer Environment Setup", __FILE__, __LINE__, "test_sanitizer_environment_setup",
         ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL},
        {"Sanitizer Symbol Availability", __FILE__, __LINE__, "test_sanitizer_symbol_availability",
         ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    // Run the test suite
    AsthraTestResult suite_result = asthra_test_run_suite(tests, metadata, test_count, &config);

    // Print final results
    printf("\n=============================================================================\n");
    printf("Sanitizer Integration Test Results\n");
    printf("=============================================================================\n");
    asthra_test_statistics_print(stats, false);

    // Cleanup
    asthra_test_statistics_destroy(stats);

    return (suite_result == ASTHRA_TEST_PASS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
