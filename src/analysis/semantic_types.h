/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type System Module Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Type descriptor operations and type system functionality
 */

#ifndef ASTHRA_SEMANTIC_TYPES_H
#define ASTHRA_SEMANTIC_TYPES_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE DESCRIPTOR MANAGEMENT
// =============================================================================

/**
 * Create a primitive type descriptor
 */
TypeDescriptor *type_descriptor_create_primitive(int primitive_kind);

/**
 * Create a struct type descriptor
 */
TypeDescriptor *type_descriptor_create_struct(const char *struct_name, size_t field_count);

/**
 * Create a slice type descriptor
 */
TypeDescriptor *type_descriptor_create_slice(TypeDescriptor *element_type);

/**
 * Create a result type descriptor
 */
TypeDescriptor *type_descriptor_create_result(TypeDescriptor *ok_type, TypeDescriptor *err_type);

/**
 * Create a pointer type descriptor
 */
TypeDescriptor *type_descriptor_create_pointer(TypeDescriptor *pointee_type);

/**
 * Create a function type descriptor
 */
TypeDescriptor *type_descriptor_create_function(void);

/**
 * Create a function type descriptor with parameters
 */
TypeDescriptor *type_descriptor_create_function_with_params(TypeDescriptor *return_type, size_t param_count);

/**
 * Create a generic instance type descriptor
 * @param base_type The generic type template (e.g., Vec<T>)
 * @param type_args Array of concrete type arguments (e.g., [i32])
 * @param type_arg_count Number of type arguments
 */
TypeDescriptor *type_descriptor_create_generic_instance(TypeDescriptor *base_type, 
                                                       TypeDescriptor **type_args, 
                                                       size_t type_arg_count);

/**
 * Create a tuple type descriptor
 * @param element_types Array of element types
 * @param element_count Number of elements (must be at least 2)
 */
TypeDescriptor *type_descriptor_create_tuple(TypeDescriptor **element_types, size_t element_count);

/**
 * Retain a type descriptor (increase reference count)
 */
void type_descriptor_retain(TypeDescriptor *type);

/**
 * Release a type descriptor (decrease reference count)
 */
void type_descriptor_release(TypeDescriptor *type);

/**
 * Check if two type descriptors are equal
 */
bool type_descriptor_equals(const TypeDescriptor *type1, const TypeDescriptor *type2);

/**
 * Generate hash code for a type descriptor
 */
size_t type_descriptor_hash(const TypeDescriptor *type);

// =============================================================================
// STRUCT TYPE OPERATIONS
// =============================================================================

/**
 * Add a field to a struct type descriptor
 */
bool type_descriptor_add_struct_field(TypeDescriptor *struct_type, 
                                     const char *field_name, 
                                     TypeDescriptor *field_type,
                                     ASTNode *field_declaration);

/**
 * Look up a field in a struct type descriptor
 */
SymbolEntry *type_descriptor_lookup_struct_field(TypeDescriptor *struct_type, const char *field_name);

// =============================================================================
// TYPE RESOLUTION UTILITIES
// =============================================================================

/**
 * Convert an AST type node to a TypeDescriptor
 */
TypeDescriptor *analyze_type_node(SemanticAnalyzer *analyzer, ASTNode *type_node);

/**
 * Get builtin type by name
 */
TypeDescriptor *get_builtin_type(const char *name);

/**
 * Create type creation helpers
 */
TypeDescriptor *create_slice_type(TypeDescriptor *element_type);
TypeDescriptor *create_result_type(TypeDescriptor *ok_type, TypeDescriptor *err_type);

/**
 * Check if a type is the Never type (bottom type)
 */
bool type_is_never(const TypeDescriptor *type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_TYPES_H 
