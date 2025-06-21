/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions Implementation
 * Struct declarations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_toplevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// STRUCT DECLARATION PARSING
// =============================================================================

ASTNode *parse_struct_decl(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!expect_token(parser, TOKEN_STRUCT)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected struct name");
        return NULL;
    }
    
    char *struct_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Parse optional type parameters: TypeParams? <- '<' TypeParam (',' TypeParam)* '>'
    ASTNodeList *type_params = NULL;
    if (match_token(parser, TOKEN_LESS_THAN)) {
        advance_token(parser);
        
        type_params = ast_node_list_create(2);
        if (!type_params) {
            free(struct_name);
            return NULL;
        }
        
        do {
            if (!match_token(parser, TOKEN_IDENTIFIER)) {
                report_error(parser, "Expected type parameter name");
                ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
            
            ASTNode *type_param = ast_create_node(AST_IDENTIFIER, parser->current_token.location);
            if (!type_param) {
                ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
            
            type_param->data.identifier.name = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);
            
            // Check for unsupported constraint syntax (e.g., T: SomeType)
            if (match_token(parser, TOKEN_COLON)) {
                report_error(parser, "Type parameter constraints are not supported. Use 'struct Name<T>' without constraints.");
                ast_free_node(type_param);
                ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
            
            ast_node_list_add(&type_params, type_param);
            
            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser));
        
        if (!expect_token(parser, TOKEN_GREATER_THAN)) {
            ast_node_list_destroy(type_params);
            free(struct_name);
            return NULL;
        }
    }
    
    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        if (type_params) ast_node_list_destroy(type_params);
        free(struct_name);
        return NULL;
    }
    
    // Parse struct content according to v1.25 grammar: StructContent <- StructFieldList / 'none'
    ASTNode **fields = NULL;
    size_t field_count = 0;
    size_t field_capacity = 4;
    
    if (match_token(parser, TOKEN_NONE)) {
        // Explicit none for empty struct (v1.19 semantic clarity)
        advance_token(parser);
        fields = NULL; // No fields
        field_count = 0;
    } else if (match_token(parser, TOKEN_VOID)) {
        // Legacy compatibility error with helpful message
        report_error(parser, "Unexpected 'void' in struct content. Use 'none' for empty structs");
        if (type_params) ast_node_list_destroy(type_params);
        free(struct_name);
        return NULL;
    } else {
        // Parse actual struct fields
        fields = malloc(field_capacity * sizeof(ASTNode*));
        if (!fields) {
            if (type_params) ast_node_list_destroy(type_params);
            free(struct_name);
            return NULL;
        }
        
        while (!match_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser)) {
            SourceLocation field_loc = parser->current_token.location;
            
            // Parse field visibility
            VisibilityType field_visibility = VISIBILITY_PRIVATE;
            if (match_token(parser, TOKEN_PUB)) {
                field_visibility = VISIBILITY_PUBLIC;
                advance_token(parser);
            } else if (match_token(parser, TOKEN_PRIV)) {
                field_visibility = VISIBILITY_PRIVATE;
                advance_token(parser);
            }
            
            if (!match_token(parser, TOKEN_IDENTIFIER)) {
                report_error(parser, "Expected field name");
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(fields[i]);
                }
                free(fields);
                if (type_params) ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
            
            char *field_name = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);
            
            if (!expect_token(parser, TOKEN_COLON)) {
                free(field_name);
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(fields[i]);
                }
                free(fields);
                if (type_params) ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
            
            ASTNode *field_type = parse_type(parser);
            if (!field_type) {
                free(field_name);
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(fields[i]);
                }
                free(fields);
                if (type_params) ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
            
            // Create struct field node
            ASTNode *field = ast_create_node(AST_STRUCT_FIELD, field_loc);
            if (!field) {
                free(field_name);
                ast_free_node(field_type);
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(fields[i]);
                }
                free(fields);
                if (type_params) ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
            
            field->data.struct_field.name = field_name;
            field->data.struct_field.type = field_type;
            field->data.struct_field.visibility = field_visibility;
            
            // Expand fields array if needed
            if (field_count >= field_capacity) {
                field_capacity *= 2;
                fields = realloc(fields, field_capacity * sizeof(ASTNode*));
                if (!fields) {
                    ast_free_node(field);
                    if (type_params) ast_node_list_destroy(type_params);
                    free(struct_name);
                    return NULL;
                }
            }
            
            fields[field_count++] = field;
            
            // Parse comma between fields (v1.12: comma separators, no trailing comma allowed)
            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
                // After comma, we must have another field (no trailing comma)
                if (match_token(parser, TOKEN_RIGHT_BRACE)) {
                    report_error(parser, "Trailing comma not allowed in struct fields (v1.12)");
                    for (size_t i = 0; i < field_count; i++) {
                        ast_free_node(fields[i]);
                    }
                    free(fields);
                    if (type_params) ast_node_list_destroy(type_params);
                    free(struct_name);
                    return NULL;
                }
            } else if (!match_token(parser, TOKEN_RIGHT_BRACE)) {
                report_error(parser, "Expected ',' between struct fields or '}' after last field");
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(fields[i]);
                }
                free(fields);
                if (type_params) ast_node_list_destroy(type_params);
                free(struct_name);
                return NULL;
            }
        }
    }
    
    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        for (size_t i = 0; i < field_count; i++) {
            ast_free_node(fields[i]);
        }
        free(fields);
        if (type_params) ast_node_list_destroy(type_params);
        free(struct_name);
        return NULL;
    }
    
    // Create struct declaration node
    ASTNode *node = ast_create_node(AST_STRUCT_DECL, start_loc);
    if (!node) {
        for (size_t i = 0; i < field_count; i++) {
            ast_free_node(fields[i]);
        }
        free(fields);
        if (type_params) ast_node_list_destroy(type_params);
        free(struct_name);
        return NULL;
    }
    
    node->data.struct_decl.name = struct_name;
    node->data.struct_decl.type_params = type_params;
    node->data.struct_decl.annotations = NULL;
    
    // Convert fields array to ASTNodeList
    if (fields && field_count > 0) {
        node->data.struct_decl.fields = ast_node_list_create(field_count);
        if (node->data.struct_decl.fields) {
            for (size_t i = 0; i < field_count; i++) {
                ast_node_list_add(&node->data.struct_decl.fields, fields[i]);
            }
        }
        free(fields);
    } else {
        node->data.struct_decl.fields = NULL;
    }
    
    return node;
} 
