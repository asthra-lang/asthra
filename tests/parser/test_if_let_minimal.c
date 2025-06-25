/**
 * Minimal If-Let Statement Parsing Test
 *
 * Tests just the core if-let parsing logic
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include only essential headers
#include "ast_node.h"
#include "lexer.h"
#include "parser_core.h"

// Test result tracking
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

// Forward declarations for functions we need
extern ASTNode *parse_if_stmt(Parser *parser);
extern ASTNode *parse_if_let_stmt(Parser *parser);

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
    printf("=== Minimal If-Let Parsing Test ===\n\n");

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

    // Test 2: Regular if statement still works
    printf("Test 2: Regular if statement\n");
    const char *source2 = "if x > 0 { return x; }";

    Parser *parser2 = create_simple_parser(source2);
    TEST_ASSERT(parser2 != NULL, "Parser creation");

    if (parser2) {
        ASTNode *stmt = parse_if_stmt(parser2);
        TEST_ASSERT(stmt != NULL, "If statement parsing");
        TEST_ASSERT(stmt && stmt->type == AST_IF_STMT, "Regular if statement");

        if (stmt) {
            ast_free_node(stmt);
        }

        destroy_simple_parser(parser2);
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
