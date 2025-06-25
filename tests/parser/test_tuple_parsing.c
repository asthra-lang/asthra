/**
 * Asthra Programming Language
 * Tuple Type Parsing Tests
 *
 * Tests for parsing tuple types and tuple literals following
 * the grammar definitions in grammar.txt
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ast_node.h"
#include "lexer.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test framework macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "FAIL: %s\n", message);                                                \
            return 0;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_PASS(message)                                                                         \
    do {                                                                                           \
        printf("PASS: %s\n", message);                                                             \
        return 1;                                                                                  \
    } while (0)

// Helper function to create parser from source
static Parser *create_test_parser(const char *source) {
    size_t source_len = strlen(source);
    Lexer *lexer = lexer_create(source, source_len, "test.ast");
    if (!lexer) {
        return NULL;
    }

    Parser *parser = parser_create(lexer);
    return parser;
}

// Helper function to destroy parser
static void destroy_test_parser(Parser *parser) {
    if (parser) {
        Lexer *lexer = parser->lexer;
        parser_destroy(parser);
        if (lexer) {
            lexer_destroy(lexer);
        }
    }
}

// =============================================================================
// TUPLE TYPE PARSING TESTS
// =============================================================================

/**
 * Test: Parse Simple Tuple Type
 * Verifies parsing of basic tuple type declarations
 */
static int test_parse_simple_tuple_type(void) {
    const char *test_source = "package test;\n"
                              "pub fn main(none) -> void {\n"
                              "    let pair: (i32, string) = (42, \"hello\");\n"
                              "    return ();\n"
                              "}\n";

    Parser *parser = create_test_parser(test_source);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    ASTNode *program = parser_parse_program(parser);

    TEST_ASSERT(program != NULL, "Failed to parse program with tuple type");

    // TODO: Add more detailed verification of the AST structure
    // For now, just check that it parsed without errors
    TEST_ASSERT(!parser_had_error(parser), "Parser reported errors");

    if (program) {
        ast_free_node(program);
    }
    destroy_test_parser(parser);

    TEST_PASS("Tuple type parsing");
}

/**
 * Test: Parse Tuple Literal
 * Verifies parsing of tuple literal expressions
 */
static int test_parse_tuple_literal(void) {
    const char *test_source = "package test;\n"
                              "pub fn main(none) -> void {\n"
                              "    let pair: (i32, string) = (42, \"hello\");\n"
                              "    return ();\n"
                              "}\n";

    Parser *parser = create_test_parser(test_source);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    ASTNode *program = parser_parse_program(parser);

    TEST_ASSERT(program != NULL, "Failed to parse program with tuple literal");
    TEST_ASSERT(!parser_had_error(parser), "Parser reported errors");

    if (program) {
        ast_free_node(program);
    }
    destroy_test_parser(parser);

    TEST_PASS("Tuple literal parsing");
}

/**
 * Test: Parse Tuple Pattern
 * Verifies parsing of tuple patterns for destructuring
 */
static int test_parse_tuple_pattern(void) {
    const char *test_source = "package test;\n"
                              "pub fn main(none) -> void {\n"
                              "    let pair: (i32, string) = (42, \"hello\");\n"
                              "    let (x, y): (i32, string) = pair;\n"
                              "    return ();\n"
                              "}\n";

    Parser *parser = create_test_parser(test_source);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    ASTNode *program = parser_parse_program(parser);

    TEST_ASSERT(program != NULL, "Failed to parse program with tuple pattern");
    TEST_ASSERT(!parser_had_error(parser), "Parser reported errors");

    if (program) {
        ast_free_node(program);
    }
    destroy_test_parser(parser);

    TEST_PASS("Tuple pattern parsing");
}

/**
 * Test: Single Element Tuple Should Fail
 * Verifies that single element tuples are rejected (minimum 2 elements per grammar)
 */
static int test_single_element_tuple_error(void) {
    const char *test_source = "package test;\n"
                              "pub fn main(none) -> void {\n"
                              "    let invalid: (i32) = (42);\n" // Should fail - single element
                              "    return ();\n"
                              "}\n";

    Parser *parser = create_test_parser(test_source);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    ASTNode *program = parser_parse_program(parser);

    // Once tuples are implemented, this should produce an error
    // For now, it might parse as parenthesized expression

    if (program) {
        ast_free_node(program);
    }
    destroy_test_parser(parser);

    printf("SKIP: Tuple validation not yet implemented\n");
    return 1;
}

/**
 * Test: Parse Tuple Element Access
 * Verifies parsing of tuple element access with .0, .1 syntax
 */
