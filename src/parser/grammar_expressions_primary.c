/**
 * Asthra Programming Language Compiler - Primary Expressions
 * Coordination file for primary expression parsing functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file coordinates the split functionality for primary expressions
 */

#include "grammar_expressions.h"
#include "grammar_statements.h"
#include "parser_ast_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the split functionality
#include "grammar_arrays.c"
#include "grammar_generics.c"
#include "grammar_identifiers.c"
#include "grammar_literals.c"

// Forward declaration for struct literal parsing
ASTNode *parse_struct_literal_with_name(Parser *parser, char *struct_name,
                                        SourceLocation start_loc);

// =============================================================================
// PRIMARY EXPRESSIONS COORDINATION
// =============================================================================

ASTNode *parse_primary(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;
    ASTNode *node = NULL;

    // Try literals first
    if ((node = parse_bool_literal(parser, start_loc)) != NULL) {
        return node;
    }
    if ((node = parse_integer_literal(parser, start_loc)) != NULL) {
        return node;
    }
    if ((node = parse_float_literal(parser, start_loc)) != NULL) {
        return node;
    }
    if ((node = parse_string_literal(parser, start_loc)) != NULL) {
        return node;
    }
    if ((node = parse_char_literal(parser, start_loc)) != NULL) {
        return node;
    }

    // Try Result keyword
    if ((node = parse_result_keyword(parser, start_loc)) != NULL) {
        return node;
    }

    // Try Option keyword
    if ((node = parse_option_keyword(parser, start_loc)) != NULL) {
        return node;
    }

    // Try identifier with potential generic arguments or special cases
    if (match_token(parser, TOKEN_IDENTIFIER)) {
        char *name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);

        // Try to parse as generic type with potential associated function call
        // This handles cases like Vec<i32>::new() or Result<T,E>::Ok()
        if (match_token(parser, TOKEN_LESS_THAN)) {
            ASTNode *generic_node = parse_identifier_with_generics(parser, name, start_loc);
            if (generic_node) {
                free(name); // parse_identifier_with_generics makes its own copy
                // The postfix parser will handle struct literal conversion if needed
                return generic_node;
            }
            // If generic parsing failed, fall through to regular identifier handling
        }

        // Check for associated function call: Type::function
        if (match_token(parser, TOKEN_DOUBLE_COLON)) {
            advance_token(parser); // consume '::'

            if (!match_token(parser, TOKEN_IDENTIFIER)) {
                report_error(parser, "Expected function name after '::'");
                free(name);
                return NULL;
            }

            char *function_name = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);

            // Create associated function call node
            node = ast_create_node(AST_ASSOCIATED_FUNC_CALL, start_loc);
            if (!node) {
                free(name);
                free(function_name);
                return NULL;
            }

            node->data.associated_func_call.struct_name = name;
            node->data.associated_func_call.function_name = function_name;
            node->data.associated_func_call.type_args = NULL; // No generic args
            node->data.associated_func_call.args = NULL;      // Will be filled by postfix parsing

            return node;
        }

        // Don't parse struct literals here in primary expressions
        // They need special handling to avoid conflicts with match/if statements
        // The solution is to parse them in specific contexts where they're expected

        // For simple cases, just create a regular identifier
        // Struct literals will be handled elsewhere
        node = ast_create_node(AST_IDENTIFIER, start_loc);
        if (!node) {
            free(name);
            return NULL;
        }

        node->data.identifier.name = name;
        return node;
    }

    // Try self keyword
    if ((node = parse_self_keyword(parser, start_loc)) != NULL) {
        return node;
    }

    // Try sizeof operator
    if (match_token(parser, TOKEN_SIZEOF)) {
        return parse_sizeof(parser);
    }

    // Try parenthesized expressions
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        return parse_parenthesized_expr(parser);
    }

    // Try array literals
    if ((node = parse_array_literal(parser)) != NULL) {
        return node;
    }

    // Try unsafe blocks as expressions
    if (match_token(parser, TOKEN_UNSAFE)) {
        advance_token(parser);

        // Parse the block that follows
        ASTNode *block = parse_block(parser);
        if (!block) {
            report_error(parser, "Expected block after 'unsafe' keyword");
            return NULL;
        }

        // Create the unsafe block node
        node = ast_create_node(AST_UNSAFE_BLOCK, start_loc);
        if (!node) {
            ast_free_node(block);
            return NULL;
        }

        node->data.unsafe_block.block = block;
        return node;
    }

    // Try if expressions
    if (match_token(parser, TOKEN_IF)) {
        // If can be used as an expression in Asthra
        // We'll use the existing parse_if_stmt function since if expressions
        // and if statements have the same structure in Asthra
        return parse_if_stmt(parser);
    }

    // No valid primary expression found
    report_error(parser, "Expected expression");
    return NULL;
}
