/**
 * Asthra Programming Language
 * Top-Level Grammar Production Tests
 * 
 * Tests for top-level grammar productions including package declarations,
 * import declarations, function declarations, struct declarations, and
 * extern declarations.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "../framework/test_data.h"
#include "parser.h"
#include "grammar_toplevel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_toplevel_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_toplevel_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TOP-LEVEL CONSTRUCTS TESTS
// =============================================================================

/**
 * Test: Parse Package Declaration
 * Verifies that package declarations are parsed correctly
 */
static AsthraTestResult test_parse_package_declaration(AsthraTestContext* context) {
    const char* test_source = "package my_package;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Parse package declaration
    ASTNode* result = parser_parse_program(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse package declaration")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify AST structure
    if (!asthra_test_assert_int_eq(context, result->type, AST_PROGRAM, 
                                  "Root should be AST_PROGRAM")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool_eq(context, parser_had_error(parser), false, 
                                   "Should parse without errors")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Import Declaration
 * Verifies that import declarations are parsed correctly
 */
static AsthraTestResult test_parse_import_declaration(AsthraTestContext* context) {
    // Grammar v1.22 requires PackageDecl before ImportDecl
    const char* test_source = "package test_pkg;\nimport \"stdlib/io\";";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parser_parse_program(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse import declaration")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool_eq(context, parser_had_error(parser), false, 
                                   "Should parse without errors")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Function Declaration
 * Verifies that function declarations are parsed correctly
 */
static AsthraTestResult test_parse_function_declaration(AsthraTestContext* context) {
    // Grammar v1.22 requires PackageDecl and explicit visibility modifier
    const char* test_source = "package test_pkg;\npub fn add(a: i32, b: i32) -> i32 { return a + b; }";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Parse the full program instead of just the function declaration
    ASTNode* result = parser_parse_program(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse function declaration")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_PROGRAM, 
                                  "Should be AST_PROGRAM")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool_eq(context, parser_had_error(parser), false, 
                                   "Should parse without errors")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Struct Declaration
 * Verifies that struct declarations are parsed correctly
 */
static AsthraTestResult test_parse_struct_declaration(AsthraTestContext* context) {
    // Grammar v1.22 requires PackageDecl and explicit visibility modifier
    const char* test_source = "package test_pkg;\npub struct Point { x: i32, y: i32 }";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Parse the full program instead of just the struct declaration
    ASTNode* result = parser_parse_program(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse struct declaration")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_PROGRAM, 
                                  "Should be AST_PROGRAM")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool_eq(context, parser_had_error(parser), false, 
                                   "Should parse without errors")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Extern Declaration
 * Verifies that extern declarations are parsed correctly
 */
static AsthraTestResult test_parse_extern_declaration(AsthraTestContext* context) {
    // Grammar v1.22 requires PackageDecl and explicit visibility modifier
    const char* test_source = "package test_pkg;\npub extern fn malloc(size: usize) -> *void;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Parse the full program instead of just the extern declaration
    ASTNode* result = parser_parse_program(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse extern declaration")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_PROGRAM, 
                                  "Should be AST_PROGRAM")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool_eq(context, parser_had_error(parser), false, 
                                   "Should parse without errors")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all top-level grammar tests
 */
AsthraTestSuite* create_grammar_toplevel_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Grammar Top-Level Tests", 
                                                     "Top-level grammar production testing");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_toplevel_tests);
    asthra_test_suite_set_teardown(suite, teardown_toplevel_tests);
    
    // Top-level constructs
    asthra_test_suite_add_test(suite, "test_parse_package_declaration", 
                              "Parse package declarations", 
                              test_parse_package_declaration);
    
    asthra_test_suite_add_test(suite, "test_parse_import_declaration", 
                              "Parse import declarations", 
                              test_parse_import_declaration);
    
    asthra_test_suite_add_test(suite, "test_parse_function_declaration", 
                              "Parse function declarations", 
                              test_parse_function_declaration);
    
    asthra_test_suite_add_test(suite, "test_parse_struct_declaration", 
                              "Parse struct declarations", 
                              test_parse_struct_declaration);
    
    asthra_test_suite_add_test(suite, "test_parse_extern_declaration", 
                              "Parse extern declarations", 
                              test_parse_extern_declaration);
    
    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Grammar Top-Level Tests ===\n\n");
    
    AsthraTestSuite* suite = create_grammar_toplevel_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED 
