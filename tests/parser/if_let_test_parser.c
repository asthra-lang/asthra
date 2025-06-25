/**
 * Asthra Programming Language
 * If-Let Statement Testing - Parser Tests Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 4.1: Parser implementation tests for if-let statements
 */

#include "if_let_test_parser.h"

// =============================================================================
// PHASE 4.1: PARSER TESTS IMPLEMENTATION
// =============================================================================

bool test_basic_if_let_parsing(void) {
    const char *source = "if let Option.Some(value) = get_option() {\n"
                         "    process_value(value);\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    ASTNode *if_let_stmt = parse_if_stmt(parser);
    bool success = (if_let_stmt != NULL && if_let_stmt->type == AST_IF_LET_STMT &&
                    validate_if_let_ast(if_let_stmt));

    if (if_let_stmt)
        ast_free_node(if_let_stmt);
    destroy_test_parser(parser);
    return success;
}

bool test_if_let_with_else(void) {
    const char *source = "if let Result.Ok(data) = parse_input() {\n"
                         "    process_data(data);\n"
                         "} else {\n"
                         "    handle_error();\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    ASTNode *if_let_stmt = parse_if_stmt(parser);
    bool success =
        (if_let_stmt != NULL && if_let_stmt->type == AST_IF_LET_STMT &&
         validate_if_let_ast(if_let_stmt) && if_let_stmt->data.if_let_stmt.else_block != NULL);

    if (if_let_stmt)
        ast_free_node(if_let_stmt);
    destroy_test_parser(parser);
    return success;
}

bool test_if_statement_integration(void) {
    const char *source = "if let Option.Some(value) = optional {\n"
                         "    return value;\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    // This should be parsed as an if-let statement through parse_if_stmt
    ASTNode *stmt = parse_if_stmt(parser);
    bool success = (stmt != NULL && stmt->type == AST_IF_LET_STMT);

    if (stmt)
        ast_free_node(stmt);
    destroy_test_parser(parser);
    return success;
}

bool test_regular_if_statement(void) {
    const char *source = "if x > 0 {\n"
                         "    return x;\n"
                         "} else {\n"
                         "    return 0;\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    ASTNode *stmt = parse_if_stmt(parser);
    bool success = (stmt != NULL && stmt->type == AST_IF_STMT);

    if (stmt)
        ast_free_node(stmt);
    destroy_test_parser(parser);
    return success;
}

bool test_nested_if_let(void) {
    const char *source = "if let Option.Some(result) = get_result() {\n"
                         "    if let Result.Ok(data) = result {\n"
                         "        process_data(data);\n"
                         "    }\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    ASTNode *outer_stmt = parse_if_stmt(parser);
    bool success = (outer_stmt != NULL && outer_stmt->type == AST_IF_LET_STMT &&
                    validate_if_let_ast(outer_stmt));

    if (outer_stmt)
        ast_free_node(outer_stmt);
    destroy_test_parser(parser);
    return success;
}

bool test_malformed_if_let_error_handling(void) {
    // Test cases that should fail to parse
    const char *malformed_cases[] = {"if let = value { }",       // Missing pattern
                                     "if let pattern value { }", // Missing '='
                                     "if let pattern = { }",     // Missing expression
                                     "if let pattern = value",   // Missing block
                                     NULL};

    bool all_failed_correctly = true;

    for (int i = 0; malformed_cases[i] != NULL; i++) {
        Parser *parser = create_test_parser(malformed_cases[i]);
        if (!parser)
            continue;

        ASTNode *stmt = parse_if_stmt(parser);
        if (stmt != NULL) {
            // This should have failed to parse
            all_failed_correctly = false;
            ast_free_node(stmt);
        }

        destroy_test_parser(parser);
    }

    return all_failed_correctly;
}

bool run_parser_tests(void) {
    TEST_SECTION("Phase 4.1: Parser Tests");
    TEST_ASSERT(test_basic_if_let_parsing(), "Basic if-let parsing");
    TEST_ASSERT(test_if_let_with_else(), "If-let with else clause");
    TEST_ASSERT(test_if_statement_integration(), "If statement integration");
    TEST_ASSERT(test_regular_if_statement(), "Regular if statement still works");
    TEST_ASSERT(test_nested_if_let(), "Nested if-let statements");
    TEST_ASSERT(test_malformed_if_let_error_handling(), "Error handling for malformed if-let");

    return (tests_failed == 0);
}
