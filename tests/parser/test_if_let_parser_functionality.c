/**
 * Asthra Programming Language
 * If-Let Parser Functionality Test
 *
 * This test validates the actual if-let parser functionality by testing
 * the parse_if_let_stmt and parse_if_stmt functions directly.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parser includes
#include "ast_node.h"
#include "grammar_statements.h"
#include "lexer.h"
#include "parser_core.h"

// Test result tracking
static int tests_passed = 0;
static int tests_total = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_total++;                                                                             \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("✓ %s\n", message);                                                             \
        } else {                                                                                   \
            tests_failed++;                                                                        \
            printf("✗ %s\n", message);                                                             \
        }                                                                                          \
    } while (0)

#define TEST_SECTION(name)                                                                         \
    do {                                                                                           \
        printf("\n=== %s ===\n", name);                                                            \
    } while (0)

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Create a simple parser from source code
 */
static Parser *create_test_parser(const char *source) {
    if (!source)
        return NULL;

    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    return parser;
}

/**
 * Cleanup parser and associated resources
 */
static void destroy_test_parser(Parser *parser) {
    if (parser) {
        parser_destroy(parser);
    }
}

// =============================================================================
// PARSER FUNCTIONALITY TESTS
// =============================================================================

/**
 * Test: Basic If-Let Parsing
 * Tests if the parser can handle basic if-let statements
 */
static bool test_basic_if_let_parsing(void) {
    printf("Testing basic if-let parsing functionality... ");

    const char *source = "if let x = y { return x; }";

    Parser *parser = create_test_parser(source);
    if (!parser) {
        TEST_ASSERT(false, "Failed to create parser");
        return false;
    }

    // Try to parse as if statement (should detect if-let and delegate)
    ASTNode *stmt = parse_if_stmt(parser);
    bool success = (stmt != NULL && stmt->type == AST_IF_LET_STMT);

    if (stmt) {
        ast_free_node(stmt);
    }
    destroy_test_parser(parser);

    TEST_ASSERT(success, "Basic if-let parsing works");
    return success;
}

/**
 * Test: If-Let with Else Clause
 * Tests if the parser can handle if-let statements with else clauses
 */
static bool test_if_let_with_else_parsing(void) {
    printf("Testing if-let with else clause parsing... ");

    const char *source = "if let x = y { return x; } else { return 0; }";

    Parser *parser = create_test_parser(source);
    if (!parser) {
        TEST_ASSERT(false, "Failed to create parser");
        return false;
    }

    ASTNode *stmt = parse_if_stmt(parser);
    bool success = (stmt != NULL && stmt->type == AST_IF_LET_STMT &&
                    stmt->data.if_let_stmt.else_block != NULL);

    if (stmt) {
        ast_free_node(stmt);
    }
    destroy_test_parser(parser);

    TEST_ASSERT(success, "If-let with else clause parsing works");
    return success;
}

/**
 * Test: Regular If Statement Still Works
 * Tests that regular if statements are not broken by if-let integration
 */
static bool test_regular_if_statement_parsing(void) {
    printf("Testing regular if statement parsing... ");

    const char *source = "if x > 0 { return x; }";

    Parser *parser = create_test_parser(source);
    if (!parser) {
        TEST_ASSERT(false, "Failed to create parser");
        return false;
    }

    ASTNode *stmt = parse_if_stmt(parser);
    bool success = (stmt != NULL && stmt->type == AST_IF_STMT);

    if (stmt) {
        ast_free_node(stmt);
    }
    destroy_test_parser(parser);

    TEST_ASSERT(success, "Regular if statement parsing still works");
    return success;
}

/**
 * Test: Parser Error Handling
 * Tests that the parser handles malformed if-let statements gracefully
 */
static bool test_parser_error_handling(void) {
    printf("Testing parser error handling... ");

    // Malformed if-let (missing assignment)
    const char *source = "if let x { return x; }";

    Parser *parser = create_test_parser(source);
    if (!parser) {
        TEST_ASSERT(false, "Failed to create parser");
        return false;
    }

    ASTNode *stmt = parse_if_stmt(parser);
    bool success = (stmt == NULL); // Should fail to parse

    if (stmt) {
        ast_free_node(stmt);
    }
    destroy_test_parser(parser);

    TEST_ASSERT(success, "Parser error handling works for malformed if-let");
    return success;
}

/**
 * Test: AST Structure Validation
 * Tests that parsed if-let statements have correct AST structure
 */
static bool test_ast_structure_validation(void) {
    printf("Testing AST structure validation... ");

    const char *source = "if let x = y { return x; }";

    Parser *parser = create_test_parser(source);
    if (!parser) {
        TEST_ASSERT(false, "Failed to create parser");
        return false;
    }

    ASTNode *stmt = parse_if_stmt(parser);
    bool success = false;

    if (stmt && stmt->type == AST_IF_LET_STMT) {
        // Check that all required components are present
        success =
            (stmt->data.if_let_stmt.pattern != NULL && stmt->data.if_let_stmt.expression != NULL &&
             stmt->data.if_let_stmt.then_block != NULL);
    }

    if (stmt) {
        ast_free_node(stmt);
    }
    destroy_test_parser(parser);

    TEST_ASSERT(success, "AST structure is correct for if-let statements");
    return success;
}

/**
 * Test: Memory Management
 * Tests that AST nodes are properly cleaned up
 */
static bool test_memory_management(void) {
    printf("Testing memory management... ");

    const char *source = "if let x = y { return x; } else { return 0; }";

    Parser *parser = create_test_parser(source);
    if (!parser) {
        TEST_ASSERT(false, "Failed to create parser");
        return false;
    }

    ASTNode *stmt = parse_if_stmt(parser);
    bool success = (stmt != NULL);

    // Test that we can safely free the AST
    if (stmt) {
        ast_free_node(stmt); // Should not crash
    }
    destroy_test_parser(parser);

    TEST_ASSERT(success, "Memory management works correctly");
    return success;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Asthra If-Let Parser Functionality Test\n");
    printf("========================================\n");

    TEST_SECTION("Phase 4.1: Basic Parser Functionality");
    test_basic_if_let_parsing();
    test_if_let_with_else_parsing();
    test_regular_if_statement_parsing();

    TEST_SECTION("Phase 4.2: Error Handling and Validation");
    test_parser_error_handling();
    test_ast_structure_validation();

    TEST_SECTION("Phase 4.3: Memory Management");
    test_memory_management();

    // Print summary
    printf("\n=== TEST SUMMARY ===\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("\n🎉 ALL PARSER TESTS PASSED!\n");
        printf("\nIf-Let Parser Functionality: ✅ FULLY OPERATIONAL\n");
        printf("- Basic if-let parsing: ✅ WORKING\n");
        printf("- If-let with else: ✅ WORKING\n");
        printf("- Regular if statements: ✅ WORKING\n");
        printf("- Error handling: ✅ WORKING\n");
        printf("- AST structure: ✅ CORRECT\n");
        printf("- Memory management: ✅ SAFE\n");
        printf("\nPhase 4 Parser Testing: ✅ COMPLETE\n");
        return 0;
    } else {
        printf("\n❌ %d parser test(s) failed.\n", tests_failed);
        return 1;
    }
}
