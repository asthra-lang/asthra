/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Pattern Matching
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for pattern matching functionality:
 * - Match statements
 * - If-let statements
 * - Result<T,E> pattern matching
 * - Enum pattern matching
 * - Struct pattern matching
 */

#include "test_ffi_assembly_common.h"

// =============================================================================
// PATTERN MATCHING TESTS
// =============================================================================

static bool test_pattern_match_generation(void) {
    printf("Testing pattern match generation...\n");
    
    // Create expression to match on
    ASTNode *match_expr = create_test_identifier("value");
    
    // Create match arms
    ASTNodeList *arms = ast_node_list_create(3);
    
    // Arm 1: Integer literal pattern
    ASTNode *arm1 = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    arm1->data.match_arm.pattern = create_test_integer_literal(1);
    arm1->data.match_arm.body = create_test_integer_literal(100);
    ast_node_list_add(&arms, arm1);
    
    // Arm 2: String literal pattern
    ASTNode *arm2 = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    arm2->data.match_arm.pattern = create_test_string_literal("test");
    arm2->data.match_arm.body = create_test_integer_literal(200);
    ast_node_list_add(&arms, arm2);
    
    // Arm 3: Wildcard pattern (default)
    ASTNode *arm3 = ast_create_node(AST_MATCH_ARM, (SourceLocation){0, 0, 0});
    arm3->data.match_arm.pattern = ast_create_node(AST_WILDCARD, (SourceLocation){0, 0, 0});
    arm3->data.match_arm.body = create_test_integer_literal(300);
    ast_node_list_add(&arms, arm3);
    
    // Create match statement
    ASTNode *match_stmt = create_test_match_stmt(match_expr, arms);
    
    // Generate pattern matching code
    bool result = ffi_generate_match_statement(g_generator, match_stmt);
    TEST_ASSERT(result, "Pattern match statement generation failed");
    
    // Verify statistics
    size_t pattern_matches = 0;
    ffi_get_generation_statistics(g_generator, NULL, &pattern_matches, NULL, NULL, NULL, NULL);
    TEST_ASSERT(pattern_matches > 0, "Pattern match statistics not updated");
    
    // Cleanup
    ast_free_node(match_stmt);
    
    TEST_SUCCESS();
}

static bool test_result_pattern_match(void) {
    printf("Testing Result<T,E> pattern matching...\n");
    
    // Create a test Result value
    ASTNode *result_value = create_test_identifier("result_value");
    
    // Create pattern for Ok variant
    ASTNode *ok_pattern = ast_create_node(AST_VARIANT_PATTERN, (SourceLocation){0, 0, 0});
    ok_pattern->data.variant_pattern.variant_name = strdup("Ok");
    ok_pattern->data.variant_pattern.binding_name = strdup("value");
    
    // Generate Result pattern matching code
    bool result = ffi_generate_result_pattern_match(g_generator, ok_pattern, REG_RAX);
    TEST_ASSERT(result, "Result pattern match generation failed");
    
    // Cleanup
    ast_free_node(ok_pattern);
    ast_free_node(result_value);
    
    TEST_SUCCESS();
}

static bool test_if_let_statement(void) {
    printf("Testing if-let statement generation...\n");
    
    // Create if-let statement node
    ASTNode *if_let = ast_create_node(AST_IF_LET_STMT, (SourceLocation){0, 0, 0});
    
    // Create pattern (Ok variant with binding)
    ASTNode *pattern = ast_create_node(AST_VARIANT_PATTERN, (SourceLocation){0, 0, 0});
    pattern->data.variant_pattern.variant_name = strdup("Ok");
    pattern->data.variant_pattern.binding_name = strdup("value");
    
    // Create expression to match against
    ASTNode *expr = create_test_identifier("result_value");
    
    // Create then branch
    ASTNode *then_branch = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    then_branch->data.block.statements = ast_node_list_create(1);
    ast_node_list_add(&then_branch->data.block.statements, 
                     create_test_integer_literal(42));
    
    // Create else branch
    ASTNode *else_branch = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    else_branch->data.block.statements = ast_node_list_create(1);
    ast_node_list_add(&else_branch->data.block.statements, 
                     create_test_integer_literal(0));
    
    // Set if-let components
    if_let->data.if_let_stmt.pattern = pattern;
    if_let->data.if_let_stmt.expression = expr;
    if_let->data.if_let_stmt.then_branch = then_branch;
    if_let->data.if_let_stmt.else_branch = else_branch;
    
    // Generate if-let statement
    bool result = ffi_generate_if_let_statement(g_generator, if_let);
    TEST_ASSERT(result, "If-let statement generation failed");
    
    // Cleanup
    ast_free_node(if_let);
    
    TEST_SUCCESS();
}

static bool test_pattern_jump_table(void) {
    printf("Testing pattern jump table generation...\n");
    
    // Create a pattern match context
    PatternMatchContext context = {0};
    context.match_type = PATTERN_MATCH_INTEGER;
    context.num_patterns = 5;
    context.pattern_values.integer_values = malloc(sizeof(int64_t) * 5);
    context.pattern_values.integer_values[0] = 1;
    context.pattern_values.integer_values[1] = 2;
    context.pattern_values.integer_values[2] = 3;
    context.pattern_values.integer_values[3] = 5;
    context.pattern_values.integer_values[4] = 8;
    
    context.pattern_labels = malloc(sizeof(char*) * 5);
    context.pattern_labels[0] = strdup(".L_case_1");
    context.pattern_labels[1] = strdup(".L_case_2");
    context.pattern_labels[2] = strdup(".L_case_3");
    context.pattern_labels[3] = strdup(".L_case_5");
    context.pattern_labels[4] = strdup(".L_case_8");
    
    context.default_label = strdup(".L_default");
    context.expression_register = REG_RAX;
    
    // Generate pattern jump table
    bool result = ffi_generate_pattern_jump_table(g_generator, &context);
    TEST_ASSERT(result, "Pattern jump table generation failed");
    
    // Cleanup
    for (int i = 0; i < 5; i++) {
        free(context.pattern_labels[i]);
    }
    free(context.pattern_labels);
    free(context.pattern_values.integer_values);
    free(context.default_label);
    
    TEST_SUCCESS();
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char *argv[]) {
    // Initialize test suite
    if (!setup_test_suite()) {
        fprintf(stderr, "Failed to set up test suite\n");
        return 1;
    }
    
    printf("=== Pattern Matching Tests ===\n");
    
    // Run tests
    run_test(test_pattern_match_generation);
    run_test(test_result_pattern_match);
    run_test(test_if_let_statement);
    run_test(test_pattern_jump_table);
    
    // Cleanup and report
    teardown_test_suite();
    
    return (g_tests_passed == g_tests_run) ? 0 : 1;
} 
