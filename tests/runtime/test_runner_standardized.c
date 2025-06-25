/**
 * Asthra Programming Language Runtime v1.2
 * Main Test Runner for Split Test Suite (Standardized)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This test runner coordinates execution of all split test modules
 * using standardized testing framework with detailed mode for advanced reporting
 */

#include "../framework/test_framework.h"
#include "test_common.h"

// =============================================================================
// RUNTIME TEST FUNCTION DECLARATIONS
// =============================================================================

// Include test function declarations
extern int test_runtime_initialization(void);
extern int test_runtime_custom_config(void);
extern int test_memory_zones(void);
extern int test_gc_operations(void);
extern int test_slice_operations(void);
extern int test_string_operations(void);
extern int test_result_pattern_matching(void);
extern int test_task_system(void);
extern int test_cryptographic_primitives(void);
extern int test_synchronization_primitives(void);
extern int test_error_handling(void);
extern int test_observability(void);

// =============================================================================
// ADAPTER FUNCTIONS FOR LEGACY TEST INTEGRATION
// =============================================================================

// Custom statistics tracking for category breakdown
typedef struct {
    int init_tests_passed;
    int memory_tests_passed;
    int slice_tests_passed;
    int string_tests_passed;
    int pattern_tests_passed;
    int task_tests_passed;
    int crypto_tests_passed;
    int sync_tests_passed;
    int error_tests_passed;
    int observability_tests_passed;
} RuntimeTestStats;

// Global statistics
static RuntimeTestStats g_runtime_stats = {0};

