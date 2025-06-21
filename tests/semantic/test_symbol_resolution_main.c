/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Main Test Runner
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main test runner for symbol resolution test suite
 */

#include "test_symbol_resolution_common.h"

// Include all test implementation files
#include "test_symbol_resolution_variables.c"
#include "test_symbol_resolution_functions.c"
#include "test_symbol_resolution_scopes.c"
#include "test_symbol_resolution_imports.c"
#include "test_symbol_resolution_types.c"
#include "test_symbol_resolution_methods.c"
#include "test_symbol_resolution_errors.c"

// =============================================================================
// TEST SUITE RUNNER
// =============================================================================

int main(void) {
    printf("Running Symbol Resolution Tests for Semantic Analyzer\n");
    printf("=====================================================\n");
    
    AsthraTestContext* context = asthra_test_create_context(NULL);
    if (!context) {
        printf("Failed to create test context\n");
        return 1;
    }
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Variable tests
    printf("Running variable declaration and lookup tests...\n");
    if (test_variable_declaration_basic(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Variable declaration basic tests passed\n");
    } else {
        printf("✗ Variable declaration basic tests failed\n");
    }
    total_tests++;
    
    if (test_variable_lookup_undefined(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Variable lookup undefined tests passed\n");
    } else {
        printf("✗ Variable lookup undefined tests failed\n");
    }
    total_tests++;
    
    if (test_variable_declaration_duplicate(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Variable declaration duplicate tests passed\n");
    } else {
        printf("✗ Variable declaration duplicate tests failed\n");
    }
    total_tests++;
    
    // Function tests
    printf("\nRunning function resolution tests...\n");
    if (test_function_declaration_and_resolution(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Function declaration and resolution tests passed\n");
    } else {
        printf("✗ Function declaration and resolution tests failed\n");
    }
    total_tests++;
    
    if (test_function_resolution_undefined(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Function resolution undefined tests passed\n");
    } else {
        printf("✗ Function resolution undefined tests failed\n");
    }
    total_tests++;
    
    // Scope tests
    printf("\nRunning scope management tests...\n");
    if (test_scope_shadowing(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Scope shadowing tests passed\n");
    } else {
        printf("✗ Scope shadowing tests failed\n");
    }
    total_tests++;
    
    if (test_scope_variable_out_of_scope(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Scope variable out of scope tests passed\n");
    } else {
        printf("✗ Scope variable out of scope tests failed\n");
    }
    total_tests++;
    
    // Import tests
    printf("\nRunning import resolution tests...\n");
    if (test_module_alias_registration(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Module alias registration tests passed\n");
    } else {
        printf("✗ Module alias registration tests failed\n");
    }
    total_tests++;
    
    if (test_import_resolution_undefined_module(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Import resolution undefined module tests passed\n");
    } else {
        printf("✗ Import resolution undefined module tests failed\n");
    }
    total_tests++;
    
    // Type tests
    printf("\nRunning type resolution tests...\n");
    if (test_type_name_resolution_builtin(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Type name resolution builtin tests passed\n");
    } else {
        printf("✗ Type name resolution builtin tests failed\n");
    }
    total_tests++;
    
    if (test_type_name_resolution_custom(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Type name resolution custom tests passed\n");
    } else {
        printf("✗ Type name resolution custom tests failed\n");
    }
    total_tests++;
    
    if (test_type_name_resolution_undefined(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Type name resolution undefined tests passed\n");
    } else {
        printf("✗ Type name resolution undefined tests failed\n");
    }
    total_tests++;
    
    // Method tests
    printf("\nRunning method resolution tests...\n");
    if (test_method_resolution_basic(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Method resolution basic tests passed\n");
    } else {
        printf("✗ Method resolution basic tests failed\n");
    }
    total_tests++;
    
    if (test_method_resolution_undefined(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Method resolution undefined tests passed\n");
    } else {
        printf("✗ Method resolution undefined tests failed\n");
    }
    total_tests++;
    
    // Error tests
    printf("\nRunning error handling tests...\n");
    if (test_error_undefined_symbol(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Error undefined symbol tests passed\n");
    } else {
        printf("✗ Error undefined symbol tests failed\n");
    }
    total_tests++;
    
    if (test_error_duplicate_symbol(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Error duplicate symbol tests passed\n");
    } else {
        printf("✗ Error duplicate symbol tests failed\n");
    }
    total_tests++;
    
    if (test_error_undefined_type(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Error undefined type tests passed\n");
    } else {
        printf("✗ Error undefined type tests failed\n");
    }
    total_tests++;
    
    // Visibility tests
    printf("\nRunning visibility tests...\n");
    if (test_visibility_public_symbol(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Visibility public symbol tests passed\n");
    } else {
        printf("✗ Visibility public symbol tests failed\n");
    }
    total_tests++;
    
    printf("\n=====================================================\n");
    printf("Test Results: %d/%d passed\n", passed_tests, total_tests);
    
    asthra_test_destroy_context(context);
    
    return (passed_tests == total_tests) ? 0 : 1;
}