/**
 * Symbol Export Test Runner
 *
 * This file coordinates and runs all symbol export and visibility tests
 * across the split test files. It provides a unified entry point for
 * running the complete symbol export test suite.
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "symbol_export_test_common.h"

// Test suite function declarations
extern AsthraTestResult run_public_symbol_export_tests(void);
extern AsthraTestResult run_private_symbol_export_tests(void);
extern AsthraTestResult run_mixed_visibility_export_tests(void);

// =============================================================================
// MAIN TEST SUITE COORDINATION
// =============================================================================

static AsthraTestResult run_symbol_export_test_suite(void) {
    printf("Running complete symbol export and visibility test suite...\n");
    printf("================================================================\n\n");

    AsthraTestResult overall_result = ASTHRA_TEST_PASS;
    int total_suites = 0;
    int passed_suites = 0;

    // Test suite definitions
    struct {
        const char *name;
        AsthraTestResult (*test_func)(void);
    } test_suites[] = {
        {"Public Symbol Export Tests", run_public_symbol_export_tests},
        {"Private Symbol Export Tests", run_private_symbol_export_tests},
        {"Mixed Visibility Export Tests", run_mixed_visibility_export_tests},
    };

    size_t suite_count = sizeof(test_suites) / sizeof(test_suites[0]);
    total_suites = (int)suite_count;

    // Run each test suite
    for (size_t i = 0; i < suite_count; i++) {
        printf("\n--- %s ---\n", test_suites[i].name);

        AsthraTestResult result = test_suites[i].test_func();

        if (result == ASTHRA_TEST_PASS) {
            printf("✅ %s: ALL TESTS PASSED\n", test_suites[i].name);
            passed_suites++;
        } else {
            printf("❌ %s: SOME TESTS FAILED\n", test_suites[i].name);
            overall_result = ASTHRA_TEST_FAIL;
        }
    }

    // Print overall summary
    printf("\n================================================================\n");
    printf("=== Symbol Export Test Suite Summary ===\n");
    printf("Total test suites: %d\n", total_suites);
    printf("Passed test suites: %d\n", passed_suites);
    printf("Failed test suites: %d\n", total_suites - passed_suites);

    if (overall_result == ASTHRA_TEST_PASS) {
        printf("🎉 ALL SYMBOL EXPORT TESTS PASSED!\n");
    } else {
        printf("💥 SOME SYMBOL EXPORT TESTS FAILED!\n");
    }
    printf("================================================================\n");

    return overall_result;
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main(void) {
    printf("Asthra Symbol Export and Visibility Tests\n");
    printf("==========================================\n");
    printf("Split Test Suite Version\n\n");

    AsthraTestResult result = run_symbol_export_test_suite();

    printf("\n");
    if (result == ASTHRA_TEST_PASS) {
        printf("✅ All symbol export tests passed!\n");
        return 0;
    } else {
        printf("❌ Some symbol export tests failed!\n");
        return 1;
    }
}
