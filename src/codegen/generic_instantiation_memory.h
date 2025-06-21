/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Memory Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Memory management for generic instantiation objects with reference counting
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_MEMORY_H
#define ASTHRA_GENERIC_INSTANTIATION_MEMORY_H

#include "generic_instantiation_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

/**
 * Create a new generic instantiation with reference counting
 */
GenericInstantiation *generic_instantiation_create(const char *concrete_name,
                                                  TypeDescriptor **type_args,
                                                  size_t type_arg_count);

/**
 * Increment reference count for an instantiation
 */
void generic_instantiation_retain(GenericInstantiation *instantiation);

/**
 * Decrement reference count and cleanup if needed
 */
void generic_instantiation_release(GenericInstantiation *instantiation);

/**
 * Create a new generic struct info object
 */
GenericStructInfo *generic_struct_info_create(const char *struct_name,
                                             ASTNode *struct_decl,
                                             TypeDescriptor *struct_type);

/**
 * Destroy a generic struct info and all its instantiations
 */
void generic_struct_info_destroy(GenericStructInfo *struct_info);

// =============================================================================
// DEBUG AND INTROSPECTION
// =============================================================================

/**
 * Generate debug string for an instantiation
 */
char *generic_instantiation_debug_string(GenericInstantiation *instantiation);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_MEMORY_H 
