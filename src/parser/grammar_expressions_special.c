/**
 * Asthra Programming Language Compiler - Special Expressions
 * Parsing of special expressions: sizeof, await, recv, parenthesized expressions
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
// SPECIAL EXPRESSIONS
// =============================================================================

ASTNode *parse_parenthesized_expr(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }

    // Check for unit expression: ()
    if (match_token(parser, TOKEN_RIGHT_PAREN)) {
        advance_token(parser);

        // Create a unit literal node for ()
        ASTNode *node = ast_create_node(AST_UNIT_LITERAL, start_loc);
        if (!node)
            return NULL;

        return node;
    }

    // Parse first expression
    ASTNode *first_expr = parse_expr(parser);
    if (!first_expr)
        return NULL;

    // Check if this is a tuple literal by looking for comma
    if (match_token(parser, TOKEN_COMMA)) {
        // This is a tuple literal
        ASTNodeList *elements = ast_node_list_create(2);
        if (!elements) {
            ast_free_node(first_expr);
            return NULL;
        }

        // Add first element
        ast_node_list_add(&elements, first_expr);

        // Consume comma
        advance_token(parser);

        // Parse remaining elements
        do {
            ASTNode *element = parse_expr(parser);
            if (!element) {
                // Clean up
                for (size_t i = 0; i < elements->count; i++) {
                    ast_free_node(elements->nodes[i]);
                }
                ast_node_list_destroy(elements);
                return NULL;
            }

            ast_node_list_add(&elements, element);

            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser) && !match_token(parser, TOKEN_RIGHT_PAREN));

        // Expect closing paren
        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            // Clean up
            for (size_t i = 0; i < elements->count; i++) {
                ast_free_node(elements->nodes[i]);
            }
            ast_node_list_destroy(elements);
            return NULL;
        }

        // Create tuple literal node
        ASTNode *node = ast_create_node(AST_TUPLE_LITERAL, start_loc);
        if (!node) {
            // Clean up
            for (size_t i = 0; i < elements->count; i++) {
                ast_free_node(elements->nodes[i]);
            }
            ast_node_list_destroy(elements);
            return NULL;
        }

        node->data.tuple_literal.elements = elements;
        return node;
    }

    // Not a tuple, just a parenthesized expression
    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        ast_free_node(first_expr);
        return NULL;
    }

    return first_expr;
}

ASTNode *parse_sizeof(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_SIZEOF)) {
        return NULL;
    }

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }

    ASTNode *type = parse_type(parser);
    if (!type)
        return NULL;

    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        ast_free_node(type);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_UNARY_EXPR, start_loc);
    if (!node) {
        ast_free_node(type);
        return NULL;
    }

    node->data.unary_expr.operator= UNOP_SIZEOF;
    node->data.unary_expr.operand = type;

    return node;
}

ASTNode *parse_await_expr(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_AWAIT)) {
        return NULL;
    }

    ASTNode *expr = parse_unary(parser);
    if (!expr)
        return NULL;

    ASTNode *node = ast_create_node(AST_AWAIT_EXPR, start_loc);
    if (!node) {
        ast_free_node(expr);
        return NULL;
    }

    node->data.await_expr.task_handle_expr = expr;
    node->data.await_expr.timeout_expr = NULL;

    return node;
}

/*
 * Note: parse_recv_expr moved to stdlib as Tier 3 concurrency feature
 * This function is preserved for reference but should not be used in core language
 *
ASTNode *parse_recv_expr(Parser *parser) {
    if (!parser) return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_RECV)) {
        return NULL;
    }

    ASTNode *expr = parse_unary(parser);
    if (!expr) return NULL;

    ASTNode *node = ast_create_node(AST_RECV_EXPR, start_loc);
    if (!node) {
        ast_free_node(expr);
        return NULL;
    }

    node->data.recv_expr.channel_expr = expr;
    node->data.recv_expr.timeout_expr = NULL;

    return node;
}
*/
