/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Core Module Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core semantic analyzer lifecycle and main analysis functions
 */

#ifndef ASTHRA_SEMANTIC_CORE_H
#define ASTHRA_SEMANTIC_CORE_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CORE ANALYZER LIFECYCLE
// =============================================================================

/**
 * Create a new semantic analyzer instance
 */
SemanticAnalyzer *semantic_analyzer_create(void);

/**
 * Destroy a semantic analyzer and free all resources
 */
void semantic_analyzer_destroy(SemanticAnalyzer *analyzer);

/**
 * Reset analyzer state for reuse
 */
void semantic_analyzer_reset(SemanticAnalyzer *analyzer);

/**
 * Enable or disable test mode for more permissive analysis
 */
void semantic_analyzer_set_test_mode(SemanticAnalyzer *analyzer, bool enable);

// =============================================================================
// MAIN ANALYSIS FUNCTIONS
// =============================================================================

/**
 * Analyze a complete program AST
 */
bool semantic_analyze_program(SemanticAnalyzer *analyzer, ASTNode *program);

/**
 * Analyze a declaration node (function, struct, extern, etc.)
 */
bool semantic_analyze_declaration(SemanticAnalyzer *analyzer, ASTNode *decl);

/**
 * Analyze a statement node
 */
bool semantic_analyze_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze an expression node
 */
bool semantic_analyze_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_CORE_H 
