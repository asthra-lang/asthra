/**
 * Asthra Programming Language v1.2 Concurrency Tests - Spawn Main Runner (Standardized)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner for comprehensive spawn functionality testing using standardized framework
 * with adapter pattern to maintain compatibility with AsthraV12TestContext
 */

#include "../framework/test_framework.h"
#include "test_spawn_common.h"

// =============================================================================
// ADAPTER PATTERN FOR LEGACY FRAMEWORK COMPATIBILITY
// =============================================================================

// Adapter structure to bridge legacy and standard frameworks
typedef struct {
    AsthraV12TestContext legacy_ctx;
    AsthraTestSuite *standard_suite;
    SpawnTestStats stats; // Custom statistics tracking
} ConcurrencyTestAdapter;

// Global adapter instance
static ConcurrencyTestAdapter g_adapter;

// Test function declarations with legacy signatures
extern ConcurrencyTestResult test_concurrency_spawn_basic(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_spawn_deterministic(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_spawn_multiple(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_spawn_with_arguments(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_spawn_multiple_statements(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult
test_concurrency_spawn_with_handle_operations(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_await_expressions(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_deterministic_behavior(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_performance_scalability(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_error_cases(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_spawn_with_c_functions(AsthraV12TestContext *ctx);
extern ConcurrencyTestResult test_concurrency_c_threads_calling_asthra(AsthraV12TestContext *ctx);

// Custom statistics structure for spawn tests
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    double total_time_ms;
    int basic_tests_passed;
    int handle_tests_passed;
    int performance_tests_passed;
    int error_tests_passed;
    int integration_tests_passed;
} SpawnTestStats;

// =============================================================================
// ADAPTER FUNCTIONS FOR LEGACY TEST INTEGRATION
// =============================================================================

// Adapter function for basic spawn tests
ConcurrencyTestResult adapter_spawn_basic(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_spawn_basic(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.basic_tests_passed++;
    return result;
}

ConcurrencyTestResult adapter_spawn_deterministic(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_spawn_deterministic(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.basic_tests_passed++;
    return result;
}

ConcurrencyTestResult adapter_spawn_multiple(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_spawn_multiple(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.basic_tests_passed++;
    return result;
}

ConcurrencyTestResult adapter_spawn_with_arguments(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_spawn_with_arguments(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.basic_tests_passed++;
    return result;
}

ConcurrencyTestResult adapter_spawn_multiple_statements(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_spawn_multiple_statements(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.basic_tests_passed++;
    return result;
}

// Adapter functions for handle/await tests
ConcurrencyTestResult adapter_spawn_with_handle_operations(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result =
        test_concurrency_spawn_with_handle_operations(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.handle_tests_passed++;
    return result;
}

ConcurrencyTestResult adapter_await_expressions(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_await_expressions(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.handle_tests_passed++;
    return result;
}

// Adapter functions for performance tests
ConcurrencyTestResult adapter_deterministic_behavior(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_deterministic_behavior(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.performance_tests_passed++;
    return result;
}

ConcurrencyTestResult adapter_performance_scalability(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_performance_scalability(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.performance_tests_passed++;
    return result;
}

// Adapter functions for error handling tests
ConcurrencyTestResult adapter_error_cases(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_error_cases(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.error_tests_passed++;
    return result;
}

// Adapter functions for C integration tests
ConcurrencyTestResult adapter_spawn_with_c_functions(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_spawn_with_c_functions(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.integration_tests_passed++;
    return result;
}

ConcurrencyTestResult adapter_c_threads_calling_asthra(AsthraTestContext *context) {
    ConcurrencyTestAdapter *adapter = (ConcurrencyTestAdapter *)context->user_data;
    ConcurrencyTestResult result = test_concurrency_c_threads_calling_asthra(&adapter->legacy_ctx);
    if (result == CONCURRENCY_TEST_PASS)
        adapter->stats.integration_tests_passed++;
    return result;
}

// =============================================================================
// CUSTOM ANALYSIS CALLBACK FOR CONCURRENCY TESTING
// =============================================================================

void concurrency_analysis_callback(AsthraTestSuite *suite, AsthraTestStatistics *stats) {
    ConcurrencyTestAdapter *adapter = &g_adapter;

    printf("\n=== SPAWN FUNCTIONALITY TEST SUMMARY ===\n");
    printf("Total Tests: %zu\n", stats->total_tests);
    printf("Passed: %zu (%.1f%%)\n", stats->passed_tests, stats->pass_rate);
    printf("Failed: %zu (%.1f%%)\n", stats->failed_tests,
           stats->total_tests > 0 ? (100.0 - stats->pass_rate) : 0.0);
    printf("Total Execution Time: %.2f ms\n", (double)stats->total_time_ns / 1000000.0);

    printf("\n=== CATEGORY BREAKDOWN ===\n");
    printf("Basic Spawn Tests: %d passed\n", adapter->stats.basic_tests_passed);
    printf("Handle/Await Tests: %d passed\n", adapter->stats.handle_tests_passed);
    printf("Performance Tests: %d passed\n", adapter->stats.performance_tests_passed);
    printf("Error Handling Tests: %d passed\n", adapter->stats.error_tests_passed);
    printf("C Integration Tests: %d passed\n", adapter->stats.integration_tests_passed);

    printf("\n=== TIER 1 CONCURRENCY VALIDATION ===\n");
    if (adapter->stats.basic_tests_passed >= 4 && adapter->stats.handle_tests_passed >= 1) {
        printf("✓ Tier 1 spawn functionality VALIDATED\n");
        printf("✓ Deterministic behavior CONFIRMED\n");
        printf("✓ Handle operations WORKING\n");
    } else {
        printf("✗ Tier 1 validation INCOMPLETE\n");
    }

    if (adapter->stats.performance_tests_passed >= 1) {
        printf("✓ Performance scalability VERIFIED\n");
    }

    if (adapter->stats.error_tests_passed >= 1) {
        printf("✓ Error handling ROBUST\n");
    }

    if (adapter->stats.integration_tests_passed >= 2) {
        printf("✓ C integration FUNCTIONAL\n");
    }

    // Overall assessment
    if (stats->pass_rate == 100.0 && adapter->stats.basic_tests_passed >= 4) {
        printf("\n✅ SPAWN FUNCTIONALITY COMPLETE - All tests passed\n");
    } else if (adapter->stats.basic_tests_passed >= 4) {
        printf("\n⚠️  SPAWN FUNCTIONALITY MOSTLY WORKING - Some optional tests failed\n");
    } else {
        printf("\n❌ SPAWN FUNCTIONALITY CRITICAL ISSUES - Core tests failed\n");
    }
}

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    // Initialize adapter
    memset(&g_adapter, 0, sizeof(g_adapter));
    test_context_init(&g_adapter.legacy_ctx);

    // Create detailed test suite with adapter support
    g_adapter.standard_suite = asthra_test_suite_create_detailed(
        "Spawn Functionality Tests",
        "Comprehensive spawn functionality testing with legacy framework compatibility");

    // Configure for detailed mode with custom main and statistics
    AsthraTestSuiteConfig config = {
        .name = "Asthra v1.2 Spawn Functionality Test Suite",
        .description = "Testing Tier 1 concurrency features with legacy compatibility",
        .lightweight_mode = false,
        .custom_main = false,
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_DETAILED,
        .default_timeout_ns = 30000000000ULL, // 30 seconds
        .parallel_execution = false};
    asthra_test_suite_set_config(g_adapter.standard_suite, &config);

    // Set custom analysis callback
    asthra_test_suite_set_analysis_callback(g_adapter.standard_suite,
                                            concurrency_analysis_callback);

    // Register basic spawn functionality tests (critical)
    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "spawn_basic",
                                         "Basic spawn functionality", adapter_spawn_basic,
                                         &g_adapter);

    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "spawn_deterministic",
                                         "Deterministic spawn behavior",
                                         adapter_spawn_deterministic, &g_adapter);

    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "spawn_multiple",
                                         "Multiple spawn operations", adapter_spawn_multiple,
                                         &g_adapter);

    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "spawn_with_arguments",
                                         "Spawn with arguments", adapter_spawn_with_arguments,
                                         &g_adapter);

    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "spawn_multiple_statements",
                                         "Spawn with multiple statements",
                                         adapter_spawn_multiple_statements, &g_adapter);

    // Register handle/await tests (critical)
    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "spawn_with_handle_operations",
                                         "Handle operations and await",
                                         adapter_spawn_with_handle_operations, &g_adapter);

    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "await_expressions",
                                         "Await expression handling", adapter_await_expressions,
                                         &g_adapter);

    // Register performance tests (medium priority)
    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "deterministic_behavior",
                                         "Deterministic behavior validation",
                                         adapter_deterministic_behavior, &g_adapter);

    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "performance_scalability",
                                         "Performance scalability", adapter_performance_scalability,
                                         &g_adapter);

    // Register error handling tests (medium priority)
    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "error_cases",
                                         "Error handling cases", adapter_error_cases, &g_adapter);

    // Register C integration tests (medium priority)
    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "spawn_with_c_functions",
                                         "Spawn with C functions", adapter_spawn_with_c_functions,
                                         &g_adapter);

    asthra_test_suite_add_test_with_data(g_adapter.standard_suite, "c_threads_calling_asthra",
                                         "C threads calling Asthra",
                                         adapter_c_threads_calling_asthra, &g_adapter);

    // Run the test suite
    ConcurrencyTestResult result = asthra_test_suite_run(g_adapter.standard_suite);

    // Cleanup
    asthra_test_suite_destroy(g_adapter.standard_suite);
    test_context_cleanup(&g_adapter.legacy_ctx);

    printf("\nSpawn functionality test suite completed.\n");

    // Return appropriate exit code
    if (result == CONCURRENCY_TEST_PASS) {
        return 0; // Success
    } else {
        return 1; // Some failures
    }
}
