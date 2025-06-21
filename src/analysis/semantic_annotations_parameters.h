/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Parameters Module
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Parameter validation for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_PARAMETERS_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_PARAMETERS_H

#include "semantic_annotations_registry.h"
#include "semantic_core.h"
#include "../parser/ast.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PARAMETER VALIDATION FUNCTIONS
// =============================================================================

/**
 * Validate annotation parameters according to schema
 * @param analyzer The semantic analyzer instance
 * @param tag_node The semantic tag node with parameters
 * @param schema The parameter schema to validate against
 * @return true if parameters are valid, false otherwise
 */
bool validate_annotation_parameters(SemanticAnalyzer *analyzer, ASTNode *tag_node, const AnnotationParameterSchema *schema);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_PARAMETERS_H 
