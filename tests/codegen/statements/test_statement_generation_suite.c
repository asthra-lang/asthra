/**
 * Asthra Programming Language Compiler
 * Statement Generation Test Suite
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main test suite for statement generation that runs all statement generation test modules
 */

#include "statement_generation_test_fixtures.h"

// =============================================================================
// EXTERNAL TEST DECLARATIONS
// =============================================================================

// Variable declaration tests
extern AsthraTestResult test_generate_variable_declarations(AsthraTestContext* context);
extern AsthraTestResult test_generate_variable_with_expression(AsthraTestContext* context);
extern AsthraTestResult test_generate_constant_declarations(AsthraTestContext* context);
extern AsthraTestResult test_generate_uninitialized_declarations(AsthraTestContext* context);
extern AsthraTestResult test_generate_typed_declarations(AsthraTestContext* context);

// Assignment statement tests
extern AsthraTestResult test_generate_simple_assignments(AsthraTestContext* context);
extern AsthraTestResult test_generate_compound_assignments(AsthraTestContext* context);
extern AsthraTestResult test_generate_multiplication_assignment(AsthraTestContext* context);
extern AsthraTestResult test_generate_division_assignment(AsthraTestContext* context);
extern AsthraTestResult test_generate_all_compound_assignments(AsthraTestContext* context);
extern AsthraTestResult test_generate_complex_assignments(AsthraTestContext* context);

// Control flow statement tests
extern AsthraTestResult test_generate_if_statements(AsthraTestContext* context);
extern AsthraTestResult test_generate_if_else_statements(AsthraTestContext* context);
extern AsthraTestResult test_generate_while_loops(AsthraTestContext* context);
extern AsthraTestResult test_generate_for_loops(AsthraTestContext* context);
extern AsthraTestResult test_generate_break_statements(AsthraTestContext* context);
extern AsthraTestResult test_generate_continue_statements(AsthraTestContext* context);
extern AsthraTestResult test_generate_nested_control_flow(AsthraTestContext* context);
extern AsthraTestResult test_generate_complex_conditionals(AsthraTestContext* context);

// Function call statement tests
extern AsthraTestResult test_generate_simple_function_calls(AsthraTestContext* context);
extern AsthraTestResult test_generate_method_calls(AsthraTestContext* context);
extern AsthraTestResult test_generate_chained_method_calls(AsthraTestContext* context);
extern AsthraTestResult test_generate_function_calls_with_args(AsthraTestContext* context);
extern AsthraTestResult test_generate_contextual_function_calls(AsthraTestContext* context);
extern AsthraTestResult test_generate_complex_function_calls(AsthraTestContext* context);
extern AsthraTestResult test_generate_void_function_calls(AsthraTestContext* context);

// Return statement tests
extern AsthraTestResult test_generate_simple_return_statements(AsthraTestContext* context);
extern AsthraTestResult test_generate_return_with_expression(AsthraTestContext* context);
extern AsthraTestResult test_generate_empty_return_statements(AsthraTestContext* context);
extern AsthraTestResult test_generate_return_with_various_expressions(AsthraTestContext* context);
extern AsthraTestResult test_generate_return_with_arithmetic(AsthraTestContext* context);
extern AsthraTestResult test_generate_return_with_logical(AsthraTestContext* context);
extern AsthraTestResult test_generate_return_with_complex_expressions(AsthraTestContext* context);

// =============================================================================
// MAIN TEST SUITE
// =============================================================================

