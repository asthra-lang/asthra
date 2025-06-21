/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Validation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Validation and error handling for generic struct instantiations
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_VALIDATION_H
#define ASTHRA_GENERIC_INSTANTIATION_VALIDATION_H

#include "generic_instantiation_types.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// VALIDATION AND ERROR HANDLING
// =============================================================================

/**
 * Validate that type arguments are compatible with generic struct constraints
 */
bool validate_instantiation_args(GenericStructInfo *struct_info,
                                TypeDescriptor **type_args,
                                size_t type_arg_count);

/**
 * Check for circular dependencies in generic instantiation
 */
bool check_instantiation_cycles(GenericRegistry *registry,
                               const char *struct_name,
                               TypeDescriptor **type_args,
                               size_t type_arg_count);

/**
 * Validate that a generic struct can be instantiated with given constraints
 */
bool validate_generic_struct_constraints(GenericStructInfo *struct_info,
                                        TypeDescriptor **type_args,
                                        size_t type_arg_count);

/**
 * Check if instantiation would create FFI compatibility issues
 */
bool validate_ffi_compatibility(GenericStructInfo *struct_info,
                               TypeDescriptor **type_args,
                               size_t type_arg_count);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_VALIDATION_H 
