/**
 * Asthra Programming Language Core Tests - Main Test Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner for core language feature tests.
 * This file provides a main entry point for running all core tests.
 */

#include "test_comprehensive.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("=== Asthra Programming Language Core Tests ===\n\n");

    // Create test metadata for the main test suite
    AsthraV12TestMetadata metadata = {
        .base = {.name = "Core Test Suite",
                 .file = __FILE__,
                 .line = __LINE__,
                 .function = "main",
                 .severity = ASTHRA_TEST_SEVERITY_HIGH,
                 .timeout_ns = ASTHRA_V1_2_TEST_TIMEOUT_LONG_NS,
                 .skip = false,
                 .skip_reason = NULL},
        .category = ASTHRA_V1_2_CATEGORY_GRAMMAR,
        .complexity = ASTHRA_V1_2_COMPLEXITY_INTERMEDIATE,
        .mode = ASTHRA_V1_2_MODE_INTEGRATION,
        .feature_description = "Core language features test suite",
        .ai_feedback_notes = "Comprehensive validation of core language functionality",
        .requires_c17_compliance = true,
        .requires_security_validation = false,
        .requires_performance_benchmark = false,
        .expected_max_duration_ns = 30000000000ULL, // 30 seconds
        .memory_limit_bytes = 50 * 1024 * 1024      // 50MB
    };

    // Create test context
    AsthraV12TestContext *ctx = asthra_test_context_create(&metadata);
    if (!ctx) {
        fprintf(stderr, "Failed to create test context\n");
        return 1;
    }

    // Run the comprehensive test suite
    printf("Running comprehensive core test suite...\n");
    AsthraTestResult result = run_v1_2_comprehensive_test_suite();

    // Print final results
    if (result == ASTHRA_TEST_PASS) {
        printf("\n✅ All core tests passed!\n");
    } else {
        printf("\n❌ Some core tests failed!\n");
    }

    // Cleanup
    asthra_v12_test_context_destroy(ctx);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
