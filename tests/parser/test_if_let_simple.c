/**
 * Simple If-Let Statement Parsing Test
 *
 * Minimal test to verify if-let parsing works
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include parser headers
#include "ast_node.h"
#include "grammar_statements.h"
#include "lexer.h"
#include "parser_core.h"

// Simple test result tracking
static int tests_passed = 0;
static int tests_total = 0;

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_total++;                                                                             \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("✓ %s\n", message);                                                             \
        } else {                                                                                   \
            printf("✗ %s\n", message);                                                             \
        }                                                                                          \
    } while (0)

// Simple parser creation for testing
static Parser *create_simple_parser(const char *source) {
    if (!source)
        return NULL;

    // Create lexer first
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer)
        return NULL;

    // Create parser
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    return parser;
}

static void destroy_simple_parser(Parser *parser) {
    if (parser) {
        parser_destroy(parser);
    }
}

int main(void) {
    printf("=== Simple If-Let Parsing Test ===\n\n");

    // Test 1: If statement integration with if-let
    printf("Test 1: If statement integration (if-let)\n");
    const char *source1 = "if let Option.Some(x) = opt { return x; }";

    Parser *parser1 = create_simple_parser(source1);
    TEST_ASSERT(parser1 != NULL, "Parser creation");

    if (parser1) {
        // This should detect if-let and handle it properly
        ASTNode *stmt = parse_if_stmt(parser1);
        TEST_ASSERT(stmt != NULL, "If statement parsing (should delegate to if-let)");
        TEST_ASSERT(stmt && stmt->type == AST_IF_LET_STMT, "Delegated to if-let correctly");

        if (stmt) {
            TEST_ASSERT(stmt->data.if_let_stmt.pattern != NULL, "Pattern exists");
            TEST_ASSERT(stmt->data.if_let_stmt.expression != NULL, "Expression exists");
            TEST_ASSERT(stmt->data.if_let_stmt.then_block != NULL, "Then block exists");
            TEST_ASSERT(stmt->data.if_let_stmt.else_block == NULL, "No else block");

            ast_free_node(stmt);
        }

        destroy_simple_parser(parser1);
    }

    printf("\n");

    // Test 2: If statement integration with if-let and else
    printf("Test 2: If statement integration (if-let with else)\n");
    const char *source2 = "if let Result.Ok(data) = result { use(data); } else { error(); }";

    Parser *parser2 = create_simple_parser(source2);
    TEST_ASSERT(parser2 != NULL, "Parser creation");

    if (parser2) {
        ASTNode *stmt = parse_if_stmt(parser2);
        TEST_ASSERT(stmt != NULL, "If statement parsing (should delegate to if-let)");
        TEST_ASSERT(stmt && stmt->type == AST_IF_LET_STMT, "Delegated to if-let correctly");

        if (stmt) {
            TEST_ASSERT(stmt->data.if_let_stmt.pattern != NULL, "Pattern exists");
            TEST_ASSERT(stmt->data.if_let_stmt.expression != NULL, "Expression exists");
            TEST_ASSERT(stmt->data.if_let_stmt.then_block != NULL, "Then block exists");
            TEST_ASSERT(stmt->data.if_let_stmt.else_block != NULL, "Else block exists");

            ast_free_node(stmt);
        }

        destroy_simple_parser(parser2);
    }

    printf("\n");

    // Test 3: Regular if statement still works
    printf("Test 3: Regular if statement\n");
    const char *source3 = "if x > 0 { return x; }";

    Parser *parser3 = create_simple_parser(source3);
    TEST_ASSERT(parser3 != NULL, "Parser creation");

    if (parser3) {
        ASTNode *stmt = parse_if_stmt(parser3);
        TEST_ASSERT(stmt != NULL, "If statement parsing");
        TEST_ASSERT(stmt && stmt->type == AST_IF_STMT, "Regular if statement");

        if (stmt) {
            ast_free_node(stmt);
        }

        destroy_simple_parser(parser3);
    }

    printf("\n");

    // Test 4: Direct parse_if_let_stmt with pre-consumed 'if' token
    printf("Test 4: Direct if-let parsing (after consuming 'if')\n");
    const char *source4 = "if let Option.Some(value) = optional { return value; }";

    Parser *parser4 = create_simple_parser(source4);
    TEST_ASSERT(parser4 != NULL, "Parser creation");

    if (parser4) {
        // Manually consume the 'if' token first, then call parse_if_let_stmt
        // Actually, let's skip this test for now since parse_if_let_stmt expects both tokens
        // We'll focus on the integration test which is the main use case
        printf("  (Skipping direct parse_if_let_stmt test - integration test covers this)\n");
        destroy_simple_parser(parser4);
    }

    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, tests_total);

    if (tests_passed == tests_total) {
        printf("✅ All tests PASSED!\n");
        return 0;
    } else {
        printf("❌ Some tests FAILED!\n");
        return 1;
    }
}
