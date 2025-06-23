/**
 * Asthra Programming Language Compiler - Annotation Grammar Productions Implementation
 * Parsing ownership, FFI, and security annotations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// ANNOTATION PARSING
// =============================================================================

ASTNode *parse_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    // Add helpful error message for @ annotations
    if (match_token(parser, TOKEN_AT)) {
        report_error(parser, 
            "@ annotations are not supported. Use #[...] syntax instead. "
            "Example: Use #[ownership(gc)] instead of @gc. "
            "See docs/AT_ANNOTATIONS_MIGRATION.md for migration guide.");
        return NULL;
    } else if (check_token(parser, TOKEN_HASH)) {
        // Look ahead to see if this is a bracketed annotation #[...] or simple ownership tag #gc
        Token next_token = peek_token(parser);
        if (next_token.type == TOKEN_LEFT_BRACKET) {
            return parse_bracketed_annotation(parser);
        } else {
            return parse_ownership_annotation(parser);
        }
    }
    
    return NULL;
}

// parse_semantic_annotation function removed - @ annotations no longer supported
// Use #[...] syntax instead. See docs/AT_ANNOTATIONS_MIGRATION.md for migration guide.

ASTNode *parse_ownership_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!expect_token(parser, TOKEN_HASH)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected ownership type after '#'");
        return NULL;
    }
    
    OwnershipType ownership;
    const char *ownership_name = parser->current_token.data.identifier.name;
    
    if (strcmp(ownership_name, "gc") == 0) {
        ownership = OWNERSHIP_GC;
    } else if (strcmp(ownership_name, "c") == 0) {
        ownership = OWNERSHIP_C;
    } else if (strcmp(ownership_name, "pinned") == 0) {
        ownership = OWNERSHIP_PINNED;
    } else {
        report_error(parser, "Unknown ownership type");
        return NULL;
    }
    
    advance_token(parser);
    
    ASTNode *node = ast_create_node(AST_OWNERSHIP_TAG, start_loc);
    if (!node) return NULL;
    
    node->data.ownership_tag.ownership = ownership;
    
    return node;
}

ASTNode *parse_ffi_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Expect "ffi" identifier
    if (!match_token(parser, TOKEN_IDENTIFIER) || 
        strcmp(parser->current_token.data.identifier.name, "ffi") != 0) {
        report_error(parser, "Expected 'ffi' annotation");
        return NULL;
    }
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected FFI transfer type");
        return NULL;
    }
    
    FFITransferType transfer_type;
    const char *transfer_name = parser->current_token.data.identifier.name;
    
    if (strcmp(transfer_name, "full") == 0) {
        transfer_type = FFI_TRANSFER_FULL;
    } else if (strcmp(transfer_name, "none") == 0) {
        transfer_type = FFI_TRANSFER_NONE;
    } else if (strcmp(transfer_name, "borrowed") == 0) {
        transfer_type = FFI_BORROWED;
    } else {
        report_error(parser, "Unknown FFI transfer type");
        return NULL;
    }
    
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_FFI_ANNOTATION, start_loc);
    if (!node) return NULL;
    
    node->data.ffi_annotation.transfer_type = transfer_type;
    
    return node;
}

// NEW: SafeFFIAnnotation parser implementing the fixed grammar
ASTNode *parse_safe_ffi_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // SafeFFIAnnotation <- FFITransferTag / LifetimeTag
    // FFITransferTag <- '#[' ('transfer_full' | 'transfer_none') ']'
    // LifetimeTag    <- '#[borrowed]'
    
    if (!expect_token(parser, TOKEN_HASH)) {
        return NULL;
    }
    
    if (!expect_token(parser, TOKEN_LEFT_BRACKET)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected FFI annotation identifier");
        return NULL;
    }
    
    FFITransferType transfer_type;
    const char *annotation_name = parser->current_token.data.identifier.name;
    
    if (strcmp(annotation_name, "transfer_full") == 0) {
        transfer_type = FFI_TRANSFER_FULL;
    } else if (strcmp(annotation_name, "transfer_none") == 0) {
        transfer_type = FFI_TRANSFER_NONE;
    } else if (strcmp(annotation_name, "borrowed") == 0) {
        transfer_type = FFI_BORROWED;
    } else {
        report_error(parser, "Unknown FFI annotation. Expected 'transfer_full', 'transfer_none', or 'borrowed'");
        return NULL;
    }
    
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_FFI_ANNOTATION, start_loc);
    if (!node) return NULL;
    
    node->data.ffi_annotation.transfer_type = transfer_type;
    
    return node;
}

ASTNode *parse_security_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Expect "security" identifier
    if (!match_token(parser, TOKEN_IDENTIFIER) || 
        strcmp(parser->current_token.data.identifier.name, "security") != 0) {
        report_error(parser, "Expected 'security' annotation");
        return NULL;
    }
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected security type");
        return NULL;
    }
    
    SecurityType security_type;
    const char *security_name = parser->current_token.data.identifier.name;
    
    if (strcmp(security_name, "constant_time") == 0) {
        security_type = SECURITY_CONSTANT_TIME;
    } else if (strcmp(security_name, "volatile_memory") == 0) {
        security_type = SECURITY_VOLATILE_MEMORY;
    } else {
        report_error(parser, "Unknown security type");
        return NULL;
    }
    
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_SECURITY_TAG, start_loc);
    if (!node) return NULL;
    
    node->data.security_tag.security_type = security_type;
    
    return node;
}

ASTNode *parse_human_review_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Expect "review" identifier
    if (!match_token(parser, TOKEN_IDENTIFIER) || 
        strcmp(parser->current_token.data.identifier.name, "review") != 0) {
        report_error(parser, "Expected 'review' annotation");
        return NULL;
    }
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected review priority");
        return NULL;
    }
    
    ReviewPriority priority;
    const char *priority_name = parser->current_token.data.identifier.name;
    
    if (strcmp(priority_name, "low") == 0) {
        priority = REVIEW_LOW;
    } else if (strcmp(priority_name, "medium") == 0) {
        priority = REVIEW_MEDIUM;
    } else if (strcmp(priority_name, "high") == 0) {
        priority = REVIEW_HIGH;
    } else {
        report_error(parser, "Unknown review priority");
        return NULL;
    }
    
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_HUMAN_REVIEW_TAG, start_loc);
    if (!node) return NULL;
    
    node->data.human_review_tag.priority = priority;
    
    return node;
}

// NEW: Parse bracketed annotations like #[deprecated], #[doc("text")], etc.
ASTNode *parse_bracketed_annotation(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Consume the # token
    if (!expect_token(parser, TOKEN_HASH)) {
        return NULL;
    }
    
    // Consume the [ token
    if (!expect_token(parser, TOKEN_LEFT_BRACKET)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected annotation name");
        return NULL;
    }
    
    char *annotation_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Parse optional parameters according to grammar:
    // AnnotationParams <- AnnotationParam (',' AnnotationParam)* / 'none'
    // AnnotationParam <- IDENT '=' AnnotationValue
    // AnnotationValue <- STRING / IDENT / INT / BOOL
    // Special case: ownership(value) format for ownership annotations
    char *parameters = NULL;
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);
        
        // Check for 'none' marker
        if (match_token(parser, TOKEN_IDENTIFIER) && 
            strcmp(parser->current_token.data.identifier.name, "none") == 0) {
            advance_token(parser);
            if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                free(annotation_name);
                return NULL;
            }
            // parameters remains NULL for 'none'
        } else {
            // Special handling for ownership annotation: ownership(gc|c|pinned)
            if (strcmp(annotation_name, "ownership") == 0) {
                if (!match_token(parser, TOKEN_IDENTIFIER)) {
                    report_error(parser, "Expected ownership type (gc, c, or pinned)");
                    free(annotation_name);
                    return NULL;
                }
                
                const char *ownership_type = parser->current_token.data.identifier.name;
                if (strcmp(ownership_type, "gc") != 0 && 
                    strcmp(ownership_type, "c") != 0 && 
                    strcmp(ownership_type, "pinned") != 0) {
                    report_error(parser, "Unknown ownership type. Expected 'gc', 'c', or 'pinned'");
                    free(annotation_name);
                    return NULL;
                }
                
                parameters = strdup(ownership_type);
                advance_token(parser);
                
                if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                    free(annotation_name);
                    free(parameters);
                    return NULL;
                }
            } else {
                // Parse standard AnnotationParam (',' AnnotationParam)*
                char param_buffer[1024] = {0};
                bool first_param = true;
                
                while (!match_token(parser, TOKEN_RIGHT_PAREN) && !at_end(parser)) {
                    if (!first_param) {
                        if (!expect_token(parser, TOKEN_COMMA)) {
                            free(annotation_name);
                            return NULL;
                        }
                        strcat(param_buffer, ", ");
                    }
                    
                    // Parse IDENT '=' AnnotationValue
                    if (!match_token(parser, TOKEN_IDENTIFIER)) {
                        report_error(parser, "Expected parameter name");
                        free(annotation_name);
                        return NULL;
                    }
                    
                    strcat(param_buffer, parser->current_token.data.identifier.name);
                    advance_token(parser);
                    
                    if (!expect_token(parser, TOKEN_ASSIGN)) {
                        free(annotation_name);
                        return NULL;
                    }
                    strcat(param_buffer, "=");
                    
                    // Parse AnnotationValue <- STRING / IDENT / INT / BOOL
                    if (match_token(parser, TOKEN_STRING)) {
                        strcat(param_buffer, "\"");
                        strcat(param_buffer, parser->current_token.data.string.value);
                        strcat(param_buffer, "\"");
                        advance_token(parser);
                    } else if (match_token(parser, TOKEN_IDENTIFIER)) {
                        strcat(param_buffer, parser->current_token.data.identifier.name);
                        advance_token(parser);
                    } else if (match_token(parser, TOKEN_INTEGER)) {
                        char int_str[32];
                        snprintf(int_str, sizeof(int_str), "%lld", (long long)parser->current_token.data.integer.value);
                        strcat(param_buffer, int_str);
                        advance_token(parser);
                    } else if (match_token(parser, TOKEN_BOOL_TRUE) || match_token(parser, TOKEN_BOOL_FALSE)) {
                        strcat(param_buffer, parser->current_token.type == TOKEN_BOOL_TRUE ? "true" : "false");
                        advance_token(parser);
                    } else {
                        report_error(parser, "Expected annotation value (string, identifier, integer, or boolean)");
                        free(annotation_name);
                        return NULL;
                    }
                    
                    first_param = false;
                }
                
                if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                    free(annotation_name);
                    return NULL;
                }
                
                // Store the parsed parameters
                if (strlen(param_buffer) > 0) {
                    parameters = strdup(param_buffer);
                }
            }
        }
    }
    
    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
        free(annotation_name);
        free(parameters);
        return NULL;
    }
    
    // Check if this is an ownership annotation and create the appropriate node type
    if (strcmp(annotation_name, "ownership") == 0 && parameters) {
        // Create AST_OWNERSHIP_TAG node for ownership annotations
        ASTNode *node = ast_create_node(AST_OWNERSHIP_TAG, start_loc);
        if (!node) {
            free(annotation_name);
            free(parameters);
            return NULL;
        }
        
        // Set ownership type based on parameter
        OwnershipType ownership;
        if (strcmp(parameters, "gc") == 0) {
            ownership = OWNERSHIP_GC;
        } else if (strcmp(parameters, "c") == 0) {
            ownership = OWNERSHIP_C;
        } else if (strcmp(parameters, "pinned") == 0) {
            ownership = OWNERSHIP_PINNED;
        } else {
            // This shouldn't happen as we validated earlier
            ownership = OWNERSHIP_GC;
        }
        
        node->data.ownership_tag.ownership = ownership;
        free(annotation_name);
        free(parameters);
        return node;
    }
    
    // Create a semantic tag node for general annotations
    ASTNode *node = ast_create_node(AST_SEMANTIC_TAG, start_loc);
    if (!node) {
        free(annotation_name);
        free(parameters);
        return NULL;
    }
    
    node->data.semantic_tag.name = annotation_name;
    
    // Store parameters in params list if we have any
    if (parameters) {
        // For now, we'll create a simple parameter list with one string node
        // This is a simplified implementation - a full implementation would parse individual parameters
        node->data.semantic_tag.params = ast_node_list_create(1);
        if (node->data.semantic_tag.params) {
            // Create a simple string node to hold the parameters
            ASTNode *param_node = ast_create_node(AST_STRING_LITERAL, start_loc);
            if (param_node) {
                param_node->data.string_literal.value = parameters;
                ast_node_list_add(&node->data.semantic_tag.params, param_node);
            } else {
                free(parameters);
            }
        } else {
            free(parameters);
        }
    } else {
        node->data.semantic_tag.params = NULL;
    }
    
    return node;
}

// =============================================================================
// ANNOTATION LIST PARSING
// =============================================================================

ASTNodeList *parse_annotation_list(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode **annotations = NULL;
    size_t annotation_count = 0;
    size_t annotation_capacity = 4;
    
    // Check if we actually have annotations before allocating
    if (!is_annotation_start(parser)) {
        return NULL;
    }
    
    annotations = malloc(annotation_capacity * sizeof(ASTNode*));
    if (!annotations) return NULL;
    
    while (is_annotation_start(parser)) {
        ASTNode *annotation = parse_annotation(parser);
        if (!annotation) {
            // If annotation parsing fails, clean up and return what we have
            break;
        }
        
        if (annotation_count >= annotation_capacity) {
            annotation_capacity *= 2;
            annotations = realloc(annotations, annotation_capacity * sizeof(ASTNode*));
            if (!annotations) {
                ast_free_node(annotation);
                return NULL;
            }
        }
        
        annotations[annotation_count++] = annotation;
    }
    
    if (annotation_count == 0) {
        free(annotations);
        return NULL;
    }
    
    // Convert annotations array to ASTNodeList
    ASTNodeList *list = ast_node_list_create(annotation_count);
    if (list) {
        for (size_t i = 0; i < annotation_count; i++) {
            ast_node_list_add(&list, annotations[i]);
        }
    }
    free(annotations);
    
    return list;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

bool is_annotation_start(Parser *parser) {
    if (!parser) return false;
    
    // Only #[...] annotations are supported now
    // Use check_token to avoid advancing the token
    if (check_token(parser, TOKEN_HASH)) {
        return true;
    }
    
    return false;
} 
