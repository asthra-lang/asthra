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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Include the split functionality
#include "grammar_literals.c"
#include "grammar_identifiers.c"
#include "grammar_arrays.c"
#include "grammar_generics.c"

// =============================================================================
// PRIMARY EXPRESSIONS COORDINATION
// =============================================================================

ASTNode *parse_primary(Parser *parser) {
    if (!parser) return NULL;
    
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
        
        // Don't try to parse generics here - they should only be parsed in type contexts
        // or when we know we're parsing a struct literal (handled in postfix)
        
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
            node->data.associated_func_call.args = NULL; // Will be filled by postfix parsing
            
            return node;
        }
        
        // Check for generic type arguments when followed by struct literal syntax
        // This handles cases like Container<i32> { ... }
        if (match_token(parser, TOKEN_LESS_THAN)) {
            // Try to parse as generic identifier - this will handle the type arguments
            ASTNode *generic_node = parse_identifier_with_generics(parser, name, start_loc);
            if (generic_node) {
                // Successfully parsed generic type, check if it's followed by struct literal
                if (match_token(parser, TOKEN_LEFT_BRACE)) {
                    // This looks like a generic struct literal
                    // Convert to STRUCT_TYPE if it's not already
                    if (generic_node->type == AST_ENUM_TYPE) {
                        // Convert enum type to struct type for struct literal parsing
                        ASTNode *struct_node = ast_create_node(AST_STRUCT_TYPE, start_loc);
                        if (struct_node) {
                            struct_node->data.struct_type.name = strdup(generic_node->data.enum_type.name);
                            struct_node->data.struct_type.type_args = generic_node->data.enum_type.type_args;
                            // Prevent double-free by clearing the original type_args
                            generic_node->data.enum_type.type_args = NULL;
                            ast_free_node(generic_node);
                            free(name); // name was already used by generic_node
                            return struct_node;
                        }
                    }
                    // If it's already a struct type or other generic node, return as is
                    free(name); // name was already used by generic_node
                    return generic_node;
                }
                // Generic type not followed by struct literal, return the parsed node
                free(name); // name was already used by generic_node
                return generic_node;
            }
            // Failed to parse as generic type, fall through to regular identifier
        }
        
        // For simple cases, just create a regular identifier
        // The postfix parser will handle dots and other operations
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
    
    // No valid primary expression found
    report_error(parser, "Expected expression");
    return NULL;
} 
