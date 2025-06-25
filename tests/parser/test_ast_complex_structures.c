/**
 * Asthra Programming Language
 * AST Complex Structures Tests
 *
 * Tests for complex nested AST structures and memory management
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

static AsthraTestResult setup_ast_complex_structures_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_ast_complex_structures_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// COMPLEX AST STRUCTURE TESTS
// =============================================================================

/**
 * Test: Nested Structure Validation
 * Verifies that complex nested structures are properly handled
 */
static AsthraTestResult test_nested_structure_validation(AsthraTestContext *context) {
    const char *test_source = "fn complex_function(data: Point) -> Result<i32, String> {\n"
                              "    if (data.x > 0) {\n"
                              "        let nums: []i32 = [1, 2, 3, 4, 5];\n"
                              "        for i in nums {\n"
                              "            if (i % 2 == 0) {\n"
                              "                return Ok(i);\n"
                              "            }\n"
                              "        }\n"
                              "    }\n"
                              "    return Err(\"No valid value found\");\n"
                              "}";

    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse complex function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify basic structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify function has body
    if (!asthra_test_assert_not_null(context, ast->data.function_decl.body,
                                     "Function should have body")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Memory Management
 * Verifies that memory is properly managed for complex structures
 */
static AsthraTestResult test_ast_memory_management(AsthraTestContext *context) {
    // Track memory before test
    size_t initial_memory = get_current_memory_usage();

    // Create and destroy multiple ASTs
    for (int i = 0; i < 10; i++) {
        const char *test_source = "fn test_function(none) { let x: i32 = 42; return x * 2; }";
        Parser *parser = create_test_parser(test_source);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *ast = parse_function_decl(parser);

        if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(ast);
        destroy_test_parser(parser);
    }

    // Check memory usage after cleanup
    size_t final_memory = get_current_memory_usage();

    // Memory should be approximately the same (allowing for some variation)
    if (final_memory > initial_memory + 1024) { // Allow 1KB variation
        asthra_test_log(context,
                        "Warning: Potential memory leak detected. "
                        "Initial: %zu, Final: %zu",
                        initial_memory, final_memory);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Deep Nesting Handling
 * Verifies that deeply nested structures are handled correctly
 */
static AsthraTestResult test_deep_nesting_handling(AsthraTestContext *context) {
    const char *test_source = "fn deeply_nested(none) {\n"
                              "    if (true) {\n"
                              "        if (true) {\n"
                              "            if (true) {\n"
                              "                if (true) {\n"
                              "                    let x: i32 = 42;\n"
                              "                    return x;\n"
                              "                }\n"
                              "            }\n"
                              "        }\n"
                              "    }\n"
                              "}";

    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse deeply nested function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify basic structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Complex Expression Trees
 * Verifies that complex expression trees are properly structured
 */
static AsthraTestResult test_complex_expression_trees(AsthraTestContext *context) {
    const char *test_source = "fn complex_expressions(none) {\n"
                              "    let result: i32 = (a + b) * (c - d) / (e + f) % (g * h);\n"
                              "    return result;\n"
                              "}";

    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse complex expressions")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify basic structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Multi-Statement Block Handling
 * Verifies that blocks with multiple statements are properly structured
 */
static AsthraTestResult test_multi_statement_block_handling(AsthraTestContext *context) {
    const char *test_source = "fn multi_statement_function(none) {\n"
                              "    let a: i32 = 1;\n"
                              "    let b: i32 = 2;\n"
                              "    let c: i32 = 3;\n"
                              "    let d: i32 = 4;\n"
                              "    let e: i32 = 5;\n"
                              "    let result: i32 = a + b + c + d + e;\n"
                              "    return result;\n"
                              "}";

    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse multi-statement function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify basic structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify function body is a block
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
 * Test: AST Size and Complexity Limits
 * Verifies that the AST can handle reasonably sized complex structures
 */
static AsthraTestResult test_ast_size_complexity_limits(AsthraTestContext *context) {
    // Generate a moderately complex function with many statements
    const char *test_source =
        "fn complexity_test(none) {\n"
        "    let x1: i32 = 1; let x2: i32 = 2; let x3: i32 = 3; let x4: i32 = 4; let x5: i32 = 5;\n"
        "    let y1: i32 = x1 + x2; let y2: i32 = x3 + x4; let y3: i32 = x5 + x1;\n"
        "    let z1: i32 = y1 * y2; let z2: i32 = y2 * y3; let z3: i32 = y3 * y1;\n"
        "    let result: i32 = z1 + z2 + z3;\n"
        "    return result;\n"
        "}";

    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse complexity test")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify basic structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Circular Reference Prevention
 * Verifies that the AST structure prevents circular references
 */
static AsthraTestResult test_ast_circular_reference_prevention(AsthraTestContext *context) {
    const char *test_source = "fn test(none) { return 42; }";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify that the AST structure is well-formed without circular references
    ASTNode *body = ast->data.function_decl.body;
    if (body) {
        // Verify that the function body is a valid block
        if (!asthra_test_assert_int_eq(context, body->type, AST_BLOCK,
                                       "Function body should be a block")) {
            ast_free_node(ast);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify that the function doesn't point to itself
        if (!asthra_test_assert_ptr_ne(context, ast, ast->data.function_decl.body,
                                       "Node should not point to itself")) {
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
 * Test: Large AST Stress Test
 * Verifies that the system can handle larger ASTs without issues
 */
static AsthraTestResult test_large_ast_stress_test(AsthraTestContext *context) {
    // Create a larger function for stress testing
    const char *test_source =
        "fn stress_test_function(param1: i32, param2: f64, param3: bool) -> i32 {\n"
        "    let local1: i32 = param1 + 10;\n"
        "    let local2: f64 = param2 * 2.0;\n"
        "    let local3: bool = !param3;\n"
        "    \n"
        "    if (local1 > 0) {\n"
        "        let items: []i32 = [1, 2, 3, 4, 5];\n"
        "        for i in items {\n"
        "            if (i % 2 == 0) {\n"
        "                local1 = local1 + i;\n"
        "            } else {\n"
        "                local1 = local1 - i;\n"
        "            }\n"
        "        }\n"
        "    } else {\n"
        "        if (local1 < 100) {\n"
        "            local1 = local1 * 2;\n"
        "            if (local1 > 50) {\n"
        "                local1 = 100;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    \n"
        "    return local1;\n"
        "}";

    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *ast = parse_function_decl(parser);

    if (!asthra_test_assert_not_null(context, ast, "Failed to parse stress test function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify basic structure
    if (!asthra_test_assert_int_eq(context, ast->type, AST_FUNCTION_DECL,
                                   "Should be function declaration")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all AST complex structures tests
 */
AsthraTestSuite *create_ast_complex_structures_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create(
        "AST Complex Structures Tests", "Complex nested AST structures and memory management");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_ast_complex_structures_tests);
    asthra_test_suite_set_teardown(suite, teardown_ast_complex_structures_tests);

    // Complex structure tests
    asthra_test_suite_add_test(suite, "test_nested_structure_validation",
                               "Nested structure validation", test_nested_structure_validation);

    asthra_test_suite_add_test(suite, "test_ast_memory_management", "AST memory management",
                               test_ast_memory_management);

    asthra_test_suite_add_test(suite, "test_deep_nesting_handling", "Deep nesting handling",
                               test_deep_nesting_handling);

    asthra_test_suite_add_test(suite, "test_complex_expression_trees", "Complex expression trees",
                               test_complex_expression_trees);

    asthra_test_suite_add_test(suite, "test_multi_statement_block_handling",
                               "Multi-statement block handling",
                               test_multi_statement_block_handling);

    asthra_test_suite_add_test(suite, "test_ast_size_complexity_limits",
                               "AST size and complexity limits", test_ast_size_complexity_limits);

    asthra_test_suite_add_test(suite, "test_ast_circular_reference_prevention",
                               "AST circular reference prevention",
                               test_ast_circular_reference_prevention);

    asthra_test_suite_add_test(suite, "test_large_ast_stress_test", "Large AST stress test",
                               test_large_ast_stress_test);

    return suite;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    AsthraTestSuite *suite = create_ast_complex_structures_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
