/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Function Declaration Analysis Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of function declarations
 */

#ifndef ASTHRA_SEMANTIC_FUNCTIONS_H
#define ASTHRA_SEMANTIC_FUNCTIONS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FUNCTION DECLARATION ANALYSIS
// =============================================================================

/**
 * Analyze a function declaration and its body
 */
bool analyze_function_declaration(SemanticAnalyzer *analyzer, ASTNode *func_decl);

// =============================================================================
// EXTERN DECLARATION ANALYSIS
// =============================================================================

/**
 * Analyze an extern function declaration
 */
bool analyze_extern_declaration(SemanticAnalyzer *analyzer, ASTNode *extern_decl);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_FUNCTIONS_H
