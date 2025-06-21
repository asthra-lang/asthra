/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Core Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core instantiation operations for generic struct monomorphization
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_CORE_H
#define ASTHRA_GENERIC_INSTANTIATION_CORE_H

#include "generic_instantiation_types.h"
#include "code_generator_core.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// GENERIC INSTANTIATION MANAGEMENT
// =============================================================================

/**
 * Request instantiation of a generic struct with specific type arguments
 * Returns existing instantiation if already created, creates new one otherwise
 */
GenericInstantiation *generic_instantiate_struct(GenericRegistry *registry,
                                                const char *struct_name,
                                                TypeDescriptor **type_args,
                                                size_t type_arg_count,
                                                CodeGenerator *generator);

/**
 * Generate a concrete name for a generic instantiation
 * Examples: Vec<i32> -> Vec_i32, HashMap<string, User> -> HashMap_string_User
 */
char *generate_concrete_struct_name(const char *generic_name, 
                                   TypeDescriptor **type_args, 
                                   size_t type_arg_count);

/**
 * Look up an existing instantiation by type arguments
 */
GenericInstantiation *find_existing_instantiation(GenericStructInfo *struct_info,
                                                 TypeDescriptor **type_args,
                                                 size_t type_arg_count);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_CORE_H 
