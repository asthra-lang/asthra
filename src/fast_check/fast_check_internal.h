/**
 * Asthra Programming Language Compiler
 * Fast Check Internal Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This header declares internal bridge functions used by fast_check stubs
 * to interact with the semantic analyzer.
 */

#ifndef FAST_CHECK_INTERNAL_H
#define FAST_CHECK_INTERNAL_H

#include "../analysis/semantic_analyzer_core.h"
#include "../parser/ast_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SEMANTIC ANALYZER BRIDGE FUNCTIONS
// =============================================================================

/**
 * Create a semantic analyzer instance
 * 
 * @return Pointer to new semantic analyzer
 */
SemanticAnalyzer *semantic_analyzer_create_internal(void);

/**
 * Destroy a semantic analyzer instance
 * 
 * @param analyzer Semantic analyzer to destroy
 */
void semantic_analyzer_destroy_internal(SemanticAnalyzer *analyzer);

/**
 * Reset semantic analyzer state
 * 
 * @param analyzer Semantic analyzer to reset
 */
void semantic_analyzer_reset_internal(SemanticAnalyzer *analyzer);

/**
 * Analyze an AST program node
 * 
 * @param analyzer Semantic analyzer
 * @param program AST program node to analyze
 * @return true on success, false on failure
 */
bool semantic_analyze_program_internal(SemanticAnalyzer *analyzer, ASTNode *program);

/**
 * Get the error count from semantic analysis
 * 
 * @param analyzer Semantic analyzer
 * @return Number of errors encountered
 */
size_t semantic_get_error_count_internal(SemanticAnalyzer *analyzer);

#ifdef __cplusplus
}
#endif

#endif // FAST_CHECK_INTERNAL_H