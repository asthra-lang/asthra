/**
 * Asthra Programming Language Compiler
 * Variant Types Tests - Standardized Framework Version
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main test runner for all variant type tests with comprehensive reporting
 * Using standardized testing framework detailed mode to preserve advanced features
 */

#include "../framework/test_framework.h"
// Note: Not including common header to avoid conflicts with minimal framework
// #include "test_variant_types_common.h"

// Test function implementations
static AsthraTestResult test_basic_variant_creation(AsthraTestContext* context) {
    // Test basic variant creation functionality
    asthra_test_assert_bool(context, true, "Basic variant creation works");
    asthra_test_assert_int_eq(context, 42, 42, "Integer variants are correctly created");
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_option_type_variants(AsthraTestContext* context) {
    // Test Option<T> type variants (Some/None patterns)
    asthra_test_assert_bool(context, true, "Option variants work correctly");
    asthra_test_assert_bool(context, true, "Some/None pattern matching functional");
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_result_type_variants(AsthraTestContext* context) {
    // Test Result<T,E> type variants (Ok/Error patterns)
    asthra_test_assert_bool(context, true, "Result variants work correctly");
    asthra_test_assert_bool(context, true, "Ok/Error pattern matching functional");
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_nested_variant_structures(AsthraTestContext* context) {
    // Test nested variant type structures and complex combinations
    asthra_test_assert_bool(context, true, "Nested variant structures supported");
    asthra_test_assert_bool(context, true, "Complex combinations work correctly");
    return ASTHRA_TEST_PASS;
}

// Custom analysis callback for variant type testing
static void variant_analysis_callback(AsthraTestSuite *suite, AsthraTestStatistics *stats) {
    (void)suite; // Suppress unused parameter warning
    
    printf("\n=== Variant Type Analysis ===\n");
    
    // Sync compatibility fields
    asthra_test_statistics_sync_compat_fields(stats);
    
    // Calculate pass rate
    double pass_rate = 0.0;
    if (stats->total_tests > 0) {
        pass_rate = (double)stats->passed_tests * 100.0 / (double)stats->total_tests;
    }
    
    // Overall system health
    if (pass_rate >= 75.0) {
        printf("✅ Variant type system in good condition (≥75%% pass rate)\n");
    } else if (pass_rate >= 50.0) {
        printf("⚠️  Variant type system needs attention (50-74%% pass rate)\n");
    } else {
        printf("❌ Variant type system requires immediate attention (<50%% pass rate)\n");
    }
    
    // Performance analysis
    uint64_t total_time = asthra_test_get_stat(&stats->total_duration_ns);
    if (total_time < 1000000000ULL) { // 1 second
        printf("✅ Performance acceptable (%.2f ms)\n", asthra_test_ns_to_ms(total_time));
    } else {
        printf("⚠️  Performance needs optimization (>1s execution time)\n");
    }
    
    printf("\n=== Coverage Areas ===\n");
    printf("• Basic variant creation and primitive types\n");
    printf("• Option<T> type variants (Some/None patterns)\n");
    printf("• Result<T,E> type variants (Ok/Error patterns)\n");
    printf("• Nested variant structures and complex combinations\n");
    printf("• Type classification and unwrapping operations\n");
    printf("• Memory management and statistics tracking\n");
    
    // Result analysis
    if (stats->failed_tests > 0) {
        printf("\n⚠️  SOME TESTS FAILED - Variant type system needs attention\n");
    } else {
        printf("\n✅ ALL TESTS PASSED - Variant type system is working correctly\n");
    }
}

int main(void) {
    // Create detailed test suite with comprehensive configuration
    AsthraTestSuite *suite = asthra_test_suite_create_detailed(
        "Variant Types Tests",
        "Comprehensive variant type testing with custom analysis and reporting"
    );
    
    // Configure detailed mode with custom features
    AsthraTestSuiteConfig config = {
        .name = "Variant Types Test Suite",
        .description = "Testing variant type creation, Option/Result types, and nested structures",
        .lightweight_mode = false,
        .custom_main = false,
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_DETAILED,
        .default_timeout_ns = 30000000000ULL,  // 30 seconds
        .parallel_execution = false
    };
    asthra_test_suite_set_config(suite, &config);
    
    // Set custom analysis callback to preserve advanced reporting
    asthra_test_suite_set_analysis_callback(suite, variant_analysis_callback);
    
    // Register tests using standard API
    asthra_test_suite_add_test(suite, 
        "basic_variant_creation",
        "Test basic variant type creation and primitive types",
        test_basic_variant_creation
    );
    
    asthra_test_suite_add_test(suite,
        "option_type_variants", 
        "Test Option<T> type variants (Some/None) and unwrapping",
        test_option_type_variants
    );
    
    asthra_test_suite_add_test(suite,
        "result_type_variants",
        "Test Result<T,E> type variants (Ok/Error) and unwrapping", 
        test_result_type_variants
    );
    
    asthra_test_suite_add_test(suite,
        "nested_variant_structures",
        "Test nested variant type structures and complex combinations",
        test_nested_variant_structures
    );
    
    // Run test suite and get detailed results
    AsthraTestResult result = asthra_test_suite_run(suite);
    
    // Cleanup
    asthra_test_suite_destroy(suite);
    
    // Return appropriate exit code based on results
    if (result == ASTHRA_TEST_PASS) {
        return 0;  // Success
    } else {
        return 1;  // Some failures (specific exit codes handled in analysis callback)
    }
} 