/**
 * Main test function for complete statement generation suite
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = create_statement_test_suite_config(
        "Complete Statement Generation Test Suite",
        "Comprehensive test suite for all statement generation including declarations, assignments, control flow, function calls, and returns",
        stats
    );
    
    // All test functions from all modules
    AsthraTestFunction tests[] = {
        // Variable declaration tests
        test_generate_variable_declarations,
        test_generate_variable_with_expression,
        test_generate_constant_declarations,
        test_generate_uninitialized_declarations,
        test_generate_typed_declarations,
        
        // Assignment statement tests
        test_generate_simple_assignments,
        test_generate_compound_assignments,
        test_generate_multiplication_assignment,
        test_generate_division_assignment,
        test_generate_all_compound_assignments,
        test_generate_complex_assignments,
        
        // Control flow statement tests
        test_generate_if_statements,
        test_generate_if_else_statements,
        test_generate_while_loops,
        test_generate_for_loops,
        test_generate_break_statements,
        test_generate_continue_statements,
        test_generate_nested_control_flow,
        test_generate_complex_conditionals,
        
        // Function call statement tests
        test_generate_simple_function_calls,
        test_generate_method_calls,
        test_generate_chained_method_calls,
        test_generate_function_calls_with_args,
        test_generate_contextual_function_calls,
        test_generate_complex_function_calls,
        test_generate_void_function_calls,
        
        // Return statement tests
        test_generate_simple_return_statements,
        test_generate_return_with_expression,
        test_generate_empty_return_statements,
        test_generate_return_with_various_expressions,
        test_generate_return_with_arithmetic,
        test_generate_return_with_logical,
        test_generate_return_with_complex_expressions
    };
    
    // Corresponding metadata for all tests
    AsthraTestMetadata metadata[] = {
        // Variable declaration tests metadata
        {"test_generate_variable_declarations", __FILE__, __LINE__, "test_generate_variable_declarations", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_variable_with_expression", __FILE__, __LINE__, "test_generate_variable_with_expression", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_constant_declarations", __FILE__, __LINE__, "test_generate_constant_declarations", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_uninitialized_declarations", __FILE__, __LINE__, "test_generate_uninitialized_declarations", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_typed_declarations", __FILE__, __LINE__, "test_generate_typed_declarations", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        
        // Assignment statement tests metadata
        {"test_generate_simple_assignments", __FILE__, __LINE__, "test_generate_simple_assignments", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_compound_assignments", __FILE__, __LINE__, "test_generate_compound_assignments", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_multiplication_assignment", __FILE__, __LINE__, "test_generate_multiplication_assignment", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_division_assignment", __FILE__, __LINE__, "test_generate_division_assignment", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_all_compound_assignments", __FILE__, __LINE__, "test_generate_all_compound_assignments", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_complex_assignments", __FILE__, __LINE__, "test_generate_complex_assignments", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        
        // Control flow statement tests metadata
        {"test_generate_if_statements", __FILE__, __LINE__, "test_generate_if_statements", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_if_else_statements", __FILE__, __LINE__, "test_generate_if_else_statements", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_while_loops", __FILE__, __LINE__, "test_generate_while_loops", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_for_loops", __FILE__, __LINE__, "test_generate_for_loops", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_break_statements", __FILE__, __LINE__, "test_generate_break_statements", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_continue_statements", __FILE__, __LINE__, "test_generate_continue_statements", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_nested_control_flow", __FILE__, __LINE__, "test_generate_nested_control_flow", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_complex_conditionals", __FILE__, __LINE__, "test_generate_complex_conditionals", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        
        // Function call statement tests metadata
        {"test_generate_simple_function_calls", __FILE__, __LINE__, "test_generate_simple_function_calls", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_method_calls", __FILE__, __LINE__, "test_generate_method_calls", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_chained_method_calls", __FILE__, __LINE__, "test_generate_chained_method_calls", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_function_calls_with_args", __FILE__, __LINE__, "test_generate_function_calls_with_args", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_contextual_function_calls", __FILE__, __LINE__, "test_generate_contextual_function_calls", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_complex_function_calls", __FILE__, __LINE__, "test_generate_complex_function_calls", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_void_function_calls", __FILE__, __LINE__, "test_generate_void_function_calls", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        
        // Return statement tests metadata
        {"test_generate_simple_return_statements", __FILE__, __LINE__, "test_generate_simple_return_statements", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_return_with_expression", __FILE__, __LINE__, "test_generate_return_with_expression", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_empty_return_statements", __FILE__, __LINE__, "test_generate_empty_return_statements", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_return_with_various_expressions", __FILE__, __LINE__, "test_generate_return_with_various_expressions", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_return_with_arithmetic", __FILE__, __LINE__, "test_generate_return_with_arithmetic", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_return_with_logical", __FILE__, __LINE__, "test_generate_return_with_logical", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_return_with_complex_expressions", __FILE__, __LINE__, "test_generate_return_with_complex_expressions", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    printf("Running Statement Generation Test Suite with %zu tests...\n", test_count);
    printf("========================================================\n\n");
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    printf("\n========================================================\n");
    printf("Statement Generation Test Suite Complete\n");
    printf("========================================================\n");
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
