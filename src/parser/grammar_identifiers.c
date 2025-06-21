/**
 * Asthra Programming Language Compiler - Identifier and Enum Constructor Parsing
 * Parsing of identifiers, enum constructors, and associated function calls
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "grammar_statements.h"
#include "parser_ast_helpers.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Parse Result keyword as enum name or type
 */
ASTNode *parse_result_keyword(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_RESULT)) return NULL;
    
    char *name = strdup("Result");
    advance_token(parser);
    
    // Check for enum constructor (Result followed by '.')
    if (match_token(parser, TOKEN_DOT)) {
        advance_token(parser); // consume '.'
        
        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected variant name after '.' in enum constructor");
            free(name);
            return NULL;
        }
        
        char *variant_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
        
        // Check for optional arguments: ('(' ArgList ')')?
        ASTNode *value = NULL;
        if (match_token(parser, TOKEN_LEFT_PAREN)) {
            advance_token(parser); // consume '('
            
            // Parse single argument if present (enum constructors take single values)
            if (!match_token(parser, TOKEN_RIGHT_PAREN)) {
                value = parse_expr(parser);
                if (!value) {
                    free(name);
                    free(variant_name);
                    return NULL;
                }
                
                // Check for additional arguments (not supported by current AST)
                if (match_token(parser, TOKEN_COMMA)) {
                    report_error(parser, "Enum constructors currently support only single values. Use a tuple for multiple values.");
                    ast_free_node(value);
                    free(name);
                    free(variant_name);
                    return NULL;
                }
            }
            
            if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                if (value) ast_free_node(value);
                free(name);
                free(variant_name);
                return NULL;
            }
        }
        
        // Create enum variant node with single value
        ASTNode *node = ast_create_node(AST_ENUM_VARIANT, start_loc);
        if (!node) {
            if (value) ast_free_node(value);
            free(name);
            free(variant_name);
            return NULL;
        }
        
        node->data.enum_variant.enum_name = name;
        node->data.enum_variant.variant_name = variant_name;
        node->data.enum_variant.value = value; // Store the single value
        
        return node;
    } else {
        // Just "Result" by itself - create a type node
        ASTNode *node = ast_create_node(AST_BASE_TYPE, start_loc);
        if (!node) {
            free(name);
            return NULL;
        }
        
        node->data.base_type.name = name;
        return node;
    }
}

/**
 * Parse Option keyword as enum name or type
 */
ASTNode *parse_option_keyword(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_OPTION)) return NULL;
    
    char *name = strdup("Option");
    advance_token(parser);
    
    // Check for enum constructor (Option followed by '.')
    if (match_token(parser, TOKEN_DOT)) {
        advance_token(parser); // consume '.'
        
        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected variant name after '.' in enum constructor");
            free(name);
            return NULL;
        }
        
        char *variant_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
        
        // Check for optional arguments: ('(' ArgList ')')?  
        ASTNode *value = NULL;
        if (match_token(parser, TOKEN_LEFT_PAREN)) {
            advance_token(parser); // consume '('
            
            // Parse single argument if present (enum constructors take single values)
            if (!match_token(parser, TOKEN_RIGHT_PAREN)) {
                value = parse_expr(parser);
                if (!value) {
                    free(name);
                    free(variant_name);
                    return NULL;
                }
                
                // Check for additional arguments (not supported by current AST)
                if (match_token(parser, TOKEN_COMMA)) {
                    report_error(parser, "Enum constructors currently support only single values. Use a tuple for multiple values.");
                    ast_free_node(value);
                    free(name);
                    free(variant_name);
                    return NULL;
                }
            }
            
            if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                if (value) ast_free_node(value);
                free(name);
                free(variant_name);
                return NULL;
            }
        }
        
        // Create enum variant node with single value
        ASTNode *node = ast_create_node(AST_ENUM_VARIANT, start_loc);
        if (!node) {
            if (value) ast_free_node(value);
            free(name);
            free(variant_name);
            return NULL;
        }
        
        node->data.enum_variant.enum_name = name;
        node->data.enum_variant.variant_name = variant_name;
        node->data.enum_variant.value = value; // Store the single value
        
        return node;
    } else {
        // Just "Option" by itself - create a type node
        ASTNode *node = ast_create_node(AST_BASE_TYPE, start_loc);
        if (!node) {
            free(name);
            return NULL;
        }
        
        node->data.base_type.name = name;
        return node;
    }
}

/**
 * Parse enum constructor with given enum name
 */
