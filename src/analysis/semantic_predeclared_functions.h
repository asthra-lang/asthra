/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Predeclared Functions Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Predeclared function definitions and management
 */

#ifndef ASTHRA_SEMANTIC_PREDECLARED_FUNCTIONS_H
#define ASTHRA_SEMANTIC_PREDECLARED_FUNCTIONS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PREDECLARED FUNCTION MANAGEMENT
// =============================================================================

/**
 * Initialize predeclared functions (log, range, panic, args, infinite)
 */
void semantic_init_predeclared_functions(SemanticAnalyzer *analyzer);

/**
 * Create a function type descriptor from a name and signature string
 */
TypeDescriptor *create_predeclared_function_type(const char *name, const char *signature);

/**
 * Get the count of predeclared functions
 */
size_t semantic_get_predeclared_function_count(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_PREDECLARED_FUNCTIONS_H