// Adapter functions to wrap legacy int-returning functions into AsthraTestResult
AsthraTestResult adapter_runtime_initialization(AsthraTestContext *context) {
    int result = test_runtime_initialization();
    if (result)
        g_runtime_stats.init_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_runtime_custom_config(AsthraTestContext *context) {
    int result = test_runtime_custom_config();
    if (result)
        g_runtime_stats.init_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_memory_zones(AsthraTestContext *context) {
    int result = test_memory_zones();
    if (result)
        g_runtime_stats.memory_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_gc_operations(AsthraTestContext *context) {
    int result = test_gc_operations();
    if (result)
        g_runtime_stats.memory_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_slice_operations(AsthraTestContext *context) {
    int result = test_slice_operations();
    if (result)
        g_runtime_stats.slice_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_string_operations(AsthraTestContext *context) {
    int result = test_string_operations();
    if (result)
        g_runtime_stats.string_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_result_pattern_matching(AsthraTestContext *context) {
    int result = test_result_pattern_matching();
    if (result)
        g_runtime_stats.pattern_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_task_system(AsthraTestContext *context) {
    int result = test_task_system();
    if (result)
        g_runtime_stats.task_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_cryptographic_primitives(AsthraTestContext *context) {
    int result = test_cryptographic_primitives();
    if (result)
        g_runtime_stats.crypto_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_synchronization_primitives(AsthraTestContext *context) {
    int result = test_synchronization_primitives();
    if (result)
        g_runtime_stats.sync_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_error_handling(AsthraTestContext *context) {
    int result = test_error_handling();
    if (result)
        g_runtime_stats.error_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult adapter_observability(AsthraTestContext *context) {
    int result = test_observability();
    if (result)
        g_runtime_stats.observability_tests_passed++;
    return result ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// =============================================================================
// CUSTOM ANALYSIS CALLBACK FOR RUNTIME TESTING
// =============================================================================

void runtime_analysis_callback(AsthraTestSuite *suite, AsthraTestStatistics *stats) {
    printf("\n====================================================\n");
    printf("=== Asthra Runtime v1.2 - Comprehensive Test Results ===\n");
    printf("====================================================\n");

    printf("\n=== OVERALL RESULTS ===\n");
    printf("Test Results: %zu/%zu passed (%.1f%%)\n", stats->passed_tests, stats->total_tests,
           stats->pass_rate);
    printf("Total Execution Time: %.2f ms\n", (double)stats->total_time_ns / 1000000.0);

    printf("\n=== CATEGORY BREAKDOWN ===\n");
    printf("[Init Tests]:          %d passed\n", g_runtime_stats.init_tests_passed);
    printf("[Memory Tests]:        %d passed\n", g_runtime_stats.memory_tests_passed);
    printf("[Slice Tests]:         %d passed\n", g_runtime_stats.slice_tests_passed);
    printf("[String Tests]:        %d passed\n", g_runtime_stats.string_tests_passed);
    printf("[Pattern Tests]:       %d passed\n", g_runtime_stats.pattern_tests_passed);
    printf("[Task Tests]:          %d passed\n", g_runtime_stats.task_tests_passed);
    printf("[Crypto Tests]:        %d passed\n", g_runtime_stats.crypto_tests_passed);
    printf("[Sync Tests]:          %d passed\n", g_runtime_stats.sync_tests_passed);
    printf("[Error Tests]:         %d passed\n", g_runtime_stats.error_tests_passed);
    printf("[Observability Tests]: %d passed\n", g_runtime_stats.observability_tests_passed);

    printf("\n=== SYSTEM HEALTH ASSESSMENT ===\n");

    // Core functionality assessment
    if (g_runtime_stats.init_tests_passed >= 2 && g_runtime_stats.memory_tests_passed >= 2) {
        printf("✅ Core runtime functionality OPERATIONAL\n");
    } else {
        printf("❌ Core runtime functionality CRITICAL ISSUES\n");
    }

    // Data structures assessment
    if (g_runtime_stats.slice_tests_passed >= 1 && g_runtime_stats.string_tests_passed >= 1) {
        printf("✅ Data structures FUNCTIONAL\n");
    } else {
        printf("⚠️  Data structures need attention\n");
    }

    // Advanced features assessment
    if (g_runtime_stats.task_tests_passed >= 1 && g_runtime_stats.crypto_tests_passed >= 1) {
        printf("✅ Advanced features WORKING\n");
    } else {
        printf("⚠️  Advanced features may have issues\n");
    }

    // Error handling and observability assessment
    if (g_runtime_stats.error_tests_passed >= 1 &&
        g_runtime_stats.observability_tests_passed >= 1) {
        printf("✅ Error handling and observability ROBUST\n");
    } else {
        printf("⚠️  Error handling and observability need improvement\n");
    }

    // Overall conclusion
    if (stats->pass_rate == 100.0) {
        printf("\n🎉 ALL TESTS PASSED! Runtime is working correctly.\n");
    } else if (stats->pass_rate >= 80.0) {
        printf("\n✅ RUNTIME MOSTLY FUNCTIONAL - Minor issues detected.\n");
    } else if (stats->pass_rate >= 60.0) {
        printf("\n⚠️  RUNTIME NEEDS ATTENTION - Significant issues detected.\n");
    } else {
        printf("\n❌ RUNTIME CRITICAL ISSUES - Major functionality broken.\n");
    }
}

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    // Create detailed test suite for advanced reporting
    AsthraTestSuite *suite = asthra_test_suite_create_detailed(
        "Runtime v1.2 Tests",
        "Comprehensive runtime functionality testing with category breakdown");

    // Configure detailed mode with comprehensive statistics
    AsthraTestSuiteConfig config = {
        .name = "Asthra Runtime v1.2 - Comprehensive Test Suite (Split)",
        .description = "Testing all runtime components with detailed category analysis",
        .lightweight_mode = false,
        .custom_main = false,
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_DETAILED,
        .default_timeout_ns = 30000000000ULL, // 30 seconds
        .parallel_execution = false};
    asthra_test_suite_set_config(suite, &config);

    // Set custom analysis callback
    asthra_test_suite_set_analysis_callback(suite, runtime_analysis_callback);

    // Register runtime initialization tests (critical)
    asthra_test_suite_add_test_with_metadata(
        suite, "runtime_initialization", "Runtime initialization functionality",
        adapter_runtime_initialization,
        &(AsthraTestMetadata){.severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                              .timeout_ns = 5000000000ULL, // 5 seconds
                              .skip = false});

    asthra_test_suite_add_test_with_metadata(
        suite, "runtime_custom_config", "Runtime custom configuration",
        adapter_runtime_custom_config,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL, .timeout_ns = 5000000000ULL, .skip = false});

    // Register memory management tests (critical)
    asthra_test_suite_add_test_with_metadata(
        suite, "memory_zones", "Memory zone management", adapter_memory_zones,
        &(AsthraTestMetadata){.severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                              .timeout_ns = 10000000000ULL, // 10 seconds
                              .skip = false});

    asthra_test_suite_add_test_with_metadata(
        suite, "gc_operations", "Garbage collection operations", adapter_gc_operations,
        &(AsthraTestMetadata){.severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                              .timeout_ns = 10000000000ULL,
                              .skip = false});

    // Register data structure tests (high priority)
    asthra_test_suite_add_test_with_metadata(
        suite, "slice_operations", "Slice operations and management", adapter_slice_operations,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_HIGH, .timeout_ns = 5000000000ULL, .skip = false});

    asthra_test_suite_add_test_with_metadata(
        suite, "string_operations", "String operations and management", adapter_string_operations,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_HIGH, .timeout_ns = 5000000000ULL, .skip = false});

    asthra_test_suite_add_test_with_metadata(
        suite, "result_pattern_matching", "Result type and pattern matching",
        adapter_result_pattern_matching,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_HIGH, .timeout_ns = 5000000000ULL, .skip = false});

    // Register advanced feature tests (medium priority)
    asthra_test_suite_add_test_with_metadata(
        suite, "task_system", "Task system functionality", adapter_task_system,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM, .timeout_ns = 10000000000ULL, .skip = false});

    asthra_test_suite_add_test_with_metadata(
        suite, "cryptographic_primitives", "Cryptographic primitives",
        adapter_cryptographic_primitives,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM, .timeout_ns = 5000000000ULL, .skip = false});

    asthra_test_suite_add_test_with_metadata(
        suite, "synchronization_primitives", "Synchronization primitives",
        adapter_synchronization_primitives,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM, .timeout_ns = 5000000000ULL, .skip = false});

    // Register error handling and observability tests (medium priority)
    asthra_test_suite_add_test_with_metadata(
        suite, "error_handling", "Error handling mechanisms", adapter_error_handling,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM, .timeout_ns = 5000000000ULL, .skip = false});

    asthra_test_suite_add_test_with_metadata(
        suite, "observability", "Observability and monitoring", adapter_observability,
        &(AsthraTestMetadata){
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM, .timeout_ns = 5000000000ULL, .skip = false});

    // Run test suite
    AsthraTestResult result = asthra_test_suite_run(suite);

    // Cleanup
    asthra_test_suite_destroy(suite);

    // Return appropriate exit code
    if (result == ASTHRA_TEST_PASS) {
        return 0; // Success
    } else {
        return 1; // Some failures
    }
}
