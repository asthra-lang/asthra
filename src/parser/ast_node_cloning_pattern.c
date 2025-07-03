/**
 * Asthra Programming Language Compiler
 * AST Node Cloning - Pattern Nodes
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains cloning implementations for pattern AST nodes.
 */

#include "ast_node_cloning_internal.h"
#include <string.h>

ASTNode *clone_pattern_node(ASTNode *node, ASTNode *clone) {
    switch (node->type) {
    case AST_MATCH_ARM:
        clone->data.match_arm.pattern = ast_clone_node(node->data.match_arm.pattern);
        clone->data.match_arm.body = ast_clone_node(node->data.match_arm.body);
        clone->data.match_arm.guard = ast_clone_node(node->data.match_arm.guard);
        break;

    case AST_TUPLE_PATTERN:
        clone->data.tuple_pattern.patterns =
            ast_node_list_clone_deep(node->data.tuple_pattern.patterns);
        break;

    case AST_ENUM_PATTERN:
        if (node->data.enum_pattern.enum_name) {
            clone->data.enum_pattern.enum_name = strdup(node->data.enum_pattern.enum_name);
        }
        if (node->data.enum_pattern.variant_name) {
            clone->data.enum_pattern.variant_name = strdup(node->data.enum_pattern.variant_name);
        }
        if (node->data.enum_pattern.binding) {
            clone->data.enum_pattern.binding = strdup(node->data.enum_pattern.binding);
        }
        clone->data.enum_pattern.pattern = ast_clone_node(node->data.enum_pattern.pattern);
        break;

    default:
        // Should not happen
        break;
    }

    return clone;
}