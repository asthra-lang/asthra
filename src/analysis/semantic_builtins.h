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

// Include modular components
#include "semantic_primitive_types.h"
#include "semantic_predeclared_functions.h"
#include "semantic_generic_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BUILTIN TYPE MANAGEMENT
// =============================================================================

/**
 * Initialize all builtin types in the analyzer
 * This includes primitive types, predeclared functions, and generic types
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
 * Initialize all predeclared identifiers
 * This includes functions and generic types
 */
void semantic_init_predeclared_identifiers(SemanticAnalyzer *analyzer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_BUILTINS_H
