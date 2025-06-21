/**
 * Asthra Programming Language Compiler - Type Grammar Productions
 * Type parsing including base types, pointer types, slice types, and composite types
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_statements.h"
#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// TYPE PARSING
// =============================================================================

ASTNode *parse_type(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Handle pointer types: *mut Type / *const Type
    if (match_token(parser, TOKEN_MULTIPLY)) {
        advance_token(parser);
        
        bool is_mutable = false;
        // Check for 'mut' or 'const' keywords
        if (match_token(parser, TOKEN_MUT)) {
            is_mutable = true;
            advance_token(parser);
        } else if (match_token(parser, TOKEN_CONST)) {
            // const is default, no need to set flag
            advance_token(parser);
        }
        
        ASTNode *pointee_type = parse_type(parser);
        if (!pointee_type) return NULL;
        
        ASTNode *node = ast_create_node(AST_PTR_TYPE, start_loc);
        if (!node) {
            ast_free_node(pointee_type);
            return NULL;
        }
        
        node->data.ptr_type.is_mutable = is_mutable;
        node->data.ptr_type.pointee_type = pointee_type;
        
        return node;
    }
    
    // Handle slice types: []Type and array types: [Type; size]
    if (match_token(parser, TOKEN_LEFT_BRACKET)) {
        advance_token(parser);
        
        // Check if it's a slice type (empty brackets)
        if (match_token(parser, TOKEN_RIGHT_BRACKET)) {
            advance_token(parser);
            
            ASTNode *element_type = parse_type(parser);
            if (!element_type) return NULL;
            
            ASTNode *node = ast_create_node(AST_SLICE_TYPE, start_loc);
            if (!node) {
                ast_free_node(element_type);
                return NULL;
            }
            
            node->data.slice_type.element_type = element_type;
            
            return node;
        } else {
            // Fixed array type: [size]Type
            // Parse the size expression first
            ASTNode *size_expr = parse_expr(parser);
            if (!size_expr) return NULL;
            
            if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
                ast_free_node(size_expr);
                return NULL;
            }
            
            // Now parse the element type
            ASTNode *element_type = parse_type(parser);
            if (!element_type) {
                ast_free_node(size_expr);
                return NULL;
            }
            
            // Create array type node
            ASTNode *node = ast_create_node(AST_ARRAY_TYPE, start_loc);
            if (!node) {
                ast_free_node(element_type);
                ast_free_node(size_expr);
                return NULL;
            }
            
            node->data.array_type.element_type = element_type;
            node->data.array_type.size = size_expr;
            
            return node;
        }
    }
    
    // Handle Result type: Result<Type, Type>
    if (match_token(parser, TOKEN_RESULT)) {
        advance_token(parser);
        
        if (!expect_token(parser, TOKEN_LESS_THAN)) {
            return NULL;
        }
        
        ASTNode *ok_type = parse_type(parser);
        if (!ok_type) return NULL;
        
        if (!expect_token(parser, TOKEN_COMMA)) {
            ast_free_node(ok_type);
            return NULL;
        }
        
        ASTNode *err_type = parse_type(parser);
        if (!err_type) {
            ast_free_node(ok_type);
            return NULL;
        }
        
        if (!expect_token(parser, TOKEN_GREATER_THAN)) {
            ast_free_node(ok_type);
            ast_free_node(err_type);
            return NULL;
        }
        
        ASTNode *node = ast_create_node(AST_RESULT_TYPE, start_loc);
        if (!node) {
            ast_free_node(ok_type);
            ast_free_node(err_type);
            return NULL;
        }
        
        node->data.result_type.ok_type = ok_type;
        node->data.result_type.err_type = err_type;
        
        return node;
    }
    
    // Handle tuple types: (Type1, Type2, ...)
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);  // consume '('
        
        // Check for empty parens - not a valid type
        if (match_token(parser, TOKEN_RIGHT_PAREN)) {
            report_error(parser, "Empty parentheses are not a valid type");
            return NULL;
        }
        
        // Try to parse as tuple type
        ASTNodeList *element_types = ast_node_list_create(2);
        if (!element_types) {
            return NULL;
        }
        
        // Parse first type
        ASTNode *first_type = parse_type(parser);
        if (!first_type) {
            // Error parsing type
            ast_node_list_destroy(element_types);
            return NULL;
        }
        
        // Check for comma - if no comma, it's a parenthesized type
        if (!match_token(parser, TOKEN_COMMA)) {
            // Single element in parens - not a tuple
            if (expect_token(parser, TOKEN_RIGHT_PAREN)) {
                // Just a parenthesized type, return it
                ast_node_list_destroy(element_types);
                return first_type;
            } else {
                // Error in parsing
                ast_free_node(first_type);
                ast_node_list_destroy(element_types);
                return NULL;
            }
        }
        
        // We have a comma, so this is a tuple type
        ast_node_list_add(&element_types, first_type);
        advance_token(parser);  // consume comma
        
        // Parse remaining types
        do {
            ASTNode *element_type = parse_type(parser);
            if (!element_type) {
                // Clean up and fail
                for (size_t i = 0; i < element_types->count; i++) {
                    ast_free_node(element_types->nodes[i]);
                }
                ast_node_list_destroy(element_types);
                return NULL;
            }
            
            ast_node_list_add(&element_types, element_type);
            
            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser));
        
        // Expect closing paren
        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            // Clean up
            for (size_t i = 0; i < element_types->count; i++) {
                ast_free_node(element_types->nodes[i]);
            }
            ast_node_list_destroy(element_types);
            return NULL;
        }
        
        // Create tuple type node
        ASTNode *node = ast_create_node(AST_TUPLE_TYPE, start_loc);
        if (!node) {
            // Clean up
            for (size_t i = 0; i < element_types->count; i++) {
                ast_free_node(element_types->nodes[i]);
            }
            ast_node_list_destroy(element_types);
            return NULL;
        }
        
        node->data.tuple_type.element_types = element_types;
        return node;
    }
    
    // Handle built-in primitive types
    if (match_token(parser, TOKEN_INT) || match_token(parser, TOKEN_FLOAT_TYPE) ||
        match_token(parser, TOKEN_I8) || match_token(parser, TOKEN_U8) ||
        match_token(parser, TOKEN_I16) || match_token(parser, TOKEN_U16) ||
        match_token(parser, TOKEN_I32) || match_token(parser, TOKEN_U32) ||
        match_token(parser, TOKEN_I64) || match_token(parser, TOKEN_U64) ||
        match_token(parser, TOKEN_F32) || match_token(parser, TOKEN_F64) ||
        match_token(parser, TOKEN_BOOL) || match_token(parser, TOKEN_STRING_TYPE) ||
        match_token(parser, TOKEN_VOID) || match_token(parser, TOKEN_USIZE) ||
        match_token(parser, TOKEN_ISIZE) || match_token(parser, TOKEN_NEVER)) {
        
        // Map token types to correct lowercase type names
        char *type_name = NULL;
        switch (parser->current_token.type) {
            case TOKEN_INT: type_name = strdup("int"); break;
            case TOKEN_FLOAT_TYPE: type_name = strdup("float"); break;
            case TOKEN_I8: type_name = strdup("i8"); break;
            case TOKEN_U8: type_name = strdup("u8"); break;
            case TOKEN_I16: type_name = strdup("i16"); break;
            case TOKEN_U16: type_name = strdup("u16"); break;
            case TOKEN_I32: type_name = strdup("i32"); break;
            case TOKEN_U32: type_name = strdup("u32"); break;
            case TOKEN_I64: type_name = strdup("i64"); break;
            case TOKEN_U64: type_name = strdup("u64"); break;
            case TOKEN_F32: type_name = strdup("f32"); break;
            case TOKEN_F64: type_name = strdup("f64"); break;
            case TOKEN_BOOL: type_name = strdup("bool"); break;
            case TOKEN_STRING_TYPE: type_name = strdup("string"); break;
            case TOKEN_VOID: type_name = strdup("void"); break;
            case TOKEN_USIZE: type_name = strdup("usize"); break;
            case TOKEN_ISIZE: type_name = strdup("isize"); break;
            case TOKEN_NEVER: type_name = strdup("Never"); break;
            default: type_name = strdup("unknown"); break;
        }
        
        advance_token(parser);
        
        ASTNode *node = ast_create_node(AST_BASE_TYPE, start_loc);
        if (!node) {
            free(type_name);
            return NULL;
        }
        
        node->data.base_type.name = type_name;
        
        return node;
    }
    
    // Handle user-defined types (struct, enum, or type aliases)
    // This covers: StructType <- IDENT and EnumType <- SimpleIdent TypeArgs?
    if (match_token(parser, TOKEN_IDENTIFIER)) {
        char *type_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
        
        // Check for type arguments: TypeArgs? <- '<' Type (',' Type)* '>'
        ASTNodeList *type_args = NULL;
        if (match_token(parser, TOKEN_LESS_THAN)) {
            advance_token(parser);
            
            type_args = ast_node_list_create(2);
            if (!type_args) {
                free(type_name);
                return NULL;
            }
            
            do {
                ASTNode *type_arg = parse_type(parser);
                if (!type_arg) {
                    ast_node_list_destroy(type_args);
                    free(type_name);
                    return NULL;
                }
                
                ast_node_list_add(&type_args, type_arg);
                
                if (match_token(parser, TOKEN_COMMA)) {
                    advance_token(parser);
                } else {
                    break;
                }
            } while (!at_end(parser));
            
            if (!expect_token(parser, TOKEN_GREATER_THAN)) {
                ast_node_list_destroy(type_args);
                free(type_name);
                return NULL;
            }
        }
        
        // Create appropriate AST node based on whether we have type arguments
        // Heuristic: types with type arguments are likely enums (Option<T>, Result<T,E>)
        // Types without type arguments are likely structs or simple enums
        // The semantic analyzer will resolve the actual type and convert as needed
        ASTNode *node;
        if (type_args) {
            // Generic type - likely an enum but could be a generic struct
            node = ast_create_node(AST_ENUM_TYPE, start_loc);
            if (!node) {
                ast_node_list_destroy(type_args);
                free(type_name);
                return NULL;
            }
            
            node->data.enum_type.name = type_name;
            node->data.enum_type.type_args = type_args;
        } else {
            // Non-generic type - likely a struct but could be a simple enum
            node = ast_create_node(AST_STRUCT_TYPE, start_loc);
            if (!node) {
                free(type_name);
                return NULL;
            }
            
            node->data.struct_type.name = type_name;
            node->data.struct_type.type_args = NULL; // No type arguments
        }
        
        return node;
    }
    
    report_error(parser, "Expected type");
    return NULL;
} 
