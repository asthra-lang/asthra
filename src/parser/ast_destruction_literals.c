/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Literals and Annotations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file contains destruction operations for literal and annotation AST nodes
 * Uses common patterns from ast_destruction_common.h
 */

#include "ast_destruction_common.h"
#include "ast_destruction_literals.h"

void ast_free_literal_nodes(ASTNode *node) {
    AST_DESTRUCTION_NULL_CHECK(node);
    
    switch (node->type) {
        case AST_STRING_LITERAL:
            AST_FREE_STRING(node->data.string_literal.value);
            break;
            
        case AST_CHAR_LITERAL:
            // No dynamic allocations for AST_CHAR_LITERAL
            break;
            
        case AST_UNIT_LITERAL:
            // No dynamic allocations for AST_UNIT_LITERAL
            break;
            
        case AST_STRUCT_LITERAL:
            AST_DESTROY_STRUCT_LITERAL(node);
            break;
            
        case AST_IDENTIFIER:
            AST_FREE_STRING(node->data.identifier.name);
            break;
            
        case AST_SEMANTIC_TAG:
            AST_FREE_STRING(node->data.semantic_tag.name);
            AST_DESTROY_NODE_LIST(node->data.semantic_tag.params);
            break;
            
        default:
            // Not a literal/annotation node
            break;
    }
} 
