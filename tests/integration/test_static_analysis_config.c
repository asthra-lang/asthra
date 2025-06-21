/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Test Suite - Configuration Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 5.2: Static Analysis Configuration Tests
 * - Test static analysis configuration setup
 * - Validate initialization and cleanup
 * - Test configuration parameter validation
 */

#include "test_static_analysis_common.h"

// =============================================================================
// STATIC ANALYSIS CONFIGURATION TESTS
// =============================================================================

AsthraTestResult test_static_analysis_configuration(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    // Test default configuration
    StaticAnalysisConfig config = ASTHRA_STATIC_ANALYSIS_DEFAULT_CONFIG;
    
    if (!asthra_test_assert_bool(context, config.enable_bounds_checking, 
                                "Default config should enable bounds checking")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, config.enable_null_checking, 
                                "Default config should enable null checking")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, config.strict_const_correctness, 
                                "Default config should enable strict const correctness")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test initialization
    asthra_static_analysis_init(&config);
    
    if (!asthra_test_assert_bool(context, asthra_is_static_analysis_enabled(), 
                                "Static analysis should be enabled after init")) {
        return ASTHRA_TEST_FAIL;
    }
    
    const StaticAnalysisConfig *current_config = asthra_get_static_analysis_config();
    
    if (!asthra_test_assert_pointer(context, (void *)current_config, 
                                   "Should return valid config pointer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, current_config->enable_bounds_checking, 
                                "Current config should match initialized config")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test cleanup
    asthra_static_analysis_cleanup();
    
    if (!asthra_test_assert_bool(context, !asthra_is_static_analysis_enabled(), 
                                "Static analysis should be disabled after cleanup")) {
        return ASTHRA_TEST_FAIL;
    }
    
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
} 

/**
 * Main test runner
 */
int main(void) {
    printf("=== Asthra Integration Tests - Static Analysis Config ===\n\n");
    
    // Create test statistics
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    if (!stats) {
        printf("Failed to create test statistics\n");
        return 1;
    }
    
    // Create test metadata
    AsthraTestMetadata metadata = {
        .name = "static_analysis_configuration",
        .file = __FILE__,
        .line = __LINE__,
        .function = "test_static_analysis_configuration",
        .severity = ASTHRA_TEST_SEVERITY_HIGH,
        .timeout_ns = 30000000000ULL,
        .skip = false,
        .skip_reason = NULL
    };
    
    AsthraTestContext* context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        printf("Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }
    
    // Run test
    AsthraTestResult result = test_static_analysis_configuration(context);
    
    // Report results
    printf("Static analysis configuration: %s\n", 
           result == ASTHRA_TEST_PASS ? "PASS" : "FAIL");
    
    // Print statistics
    printf("\n=== Test Statistics ===\n");
    printf("Tests run:       1\n");
    printf("Tests passed:    %d\n", result == ASTHRA_TEST_PASS ? 1 : 0);
    printf("Tests failed:    %d\n", result == ASTHRA_TEST_FAIL ? 1 : 0);
    printf("Tests skipped:   0\n");
    printf("Tests error:     0\n");
    printf("Tests timeout:   0\n");
    printf("Total duration:  0.000 ms\n");
    printf("Max duration:    0.000 ms\n");
    printf("Min duration:    0.000 ms\n");
    printf("Assertions:      %llu checked, %llu failed\n", 
           asthra_test_get_stat(&stats->assertions_checked), 
           asthra_test_get_stat(&stats->assertions_failed));
    printf("========================\n");
    printf("Integration tests: %d/1 passed\n", result == ASTHRA_TEST_PASS ? 1 : 0);
    
    // Cleanup
    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);
    
    return result == ASTHRA_TEST_PASS ? 0 : 1;
} 
