/**
 * Asthra Programming Language v1.2 Test Execution Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of test execution logic and output formatting
 */

#include "test_execution.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// =============================================================================
// OUTPUT FUNCTIONS
// =============================================================================

void print_test_header(void) {
    printf("================================================================================\n");
    printf("                    Asthra Programming Language v1.2\n");
    printf("                      Comprehensive Test Suite\n");
    printf("================================================================================\n");
    printf("Testing all v1.2 features with focus on AI code generation feedback\n");
    printf("Coverage: Grammar, Pattern Matching, String Ops, Slice Management,\n");
    printf("          Enhanced FFI, Memory Safety, Concurrency, Security, Integration\n");
    printf("================================================================================\n\n");
}

void print_category_header(const char *category) {
    printf("\n--- %s Tests ---\n", category);
}

void print_test_result(const char *test_name, AsthraTestResult result, double duration_ms) {
    const char *status_str;
    const char *color_code = "";
    
    switch (result) {
        case ASTHRA_TEST_PASS:
            status_str = "PASS";
            color_code = "\033[32m"; // Green
            break;
        case ASTHRA_TEST_FAIL:
            status_str = "FAIL";
            color_code = "\033[31m"; // Red
            break;
        case ASTHRA_TEST_ERROR:
            status_str = "ERROR";
            color_code = "\033[35m"; // Magenta
            break;
        case ASTHRA_TEST_SKIP:
            status_str = "SKIP";
            color_code = "\033[33m"; // Yellow
            break;
        case ASTHRA_TEST_TIMEOUT:
            status_str = "TIMEOUT";
            color_code = "\033[91m"; // Bright Red
            break;
        case ASTHRA_TEST_RESULT_COUNT:
            // This should never be used as a result
            status_str = "UNKNOWN";
            color_code = "\033[37m"; // White
            break;
    }
    
    printf("  %-40s [%s%s\033[0m] (%.2f ms)\n", test_name, color_code, status_str, duration_ms);
}

void print_test_summary(const TestSummary *summary) {
    printf("\n================================================================================\n");
    printf("                              TEST SUMMARY\n");
    printf("================================================================================\n");
    
    printf("Total Tests:       %d\n", summary->total_tests);
    printf("Passed:           %d (%.1f%%)\n", summary->passed_tests, 
           (double)summary->passed_tests / summary->total_tests * 100.0);
    printf("Failed:           %d (%.1f%%)\n", summary->failed_tests,
           (double)summary->failed_tests / summary->total_tests * 100.0);
    printf("Errors:           %d (%.1f%%)\n", summary->error_tests,
           (double)summary->error_tests / summary->total_tests * 100.0);
    printf("Skipped:          %d (%.1f%%)\n", summary->skipped_tests,
           (double)summary->skipped_tests / summary->total_tests * 100.0);
    
    printf("\nSpecialized Tests:\n");
    printf("Performance:      %d\n", summary->performance_tests);
    printf("Security:         %d\n", summary->security_tests);
    
    printf("\nExecution Time:   %.2f seconds\n", summary->total_duration_seconds);
    if (summary->total_throughput > 0) {
        printf("Avg Throughput:   %.0f ops/sec\n", summary->total_throughput);
    }
    
    if (summary->failure_count > 0) {
        printf("\n--- FAILURES ---\n");
        for (int i = 0; i < summary->failure_count; i++) {
            printf("  %s\n", summary->failure_messages[i]);
        }
    }
    
    printf("\n================================================================================\n");
    
    if (summary->failed_tests == 0 && summary->error_tests == 0) {
        printf("\033[32m✓ ALL TESTS PASSED - Asthra v1.2 features are working correctly!\033[0m\n");
    } else {
        printf("\033[31m✗ SOME TESTS FAILED - Review failures and fix issues\033[0m\n");
    }
    
    printf("================================================================================\n");
}

// =============================================================================
// TEST EXECUTION LOGIC
// =============================================================================

AsthraTestResult execute_single_test(const TestRegistryEntry *entry, TestSummary *summary) {
    if (!entry || !summary) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Create test context
    AsthraV12TestMetadata metadata = {0};
    AsthraV12TestContext *ctx = asthra_test_context_create(&metadata);
    
    // Record test metadata
    asthra_v12_record_ai_feedback(ctx, "test_name", entry->name);
    asthra_v12_record_ai_feedback(ctx, "category", entry->category);
    asthra_v12_record_ai_feedback(ctx, "is_performance", entry->is_performance_test ? "true" : "false");
    asthra_v12_record_ai_feedback(ctx, "is_security", entry->is_security_test ? "true" : "false");
    
    // Execute test with timing
    struct timespec test_start, test_end;
    clock_gettime(CLOCK_MONOTONIC, &test_start);
    
    AsthraTestResult result = entry->test_func(ctx);
    
    clock_gettime(CLOCK_MONOTONIC, &test_end);
    
    // Calculate test duration
    double test_duration_ms = (test_end.tv_sec - test_start.tv_sec) * 1000.0 +
                             (test_end.tv_nsec - test_start.tv_nsec) / 1000000.0;
    
    // Update summary
    summary->total_tests++;
    
    switch (result) {
        case ASTHRA_TEST_PASS:
            summary->passed_tests++;
            break;
        case ASTHRA_TEST_FAIL:
            summary->failed_tests++;
            test_summary_add_failure(summary, entry->name, "Test assertions failed");
            break;
        case ASTHRA_TEST_ERROR:
            summary->error_tests++;
            test_summary_add_failure(summary, entry->name, "Test execution error");
            break;
        case ASTHRA_TEST_SKIP:
            summary->skipped_tests++;
            break;
        case ASTHRA_TEST_TIMEOUT:
            summary->error_tests++;
            test_summary_add_failure(summary, entry->name, "Test execution timeout");
            break;
        case ASTHRA_TEST_RESULT_COUNT:
            // This should never be used as a result
            summary->error_tests++;
            test_summary_add_failure(summary, entry->name, "Invalid test result");
            break;
    }
    
    if (entry->is_performance_test) {
        summary->performance_tests++;
        if (ctx->benchmark.throughput_ops_per_sec > 0) {
            summary->total_throughput += ctx->benchmark.throughput_ops_per_sec;
        }
    }
    
    if (entry->is_security_test) {
        summary->security_tests++;
    }
    
    // Print result
    print_test_result(entry->name, result, test_duration_ms);
    
    // Cleanup test context
    asthra_v12_test_context_destroy(ctx);
    
    return result;
}

bool should_skip_test(const TestRegistryEntry *entry, 
                     bool run_performance_tests, 
                     bool run_security_tests, 
                     const char *filter_category) {
    if (!entry) {
        return true;
    }
    
    // Apply category filter
    if (filter_category && strcmp(entry->category, filter_category) != 0) {
        return true;
    }
    
    // Apply performance test filter
    if (!run_performance_tests && entry->is_performance_test) {
        return true;
    }
    
    // Apply security test filter
    if (!run_security_tests && entry->is_security_test) {
        return true;
    }
    
    return false;
} 
