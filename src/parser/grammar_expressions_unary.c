/**
 * Asthra Programming Language Compiler - Unary Expressions
 * Unary operators (prefix operators like -, !, ~, *, &)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "parser_token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// UNARY EXPRESSIONS
// =============================================================================

ASTNode *parse_unary(Parser *parser) {
    if (!parser)
        return NULL;

    // Handle special unary expressions first
    if (match_token(parser, TOKEN_SIZEOF)) {
        return parse_sizeof(parser);
    } else if (match_token(parser, TOKEN_AWAIT)) {
        return parse_await_expr(parser);
    } // Note: TOKEN_RECV moved to stdlib (Tier 3)
    // else if (match_token(parser, TOKEN_RECV)) {
    //     return parse_recv_expr(parser);
    // }

    // Parse UnaryPrefix PostfixExpr according to new grammar
    return parse_unary_prefix(parser);
}

ASTNode *parse_unary_prefix(Parser *parser) {
    if (!parser)
        return NULL;

    // Parse optional LogicalPrefix
    if (match_token(parser, TOKEN_MINUS) || match_token(parser, TOKEN_LOGICAL_NOT) ||
        match_token(parser, TOKEN_BITWISE_NOT)) {
        SourceLocation op_loc = parser->current_token.location;
        UnaryOperator logical_op;

        // Determine which operator based on current token
        if (parser->current_token.type == TOKEN_MINUS) {
            logical_op = UNOP_MINUS;
        } else if (parser->current_token.type == TOKEN_LOGICAL_NOT) {
            logical_op = UNOP_NOT;
        } else { // TOKEN_BITWISE_NOT
            logical_op = UNOP_BITWISE_NOT;
        }

        advance_token(parser);

        // Parse optional PointerPrefix or PostfixExpr
        ASTNode *operand = parse_pointer_prefix_or_postfix(parser);
        if (!operand)
            return NULL;

        ASTNode *logical_expr = ast_create_node(AST_UNARY_EXPR, op_loc);
        if (!logical_expr) {
            ast_free_node(operand);
            return NULL;
        }

        logical_expr->data.unary_expr.operator= logical_op;
        logical_expr->data.unary_expr.operand = operand;

        return logical_expr;
    }

    // No logical prefix, try pointer prefix or postfix
    return parse_pointer_prefix_or_postfix(parser);
}

ASTNode *parse_pointer_prefix_or_postfix(Parser *parser) {
    if (!parser)
        return NULL;

    // Parse optional PointerPrefix
    if (match_token(parser, TOKEN_MULTIPLY) || match_token(parser, TOKEN_BITWISE_AND)) {
        SourceLocation op_loc = parser->current_token.location;
        UnaryOperator pointer_op;

        // Determine which operator based on current token
        if (parser->current_token.type == TOKEN_MULTIPLY) {
            pointer_op = UNOP_DEREF;
        } else { // TOKEN_BITWISE_AND
            pointer_op = UNOP_ADDRESS_OF;
        }

        advance_token(parser);

        // Special case: if the next token is also a unary operator, we need to parse it as such
        // This handles cases like *&var where & is also a unary operator
        ASTNode *operand = NULL;
        if (match_token(parser, TOKEN_BITWISE_AND) || match_token(parser, TOKEN_MULTIPLY) ||
            match_token(parser, TOKEN_MINUS) || match_token(parser, TOKEN_LOGICAL_NOT) ||
            match_token(parser, TOKEN_BITWISE_NOT)) {
            // Recursively parse the nested unary expression
            operand = parse_unary_prefix(parser);
        } else {
            // Parse PostfixExpr for non-unary operands
            operand = parse_postfix_expr(parser);
        }

        if (!operand) {
            report_error(parser, "Expected expression after unary operator");
            return NULL;
        }

        ASTNode *pointer_expr = ast_create_node(AST_UNARY_EXPR, op_loc);
        if (!pointer_expr) {
            ast_free_node(operand);
            return NULL;
        }

        pointer_expr->data.unary_expr.operator= pointer_op;
        pointer_expr->data.unary_expr.operand = operand;

        return pointer_expr;
    }

    // No pointer prefix, parse PostfixExpr directly
    return parse_postfix_expr(parser);
}