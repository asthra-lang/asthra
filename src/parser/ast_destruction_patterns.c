/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Patterns
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file contains destruction operations for pattern matching AST nodes
 * Uses common patterns from ast_destruction_common.h
 */

#include "ast_destruction_common.h"
#include "ast_destruction_patterns.h"

void ast_free_pattern_nodes(ASTNode *node) {
    AST_DESTRUCTION_NULL_CHECK(node);
    
    switch (node->type) {
        case AST_MATCH_ARM:
            AST_RELEASE_NODE(node->data.match_arm.pattern);
            AST_RELEASE_NODE(node->data.match_arm.body);
            break;
            
        case AST_ENUM_PATTERN:
            AST_FREE_STRING(node->data.enum_pattern.enum_name);
            AST_FREE_STRING(node->data.enum_pattern.variant_name);
            AST_FREE_STRING(node->data.enum_pattern.binding);
            break;
            
        case AST_STRUCT_PATTERN:
            AST_FREE_STRING(node->data.struct_pattern.struct_name);
            AST_DESTROY_NODE_LIST(node->data.struct_pattern.type_args);
            AST_DESTROY_NODE_LIST(node->data.struct_pattern.field_patterns);
            break;
            
        case AST_FIELD_PATTERN:
            AST_FREE_STRING(node->data.field_pattern.field_name);
            AST_FREE_STRING(node->data.field_pattern.binding_name);
            break;
            
        default:
            // Not a pattern node
            break;
    }
} 
