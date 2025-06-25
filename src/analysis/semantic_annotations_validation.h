/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Validation Module
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core validation functions for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_VALIDATION_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_VALIDATION_H

#include "../parser/ast.h"
#include "../parser/ast_node_list.h"
#include "semantic_annotations_registry.h"
#include "semantic_core.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CORE VALIDATION FUNCTIONS
// =============================================================================

/**
 * Analyze a semantic tag annotation
 * @param analyzer The semantic analyzer instance
 * @param tag_node The AST node representing the semantic tag
 * @return true if the annotation is valid, false otherwise
 */
bool analyze_semantic_tag(SemanticAnalyzer *analyzer, ASTNode *tag_node);

/**
 * Validate annotation context appropriateness
 * @param analyzer The semantic analyzer instance
 * @param annotation_name The name of the annotation
 * @param target_node The node being annotated
 * @return true if context is appropriate, false otherwise
 */
bool validate_annotation_context(SemanticAnalyzer *analyzer, const char *annotation_name,
                                 ASTNode *target_node);

/**
 * Validate annotation context with inheritance support
 * @param analyzer The semantic analyzer instance
 * @param annotation_name The name of the annotation
 * @param target_node The node being annotated
 * @param allow_inheritance Whether to allow relaxed context checking
 * @return true if context is appropriate, false otherwise
 */
bool validate_annotation_context_with_inheritance(SemanticAnalyzer *analyzer,
                                                  const char *annotation_name, ASTNode *target_node,
                                                  bool allow_inheritance);

/**
 * Check if a node has non-deterministic annotation
 * @param node The AST node to check
 * @return true if the node has #[non_deterministic] annotation, false otherwise
 */
bool has_non_deterministic_annotation(ASTNode *node);

// =============================================================================
// ANNOTATION ANALYSIS INTEGRATION
// =============================================================================

/**
 * Analyze annotations on declarations
 * @param analyzer The semantic analyzer instance
 * @param decl The declaration node to analyze
 * @return true if all annotations are valid, false otherwise
 */
bool analyze_declaration_annotations(SemanticAnalyzer *analyzer, ASTNode *decl);

/**
 * Analyze annotations on statements
 * @param analyzer The semantic analyzer instance
 * @param stmt The statement node to analyze
 * @return true if all annotations are valid, false otherwise
 */
bool analyze_statement_annotations(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze annotations on expressions
 * @param analyzer The semantic analyzer instance
 * @param expr The expression node to analyze
 * @return true if all annotations are valid, false otherwise
 */
bool analyze_expression_annotations(SemanticAnalyzer *analyzer, ASTNode *expr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_VALIDATION_H
