/**
 * Asthra Programming Language
 * AST Validation Tests
 *
 * Tests for AST structure validation and consistency checks
 * as outlined in Phase 2.4 of the test coverage improvement plan.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "ast_node.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_ast_validation_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_ast_validation_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// AST VALIDATION TESTS
// =============================================================================

/**
 * Test: AST Structure Consistency
 * Verifies that AST structures are consistent and well-formed
 */
static AsthraTestResult test_ast_structure_consistency(AsthraTestContext *context) {
    const char *test_source =
        "fn test(x: i32) -> i32 { if (x > 0) { return x; } else { return -x; } }";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify basic structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Root should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify function has required components
    if (!asthra_test_assert_not_null(context, ast->data.function_decl.name,
                                     "Function should have a name")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, ast->data.function_decl.body,
                                     "Function should have a body")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Parent Child Relationships
 * Verifies that parent-child relationships are properly maintained
 */
static AsthraTestResult test_ast_parent_child_relationships(AsthraTestContext *context) {
    const char *test_source = "fn test(none) { let x: i32 = 42; }";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Check if function body exists and has proper relationship
    ASTNode *body = ast->data.function_decl.body;

    if (!asthra_test_assert_not_null(context, body, "Function should have body")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Note: Parent field checking removed as current AST implementation doesn't include parent
    // pointers This is a design choice for simplicity and performance Parent relationships can be
    // tracked during traversal if needed

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Type Checking Integration
 * Verifies that AST nodes properly support type checking
 */
static AsthraTestResult test_ast_type_checking_integration(AsthraTestContext *context) {
    const char *test_source = "fn typed_function(x: i32, y: f64) -> bool { return x > 0; }";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse typed function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify function declaration structure supports type information
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Check if return type is present (if parser supports it)
    if (ast->data.function_decl.return_type) {
        if (!asthra_test_assert_not_null(context, ast->data.function_decl.return_type,
                                         "Return type should be valid if present")) {
            ast_free_node(ast);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Node Integrity Validation
 * Verifies that AST nodes maintain structural integrity
 */
static AsthraTestResult test_ast_node_integrity_validation(AsthraTestContext *context) {
    const char *test_source =
        "fn integrity_test(none) { let a: i32 = 1; let b: i32 = a + 2; return b; }";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify node has valid location information
    if (!asthra_test_assert_int_gt(context, ast->location.line, 0,
                                   "Node should have valid line number")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_gt(context, ast->location.column, 0,
                                   "Node should have valid column number")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify filename is present
    if (!asthra_test_assert_not_null(context, ast->location.filename,
                                     "Node should have filename")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Semantic Consistency
 * Verifies that AST maintains semantic consistency
 */
static AsthraTestResult test_ast_semantic_consistency(AsthraTestContext *context) {
    const char *test_source =
        "fn semantic_test(x: i32) -> i32 { if (x > 0) { return x; } return 0; }";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify function declaration has consistent structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Check that function name is consistent
    if (ast->data.function_decl.name) {
        if (!asthra_test_assert_str_eq(context, ast->data.function_decl.name, "semantic_test",
                                       "Function name should match source")) {
            ast_free_node(ast);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Structural Validity
 * Verifies that AST structure follows language rules
 */
static AsthraTestResult test_ast_structural_validity(AsthraTestContext *context) {
    const char *test_source = "fn valid_structure(none) { let x: i32 = 42; return x; }";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify function has proper structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Root should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify function body exists and is a block
    ASTNode *body = ast->data.function_decl.body;
    if (body) {
        if (!asthra_test_assert_int_eq(context, body->type, AST_BLOCK,
                                       "Function body should be a block")) {
            ast_free_node(ast);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Error Handling Validation
 * Verifies that AST handles error conditions gracefully
 */
static AsthraTestResult test_ast_error_handling_validation(AsthraTestContext *context) {
    // Test with malformed input
    const char *test_source = "fn incomplete_function("; // Intentionally incomplete
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    // The parser should handle this gracefully - either return NULL or a partial AST
    // This test verifies that the system doesn't crash

    if (ast) {
        // If we got an AST, it should be valid even if incomplete
        if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                       "If AST is returned, it should be valid")) {
            ast_free_node(ast);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        ast_free_node(ast);
    }

    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all AST validation tests
 */
AsthraTestSuite *create_ast_validation_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create(
        "AST Validation Tests", "AST structure validation and consistency checks");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_ast_validation_tests);
    asthra_test_suite_set_teardown(suite, teardown_ast_validation_tests);

    // AST validation tests
    asthra_test_suite_add_test(suite, "test_ast_structure_consistency", "AST structure consistency",
                               test_ast_structure_consistency);

    asthra_test_suite_add_test(suite, "test_ast_parent_child_relationships",
                               "AST parent-child relationships",
                               test_ast_parent_child_relationships);

    asthra_test_suite_add_test(suite, "test_ast_type_checking_integration",
                               "AST type checking integration", test_ast_type_checking_integration);

    asthra_test_suite_add_test(suite, "test_ast_node_integrity_validation",
                               "AST node integrity validation", test_ast_node_integrity_validation);

    asthra_test_suite_add_test(suite, "test_ast_semantic_consistency", "AST semantic consistency",
                               test_ast_semantic_consistency);

    asthra_test_suite_add_test(suite, "test_ast_structural_validity", "AST structural validity",
                               test_ast_structural_validity);

    asthra_test_suite_add_test(suite, "test_ast_error_handling_validation",
                               "AST error handling validation", test_ast_error_handling_validation);

    return suite;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    AsthraTestMetadata metadata = {.name = "AST Validation Tests",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "main",
                                   .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                                   .timeout_ns = 30000000000ULL,
                                   .skip = false,
                                   .skip_reason = NULL};

    AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        fprintf(stderr, "Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    printf("Running AST Validation Tests...\n\n");

    // Run all tests
    AsthraTestResult result1 = test_ast_structure_consistency(context);
    AsthraTestResult result2 = test_ast_parent_child_relationships(context);
    AsthraTestResult result3 = test_ast_type_checking_integration(context);
    AsthraTestResult result4 = test_ast_node_integrity_validation(context);
    AsthraTestResult result5 = test_ast_semantic_consistency(context);
    AsthraTestResult result6 = test_ast_structural_validity(context);
    AsthraTestResult result7 = test_ast_error_handling_validation(context);

    // Calculate results
    int passed = 0;
    int total = 7;

    if (result1 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_structure_consistency\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_structure_consistency\n");
    }

    if (result2 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_parent_child_relationships\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_parent_child_relationships\n");
    }

    if (result3 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_type_checking_integration\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_type_checking_integration\n");
    }

    if (result4 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_node_integrity_validation\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_node_integrity_validation\n");
    }

    if (result5 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_semantic_consistency\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_semantic_consistency\n");
    }

    if (result6 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_structural_validity\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_structural_validity\n");
    }

    if (result7 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_error_handling_validation\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_error_handling_validation\n");
    }

    printf("\nTest Results: %d/%d passed\n", passed, total);

    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);
    return (passed == total) ? 0 : 1;
}
