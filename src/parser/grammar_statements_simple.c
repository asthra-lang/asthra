/**
 * Asthra Programming Language Compiler - Simple Statement Grammar Productions
 * Basic statements including return, expression, break, and continue
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "grammar_statements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// SIMPLE STATEMENT PARSING
// =============================================================================

ASTNode *parse_return_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_RETURN)) {
        return NULL;
    }

    // Always require expression
    ASTNode *expression = parse_expr(parser);
    if (!expression) {
        report_error(parser, "Expected expression after 'return' keyword. "
                             "Void returns must use 'return ();'");
        return NULL;
    }

    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        ast_free_node(expression);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_RETURN_STMT, start_loc);
    if (!node) {
        ast_free_node(expression);
        return NULL;
    }

    node->data.return_stmt.expression = expression; // Always non-NULL in current version

    return node;
}

ASTNode *parse_expr_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // First parse as expression
    ASTNode *expression = parse_expr(parser);
    if (!expression)
        return NULL;

    // Check if this is actually an assignment that we missed
    if (match_token(parser, TOKEN_ASSIGN)) {
        // This is an assignment, not an expression statement
        // The expression we parsed should be a valid lvalue
        advance_token(parser); // consume '='

        ASTNode *rvalue = parse_expr(parser);
        if (!rvalue) {
            ast_free_node(expression);
            return NULL;
        }

        if (!expect_token(parser, TOKEN_SEMICOLON)) {
            ast_free_node(expression);
            ast_free_node(rvalue);
            return NULL;
        }

        // Create assignment node
        ASTNode *assign = ast_create_node(AST_ASSIGNMENT, start_loc);
        if (!assign) {
            ast_free_node(expression);
            ast_free_node(rvalue);
            return NULL;
        }

        assign->data.assignment.target = expression; // Use the expression as lvalue
        assign->data.assignment.value = rvalue;

        return assign;
    }

    // Not an assignment, expect semicolon for expression statement
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        ast_free_node(expression);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_EXPR_STMT, start_loc);
    if (!node) {
        ast_free_node(expression);
        return NULL;
    }

    node->data.expr_stmt.expression = expression;

    return node;
}

ASTNode *parse_assign_stmt(Parser *parser) {
    // This function is kept for backward compatibility
    // The grammar doesn't have AssignStmt as a separate production
    // Assignment is handled as an expression
    // The parsing logic is integrated into parse_statement and parse_expr_stmt
    report_error(parser, "Direct assignment statement parsing is not supported. "
                         "Assignments are parsed as expressions or in statement context.");
    return NULL;
}

ASTNode *parse_break_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect 'break' keyword
    if (!expect_token(parser, TOKEN_BREAK)) {
        return NULL;
    }

    // Expect semicolon
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        report_error(parser, "Expected ';' after 'break' statement");
        return NULL;
    }

    // Create the break statement node
    ASTNode *node = ast_create_node(AST_BREAK_STMT, start_loc);
    if (!node) {
        return NULL;
    }

    return node;
}

ASTNode *parse_continue_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect 'continue' keyword
    if (!expect_token(parser, TOKEN_CONTINUE)) {
        return NULL;
    }

    // Expect semicolon
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        report_error(parser, "Expected ';' after 'continue' statement");
        return NULL;
    }

    // Create the continue statement node
    ASTNode *node = ast_create_node(AST_CONTINUE_STMT, start_loc);
    if (!node) {
        return NULL;
    }

    return node;
}