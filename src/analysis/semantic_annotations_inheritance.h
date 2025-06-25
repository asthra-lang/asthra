/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Inheritance Module
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Inheritance and dependency validation for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_INHERITANCE_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_INHERITANCE_H

#include "../parser/ast.h"
#include "../parser/ast_node_list.h"
#include "semantic_annotations_registry.h"
#include "semantic_core.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// INHERITANCE AND DEPENDENCY FUNCTIONS
// =============================================================================

/**
 * Analyze annotation inheritance from parent scopes
 * @param analyzer The semantic analyzer instance
 * @param node The AST node to analyze for inheritance
 * @return true if inheritance analysis passes, false otherwise
 */
bool analyze_annotation_inheritance(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Validate annotation dependencies and requirements
 * @param analyzer The semantic analyzer instance
 * @param node The AST node being validated
 * @param annotations The list of annotations to check dependencies for
 * @return true if all dependencies are satisfied, false otherwise
 */
bool validate_annotation_dependencies(SemanticAnalyzer *analyzer, ASTNode *node,
                                      ASTNodeList *annotations);

/**
 * Check annotation scope resolution and accessibility
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the annotation is used
 * @param annotation_name The name of the annotation to check
 * @return true if annotation is accessible in current scope, false otherwise
 */
bool check_annotation_scope_resolution(SemanticAnalyzer *analyzer, ASTNode *node,
                                       const char *annotation_name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_INHERITANCE_H
