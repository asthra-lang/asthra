/**
 * Asthra Programming Language v1.2 Integration Tests - Main Entry Point
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main integration test file that orchestrates all integration test scenarios.
 * This file includes all the modular integration test files and provides
 * a central entry point for running comprehensive integration tests.
 */

#include "test_integration_common.h"

// Include all integration test modules
// Note: The actual test functions are defined in their respective files
// and linked together during compilation

// =============================================================================
// INTEGRATION TEST SUITE RUNNER
// =============================================================================

/**
 * Run all integration tests in sequence
 */
AsthraTestResult run_all_integration_tests(AsthraV12TestContext *ctx) {
    AsthraTestResult result;
    
    // Test 1: Concurrent Text Processing with FFI and Error Handling
    printf("Running integration test: Concurrent Text Processing...\n");
    result = test_integration_concurrent_text_processing(ctx);
    if (result != ASTHRA_TEST_PASS) {
        printf("FAILED: Concurrent Text Processing\n");
        return result;
    }
    printf("PASSED: Concurrent Text Processing\n");
    
    // Test 2: Concurrent Slice Management with Memory Safety
    printf("Running integration test: Concurrent Slice Management...\n");
    result = test_integration_concurrent_slice_management(ctx);
    if (result != ASTHRA_TEST_PASS) {
        printf("FAILED: Concurrent Slice Management\n");
        return result;
    }
    printf("PASSED: Concurrent Slice Management\n");
    
    // Test 3: Pattern Matching with Result Types and FFI
    printf("Running integration test: Pattern Matching with FFI...\n");
    result = test_integration_pattern_matching_with_ffi(ctx);
    if (result != ASTHRA_TEST_PASS) {
        printf("FAILED: Pattern Matching with FFI\n");
        return result;
    }
    printf("PASSED: Pattern Matching with FFI\n");
    
    // Test 4: Security Features with Concurrent Access
    printf("Running integration test: Security with Concurrency...\n");
    result = test_integration_security_with_concurrency(ctx);
    if (result != ASTHRA_TEST_PASS) {
        printf("FAILED: Security with Concurrency\n");
        return result;
    }
    printf("PASSED: Security with Concurrency\n");
    
    // Test 5: Comprehensive Performance Integration Test
    printf("Running integration test: Performance Comprehensive...\n");
    result = test_integration_performance_comprehensive(ctx);
    if (result != ASTHRA_TEST_PASS) {
        printf("FAILED: Performance Comprehensive\n");
        return result;
    }
    printf("PASSED: Performance Comprehensive\n");
    
    printf("All integration tests passed successfully!\n");
    return ASTHRA_TEST_PASS;
}

/**
 * Integration test suite summary and reporting
 */
void print_integration_test_summary(AsthraV12TestContext *ctx) {
    printf("\n=== Integration Test Summary ===\n");
    printf("Test Framework: Asthra v1.2 Integration Tests\n");
    printf("Features Tested:\n");
    printf("  - Concurrent Text Processing with FFI\n");
    printf("  - Slice Management with Memory Safety\n");
    printf("  - Pattern Matching with Result Types\n");
    printf("  - Security Features with Concurrency\n");
    printf("  - Comprehensive Performance Testing\n");
    
    if (ctx->benchmark.throughput_ops_per_sec > 0) {
        printf("\nPerformance Metrics:\n");
        printf("  - Throughput: %.0f ops/sec\n", ctx->benchmark.throughput_ops_per_sec);
        printf("  - Duration: %.2f ms\n", (double)ctx->benchmark.min_duration_ns / 1e6);
    }
    
    printf("================================\n\n");
}

/**
 * Main integration test entry point
 */
int main(int argc, char *argv[]) {
    (void)argc; // Suppress unused parameter warning
    (void)argv; // Suppress unused parameter warning
    
    printf("Asthra Programming Language v1.2 Integration Tests\n");
    printf("==================================================\n\n");
    
    // Initialize test context
    AsthraV12TestContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    // Initialize base context
    ctx.base.test_name = "Integration Tests";
    ctx.base.passed = 0;
    ctx.base.failed = 0;
    ctx.base.assertions = 0;
    
    // Run all integration tests
    AsthraTestResult result = run_all_integration_tests(&ctx);
    
    // Print summary
    print_integration_test_summary(&ctx);
    
    // Return appropriate exit code
    if (result == ASTHRA_TEST_PASS) {
        printf("SUCCESS: All integration tests passed\n");
        return 0;
    } else {
        printf("FAILURE: Some integration tests failed\n");
        return 1;
    }
} 
