/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Resolution Helpers Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Type resolution utilities and helpers
 */

#ifndef ASTHRA_SEMANTIC_TYPES_RESOLUTION_H
#define ASTHRA_SEMANTIC_TYPES_RESOLUTION_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE RESOLUTION HELPERS
// =============================================================================

/**
 * Resolve type from AST node
 */
TypeDescriptor *resolve_type_from_ast(SemanticAnalyzer *analyzer, ASTNode *type_node);

/**
 * Create type node from descriptor
 */
ASTNode *create_type_node_from_descriptor(TypeDescriptor *type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_TYPES_RESOLUTION_H 
