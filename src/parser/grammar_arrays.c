/**
 * Asthra Programming Language Compiler - Array Literal Parsing
 * Parsing of array literals with support for:
 * - Basic array literals: [1, 2, 3]
 * - Empty arrays using 'none': [none]
 * - Repeated element syntax: [value; count]
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "parser_ast_helpers.h"
#include <stdlib.h>
#include <string.h>

// Forward declaration for parsing const expressions
static ASTNode *parse_const_expr_for_array(Parser *parser);

/**
 * Parse a constant expression for array size/count
 * For now, this accepts simple expressions that should be constant
 */
static ASTNode *parse_const_expr_for_array(Parser *parser) {
    // For the initial implementation, we'll parse a regular expression
    // and let semantic analysis verify it's actually constant
    return parse_expr(parser);
}

/**
 * Parse array literal - Parse according to v1.23 grammar:
 * ArrayContent <- RepeatedElements / ArrayElements / 'none'
 * RepeatedElements <- ConstExpr ';' ConstExpr
 * ArrayElements <- Expr (',' Expr)*
 */
ASTNode *parse_array_literal(Parser *parser) {
    if (!parser || !match_token(parser, TOKEN_LEFT_BRACKET))
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    advance_token(parser); // consume '['

    // Check for explicit 'none' marker for empty arrays
    if (match_token(parser, TOKEN_NONE)) {
        advance_token(parser);

        if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
            return NULL;
        }

        ASTNode *array = ast_create_node(AST_ARRAY_LITERAL, start_loc);
        if (array) {
            array->data.array_literal.elements = NULL;
        }
        return array;
    }

    // Check for legacy void (error)
    if (match_token(parser, TOKEN_VOID)) {
        report_error(parser, "Unexpected 'void' in array literal. Use 'none' for empty arrays");
        return NULL;
    }

    // Try to parse as repeated element syntax [value; count]
    ASTNode *first_expr = parse_expr(parser);
    if (!first_expr) {
        return NULL;
    }

    // Check if this is repeated element syntax
    if (match_token(parser, TOKEN_SEMICOLON)) {
        advance_token(parser); // consume ';'

        // Parse the count expression
        ASTNode *count_expr = parse_const_expr_for_array(parser);
        if (!count_expr) {
            ast_free_node(first_expr);
            report_error(parser, "Expected count expression after ';' in repeated array syntax");
            return NULL;
        }

        if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
            ast_free_node(first_expr);
            ast_free_node(count_expr);
            return NULL;
        }

        // Create a repeated array literal node
        // For now, we'll use AST_ARRAY_LITERAL with a special marker
        // In a full implementation, we'd add a new AST node type
        ASTNode *array = ast_create_node(AST_ARRAY_LITERAL, start_loc);
        if (!array) {
            ast_free_node(first_expr);
            ast_free_node(count_expr);
            return NULL;
        }

        // Store both the value and count in a special way
        // For now, we'll create a 2-element list with special semantics
        array->data.array_literal.elements = ast_node_list_create(2);
        if (array->data.array_literal.elements) {
            // Add a marker node to indicate this is a repeated array
            ASTNode *marker = ast_create_node(AST_IDENTIFIER, start_loc);
            if (marker) {
                marker->data.identifier.name = strdup("__repeated_array__");
            }
            ast_node_list_add(&array->data.array_literal.elements, marker);
            ast_node_list_add(&array->data.array_literal.elements, first_expr);
            ast_node_list_add(&array->data.array_literal.elements, count_expr);
        }

        return array;
    }

    // Not repeated syntax, parse as regular array with comma-separated elements
    ASTNode **elements = NULL;
    size_t element_count = 0;
    size_t element_capacity = 4;

    elements = malloc(element_capacity * sizeof(ASTNode *));
    if (!elements) {
        ast_free_node(first_expr);
        return NULL;
    }

    // Add the first expression we already parsed
    elements[element_count++] = first_expr;

    // Parse remaining elements if there's a comma
    if (match_token(parser, TOKEN_COMMA)) {
        advance_token(parser);

        while (!match_token(parser, TOKEN_RIGHT_BRACKET) && !at_end(parser)) {
            ASTNode *element = parse_expr(parser);
            if (!element) {
                for (size_t i = 0; i < element_count; i++) {
                    ast_free_node(elements[i]);
                }
                free(elements);
                return NULL;
            }

            if (element_count >= element_capacity) {
                element_capacity *= 2;
                ASTNode **new_elements = realloc(elements, element_capacity * sizeof(ASTNode *));
                if (!new_elements) {
                    ast_free_node(element);
                    for (size_t i = 0; i < element_count; i++) {
                        ast_free_node(elements[i]);
                    }
                    free(elements);
                    return NULL;
                }
                elements = new_elements;
            }

            elements[element_count++] = element;

            if (!match_token(parser, TOKEN_COMMA)) {
                break;
            } else {
                advance_token(parser);
            }
        }
    }

    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
        for (size_t i = 0; i < element_count; i++) {
            ast_free_node(elements[i]);
        }
        free(elements);
        return NULL;
    }

    ASTNode *array = ast_create_node(AST_ARRAY_LITERAL, start_loc);
    if (!array) {
        for (size_t i = 0; i < element_count; i++) {
            ast_free_node(elements[i]);
        }
        free(elements);
        return NULL;
    }

    // Convert elements array to ASTNodeList
    array->data.array_literal.elements = ast_node_list_create(element_count);
    if (array->data.array_literal.elements) {
        for (size_t i = 0; i < element_count; i++) {
            ast_node_list_add(&array->data.array_literal.elements, elements[i]);
        }
    }
    free(elements);

    return array;
}