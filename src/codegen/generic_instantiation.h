/**
 * Asthra Programming Language Compiler
 * Generic Instantiation and Monomorphization
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 4: Code Generation for Generic Structs
 * Implements monomorphization of generic structs into concrete C struct definitions
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_H
#define ASTHRA_GENERIC_INSTANTIATION_H

#include "code_generator_types.h"
#include "../analysis/semantic_analyzer.h"
#include "../parser/ast.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// GENERIC INSTANTIATION DATA STRUCTURES
// =============================================================================

// Type definitions are now in generic_instantiation_types.h to avoid redefinition
// Include the types header to get the struct definitions
#include "generic_instantiation_types.h"

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

// =============================================================================
// CODE GENERATION FOR INSTANTIATIONS
// =============================================================================

/**
 * Generate C struct definition for a concrete instantiation
 */
bool generate_concrete_struct_definition(CodeGenerator *generator,
                                        GenericInstantiation *instantiation,
                                        GenericStructInfo *struct_info);

/**
 * Generate all struct definitions for instantiations in the registry
 */
bool generate_all_instantiated_structs(CodeGenerator *generator, 
                                      GenericRegistry *registry,
                                      char *output_buffer,
                                      size_t buffer_size);

/**
 * Generate field access functions for concrete structs (optional optimization)
 */
bool generate_struct_field_accessors(CodeGenerator *generator,
                                    GenericInstantiation *instantiation,
                                    GenericStructInfo *struct_info);

// =============================================================================
// TYPE SUBSTITUTION UTILITIES
// =============================================================================

/**
 * Substitute type parameters in a field type with concrete types
 */
TypeDescriptor *substitute_type_parameters(TypeDescriptor *field_type,
                                          char **type_param_names,
                                          TypeDescriptor **type_args,
                                          size_t type_param_count);

/**
 * Check if a type descriptor references any type parameters
 */
bool type_contains_type_parameters(TypeDescriptor *type,
                                  char **type_param_names,
                                  size_t type_param_count);

/**
 * Convert a TypeDescriptor to its C type string representation
 */
char *type_descriptor_to_c_string(TypeDescriptor *type);

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
 * Check for circular dependencies in generic instantiations
 */
bool check_instantiation_cycles(GenericRegistry *registry,
                               const char *struct_name,
                               TypeDescriptor **type_args,
                               size_t type_arg_count);

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

/**
 * Create a new generic instantiation
 */
GenericInstantiation *generic_instantiation_create(const char *concrete_name,
                                                  TypeDescriptor **type_args,
                                                  size_t type_arg_count);

/**
 * Retain a reference to a generic instantiation
 */
void generic_instantiation_retain(GenericInstantiation *instantiation);

/**
 * Release a reference to a generic instantiation
 */
void generic_instantiation_release(GenericInstantiation *instantiation);

/**
 * Create a new generic struct info
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
 * Print instantiation statistics
 */
void generic_registry_print_stats(GenericRegistry *registry);

/**
 * Get instantiation information for debugging
 */
char *generic_instantiation_debug_string(GenericInstantiation *instantiation);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_H 
