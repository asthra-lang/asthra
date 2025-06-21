/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Code Generation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Code generation for concrete struct definitions from generic instantiations
 */

#ifndef ASTHRA_GENERIC_INSTANTIATION_CODEGEN_H
#define ASTHRA_GENERIC_INSTANTIATION_CODEGEN_H

#include "generic_instantiation_types.h"
#include "code_generator_core.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GENERIC_INSTANTIATION_CODEGEN_H 