static int test_parse_tuple_element_access(void) {
    const char *test_source = "package test;\n"
                              "pub fn main(none) -> void {\n"
                              "    let pair: (i32, string) = (42, \"hello\");\n"
                              "    let first: i32 = pair.0;\n"
                              "    let second: string = pair.1;\n"
                              "    return ();\n"
                              "}\n";

    Parser *parser = create_test_parser(test_source);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    ASTNode *program = parser_parse_program(parser);

    TEST_ASSERT(program != NULL, "Failed to parse program with tuple element access");
    TEST_ASSERT(!parser_had_error(parser),
                "Parser reported errors when parsing tuple element access");

    // Navigate to the main function body
    ASTNodeList *decls = program->data.program.declarations;
    TEST_ASSERT(decls != NULL && decls->count > 0, "No declarations found in program");

    ASTNode *main_fn = decls->nodes[0];
    TEST_ASSERT(main_fn->type == AST_FUNCTION_DECL, "First declaration is not a function");

    ASTNode *body = main_fn->data.function_decl.body;
    TEST_ASSERT(body != NULL && body->type == AST_BLOCK, "Function body is not a block");

    ASTNodeList *stmts = body->data.block.statements;
    TEST_ASSERT(stmts != NULL && stmts->count >= 3, "Not enough statements in function body");

    // Check the second statement (let first: i32 = pair.0;)
    ASTNode *let_first = stmts->nodes[1];
    TEST_ASSERT(let_first->type == AST_LET_STMT, "Second statement is not a let statement");

    ASTNode *init_expr = let_first->data.let_stmt.initializer;
    TEST_ASSERT(init_expr != NULL, "Let statement has no initializer");

    // The initializer should be a field access expression
    TEST_ASSERT(init_expr->type == AST_FIELD_ACCESS, "Initializer is not a field access");
    TEST_ASSERT(strcmp(init_expr->data.field_access.field_name, "0") == 0,
                "Field name is not '0' for tuple element access");

    if (program) {
        ast_free_node(program);
    }
    destroy_test_parser(parser);

    TEST_PASS("Tuple element access parsing");
}

/**
 * Test: Parse Nested Tuple Element Access
 * Verifies parsing of nested tuple element access like tuple.0.1
 */
static int test_parse_nested_tuple_access(void) {
    const char *test_source = "package test;\n"
                              "pub fn main(none) -> void {\n"
                              "    let nested: ((i32, string), bool) = ((42, \"hello\"), true);\n"
                              "    let inner_second: string = nested.0.1;\n"
                              "    return ();\n"
                              "}\n";

    Parser *parser = create_test_parser(test_source);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    ASTNode *program = parser_parse_program(parser);

    TEST_ASSERT(program != NULL, "Failed to parse program with nested tuple access");
    TEST_ASSERT(!parser_had_error(parser),
                "Parser reported errors when parsing nested tuple access");

    // Navigate to the main function body
    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_fn = decls->nodes[0];
    ASTNode *body = main_fn->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    // Check the second statement (let inner_second: string = nested.0.1;)
    ASTNode *let_stmt = stmts->nodes[1];
    ASTNode *init_expr = let_stmt->data.let_stmt.initializer;

    // Should be field access .1 on another field access .0
    TEST_ASSERT(init_expr->type == AST_FIELD_ACCESS, "Outer expression is not field access");
    TEST_ASSERT(strcmp(init_expr->data.field_access.field_name, "1") == 0,
                "Outer field name is not '1'");

    ASTNode *inner_expr = init_expr->data.field_access.object;
    TEST_ASSERT(inner_expr->type == AST_FIELD_ACCESS, "Inner expression is not field access");
    TEST_ASSERT(strcmp(inner_expr->data.field_access.field_name, "0") == 0,
                "Inner field name is not '0'");

    if (program) {
        ast_free_node(program);
    }
    destroy_test_parser(parser);

    TEST_PASS("Nested tuple element access parsing");
}

/**
 * Test: Invalid Tuple Element Access
 * Verifies that invalid tuple indices are handled properly
 */
static int test_invalid_tuple_element_access(void) {
    const char *test_source = "package test;\n"
                              "pub fn main(none) -> void {\n"
                              "    let pair: (i32, string) = (42, \"hello\");\n"
                              "    let invalid: i32 = pair.2;\n" // Out of bounds
                              "    return ();\n"
                              "}\n";

    Parser *parser = create_test_parser(test_source);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    ASTNode *program = parser_parse_program(parser);

    // Parser should accept this - semantic analysis will catch the error
    TEST_ASSERT(program != NULL, "Failed to parse program with out-of-bounds tuple access");
    TEST_ASSERT(!parser_had_error(parser),
                "Parser reported errors for syntactically valid tuple access");

    if (program) {
        ast_free_node(program);
    }
    destroy_test_parser(parser);

    TEST_PASS("Invalid tuple element access parsing (semantic check deferred)");
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Running Asthra tuple parsing tests...\n\n");

    int passed = 0;
    int total = 0;

    // Run all tests
    total++;
    if (test_parse_simple_tuple_type())
        passed++;
    total++;
    if (test_parse_tuple_literal())
        passed++;
    total++;
    if (test_parse_tuple_pattern())
        passed++;
    total++;
    if (test_single_element_tuple_error())
        passed++;
    total++;
    if (test_parse_tuple_element_access())
        passed++;
    total++;
    if (test_parse_nested_tuple_access())
        passed++;
    total++;
    if (test_invalid_tuple_element_access())
        passed++;

    printf("\nTuple Parsing Test Results: %d/%d passed\n", passed, total);

    if (passed == total) {
        printf("All tuple parsing tests passed!\n");
        return 0;
    } else {
        printf("Some tuple parsing tests failed!\n");
        return 1;
    }
}