static ASTNode *parse_enum_constructor(Parser *parser, char *enum_name, SourceLocation start_loc) {
    if (!parser || !enum_name) return NULL;
    
    advance_token(parser); // consume '.'
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected variant name after '.' in enum constructor");
        return NULL;
    }
    
    char *variant_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Check for optional arguments: ('(' ArgList ')')?
    ASTNode *value = NULL;
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser); // consume '('
        
        // Parse single argument if present (enum constructors take single values)
        if (!match_token(parser, TOKEN_RIGHT_PAREN)) {
            value = parse_expr(parser);
            if (!value) {
                free(variant_name);
                return NULL;
            }
            
            // Check for additional arguments (not supported by current AST)
            if (match_token(parser, TOKEN_COMMA)) {
                report_error(parser, "Enum constructors currently support only single values. Use a tuple for multiple values.");
                ast_free_node(value);
                free(variant_name);
                return NULL;
            }
        }
        
        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            if (value) ast_free_node(value);
            free(variant_name);
            return NULL;
        }
    }
    
    // Create enum variant node with single value
    ASTNode *node = ast_create_node(AST_ENUM_VARIANT, start_loc);
    if (!node) {
        if (value) ast_free_node(value);
        free(variant_name);
        return NULL;
    }
    
    node->data.enum_variant.enum_name = strdup(enum_name);
    node->data.enum_variant.variant_name = variant_name;
    node->data.enum_variant.value = value; // Store the single value
    
    return node;
}

/**
 * Parse associated function call with given struct name
 */
static ASTNode *parse_associated_function_call(Parser *parser, char *struct_name, ASTNodeList *type_args, SourceLocation start_loc) {
    if (!parser || !struct_name) return NULL;
    
    advance_token(parser); // consume '::'
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected function name after '::'");
        return NULL;
    }
    
    char *function_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Create associated function call node (arguments will be added by postfix parsing)
    ASTNode *node = ast_create_node(AST_ASSOCIATED_FUNC_CALL, start_loc);
    if (!node) {
        free(function_name);
        return NULL;
    }
    
    node->data.associated_func_call.struct_name = strdup(struct_name);
    node->data.associated_func_call.function_name = function_name;
    node->data.associated_func_call.type_args = type_args; // Store generic args (may be NULL)
    node->data.associated_func_call.args = NULL; // Will be filled by postfix parsing
    
    return node;
}

/**
 * Check if identifier looks like enum constructor based on heuristics
 */
static bool looks_like_enum_constructor(Parser *parser) {
    if (!parser) return false;
    
    Token peek1 = peek_token(parser);
    
    // Check if the identifier after dot looks like an enum variant (typically uppercase)
    if (peek1.type == TOKEN_IDENTIFIER && peek1.data.identifier.name) {
        char first_char = peek1.data.identifier.name[0];
        // Check if variant name starts with uppercase (common convention for enum variants)
        if (first_char >= 'A' && first_char <= 'Z') {
            return true;
        }
    }
    
    return false;
}

/**
 * Parse identifier and handle special cases (enum constructors, associated functions)
 */
ASTNode *parse_identifier(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_IDENTIFIER)) return NULL;
    
    char *name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Check for enum constructor (identifier followed by '.')
    if (match_token(parser, TOKEN_DOT)) {
        // Use heuristic to disambiguate from field access
        if (!looks_like_enum_constructor(parser)) {
            // This is likely field access, let postfix parser handle it
            ASTNode *node = ast_create_node(AST_IDENTIFIER, start_loc);
            if (!node) {
                free(name);
                return NULL;
            }
            
            node->data.identifier.name = name;
            return node;
        }
        
        // Parse as enum constructor
        ASTNode *enum_node = parse_enum_constructor(parser, name, start_loc);
        free(name);
        return enum_node;
    }
    // Check for associated function call (identifier followed by '::')
    else if (match_token(parser, TOKEN_DOUBLE_COLON)) {
        ASTNode *func_node = parse_associated_function_call(parser, name, NULL, start_loc);
        free(name);
        return func_node;
    }
    
    // Regular identifier
    ASTNode *node = ast_create_node(AST_IDENTIFIER, start_loc);
    if (!node) {
        free(name);
        return NULL;
    }
    
    node->data.identifier.name = name;
    return node;
}

/**
 * Parse 'self' keyword as identifier
 */
ASTNode *parse_self_keyword(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_SELF)) return NULL;
    
    char *name = strdup("self");
    advance_token(parser);
    
    ASTNode *node = ast_create_node(AST_IDENTIFIER, start_loc);
    if (!node) {
        free(name);
        return NULL;
    }
    
    node->data.identifier.name = name;
    return node;
}