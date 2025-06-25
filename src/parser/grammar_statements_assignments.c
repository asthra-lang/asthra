/**
 * Asthra Programming Language Compiler - Assignment Statement Grammar Productions
 * L-value parsing for assignments
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
// ASSIGNMENT L-VALUE PARSING
// =============================================================================

ASTNode *parse_lvalue(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Parse SimpleIdent
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected identifier in lvalue");
        return NULL;
    }

    char *name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    ASTNode *lvalue = ast_create_node(AST_IDENTIFIER, start_loc);
    if (!lvalue) {
        free(name);
        return NULL;
    }

    lvalue->data.identifier.name = name;

    // Parse LValueSuffix*
    while (true) {
        if (match_token(parser, TOKEN_DOT)) {
            // Field access: '.' SimpleIdent
            advance_token(parser);

            if (!match_token(parser, TOKEN_IDENTIFIER)) {
                report_error(parser, "Expected field name after '.'");
                ast_free_node(lvalue);
                return NULL;
            }

            char *field_name = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);

            ASTNode *field_access =
                ast_create_node(AST_FIELD_ACCESS, parser->current_token.location);
            if (!field_access) {
                free(field_name);
                ast_free_node(lvalue);
                return NULL;
            }

            field_access->data.field_access.object = lvalue;
            field_access->data.field_access.field_name = field_name;
            lvalue = field_access;

        } else if (match_token(parser, TOKEN_LEFT_BRACKET)) {
            // Array indexing: '[' Expr ']'
            advance_token(parser);

            ASTNode *index = parse_expr(parser);
            if (!index) {
                ast_free_node(lvalue);
                return NULL;
            }

            if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
                ast_free_node(index);
                ast_free_node(lvalue);
                return NULL;
            }

            ASTNode *array_access =
                ast_create_node(AST_INDEX_ACCESS, parser->current_token.location);
            if (!array_access) {
                ast_free_node(index);
                ast_free_node(lvalue);
                return NULL;
            }

            array_access->data.index_access.array = lvalue;
            array_access->data.index_access.index = index;
            lvalue = array_access;

        } else if (match_token(parser, TOKEN_MULTIPLY)) {
            // Pointer dereference: '*'
            advance_token(parser);

            ASTNode *deref = ast_create_node(AST_UNARY_EXPR, parser->current_token.location);
            if (!deref) {
                ast_free_node(lvalue);
                return NULL;
            }

            deref->data.unary_expr.operator= UNOP_DEREF;
            deref->data.unary_expr.operand = lvalue;
            lvalue = deref;

        } else {
            // No more suffixes
            break;
        }
    }

    return lvalue;
}