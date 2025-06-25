/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Conflicts Module
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Conflict detection and duplicate checking for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_CONFLICTS_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_CONFLICTS_H

#include "../parser/ast.h"
#include "../parser/ast_node_list.h"
#include "semantic_annotations_registry.h"
#include "semantic_core.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFLICT DETECTION FUNCTIONS
// =============================================================================

/**
 * Check for annotation conflicts in a list of annotations
 * @param analyzer The semantic analyzer instance
 * @param node The AST node being annotated
 * @param annotations The list of annotations to check
 * @return true if no conflicts found, false if conflicts detected
 */
bool check_annotation_conflicts(SemanticAnalyzer *analyzer, ASTNode *node,
                                ASTNodeList *annotations);

/**
 * Check for duplicate annotations in a list
 * @param analyzer The semantic analyzer instance
 * @param node The AST node being annotated
 * @param annotations The list of annotations to check
 * @return true if no duplicates found, false if duplicates detected
 */
bool check_duplicate_annotations(SemanticAnalyzer *analyzer, ASTNode *node,
                                 ASTNodeList *annotations);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_CONFLICTS_H
