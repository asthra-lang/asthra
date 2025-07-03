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

#include "ast_destruction_patterns.h"
#include "ast_destruction_common.h"

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
        AST_RELEASE_NODE(node->data.enum_pattern.pattern);
        break;

    default:
        // Not a pattern node
        break;
    }
}
