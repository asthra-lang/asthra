/**
 * FFI Test Stubs - AST and Parser Implementation
 * 
 * Provides enhanced AST and parser stub functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_stubs_ast_parser.h"
#include "ffi_stubs_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// ENHANCED AST IMPLEMENTATION
// =============================================================================

EnhancedASTNode* enhanced_ast_create_node(ASTNodeType type, void *data, size_t data_size) {
    EnhancedASTNode *node = ffi_allocate_memory(sizeof(EnhancedASTNode)); // Use FFI allocator
    if (!node) return NULL;
    
    node->node_type = type;
    node->data = data;
    node->data_size = data_size;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    atomic_store(&node->ref_count, 1);
    node->is_managed = true;
    
    return node;
}

void enhanced_ast_retain_node(EnhancedASTNode *node) {
    if (node && node->is_managed) {
        atomic_fetch_add(&node->ref_count, 1);
    }
}

void enhanced_ast_release_node(EnhancedASTNode *node) {
    if (!node || !node->is_managed) return;
    
    uint32_t old_count = atomic_fetch_sub(&node->ref_count, 1);
    if (old_count == 1) {
        // Last reference, free the node
        enhanced_ast_release_node(node->left);
        enhanced_ast_release_node(node->right);
        if (node->data) {
            ffi_free_memory(node->data, node->data_size); // Use FFI deallocator
        }
        ffi_free_memory(node, sizeof(EnhancedASTNode)); // Use FFI deallocator
    }
}

void ast_free_node(ASTNode *node) {
    if (node) {
        // This is a legacy wrapper. For proper FFI memory tracking,
        // all AST node allocations should eventually go through enhanced_ast_create_node
        // and be released via enhanced_ast_release_node.
        // For now, we will just free the top-level node if it's not managed.
        EnhancedASTNode *enhanced_node = (EnhancedASTNode*)node;
        if (enhanced_node->is_managed) {
            enhanced_ast_release_node(enhanced_node);
        } else {
            // If not managed by enhanced_ast_create_node, assume it was malloced directly
            // and perform a basic free to prevent immediate leaks in tests.
            // This path should ideally be removed once all AST creation is unified.
            free(enhanced_node->data);
            free(enhanced_node);
        }
    }
}

// =============================================================================
// ENHANCED PARSER IMPLEMENTATION
// =============================================================================

EnhancedParser* enhanced_parser_create(void) {
    EnhancedParser *parser = malloc(sizeof(EnhancedParser));
    if (!parser) return NULL;
    
    parser->initialized = true;
    atomic_store(&parser->parse_count, 0);
    atomic_store(&parser->successful_parses, 0);
    atomic_store(&parser->failed_parses, 0);
    parser->last_error[0] = '\0';
    
    printf("[PARSER] Enhanced parser created\n");
    return parser;
}

void enhanced_parser_destroy(EnhancedParser *parser) {
    if (parser) {
        printf("[PARSER] Enhanced parser destroyed\n");
        free(parser);
    }
}

EnhancedASTNode* enhanced_parser_parse_program(EnhancedParser *parser, const char *input) {
    if (!parser || !parser->initialized || !input) {
        if (parser) {
            atomic_fetch_add(&parser->failed_parses, 1);
            strncpy(parser->last_error, "Invalid parser or input", sizeof(parser->last_error) - 1);
        }
        return NULL;
    }
    
    atomic_fetch_add(&parser->parse_count, 1);
    
    // Create a mock enhanced AST node for testing
    // Make a copy of the input data since it will be freed later
    size_t input_len = strlen(input);
    void *input_copy = ffi_allocate_memory(input_len + 1);
    if (input_copy) {
        memcpy(input_copy, input, input_len + 1);
    }
    
    EnhancedASTNode *enhanced_ast = enhanced_ast_create_node(
        AST_PROGRAM, 
        input_copy, 
        input_len + 1
    );
    
    if (enhanced_ast) {
        atomic_fetch_add(&parser->successful_parses, 1);
        printf("[PARSER] Successfully parsed program with %zu characters\n", strlen(input));
    } else {
        atomic_fetch_add(&parser->failed_parses, 1);
        strncpy(parser->last_error, "Enhanced AST creation failed", sizeof(parser->last_error) - 1);
    }
    
    return enhanced_ast;
}

ASTNode* parser_parse(const char *input) {
    if (!input) return NULL;
    
    // Create a simple mock AST node for testing
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node) {
        node->type = AST_PROGRAM;
        // Initialize all fields to default values
        node->location = (SourceLocation){0};
        node->type_info = NULL;
        node->ref_count = 1;
        memset(&node->flags, 0, sizeof(node->flags));
    }
    return node;
}