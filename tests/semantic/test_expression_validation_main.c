/**
 * Expression Validation Tests - Main Integration File
 * 
 * This file includes all the modular expression validation test components
 * and serves as the replacement for expression_validation_test.c
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// Include all modular implementations
#include "test_expression_validation_binary.c"
#include "test_expression_validation_unary.c"
#include "test_expression_validation_function_calls.c"
#include "test_expression_validation_field_access.c"
#include "test_expression_validation_arrays.c"
#include "test_expression_validation_methods.c"
#include "test_expression_validation_type_casts.c"
#include "test_expression_validation_complex.c"

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(int argc, char* argv[]) {
    printf("Running Expression Validation Tests\n");
    printf("====================================\n\n");
    
    AsthraTestContext* context = asthra_test_create_context(NULL);
    if (!context) {
        printf("Failed to create test context\n");
        return 1;
    }
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Binary operator tests
    printf("Running binary arithmetic operator tests...\n");
    if (test_binary_arithmetic_operators(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Binary arithmetic operator tests passed\n");
    } else {
        printf("✗ Binary arithmetic operator tests failed\n");
    }
    total_tests++;
    
    printf("\nRunning binary comparison operator tests...\n");
    if (test_binary_comparison_operators(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Binary comparison operator tests passed\n");
    } else {
        printf("✗ Binary comparison operator tests failed\n");
    }
    total_tests++;
    
    printf("\nRunning binary logical operator tests...\n");
    if (test_binary_logical_operators(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Binary logical operator tests passed\n");
    } else {
        printf("✗ Binary logical operator tests failed\n");
    }
    total_tests++;
    
    // Unary operator tests
    printf("\nRunning unary operator tests...\n");
    if (test_unary_operators(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Unary operator tests passed\n");
    } else {
        printf("✗ Unary operator tests failed\n");
    }
    total_tests++;
    
    // Function call tests
    printf("\nRunning function call validation tests...\n");
    if (test_function_call_validation(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Function call validation tests passed\n");
    } else {
        printf("✗ Function call validation tests failed\n");
    }
    total_tests++;
    
    // Field access tests
    printf("\nRunning field access validation tests...\n");
    if (test_field_access_validation(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Field access validation tests passed\n");
    } else {
        printf("✗ Field access validation tests failed\n");
    }
    total_tests++;
    
    // Array/slice operation tests
    printf("\nRunning array/slice operation tests...\n");
    if (test_array_slice_operations(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Array/slice operation tests passed\n");
    } else {
        printf("✗ Array/slice operation tests failed\n");
    }
    total_tests++;
    
    // Method call tests
    printf("\nRunning method call validation tests...\n");
    if (test_method_call_validation(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Method call validation tests passed\n");
    } else {
        printf("✗ Method call validation tests failed\n");
    }
    total_tests++;
    
    // Type cast tests
    printf("\nRunning type cast validation tests...\n");
    if (test_type_cast_validation(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Type cast validation tests passed\n");
    } else {
        printf("✗ Type cast validation tests failed\n");
    }
    total_tests++;
    
    // Complex expression tests
    printf("\nRunning complex expression tree tests...\n");
    if (test_complex_expression_trees(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Complex expression tree tests passed\n");
    } else {
        printf("✗ Complex expression tree tests failed\n");
    }
    total_tests++;
    
    printf("\n====================================\n");
    printf("Test Results: %d/%d passed\n", passed_tests, total_tests);
    
    asthra_test_destroy_context(context);
    
    return (passed_tests == total_tests) ? 0 : 1;
}