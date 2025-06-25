/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Builtin Types and Predeclared Identifiers Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Builtin type definitions and predeclared identifier management
 */

#ifndef ASTHRA_SEMANTIC_BUILTINS_H
#define ASTHRA_SEMANTIC_BUILTINS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BUILTIN TYPE MANAGEMENT
// =============================================================================

/**
 * Initialize builtin types in the analyzer
 */
void semantic_init_builtin_types(SemanticAnalyzer *analyzer);

/**
 * Get a builtin type by name
 */
TypeDescriptor *semantic_get_builtin_type(SemanticAnalyzer *analyzer, const char *name);

/**
 * Check if a type descriptor is a builtin type
 */
bool semantic_is_builtin_type(const TypeDescriptor *type);

// =============================================================================
// PREDECLARED IDENTIFIERS
// =============================================================================

/**
 * Initialize predeclared identifiers (log, range, etc.)
 */
void semantic_init_predeclared_identifiers(SemanticAnalyzer *analyzer);

/**
 * Initialize builtin generic types (Option, Result, etc.)
 */
void semantic_init_builtin_generic_types(SemanticAnalyzer *analyzer);

/**
 * Create a function type descriptor from a name and signature string
 */
TypeDescriptor *create_predeclared_function_type(const char *name, const char *signature);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_BUILTINS_H
