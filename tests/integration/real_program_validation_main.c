/**
 * Real Program Validation Tests - Main Test Runner
 * 
 * Phase 3: Real Testing Infrastructure Implementation
 * Day 2: Complete Program Test Cases
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_validation_common.h"

// Include all test implementation files
#include "real_program_validation_valid_tests.c"
#include "real_program_validation_invalid_tests.c"
#include "real_program_validation_performance_tests.c"
#include "real_program_validation_feature_tests.c"

// =============================================================================
// MAIN EXECUTION FUNCTIONS
// =============================================================================

/**
 * Run comprehensive real program validation
 */
int run_comprehensive_validation(bool verbose) {
    TestSuiteConfig config = create_default_config();
    config.verbose_output = verbose;
    config.stop_on_first_failure = false;
    
    printf("=== ASTHRA REAL PROGRAM VALIDATION SUITE ===\n");
    printf("Phase 3: Real Testing Infrastructure Implementation\n");
    printf("Testing complete programs to prevent false positives\n\n");
    
    int total_failures = 0;
    
    // Test 1: Valid Programs
    printf("1. Running Valid Program Tests...\n");
    RealProgramTestSuite* valid_suite = create_valid_program_test_suite();
    if (valid_suite) {
        bool valid_success = run_test_suite(valid_suite, &config);
        if (!valid_success) {
            total_failures += valid_suite->failed;
            printf("❌ Valid program tests failed: %zu failures\n", valid_suite->failed);
        } else {
            printf("✅ All valid program tests passed\n");
        }
        print_test_suite_results(valid_suite, &config);
        cleanup_test_suite(valid_suite);
    } else {
        printf("❌ Failed to create valid program test suite\n");
        total_failures++;
    }
    
    // Test 2: Invalid Programs
    printf("\n2. Running Invalid Program Tests...\n");
    RealProgramTestSuite* invalid_suite = create_invalid_program_test_suite();
    if (invalid_suite) {
        bool invalid_success = run_test_suite(invalid_suite, &config);
        if (!invalid_success) {
            total_failures += invalid_suite->failed;
            printf("❌ Invalid program tests failed: %zu failures\n", invalid_suite->failed);
        } else {
            printf("✅ All invalid program tests passed\n");
        }
        print_test_suite_results(invalid_suite, &config);
        cleanup_test_suite(invalid_suite);
    } else {
        printf("❌ Failed to create invalid program test suite\n");
        total_failures++;
    }
    
    // Test 3: Performance Tests
    printf("\n3. Running Performance Tests...\n");
    RealProgramTestSuite* perf_suite = create_performance_test_suite();
    if (perf_suite) {
        bool perf_success = run_test_suite(perf_suite, &config);
        if (!perf_success) {
            total_failures += perf_suite->failed;
            printf("❌ Performance tests failed: %zu failures\n", perf_suite->failed);
        } else {
            printf("✅ All performance tests passed\n");
        }
        print_test_suite_results(perf_suite, &config);
        cleanup_test_suite(perf_suite);
    } else {
        printf("❌ Failed to create performance test suite\n");
        total_failures++;
    }
    
    // Test 4: Feature Validation
    printf("\n4. Running Feature Validation Tests...\n");
    
    bool multiline_ok = validate_multiline_strings_feature(&config);
    bool types_ok = validate_type_system_feature(&config);
    bool functions_ok = validate_function_declarations_feature(&config);
    bool if_let_ok = validate_if_let_feature(&config);
    
    printf("Feature validation results:\n");
    printf("  Multi-line strings: %s\n", multiline_ok ? "✅ WORKING" : "❌ BROKEN");
    printf("  Type system: %s\n", types_ok ? "✅ WORKING" : "❌ BROKEN");
    printf("  Function declarations: %s\n", functions_ok ? "✅ WORKING" : "❌ BROKEN");
    printf("  If-let statements: %s\n", if_let_ok ? "✅ WORKING" : "❌ BROKEN");
    
    int working_features = multiline_ok + types_ok + functions_ok + if_let_ok;
    printf("  Overall feature health: %d/4 features working (%.1f%%)\n", 
           working_features, (double)working_features / 4.0 * 100.0);
    
    if (working_features < 4) {
        total_failures += (4 - working_features);
    }
    
    // Final Summary
    printf("\n=== FINAL VALIDATION RESULTS ===\n");
    if (total_failures == 0) {
        printf("🎉 ALL TESTS PASSED - Real program validation successful!\n");
        printf("✅ No false positives detected\n");
        printf("✅ All claimed features work in real programs\n");
        printf("✅ Parser integration is functional\n");
        return 0;
    } else {
        printf("❌ VALIDATION FAILED - %d issues detected\n", total_failures);
        printf("🚨 False positives or broken features identified\n");
        printf("🔧 Phase 1 (Type System Repair) may be needed\n");
        return 1;
    }
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char* argv[]) {
    bool verbose = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --verbose, -v    Enable verbose output\n");
            printf("  --help, -h       Show this help message\n");
            printf("\n");
            printf("This program runs comprehensive real program validation tests\n");
            printf("to prevent false positives and ensure genuine functionality.\n");
            return 0;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            fprintf(stderr, "Use --help for usage information.\n");
            return 1;
        }
    }
    
    printf("Asthra Real Program Validation Test Suite\n");
    printf("Phase 3: Real Testing Infrastructure Implementation\n");
    printf("Verbose output: %s\n\n", verbose ? "enabled" : "disabled");
    
    return run_comprehensive_validation(verbose);
}