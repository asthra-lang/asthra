/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Generic Types Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Builtin generic type definitions (Option, Result)
 */

#ifndef ASTHRA_SEMANTIC_GENERIC_TYPES_H
#define ASTHRA_SEMANTIC_GENERIC_TYPES_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BUILTIN GENERIC TYPE MANAGEMENT
// =============================================================================

/**
 * Initialize builtin generic types (Option<T>, Result<T, E>)
 */
void semantic_init_builtin_generic_types(SemanticAnalyzer *analyzer);

/**
 * Create Option<T> type instance with specific type parameter
 */
TypeDescriptor *semantic_create_option_instance(SemanticAnalyzer *analyzer,
                                                TypeDescriptor *element_type);

/**
 * Create Result<T, E> type instance with specific type parameters
 */
TypeDescriptor *semantic_create_result_instance(SemanticAnalyzer *analyzer, TypeDescriptor *ok_type,
                                                TypeDescriptor *err_type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_GENERIC_TYPES_H