/**
 * Asthra Programming Language
 * If-Let Statement Parsing Tests
 * 
 * Tests for if-let statement parsing implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "grammar_statements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

/**
 * Test: Basic if-let parsing
 * Verifies that basic if-let statements are parsed correctly
 */
static bool test_basic_if_let_parsing(void) {
    printf("Testing basic if-let parsing... ");
    
    const char *source = 
        "if let Option.Some(value) = get_option() {\n"
        "    process_value(value);\n"
        "}";
    
    Parser *parser = create_test_parser(source);
    if (!parser) {
        printf("FAIL - Failed to create parser\n");
        return false;
    }
    
    ASTNode *if_let_stmt = parse_if_let_stmt(parser);
    if (!if_let_stmt) {
        printf("FAIL - Failed to parse if-let statement\n");
        destroy_test_parser(parser);
        return false;
    }
    
    if (if_let_stmt->type != AST_IF_LET_STMT) {
        printf("FAIL - Expected AST_IF_LET_STMT, got %d\n", if_let_stmt->type);
        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    // Validate AST structure
    if (!if_let_stmt->data.if_let_stmt.pattern) {
        printf("FAIL - Pattern is NULL\n");
        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    if (!if_let_stmt->data.if_let_stmt.expression) {
        printf("FAIL - Expression is NULL\n");
        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    if (!if_let_stmt->data.if_let_stmt.then_block) {
        printf("FAIL - Then block is NULL\n");
        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    // else_block should be NULL for this test
    if (if_let_stmt->data.if_let_stmt.else_block) {
        printf("FAIL - Else block should be NULL\n");
        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    ast_free_node(if_let_stmt);
    destroy_test_parser(parser);
    printf("PASS\n");
    return true;
}

/**
 * Test: If-let with else clause
 * Verifies that if-let statements with else clauses are parsed correctly
 */
static bool test_if_let_with_else(void) {
    printf("Testing if-let with else clause... ");
    
    const char *source = 
        "if let Result.Ok(data) = parse_input() {\n"
        "    process_data(data);\n"
        "} else {\n"
        "    handle_error();\n"
        "}";
    
    Parser *parser = create_test_parser(source);
    if (!parser) {
        printf("FAIL - Failed to create parser\n");
        return false;
    }
    
    ASTNode *if_let_stmt = parse_if_let_stmt(parser);
    if (!if_let_stmt) {
        printf("FAIL - Failed to parse if-let statement\n");
        destroy_test_parser(parser);
        return false;
    }
    
    if (if_let_stmt->type != AST_IF_LET_STMT) {
        printf("FAIL - Expected AST_IF_LET_STMT\n");
        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    // Validate that else block exists
    if (!if_let_stmt->data.if_let_stmt.else_block) {
        printf("FAIL - Else block is NULL\n");
        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    ast_free_node(if_let_stmt);
    destroy_test_parser(parser);
    printf("PASS\n");
    return true;
}

/**
 * Test: If statement integration
 * Verifies that parse_if_stmt correctly delegates to parse_if_let_stmt
 */
static bool test_if_statement_integration(void) {
    printf("Testing if statement integration... ");
    
    const char *source = 
        "if let Option.Some(value) = optional {\n"
        "    return value;\n"
        "}";
    
    Parser *parser = create_test_parser(source);
    if (!parser) {
        printf("FAIL - Failed to create parser\n");
        return false;
    }
    
    // This should be parsed as an if-let statement through parse_if_stmt
    ASTNode *stmt = parse_if_stmt(parser);
    if (!stmt) {
        printf("FAIL - Failed to parse if statement\n");
        destroy_test_parser(parser);
        return false;
    }
    
    if (stmt->type != AST_IF_LET_STMT) {
        printf("FAIL - Expected AST_IF_LET_STMT, got %d\n", stmt->type);
        ast_free_node(stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    ast_free_node(stmt);
    destroy_test_parser(parser);
    printf("PASS\n");
    return true;
}

/**
 * Test: Regular if statement still works
 * Verifies that regular if statements are not broken by if-let integration
 */
static bool test_regular_if_statement(void) {
    printf("Testing regular if statement still works... ");
    
    const char *source = 
        "if x > 0 {\n"
        "    return x;\n"
        "} else {\n"
        "    return 0;\n"
        "}";
    
    Parser *parser = create_test_parser(source);
    if (!parser) {
        printf("FAIL - Failed to create parser\n");
        return false;
    }
    
    ASTNode *stmt = parse_if_stmt(parser);
    if (!stmt) {
        printf("FAIL - Failed to parse if statement\n");
        destroy_test_parser(parser);
        return false;
    }
    
    if (stmt->type != AST_IF_STMT) {
        printf("FAIL - Expected AST_IF_STMT, got %d\n", stmt->type);
        ast_free_node(stmt);
        destroy_test_parser(parser);
        return false;
    }
    
    ast_free_node(stmt);
    destroy_test_parser(parser);
    printf("PASS\n");
    return true;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== If-Let Statement Parsing Tests ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Run tests
    total++; if (test_basic_if_let_parsing()) passed++;
    total++; if (test_if_let_with_else()) passed++;
    total++; if (test_if_statement_integration()) passed++;
    total++; if (test_regular_if_statement()) passed++;
    
    // Print results
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    if (passed == total) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
} 
