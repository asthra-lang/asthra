/**
 * Asthra Programming Language
 * Core Test Runner Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Provides core test execution framework functions
 * without any FFI stubs that might conflict with compiler libraries.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// GLOBAL TEST COUNTERS (PROVIDES MISSING SYMBOLS)
// =============================================================================

size_t tests_run = 0;
size_t tests_passed = 0;
size_t tests_failed = 0;

// =============================================================================
// RUNTIME INITIALIZATION AND CLEANUP
// =============================================================================

void test_runtime_init(void) {
    // Reset test counters
    tests_run = 0;
    tests_passed = 0;
    tests_failed = 0;

    printf("Test runtime initialized\n");
}

void test_runtime_cleanup(void) {
    printf("Test runtime cleanup complete\n");
}

// =============================================================================
// TEST RESULT REPORTING
// =============================================================================

void print_test_results(void) {
    printf("\n==================================================\n");
    printf("Test Results\n");
    printf("==================================================\n");
    printf("Tests run: %zu\n", tests_run);
    printf("Tests passed: %zu\n", tests_passed);

    if (tests_failed > 0) {
        printf("Tests failed: %zu\n", tests_failed);
        printf("Success rate: %.1f%%\n",
               tests_run > 0 ? (100.0 * (double)tests_passed / (double)tests_run) : 0.0);
        printf("\n❌ Some tests failed\n");
    } else {
        printf("Success rate: %.1f%%\n",
               tests_run > 0 ? (100.0 * (double)tests_passed / (double)tests_run) : 0.0);
        printf("\n🎉 All tests passed!\n");
    }
}

// =============================================================================
// UNIFIED TEST SUITE RUNNER
// =============================================================================

int run_test_suite(const char *suite_name, void (*tests[])(void), size_t test_count) {
    printf("\nRunning %s Test Suite\n", suite_name);
    printf("=====================================\n");

    size_t initial_failed = tests_failed;

    for (size_t i = 0; i < test_count; i++) {
        if (tests[i]) {
            printf("\n--- Running test %zu ---\n", i + 1);
            tests[i]();
        }
    }

    printf("\n%s Results:\n", suite_name);
    printf("  Tests in this suite: %zu\n", test_count);
    printf("  Failed in this suite: %zu\n", tests_failed - initial_failed);

    return (int)(tests_failed - initial_failed);
}