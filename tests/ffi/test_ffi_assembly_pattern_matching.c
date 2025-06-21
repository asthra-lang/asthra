/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Pattern Matching
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for pattern matching code generation including Result<T,E> types
 */

#include "test_ffi_assembly_common.h"

// =============================================================================
// PATTERN MATCHING TESTS
// =============================================================================

static bool test_pattern_match_generation(void) {
    printf("Testing pattern match generation...\n");
    
    // Create match expression
    ASTNode *match_expr = create_test_identifier("result_value");
    
    // Create match arms (simplified)
    ASTNodeList *arms = ast_node_list_create(2);
    
    // Ok arm
    ASTNode *ok_arm = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    ok_arm->data.match_arm.pattern = create_test_identifier("Result.Ok");
    ok_arm->data.match_arm.body = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    ast_node_list_add(&arms, ok_arm);
    
    // Err arm
    ASTNode *err_arm = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    err_arm->data.match_arm.pattern = create_test_identifier("Result.Err");
    err_arm->data.match_arm.body = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    ast_node_list_add(&arms, err_arm);
    
    // Create match statement
    ASTNode *match_stmt = create_test_match_stmt(match_expr, arms);
    
    // Generate pattern match
    bool result = ffi_generate_match_statement(g_generator, match_stmt);
    TEST_ASSERT(result, "Pattern match generation failed");
    
    // Verify statistics
    size_t pattern_matches = 0;
    ffi_get_generation_statistics(g_generator, NULL, &pattern_matches, NULL, NULL, NULL, NULL);
    TEST_ASSERT(pattern_matches > 0, "Pattern match statistics not updated");
    
    // Cleanup - ast_free_node will free the arms list
    ast_free_node(match_stmt);
    
    TEST_SUCCESS();
}

static bool test_result_pattern_match(void) {
    printf("Testing Result<T,E> pattern matching...\n");
    
    // Create Result pattern
    ASTNode *pattern = create_test_identifier("Result.Ok");
    
    // Generate Result pattern match
    bool result = ffi_generate_result_pattern_match(g_generator, pattern, REG_RAX);
    TEST_ASSERT(result, "Result pattern match generation failed");
    
    // Cleanup
    ast_free_node(pattern);
    
    TEST_SUCCESS();
}

static bool test_result_construction(void) {
    printf("Testing Result<T,E> construction...\n");
    
    // Test Ok construction
    bool result = ffi_generate_result_construction(g_generator, true, REG_RDI, NULL, REG_RAX);
    TEST_ASSERT(result, "Result.Ok construction failed");
    
    // Test Err construction
    result = ffi_generate_result_construction(g_generator, false, REG_RDI, NULL, REG_RAX);
    TEST_ASSERT(result, "Result.Err construction failed");
    
    TEST_SUCCESS();
}

static bool test_exhaustive_pattern_matching(void) {
    printf("Testing exhaustive pattern matching...\n");
    
    // Create enum-style match expression
    ASTNode *match_expr = create_test_identifier("status");
    
    // Create match arms for all enum variants
    ASTNodeList *arms = ast_node_list_create(3);
    
    // Success arm
    ASTNode *success_arm = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    success_arm->data.match_arm.pattern = create_test_identifier("Status.Success");
    success_arm->data.match_arm.body = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    ast_node_list_add(&arms, success_arm);
    
    // Warning arm
    ASTNode *warning_arm = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    warning_arm->data.match_arm.pattern = create_test_identifier("Status.Warning");
    warning_arm->data.match_arm.body = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    ast_node_list_add(&arms, warning_arm);
    
    // Error arm
    ASTNode *error_arm = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    error_arm->data.match_arm.pattern = create_test_identifier("Status.Error");
    error_arm->data.match_arm.body = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    ast_node_list_add(&arms, error_arm);
    
    // Create match statement
    ASTNode *match_stmt = create_test_match_stmt(match_expr, arms);
    
    // Generate exhaustive pattern match
    bool result = ffi_generate_match_statement(g_generator, match_stmt);
    TEST_ASSERT(result, "Exhaustive pattern match generation failed");
    
    // Cleanup
    ast_free_node(match_stmt);
    
    TEST_SUCCESS();
}

static bool test_pattern_destructuring(void) {
    printf("Testing pattern destructuring...\n");
    
    // Create a tuple destructuring pattern
    ASTNode *match_expr = create_test_identifier("tuple_value");
    
    // Create destructuring pattern (simplified)
    ASTNodeList *arms = ast_node_list_create(1);
    ASTNode *destructure_arm = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    
    // Create a pattern that destructures (a, b)
    ASTNode *tuple_pattern = ast_create_node(AST_TUPLE_PATTERN, (SourceLocation){0, 0, 0});
    ASTNodeList *pattern_elements = ast_node_list_create(2);
    ast_node_list_add(&pattern_elements, create_test_identifier("a"));
    ast_node_list_add(&pattern_elements, create_test_identifier("b"));
    tuple_pattern->data.tuple_pattern.elements = pattern_elements;
    
    destructure_arm->data.match_arm.pattern = tuple_pattern;
    destructure_arm->data.match_arm.body = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    ast_node_list_add(&arms, destructure_arm);
    
    // Create match statement
    ASTNode *match_stmt = create_test_match_stmt(match_expr, arms);
    
    // Generate pattern destructuring
    bool result = ffi_generate_match_statement(g_generator, match_stmt);
    TEST_ASSERT(result, "Pattern destructuring generation failed");
    
    // Cleanup
    ast_free_node(match_stmt);
    
    TEST_SUCCESS();
}

// =============================================================================
// TEST FUNCTION ARRAY
// =============================================================================

test_function_t ffi_pattern_matching_tests[] = {
    test_pattern_match_generation,
    test_result_pattern_match,
    test_result_construction,
    test_exhaustive_pattern_matching,
    test_pattern_destructuring,
};

const size_t ffi_pattern_matching_test_count = sizeof(ffi_pattern_matching_tests) / sizeof(ffi_pattern_matching_tests[0]);

// =============================================================================
// TEST SUITE RUNNER
// =============================================================================

int run_ffi_pattern_matching_tests(void) {
    printf("Running FFI Pattern Matching Tests...\n");
    printf("=====================================\n\n");
    
    if (!setup_test_suite()) {
        return 1;
    }
    
    // Enable pattern matching optimization
    g_generator->config.optimize_pattern_matching = true;
    g_generator->config.enable_bounds_checking = true;
    
    for (size_t i = 0; i < ffi_pattern_matching_test_count; i++) {
        run_test(ffi_pattern_matching_tests[i]);
    }
    
    teardown_test_suite();
    
    return (g_tests_passed == g_tests_run) ? 0 : 1;
} 
