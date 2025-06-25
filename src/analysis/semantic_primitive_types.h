/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Primitive Types Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Primitive type definitions and management
 */

#ifndef ASTHRA_SEMANTIC_PRIMITIVE_TYPES_H
#define ASTHRA_SEMANTIC_PRIMITIVE_TYPES_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PRIMITIVE TYPE DEFINITIONS
// =============================================================================

/**
 * Get the static array of builtin primitive types
 * Returns pointer to the first element, count is PRIMITIVE_COUNT
 */
TypeDescriptor *semantic_get_primitive_types_array(void);

/**
 * Initialize primitive types in the analyzer
 */
void semantic_init_primitive_types(SemanticAnalyzer *analyzer);

/**
 * Get a primitive type by name (without analyzer context)
 */
TypeDescriptor *get_builtin_type(const char *name);

/**
 * Check if a type descriptor is a primitive type
 */
bool semantic_is_primitive_type(const TypeDescriptor *type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_PRIMITIVE_TYPES_H