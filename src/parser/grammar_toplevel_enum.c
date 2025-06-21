/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions Implementation
 * Enum declarations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_toplevel.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// ENUM DECLARATION PARSING
// =============================================================================

ASTNode *parse_enum_decl(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!expect_token(parser, TOKEN_ENUM)) {
        return NULL;
    }
    
    // Allow Result as enum name even though it's a keyword
    char *enum_name = NULL;
    if (match_token(parser, TOKEN_IDENTIFIER)) {
        enum_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
    } else if (match_token(parser, TOKEN_RESULT)) {
        enum_name = strdup("Result");
        advance_token(parser);
    } else {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                 "Expected enum name, got %s", 
                 token_type_name(parser->current_token.type));
        report_error(parser, error_msg);
        return NULL;
    }
    
    // Parse optional type parameters: TypeParams? <- '<' TypeParam (',' TypeParam)* '>'
    ASTNodeList *type_params = NULL;
    if (match_token(parser, TOKEN_LESS_THAN)) {
        advance_token(parser);
        
        type_params = ast_node_list_create(2);
        if (!type_params) {
            free(enum_name);
            return NULL;
        }
        
        do {
            if (!match_token(parser, TOKEN_IDENTIFIER)) {
                report_error(parser, "Expected type parameter name");
                ast_node_list_destroy(type_params);
                free(enum_name);
                return NULL;
            }
            
            ASTNode *type_param = ast_create_node(AST_IDENTIFIER, parser->current_token.location);
            if (!type_param) {
                ast_node_list_destroy(type_params);
                free(enum_name);
                return NULL;
            }
            
            type_param->data.identifier.name = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);
            
            // Check for unsupported constraint syntax (e.g., T: SomeType)
            if (match_token(parser, TOKEN_COLON)) {
                report_error(parser, "Type parameter constraints are not supported. Use 'enum Name<T>' without constraints.");
                ast_free_node(type_param);
                ast_node_list_destroy(type_params);
                free(enum_name);
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
            free(enum_name);
            return NULL;
        }
    }
    
    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        if (type_params) ast_node_list_destroy(type_params);
        free(enum_name);
        return NULL;
    }
    
    // Parse enum content according to current grammar: EnumContent <- EnumVariantList / 'none'
    ASTNode **variants = NULL;
    size_t variant_count = 0;
    size_t variant_capacity = 4;
    
    if (match_token(parser, TOKEN_NONE)) {
        // Explicit none for empty enum content (semantic clarity)
        advance_token(parser);
        variants = NULL; // No variants
        variant_count = 0;
    } else if (match_token(parser, TOKEN_VOID)) {
        // Legacy compatibility error with helpful message
        report_error(parser, "Unexpected 'void' in enum content. Use 'none' for empty enums");
        if (type_params) ast_node_list_destroy(type_params);
        free(enum_name);
        return NULL;
    } else {
        // Parse actual enum variants: EnumVariantList <- EnumVariant (',' EnumVariant)*
        variants = malloc(variant_capacity * sizeof(ASTNode*));
        if (!variants) {
            if (type_params) ast_node_list_destroy(type_params);
            free(enum_name);
            return NULL;
        }
        
        // Parse first variant (required if not void)
        do {
            SourceLocation variant_loc = parser->current_token.location;
            
            // Parse variant visibility: VisibilityModifier?
            VisibilityType variant_visibility = VISIBILITY_PRIVATE;
            if (match_token(parser, TOKEN_PUB)) {
                variant_visibility = VISIBILITY_PUBLIC;
                advance_token(parser);
            } else if (match_token(parser, TOKEN_PRIV)) {
                variant_visibility = VISIBILITY_PRIVATE;
                advance_token(parser);
            }
            
            if (!match_token(parser, TOKEN_IDENTIFIER)) {
                report_error(parser, "Expected enum variant name");
                for (size_t i = 0; i < variant_count; i++) {
                    ast_free_node(variants[i]);
                }
                free(variants);
                if (type_params) ast_node_list_destroy(type_params);
                free(enum_name);
                return NULL;
            }
            
            char *variant_name = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);
            
            // Parse optional enum variant data: EnumVariantData? <- '(' Type ')' / '(' TypeList ')'
            ASTNode *associated_type = NULL;
            if (match_token(parser, TOKEN_LEFT_PAREN)) {
                advance_token(parser);
                
                associated_type = parse_type(parser);
                if (!associated_type) {
                    free(variant_name);
                    for (size_t i = 0; i < variant_count; i++) {
                        ast_free_node(variants[i]);
                    }
                    free(variants);
                    if (type_params) ast_node_list_destroy(type_params);
                    free(enum_name);
                    return NULL;
                }
                
                if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                    free(variant_name);
                    ast_free_node(associated_type);
                    for (size_t i = 0; i < variant_count; i++) {
                        ast_free_node(variants[i]);
                    }
                    free(variants);
                    if (type_params) ast_node_list_destroy(type_params);
                    free(enum_name);
                    return NULL;
                }
            }
            
            // Parse optional explicit value (for C-style enums): '=' Expr
            ASTNode *explicit_value = NULL;
            if (match_token(parser, TOKEN_ASSIGN)) {
                advance_token(parser);
                
                explicit_value = parse_expr(parser);
                if (!explicit_value) {
                    free(variant_name);
                    if (associated_type) ast_free_node(associated_type);
                    for (size_t i = 0; i < variant_count; i++) {
                        ast_free_node(variants[i]);
                    }
                    free(variants);
                    if (type_params) ast_node_list_destroy(type_params);
                    free(enum_name);
                    return NULL;
                }
            }
            
            // Create enum variant declaration node
            ASTNode *variant = ast_create_node(AST_ENUM_VARIANT_DECL, variant_loc);
            if (!variant) {
                free(variant_name);
                if (associated_type) ast_free_node(associated_type);
                if (explicit_value) ast_free_node(explicit_value);
                for (size_t i = 0; i < variant_count; i++) {
                    ast_free_node(variants[i]);
                }
                free(variants);
                if (type_params) ast_node_list_destroy(type_params);
                free(enum_name);
                return NULL;
            }
            
            variant->data.enum_variant_decl.name = variant_name;
            variant->data.enum_variant_decl.associated_type = associated_type;
            variant->data.enum_variant_decl.value = explicit_value;
            variant->data.enum_variant_decl.visibility = variant_visibility;
            
            // Expand variants array if needed
            if (variant_count >= variant_capacity) {
                variant_capacity *= 2;
                variants = realloc(variants, variant_capacity * sizeof(ASTNode*));
                if (!variants) {
                    ast_free_node(variant);
                    if (type_params) ast_node_list_destroy(type_params);
                    free(enum_name);
                    return NULL;
                }
            }
            
            variants[variant_count++] = variant;
            
            // Parse comma between variants (v1.12: no trailing comma allowed)
            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
                // After comma, we must have another variant (no trailing comma)
                if (match_token(parser, TOKEN_RIGHT_BRACE)) {
                    report_error(parser, "Trailing comma not allowed in enum variants");
                    for (size_t i = 0; i < variant_count; i++) {
                        ast_free_node(variants[i]);
                    }
                    free(variants);
                    if (type_params) ast_node_list_destroy(type_params);
                    free(enum_name);
                    return NULL;
                }
            } else if (!match_token(parser, TOKEN_RIGHT_BRACE)) {
                report_error(parser, "Expected ',' between enum variants or '}' after last variant");
                for (size_t i = 0; i < variant_count; i++) {
                    ast_free_node(variants[i]);
                }
                free(variants);
                if (type_params) ast_node_list_destroy(type_params);
                free(enum_name);
                return NULL;
            }
        } while (!match_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser));
    }
    
    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        for (size_t i = 0; i < variant_count; i++) {
            ast_free_node(variants[i]);
        }
        free(variants);
        if (type_params) ast_node_list_destroy(type_params);
        free(enum_name);
        return NULL;
    }
    
    // Create enum declaration node
    ASTNode *node = ast_create_node(AST_ENUM_DECL, start_loc);
    if (!node) {
        for (size_t i = 0; i < variant_count; i++) {
            ast_free_node(variants[i]);
        }
        free(variants);
        if (type_params) ast_node_list_destroy(type_params);
        free(enum_name);
        return NULL;
    }
    
    node->data.enum_decl.name = enum_name;
    node->data.enum_decl.type_params = type_params;
    node->data.enum_decl.annotations = NULL;
    node->data.enum_decl.visibility = VISIBILITY_PRIVATE; // Will be set by caller if pub
    
    // Convert variants array to ASTNodeList
    if (variants && variant_count > 0) {
        node->data.enum_decl.variants = ast_node_list_create(variant_count);
        if (node->data.enum_decl.variants) {
            for (size_t i = 0; i < variant_count; i++) {
                ast_node_list_add(&node->data.enum_decl.variants, variants[i]);
            }
        }
        free(variants);
    } else {
        node->data.enum_decl.variants = NULL;
    }
    
    return node;
} 
