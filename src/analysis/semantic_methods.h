/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Method and Impl Block Analysis Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of impl blocks, methods, and visibility checking
 */

#ifndef ASTHRA_SEMANTIC_METHODS_H
#define ASTHRA_SEMANTIC_METHODS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// IMPL BLOCK AND METHOD ANALYSIS
// =============================================================================

/**
 * Analyze an impl block and its methods
 */
bool analyze_impl_block(SemanticAnalyzer *analyzer, ASTNode *impl_block);

/**
 * Analyze a method declaration within an impl block
 */
bool analyze_method_declaration(SemanticAnalyzer *analyzer, ASTNode *method_decl,
                                const char *struct_name);

/**
 * Check visibility rules for method access
 */
bool check_method_visibility(SemanticAnalyzer *analyzer, const char *struct_name,
                             const char *method_name, SourceLocation location);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_METHODS_H
