/**
 * Asthra Programming Language v1.2 Comprehensive Test Suite - Test Runners
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runners and execution logic for the comprehensive v1.2 test suite.
 */

#include "test_comprehensive_runners.h"
#include "test_comprehensive_core.h"
#include "test_comprehensive_benchmark.h"
#include "test_comprehensive_security.h"
#include "test_comprehensive_reporting.h"
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// MAIN TEST RUNNERS
// =============================================================================

AsthraTestResult run_v1_2_comprehensive_test_suite(void) {
    printf("=== Asthra v1.2 Comprehensive Test Suite ===\n");

    // Initialize global statistics if not already done
    AsthraTestStatistics *stats = asthra_v12_get_global_stats();
    if (!stats) {
        stats = asthra_test_statistics_create();
        if (!stats) {
            printf("Failed to initialize test statistics\n");
            return ASTHRA_TEST_ERROR;
        }
        asthra_v12_set_global_stats(stats);
    }

    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    // Run all test categories
    printf("Running Grammar Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_GRAMMAR) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    printf("Running Pattern Matching Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_PATTERN_MATCHING) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    printf("Running String Operations Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_STRING_OPERATIONS) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    printf("Running Slice Management Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_SLICE_MANAGEMENT) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    printf("Running Enhanced FFI Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_ENHANCED_FFI) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    printf("Running Concurrency Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_CONCURRENCY) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    printf("Running Security Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_SECURITY) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    printf("Running Integration Tests...\n");
    if (run_v1_2_category_tests(ASTHRA_V1_2_CATEGORY_INTEGRATION) != ASTHRA_TEST_PASS) {
        overall_result = ASTHRA_TEST_FAIL;
    }

    // Print summary
    stats = asthra_v12_get_global_stats();
    uint64_t total_tests = asthra_test_get_stat(&stats->tests_run);
    uint64_t passed_tests = asthra_test_get_stat(&stats->tests_passed);
    uint64_t failed_tests = asthra_test_get_stat(&stats->tests_failed);

    printf("Total Tests: %llu\n", (unsigned long long)total_tests);
    printf("Passed: %llu\n", (unsigned long long)passed_tests);
    printf("Failed: %llu\n", (unsigned long long)failed_tests);

    // Generate comprehensive report
    asthra_v12_generate_test_report("asthra_v1_2_test_report.md");

    return overall_result;
}

AsthraTestResult run_v1_2_category_tests(AsthraV12TestCategory category) {
    AsthraTestResult result = ASTHRA_TEST_PASS;

    switch (category) {
        case ASTHRA_V1_2_CATEGORY_GRAMMAR: {
            // Run grammar tests
            AsthraV12TestMetadata metadata = {
                .base = {
                    .name = "Grammar Tests",
                    .file = __FILE__,
                    .line = __LINE__,
                    .function = __func__,
                    .severity = ASTHRA_TEST_SEVERITY_HIGH,
                    .timeout_ns = ASTHRA_V1_2_TEST_TIMEOUT_DEFAULT_NS,
                    .skip = false,
                    .skip_reason = NULL
                },
                .category = ASTHRA_V1_2_CATEGORY_GRAMMAR,
                .complexity = ASTHRA_V1_2_COMPLEXITY_BASIC,
                .mode = ASTHRA_V1_2_MODE_UNIT,
                .feature_description = "Grammar disambiguation and parsing validation",
                .ai_feedback_notes = "Critical for AI code generation accuracy",
                .requires_c17_compliance = true,
                .requires_security_validation = false,
                .requires_performance_benchmark = false,
                .expected_max_duration_ns = 1000000000ULL, // 1 second
                .memory_limit_bytes = 1024 * 1024 // 1MB
            };

            AsthraV12TestContext *ctx = asthra_test_context_create(&metadata);
            if (!ctx) return ASTHRA_TEST_ERROR;

            // Run individual grammar tests
            if (test_grammar_postfix_expr_disambiguation(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;
            if (test_grammar_precedence_rules(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;
            if (test_grammar_zero_parsing_conflicts(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;

            asthra_v12_test_context_destroy(ctx);
            break;
        }

        case ASTHRA_V1_2_CATEGORY_PATTERN_MATCHING: {
            // Run pattern matching tests
            AsthraV12TestMetadata metadata = {
                .base = {
                    .name = "Pattern Matching Tests",
                    .file = __FILE__,
                    .line = __LINE__,
                    .function = __func__,
                    .severity = ASTHRA_TEST_SEVERITY_HIGH,
                    .timeout_ns = ASTHRA_V1_2_TEST_TIMEOUT_DEFAULT_NS,
                    .skip = false,
                    .skip_reason = NULL
                },
                .category = ASTHRA_V1_2_CATEGORY_PATTERN_MATCHING,
                .complexity = ASTHRA_V1_2_COMPLEXITY_INTERMEDIATE,
                .mode = ASTHRA_V1_2_MODE_UNIT,
                .feature_description = "Result<T,E> pattern matching and error handling",
                .ai_feedback_notes = "Essential for robust error handling patterns",
                .requires_c17_compliance = true,
                .requires_security_validation = false,
                .requires_performance_benchmark = true,
                .expected_max_duration_ns = 2000000000ULL, // 2 seconds
                .memory_limit_bytes = 2 * 1024 * 1024 // 2MB
            };

            AsthraV12TestContext *ctx = asthra_test_context_create(&metadata);
            if (!ctx) return ASTHRA_TEST_ERROR;

            // Run pattern matching tests
            if (test_result_exhaustive_matching(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;
            if (test_pattern_matching_zero_cost(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;

            asthra_v12_test_context_destroy(ctx);
            break;
        }

        case ASTHRA_V1_2_CATEGORY_SECURITY: {
            // Run security tests
            AsthraV12TestMetadata metadata = {
                .base = {
                    .name = "Security Tests",
                    .file = __FILE__,
                    .line = __LINE__,
                    .function = __func__,
                    .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                    .timeout_ns = ASTHRA_V1_2_TEST_TIMEOUT_LONG_NS,
                    .skip = false,
                    .skip_reason = NULL
                },
                .category = ASTHRA_V1_2_CATEGORY_SECURITY,
                .complexity = ASTHRA_V1_2_COMPLEXITY_ADVANCED,
                .mode = ASTHRA_V1_2_MODE_SECURITY,
                .feature_description = "Constant-time operations and side-channel resistance",
                .ai_feedback_notes = "Critical for cryptographic and security-sensitive code",
                .requires_c17_compliance = true,
                .requires_security_validation = true,
                .requires_performance_benchmark = true,
                .expected_max_duration_ns = 10000000000ULL, // 10 seconds
                .memory_limit_bytes = 10 * 1024 * 1024 // 10MB
            };

            AsthraV12TestContext *ctx = asthra_test_context_create(&metadata);
            if (!ctx) return ASTHRA_TEST_ERROR;

            // Run security tests
            if (test_security_constant_time_verification(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;
            if (test_security_memory_zeroing(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;
            if (test_security_csprng_functionality(ctx) != ASTHRA_TEST_PASS) result = ASTHRA_TEST_FAIL;

            asthra_v12_test_context_destroy(ctx);
            break;
        }

        default:
            printf("Warning: Test category %d not implemented yet\n", category);
            break;
    }

    return result;
}

AsthraTestResult run_v1_2_performance_benchmarks(void) {
    printf("=== Performance Benchmarks ===\n");

    AsthraV12TestMetadata metadata = {
        .base = {
            .name = "Performance Benchmarks",
            .file = __FILE__,
            .line = __LINE__,
            .function = __func__,
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
            .timeout_ns = ASTHRA_V1_2_TEST_TIMEOUT_LONG_NS,
            .skip = false,
            .skip_reason = NULL
        },
        .category = ASTHRA_V1_2_CATEGORY_PERFORMANCE,
        .complexity = ASTHRA_V1_2_COMPLEXITY_STRESS,
        .mode = ASTHRA_V1_2_MODE_PERFORMANCE,
        .feature_description = "Comprehensive performance benchmarks for all v1.2 features",
        .ai_feedback_notes = "Performance validation for AI-generated optimizations",
        .requires_c17_compliance = true,
        .requires_security_validation = false,
        .requires_performance_benchmark = true,
        .expected_max_duration_ns = 30000000000ULL, // 30 seconds
        .memory_limit_bytes = 100 * 1024 * 1024 // 100MB
    };

    AsthraV12TestContext *ctx = asthra_test_context_create(&metadata);
    if (!ctx) return ASTHRA_TEST_ERROR;

    AsthraTestResult result = ASTHRA_TEST_PASS;

    // Run performance benchmarks
    printf("Running pattern matching benchmarks...\n");
    if (test_pattern_matching_benchmark(ctx) != ASTHRA_TEST_PASS) {
        result = ASTHRA_TEST_FAIL;
    }

    printf("Running optimization validation...\n");
    if (test_pattern_matching_optimization(ctx) != ASTHRA_TEST_PASS) {
        result = ASTHRA_TEST_FAIL;
    }

    // Print benchmark results
            asthra_benchmark_print_results(ctx);

    asthra_v12_test_context_destroy(ctx);
    return result;
}

AsthraTestResult run_v1_2_security_validation(void) {
    printf("=== Security Validation ===\n");

    AsthraV12TestMetadata metadata = {
        .base = {
            .name = "Security Validation",
            .file = __FILE__,
            .line = __LINE__,
            .function = __func__,
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = ASTHRA_V1_2_TEST_TIMEOUT_LONG_NS,
            .skip = false,
            .skip_reason = NULL
        },
        .category = ASTHRA_V1_2_CATEGORY_SECURITY,
        .complexity = ASTHRA_V1_2_COMPLEXITY_ADVANCED,
        .mode = ASTHRA_V1_2_MODE_SECURITY,
        .feature_description = "Comprehensive security validation for all v1.2 features",
        .ai_feedback_notes = "Security validation for AI-generated cryptographic code",
        .requires_c17_compliance = true,
        .requires_security_validation = true,
        .requires_performance_benchmark = false,
        .expected_max_duration_ns = 60000000000ULL, // 60 seconds
        .memory_limit_bytes = 50 * 1024 * 1024 // 50MB
    };

    AsthraV12TestContext *ctx = asthra_test_context_create(&metadata);
    if (!ctx) return ASTHRA_TEST_ERROR;

    AsthraTestResult result = ASTHRA_TEST_PASS;

    // Run comprehensive security tests
    printf("Running constant-time verification...\n");
    if (test_security_constant_time_verification(ctx) != ASTHRA_TEST_PASS) {
        result = ASTHRA_TEST_FAIL;
    }

    printf("Running side-channel resistance tests...\n");
    if (test_security_side_channel_resistance(ctx) != ASTHRA_TEST_PASS) {
        result = ASTHRA_TEST_FAIL;
    }

    printf("Running entropy quality tests...\n");
    if (test_security_entropy_testing(ctx) != ASTHRA_TEST_PASS) {
        result = ASTHRA_TEST_FAIL;
    }

    // Print security validation results
    asthra_v12_security_print_results(ctx);

    asthra_v12_test_context_destroy(ctx);
    return result;
} 
