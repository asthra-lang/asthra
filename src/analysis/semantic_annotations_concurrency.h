/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Concurrency Module
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tier 1/2 concurrency annotation handling for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_CONCURRENCY_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_CONCURRENCY_H

#include "semantic_annotations_registry.h"
#include "semantic_core.h"
#include "../parser/ast.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONCURRENCY ANNOTATION FUNCTIONS
// =============================================================================

/**
 * Validate non-deterministic annotation
 * @param analyzer The semantic analyzer instance
 * @param tag_node The AST node representing the semantic tag
 * @param target_node The AST node that is annotated (function, statement, etc.)
 * @return true if the annotation is valid and properly applied, false otherwise
 */
bool validate_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *tag_node, ASTNode *target_node);

/**
 * Check if a node requires non-deterministic annotation
 * @param analyzer The semantic analyzer instance
 * @param node The AST node to check
 * @return true if the node requires #[non_deterministic] annotation, false otherwise
 */
bool requires_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Validate that Tier 2 concurrency features have non-deterministic annotation
 * @param analyzer The semantic analyzer instance
 * @param node The AST node to validate
 * @return true if validation passes, false if annotation is missing or invalid
 */
bool validate_tier2_concurrency_annotation(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Enhanced analysis for Tier 1 concurrency features (spawn, spawn_with_handle, await)
 * @param analyzer The semantic analyzer instance
 * @param node The AST node representing the Tier 1 concurrency feature
 * @return true if analysis passes, false otherwise
 */
bool analyze_tier1_concurrency_feature(SemanticAnalyzer *analyzer, ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_CONCURRENCY_H 
