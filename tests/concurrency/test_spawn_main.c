/**
 * Asthra Programming Language v1.2 Concurrency Tests - Spawn Main Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner for comprehensive spawn functionality testing
 * orchestrating all test modules with performance analysis and reporting.
 */

#include "test_spawn_common.h"

// =============================================================================
// MAIN TEST RUNNER AND ORCHESTRATION
// =============================================================================

// Test statistics tracking
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

// Initialize test statistics
static void init_spawn_test_stats(SpawnTestStats *stats) {
    memset(stats, 0, sizeof(SpawnTestStats));
}

// Update test statistics
static void update_spawn_test_stats(SpawnTestStats *stats, ConcurrencyTestResult result,
                                    const char *category) {
    stats->total_tests++;
    if (result == CONCURRENCY_TEST_PASS) {
        stats->passed_tests++;
        if (strcmp(category, "basic") == 0)
            stats->basic_tests_passed++;
        else if (strcmp(category, "handle") == 0)
            stats->handle_tests_passed++;
        else if (strcmp(category, "performance") == 0)
            stats->performance_tests_passed++;
        else if (strcmp(category, "error") == 0)
            stats->error_tests_passed++;
        else if (strcmp(category, "integration") == 0)
            stats->integration_tests_passed++;
    } else {
        stats->failed_tests++;
    }
}

// Print test statistics
static void print_spawn_test_stats(SpawnTestStats *stats) {
    printf("\n=== SPAWN FUNCTIONALITY TEST SUMMARY ===\n");
    printf("Total Tests: %d\n", stats->total_tests);
    printf("Passed: %d (%.1f%%)\n", stats->passed_tests,
           (stats->total_tests > 0) ? (100.0 * stats->passed_tests / stats->total_tests) : 0.0);
    printf("Failed: %d (%.1f%%)\n", stats->failed_tests,
           (stats->total_tests > 0) ? (100.0 * stats->failed_tests / stats->total_tests) : 0.0);
    printf("Total Execution Time: %.2f ms\n", stats->total_time_ms);

    printf("\n=== CATEGORY BREAKDOWN ===\n");
    printf("Basic Spawn Tests: %d passed\n", stats->basic_tests_passed);
    printf("Handle/Await Tests: %d passed\n", stats->handle_tests_passed);
    printf("Performance Tests: %d passed\n", stats->performance_tests_passed);
    printf("Error Handling Tests: %d passed\n", stats->error_tests_passed);
    printf("C Integration Tests: %d passed\n", stats->integration_tests_passed);

    printf("\n=== TIER 1 CONCURRENCY VALIDATION ===\n");
    if (stats->basic_tests_passed >= 4 && stats->handle_tests_passed >= 1) {
        printf("✓ Tier 1 spawn functionality VALIDATED\n");
        printf("✓ Deterministic behavior CONFIRMED\n");
        printf("✓ Handle operations WORKING\n");
    } else {
        printf("✗ Tier 1 validation INCOMPLETE\n");
    }

    if (stats->performance_tests_passed >= 1) {
        printf("✓ Performance scalability VERIFIED\n");
    }

    if (stats->error_tests_passed >= 1) {
        printf("✓ Error handling ROBUST\n");
    }

    if (stats->integration_tests_passed >= 2) {
        printf("✓ C integration FUNCTIONAL\n");
    }
}

// Main test execution function
int main(void) {
    printf("Asthra v1.2 Spawn Functionality Test Suite\n");
    printf("=========================================\n\n");

    AsthraV12TestContext ctx;
    test_context_init(&ctx);

    SpawnTestStats stats;
    init_spawn_test_stats(&stats);

    struct timespec suite_start, suite_end;
    clock_gettime(CLOCK_MONOTONIC, &suite_start);

    // =================================================================
    // BASIC SPAWN FUNCTIONALITY TESTS
    // =================================================================
    printf("Running Basic Spawn Functionality Tests...\n");

    ConcurrencyTestResult result;

    printf("  test_concurrency_spawn_basic... ");
    result = test_concurrency_spawn_basic(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "basic");

    printf("  test_concurrency_spawn_deterministic... ");
    result = test_concurrency_spawn_deterministic(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "basic");

    printf("  test_concurrency_spawn_multiple... ");
    result = test_concurrency_spawn_multiple(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "basic");

    printf("  test_concurrency_spawn_with_arguments... ");
    result = test_concurrency_spawn_with_arguments(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "basic");

    printf("  test_concurrency_spawn_multiple_statements... ");
    result = test_concurrency_spawn_multiple_statements(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "basic");

    // =================================================================
    // HANDLE OPERATIONS AND AWAIT TESTS
    // =================================================================
    printf("\nRunning Handle Operations and Await Tests...\n");

    printf("  test_concurrency_spawn_with_handle_operations... ");
    result = test_concurrency_spawn_with_handle_operations(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "handle");

    printf("  test_concurrency_await_expressions... ");
    result = test_concurrency_await_expressions(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "handle");

    // =================================================================
    // PERFORMANCE AND BEHAVIOR TESTS
    // =================================================================
    printf("\nRunning Performance and Behavior Tests...\n");

    printf("  test_concurrency_deterministic_behavior... ");
    result = test_concurrency_deterministic_behavior(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "performance");

    printf("  test_concurrency_performance_scalability... ");
    result = test_concurrency_performance_scalability(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "performance");

    // =================================================================
    // ERROR HANDLING TESTS
    // =================================================================
    printf("\nRunning Error Handling Tests...\n");

    printf("  test_concurrency_error_cases... ");
    result = test_concurrency_error_cases(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "error");

    // =================================================================
    // C INTEGRATION TESTS
    // =================================================================
    printf("\nRunning C Integration Tests...\n");

    printf("  test_concurrency_spawn_with_c_functions... ");
    result = test_concurrency_spawn_with_c_functions(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "integration");

    printf("  test_concurrency_c_threads_calling_asthra... ");
    result = test_concurrency_c_threads_calling_asthra(&ctx);
    printf("%s\n", (result == CONCURRENCY_TEST_PASS) ? "PASS" : "FAIL");
    update_spawn_test_stats(&stats, result, "integration");

    // =================================================================
    // FINAL STATISTICS AND CLEANUP
    // =================================================================
    clock_gettime(CLOCK_MONOTONIC, &suite_end);
    stats.total_time_ms = timespec_to_ms(&suite_start, &suite_end);

    print_spawn_test_stats(&stats);

    test_context_cleanup(&ctx);

    printf("\nSpawn functionality test suite completed.\n");
    return (stats.failed_tests == 0) ? 0 : 1;
}
