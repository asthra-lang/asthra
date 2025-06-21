/**
 * Asthra Programming Language
 * Grammar Tests Main Runner
 * 
 * Main test runner that executes all grammar test suites including
 * top-level constructs, types, expressions, statements, and patterns.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declarations for test suite creators
AsthraTestSuite* create_grammar_toplevel_test_suite(void);
AsthraTestSuite* create_grammar_types_test_suite(void);
AsthraTestSuite* create_grammar_expressions_test_suite(void);
AsthraTestSuite* create_grammar_statements_test_suite(void);
AsthraTestSuite* create_grammar_patterns_test_suite(void);

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Asthra Grammar Tests - Comprehensive Suite ===\n\n");
    
    // Track overall results
    int total_suites = 0;
    int passed_suites = 0;
    
    // Create and run top-level tests
    printf("Running Top-Level Grammar Tests...\n");
    AsthraTestSuite* toplevel_suite = create_grammar_toplevel_test_suite();
    if (toplevel_suite) {
        AsthraTestResult result = asthra_test_suite_run(toplevel_suite);
        total_suites++;
        if (result == ASTHRA_TEST_PASS) {
            passed_suites++;
            printf("✅ Top-Level Grammar Tests: PASSED\n\n");
        } else {
            printf("❌ Top-Level Grammar Tests: FAILED\n\n");
        }
        asthra_test_suite_destroy(toplevel_suite);
    } else {
        printf("❌ Failed to create top-level test suite\n\n");
        total_suites++;
    }
    
    // Create and run type tests
    printf("Running Type Grammar Tests...\n");
    AsthraTestSuite* types_suite = create_grammar_types_test_suite();
    if (types_suite) {
        AsthraTestResult result = asthra_test_suite_run(types_suite);
        total_suites++;
        if (result == ASTHRA_TEST_PASS) {
            passed_suites++;
            printf("✅ Type Grammar Tests: PASSED\n\n");
        } else {
            printf("❌ Type Grammar Tests: FAILED\n\n");
        }
        asthra_test_suite_destroy(types_suite);
    } else {
        printf("❌ Failed to create type test suite\n\n");
        total_suites++;
    }
    
    // Create and run expression tests
    printf("Running Expression Grammar Tests...\n");
    AsthraTestSuite* expressions_suite = create_grammar_expressions_test_suite();
    if (expressions_suite) {
        AsthraTestResult result = asthra_test_suite_run(expressions_suite);
        total_suites++;
        if (result == ASTHRA_TEST_PASS) {
            passed_suites++;
            printf("✅ Expression Grammar Tests: PASSED\n\n");
        } else {
            printf("❌ Expression Grammar Tests: FAILED\n\n");
        }
        asthra_test_suite_destroy(expressions_suite);
    } else {
        printf("❌ Failed to create expression test suite\n\n");
        total_suites++;
    }
    
    // Create and run statement tests
    printf("Running Statement Grammar Tests...\n");
    AsthraTestSuite* statements_suite = create_grammar_statements_test_suite();
    if (statements_suite) {
        AsthraTestResult result = asthra_test_suite_run(statements_suite);
        total_suites++;
        if (result == ASTHRA_TEST_PASS) {
            passed_suites++;
            printf("✅ Statement Grammar Tests: PASSED\n\n");
        } else {
            printf("❌ Statement Grammar Tests: FAILED\n\n");
        }
        asthra_test_suite_destroy(statements_suite);
    } else {
        printf("❌ Failed to create statement test suite\n\n");
        total_suites++;
    }
    
    // Create and run pattern tests
    printf("Running Pattern Grammar Tests...\n");
    AsthraTestSuite* patterns_suite = create_grammar_patterns_test_suite();
    if (patterns_suite) {
        AsthraTestResult result = asthra_test_suite_run(patterns_suite);
        total_suites++;
        if (result == ASTHRA_TEST_PASS) {
            passed_suites++;
            printf("✅ Pattern Grammar Tests: PASSED\n\n");
        } else {
            printf("❌ Pattern Grammar Tests: FAILED\n\n");
        }
        asthra_test_suite_destroy(patterns_suite);
    } else {
        printf("❌ Failed to create pattern test suite\n\n");
        total_suites++;
    }
    
    // Print summary
    printf("====================================\n");
    printf("Grammar Test Suite Summary\n");
    printf("====================================\n");
    printf("Total Suites: %d\n", total_suites);
    printf("Passed: %d\n", passed_suites);
    printf("Failed: %d\n", total_suites - passed_suites);
    printf("Success Rate: %.1f%%\n", (total_suites > 0) ? (100.0 * passed_suites / total_suites) : 0.0);
    
    if (passed_suites == total_suites) {
        printf("🎉 All grammar tests passed!\n");
        return 0;
    } else {
        printf("⚠️  Some grammar tests failed.\n");
        return 1;
    }
} 
