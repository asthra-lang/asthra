/**
 * Asthra Programming Language Compiler
 * Variant Types Tests - Main Orchestrator
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner for all variant type tests with comprehensive reporting
 */

#include "test_variant_types_common.h"

// Test function declarations
extern AsthraTestResult test_basic_variant_creation(AsthraTestContext *context);
extern AsthraTestResult test_option_type_variants(AsthraTestContext *context);
extern AsthraTestResult test_result_type_variants(AsthraTestContext *context);
extern AsthraTestResult test_nested_variant_structures(AsthraTestContext *context);

// Test metadata for all tests
typedef struct {
    const char *name;
    const char *description;
    AsthraTestResult (*test_func)(AsthraTestContext *);
    bool is_critical;
} VariantTestInfo;

static VariantTestInfo variant_tests[] = {
    {"Basic Variant Creation", "Test basic variant type creation and primitive types",
     test_basic_variant_creation, true},
    {"Option Type Variants", "Test Option<T> type variants (Some/None) and unwrapping",
     test_option_type_variants, true},
    {"Result Type Variants", "Test Result<T,E> type variants (Ok/Error) and unwrapping",
     test_result_type_variants, true},
    {"Nested Variant Structures", "Test nested variant type structures and complex combinations",
     test_nested_variant_structures, false}};

static const int NUM_VARIANT_TESTS = sizeof(variant_tests) / sizeof(variant_tests[0]);

// Test statistics
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    int critical_passed;
    int critical_failed;
    int optional_passed;
    int optional_failed;
    double pass_rate;
    double critical_pass_rate;
} VariantTestStats;

static void print_test_header(void) {
    printf("=== Asthra Variant Types Test Suite ===\n");
    printf("Testing variant type creation, Option/Result types, and nested structures\n");
    printf("Framework: Minimal Test Framework\n");
    printf("Total Tests: %d\n", NUM_VARIANT_TESTS);
    printf("\n");
}

static void run_variant_test(const VariantTestInfo *test_info, VariantTestStats *stats,
                             AsthraTestContext *context) {
    printf("Running: %s... ", test_info->name);
    fflush(stdout);

    AsthraTestResult result = test_info->test_func(context);

    if (result == ASTHRA_TEST_PASS) {
        printf("✅ PASS\n");
        stats->passed_tests++;
        if (test_info->is_critical) {
            stats->critical_passed++;
        } else {
            stats->optional_passed++;
        }
    } else {
        printf("❌ FAIL\n");
        stats->failed_tests++;
        if (test_info->is_critical) {
            stats->critical_failed++;
        } else {
            stats->optional_failed++;
        }
    }

    printf("  Description: %s\n", test_info->description);
    printf("  Priority: %s\n", test_info->is_critical ? "Critical" : "Optional");
    printf("\n");
}

static void calculate_statistics(VariantTestStats *stats) {
    stats->total_tests = stats->passed_tests + stats->failed_tests;
    stats->pass_rate =
        (stats->total_tests > 0) ? (stats->passed_tests * 100.0) / stats->total_tests : 0.0;

    int total_critical = stats->critical_passed + stats->critical_failed;
    stats->critical_pass_rate =
        (total_critical > 0) ? (stats->critical_passed * 100.0) / total_critical : 0.0;
}

static void print_test_summary(const VariantTestStats *stats) {
    printf("=== Test Summary ===\n");
    printf("Total Tests: %d\n", stats->total_tests);
    printf("Passed: %d\n", stats->passed_tests);
    printf("Failed: %d\n", stats->failed_tests);
    printf("Overall Pass Rate: %.1f%%\n", stats->pass_rate);
    printf("\n");

    printf("=== Critical Tests ===\n");
    printf("Critical Passed: %d\n", stats->critical_passed);
    printf("Critical Failed: %d\n", stats->critical_failed);
    printf("Critical Pass Rate: %.1f%%\n", stats->critical_pass_rate);
    printf("\n");

    printf("=== Optional Tests ===\n");
    printf("Optional Passed: %d\n", stats->optional_passed);
    printf("Optional Failed: %d\n", stats->optional_failed);
    printf("\n");
}

static void print_test_analysis(const VariantTestStats *stats) {
    printf("=== Analysis ===\n");

    if (stats->critical_pass_rate == 100.0) {
        printf("✅ All critical variant type functionality is working correctly\n");
    } else {
        printf("❌ Critical variant type functionality has issues\n");
    }

    if (stats->pass_rate >= 75.0) {
        printf("✅ Variant type system is in good condition (≥75%% pass rate)\n");
    } else if (stats->pass_rate >= 50.0) {
        printf("⚠️  Variant type system needs attention (50-74%% pass rate)\n");
    } else {
        printf("❌ Variant type system requires immediate attention (<50%% pass rate)\n");
    }

    printf("\n");
    printf("=== Coverage Areas ===\n");
    printf("• Basic variant creation and primitive types\n");
    printf("• Option<T> type variants (Some/None patterns)\n");
    printf("• Result<T,E> type variants (Ok/Error patterns)\n");
    printf("• Nested variant structures and complex combinations\n");
    printf("• Type classification and unwrapping operations\n");
    printf("• Memory management and statistics tracking\n");
    printf("\n");
}

// Main test runner
int main(void) {
    print_test_header();

    AsthraTestContext context = {0};
    VariantTestStats stats = {0};

    // Run all variant type tests
    for (int i = 0; i < NUM_VARIANT_TESTS; i++) {
        run_variant_test(&variant_tests[i], &stats, &context);
    }

    // Calculate and display results
    calculate_statistics(&stats);
    print_test_summary(&stats);
    print_test_analysis(&stats);

    // Return appropriate exit code
    if (stats.critical_failed > 0) {
        printf("❌ CRITICAL TESTS FAILED - Variant type system has serious issues\n");
        return 2; // Critical failure
    } else if (stats.failed_tests > 0) {
        printf("⚠️  SOME TESTS FAILED - Variant type system needs attention\n");
        return 1; // Some failures
    } else {
        printf("✅ ALL TESTS PASSED - Variant type system is working correctly\n");
        return 0; // Success
    }
}
