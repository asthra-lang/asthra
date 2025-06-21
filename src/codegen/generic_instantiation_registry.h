/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Registry Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Registry management for generic struct instantiations with thread-safe operations
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_REGISTRY_H
#define ASTHRA_GENERIC_INSTANTIATION_REGISTRY_H

#include "generic_instantiation_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// GENERIC REGISTRY MANAGEMENT
// =============================================================================

/**
 * Create a new generic registry for tracking instantiations
 */
GenericRegistry *generic_registry_create(void);

/**
 * Destroy a generic registry and all its instantiations
 */
void generic_registry_destroy(GenericRegistry *registry);

/**
 * Register a generic struct declaration for future instantiation
 */
bool generic_registry_register_struct(GenericRegistry *registry, 
                                     const char *struct_name,
                                     ASTNode *struct_decl,
                                     TypeDescriptor *struct_type);

/**
 * Look up a generic struct by name
 */
GenericStructInfo *generic_registry_lookup_struct(GenericRegistry *registry, 
                                                 const char *struct_name);

/**
 * Print registry statistics for debugging
 */
void generic_registry_print_stats(GenericRegistry *registry);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_REGISTRY_H 
