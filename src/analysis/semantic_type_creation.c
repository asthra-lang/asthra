/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Creation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Factory functions for creating various type descriptors
 */

#include "semantic_type_creation.h"
#include "semantic_type_descriptors.h"
#include "semantic_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// TYPE DESCRIPTOR CREATION FUNCTIONS
// =============================================================================

TypeDescriptor *type_descriptor_create_struct(const char *name, size_t field_count) {
    if (!name) return NULL;
    
    TypeDescriptor *struct_type = malloc(sizeof(TypeDescriptor));
    if (!struct_type) return NULL;
    
    struct_type->category = TYPE_STRUCT;
    struct_type->flags.is_mutable = false;
    struct_type->flags.is_owned = false;
    struct_type->flags.is_borrowed = false;
    struct_type->flags.is_constant = false;
    struct_type->flags.is_volatile = false;
    struct_type->flags.is_atomic = false;
    struct_type->flags.is_ffi_compatible = true;
    struct_type->flags.reserved = 0;
    
    struct_type->size = 0; // Would be calculated based on fields
    struct_type->alignment = 1;
    struct_type->name = strdup(name);
    atomic_init(&struct_type->ref_count, 1); // Initialize reference count
    
    // Initialize struct data
    struct_type->data.struct_type.field_count = field_count;
    struct_type->data.struct_type.fields = NULL; // Could be allocated if needed
    struct_type->data.struct_type.methods = NULL;
    
    return struct_type;
}

TypeDescriptor *type_descriptor_create_pointer(TypeDescriptor *pointee_type) {
    if (!pointee_type) return NULL;
    
    TypeDescriptor *ptr_type = malloc(sizeof(TypeDescriptor));
    if (!ptr_type) return NULL;
    
    ptr_type->category = TYPE_POINTER;
    ptr_type->flags.is_mutable = false;
    ptr_type->flags.is_owned = false;
    ptr_type->flags.is_borrowed = false;
    ptr_type->flags.is_constant = false;
    ptr_type->flags.is_volatile = false;
    ptr_type->flags.is_atomic = false;
    ptr_type->flags.is_ffi_compatible = true;
    ptr_type->flags.reserved = 0;
    
    ptr_type->size = sizeof(void*);
    ptr_type->alignment = _Alignof(void*);
    ptr_type->name = NULL;
    
    ptr_type->data.pointer.pointee_type = pointee_type;
    type_descriptor_retain(pointee_type); // Retain the pointee type
    
    atomic_init(&ptr_type->ref_count, 1);
    
    return ptr_type;
}

TypeDescriptor *type_descriptor_create_slice(TypeDescriptor *element_type) {
    if (!element_type) return NULL;
    
    TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
    if (!slice_type) return NULL;
    
    slice_type->category = TYPE_SLICE;
    slice_type->flags.is_mutable = false;
    slice_type->flags.is_owned = false;
    slice_type->flags.is_borrowed = false;
    slice_type->flags.is_constant = false;
    slice_type->flags.is_volatile = false;
    slice_type->flags.is_atomic = false;
    slice_type->flags.is_ffi_compatible = true;
    slice_type->flags.reserved = 0;
    
    slice_type->size = sizeof(void*) + sizeof(size_t); // ptr + len
    slice_type->alignment = _Alignof(void*);
    slice_type->name = NULL;
    
    slice_type->data.slice.element_type = element_type;
    type_descriptor_retain(element_type); // Retain the element type
    
    atomic_init(&slice_type->ref_count, 1);
    
    return slice_type;
}

TypeDescriptor *type_descriptor_create_array(TypeDescriptor *element_type, size_t size) {
    if (!element_type || size == 0) return NULL;
    
    TypeDescriptor *array_type = malloc(sizeof(TypeDescriptor));
    if (!array_type) return NULL;
    
    array_type->category = TYPE_ARRAY;
    array_type->flags.is_mutable = false;
    array_type->flags.is_owned = false;
    array_type->flags.is_borrowed = false;
    array_type->flags.is_constant = false;
    array_type->flags.is_volatile = false;
    array_type->flags.is_atomic = false;
    array_type->flags.is_ffi_compatible = true; // Arrays are FFI compatible
    array_type->flags.reserved = 0;
    
    // Array size is element size * number of elements
    array_type->size = element_type->size * size;
    array_type->alignment = element_type->alignment;
    
    // Create name like "[5]i32"
    char name_buffer[256];
    const char *elem_name = element_type->name ? element_type->name : "unknown";
    snprintf(name_buffer, sizeof(name_buffer), "[%zu]%s", size, elem_name);
    array_type->name = strdup(name_buffer);
    
    array_type->data.array.element_type = element_type;
    array_type->data.array.size = size;
    type_descriptor_retain(element_type); // Retain the element type
    
    atomic_init(&array_type->ref_count, 1);
    
    return array_type;
}

TypeDescriptor *type_descriptor_create_result(TypeDescriptor *ok_type, TypeDescriptor *err_type) {
    if (!ok_type || !err_type) return NULL;
    
    TypeDescriptor *result_type = malloc(sizeof(TypeDescriptor));
    if (!result_type) return NULL;
    
    result_type->category = TYPE_RESULT;
    result_type->flags.is_mutable = false;
    result_type->flags.is_owned = false;
    result_type->flags.is_borrowed = false;
    result_type->flags.is_constant = false;
    result_type->flags.is_volatile = false;
    result_type->flags.is_atomic = false;
    result_type->flags.is_ffi_compatible = false; // Result types are not FFI compatible
    result_type->flags.reserved = 0;
    
    // Result size is a tagged union - size of largest variant plus tag
    size_t ok_size = ok_type->size;
    size_t err_size = err_type->size;
    result_type->size = (ok_size > err_size ? ok_size : err_size) + sizeof(int);
    result_type->alignment = _Alignof(void*);
    result_type->name = NULL;
    
    result_type->data.result.ok_type = ok_type;
    result_type->data.result.err_type = err_type;
    type_descriptor_retain(ok_type); // Retain the OK type
    type_descriptor_retain(err_type); // Retain the error type
    
    atomic_init(&result_type->ref_count, 1);
    
    return result_type;
}

TypeDescriptor *type_descriptor_create_function(void) {
    TypeDescriptor *func_type = calloc(1, sizeof(TypeDescriptor));
    if (!func_type) {
        return NULL;
    }
    
    func_type->category = TYPE_FUNCTION;
    func_type->flags = (TypeFlags){0}; // Initialize all flags to false
    func_type->size = sizeof(void*); // Function pointer size
    func_type->alignment = _Alignof(void*);
    func_type->name = NULL; // Function types don't have names
    
    // Initialize function-specific data
    func_type->data.function.return_type = NULL; // Will be set later
    func_type->data.function.param_count = 0; // Will be set later
    func_type->data.function.param_types = NULL; // Will be allocated later if needed
    
    // Initialize FFI-specific fields
    func_type->data.function.is_extern = false;
    func_type->data.function.extern_name = NULL;
    func_type->data.function.ffi_annotations = NULL;
    func_type->data.function.ffi_annotation_count = 0;
    func_type->data.function.requires_ffi_marshaling = false;
    
    // Initialize reference count to 1
    atomic_init(&func_type->ref_count, 1);
    
    return func_type;
}

TypeDescriptor *type_descriptor_create_function_with_params(TypeDescriptor *return_type, size_t param_count) {
    if (!return_type) {
        return NULL;
    }
    
    TypeDescriptor *func_type = calloc(1, sizeof(TypeDescriptor));
    if (!func_type) {
        return NULL;
    }
    
    func_type->category = TYPE_FUNCTION;
    func_type->flags = (TypeFlags){0}; // Initialize all flags to false
    func_type->size = sizeof(void*); // Function pointer size
    func_type->alignment = _Alignof(void*);
    func_type->name = NULL; // Function types don't have names
    
    // Initialize function-specific data
    func_type->data.function.return_type = return_type;
    type_descriptor_retain(return_type); // Retain the return type
    
    func_type->data.function.param_count = param_count;
    if (param_count > 0) {
        func_type->data.function.param_types = calloc(param_count, sizeof(TypeDescriptor*));
        if (!func_type->data.function.param_types) {
            type_descriptor_release(return_type);
            free(func_type);
            return NULL;
        }
    } else {
        func_type->data.function.param_types = NULL;
    }
    
    // Initialize FFI-specific fields
    func_type->data.function.is_extern = false;
    func_type->data.function.extern_name = NULL;
    func_type->data.function.ffi_annotations = NULL;
    func_type->data.function.ffi_annotation_count = 0;
    func_type->data.function.requires_ffi_marshaling = false;
    
    // Initialize reference count to 1
    atomic_init(&func_type->ref_count, 1);
    
    return func_type;
}

TypeDescriptor *type_descriptor_create_generic_instance(TypeDescriptor *base_type, 
                                                       TypeDescriptor **type_args, 
                                                       size_t type_arg_count) {
    if (!base_type || !type_args || type_arg_count == 0) {
        return NULL;
    }
    
    // Validate that base type is actually generic (struct or enum)
    if (base_type->category != TYPE_STRUCT && base_type->category != TYPE_ENUM) {
        return NULL;
    }
    
    TypeDescriptor *instance_type = malloc(sizeof(TypeDescriptor));
    if (!instance_type) {
        return NULL;
    }
    
    instance_type->category = TYPE_GENERIC_INSTANCE;
    instance_type->flags = base_type->flags; // Inherit flags from base type
    instance_type->size = base_type->size; // Will be computed properly later
    instance_type->alignment = base_type->alignment;
    instance_type->name = NULL; // Will be set to canonical name later
    
    // Set up generic instance data
    instance_type->data.generic_instance.base_type = base_type;
    type_descriptor_retain(base_type); // Retain the base type
    
    // Copy type arguments
    instance_type->data.generic_instance.type_arg_count = type_arg_count;
    instance_type->data.generic_instance.type_args = malloc(type_arg_count * sizeof(TypeDescriptor*));
    if (!instance_type->data.generic_instance.type_args) {
        type_descriptor_release(base_type);
        free(instance_type);
        return NULL;
    }
    
    for (size_t i = 0; i < type_arg_count; i++) {
        instance_type->data.generic_instance.type_args[i] = type_args[i];
        type_descriptor_retain(type_args[i]); // Retain each type argument
    }
    
    // Generate canonical name (e.g., "Vec<i32>")
    if (!base_type->name) {
        // Base type missing name - this shouldn't happen for valid structs
        type_descriptor_release(base_type);
        for (size_t i = 0; i < type_arg_count; i++) {
            type_descriptor_release(type_args[i]);
        }
        free(instance_type->data.generic_instance.type_args);
        free(instance_type);
        return NULL;
    }
    size_t name_size = strlen(base_type->name) + 3; // base + "<" + ">" + null
    for (size_t i = 0; i < type_arg_count; i++) {
        if (type_args[i]->name) {
            name_size += strlen(type_args[i]->name);
        } else {
            name_size += 10; // Reserve space for anonymous types
        }
        if (i > 0) name_size += 2; // ", "
    }
    
    char *canonical_name = malloc(name_size);
    if (canonical_name) {
        strcpy(canonical_name, base_type->name);
        strcat(canonical_name, "<");
        for (size_t i = 0; i < type_arg_count; i++) {
            if (i > 0) strcat(canonical_name, ", ");
            if (type_args[i]->name) {
                strcat(canonical_name, type_args[i]->name);
            } else {
                strcat(canonical_name, "?");
            }
        }
        strcat(canonical_name, ">");
        instance_type->data.generic_instance.canonical_name = canonical_name;
        instance_type->name = canonical_name;  // Note: This points to the same memory as canonical_name
                                               // The generic instance cleanup will free canonical_name
                                               // So we must not free name separately
    }
    
    // Initialize reference count to 1
    atomic_init(&instance_type->ref_count, 1);
    
    return instance_type;
}

TypeDescriptor *type_descriptor_create_tuple(TypeDescriptor **element_types, size_t element_count) {
    if (!element_types || element_count < 2) return NULL;
    
    TypeDescriptor *tuple_type = malloc(sizeof(TypeDescriptor));
    if (!tuple_type) return NULL;
    
    tuple_type->category = TYPE_TUPLE;
    tuple_type->flags.is_mutable = false;
    tuple_type->flags.is_owned = false;
    tuple_type->flags.is_borrowed = false;
    tuple_type->flags.is_constant = false;
    tuple_type->flags.is_volatile = false;
    tuple_type->flags.is_atomic = false;
    tuple_type->flags.is_ffi_compatible = true; // Tuples can be FFI compatible if all elements are
    tuple_type->flags.reserved = 0;
    
    // Calculate size and alignment based on element types
    size_t total_size = 0;
    size_t max_alignment = 1;
    
    // Allocate arrays for element types and offsets
    tuple_type->data.tuple.element_count = element_count;
    tuple_type->data.tuple.element_types = malloc(element_count * sizeof(TypeDescriptor*));
    tuple_type->data.tuple.element_offsets = malloc(element_count * sizeof(size_t));
    
    if (!tuple_type->data.tuple.element_types || !tuple_type->data.tuple.element_offsets) {
        free(tuple_type->data.tuple.element_types);
        free(tuple_type->data.tuple.element_offsets);
        free(tuple_type);
        return NULL;
    }
    
    // Copy element types and calculate layout
    for (size_t i = 0; i < element_count; i++) {
        tuple_type->data.tuple.element_types[i] = element_types[i];
        type_descriptor_retain(element_types[i]); // Retain each element type
        
        // Update max alignment
        if (element_types[i]->alignment > max_alignment) {
            max_alignment = element_types[i]->alignment;
        }
        
        // Align current offset to element alignment
        size_t element_alignment = element_types[i]->alignment;
        if (element_alignment > 0) {
            total_size = (total_size + element_alignment - 1) & ~(element_alignment - 1);
        }
        
        // Store offset and add element size
        tuple_type->data.tuple.element_offsets[i] = total_size;
        total_size += element_types[i]->size;
        
        // Check FFI compatibility
        if (!element_types[i]->flags.is_ffi_compatible) {
            tuple_type->flags.is_ffi_compatible = false;
        }
    }
    
    // Align total size to max alignment
    if (max_alignment > 0) {
        total_size = (total_size + max_alignment - 1) & ~(max_alignment - 1);
    }
    
    tuple_type->size = total_size;
    tuple_type->alignment = max_alignment;
    
    // Generate name for tuple type (e.g., "(i32, string)")
    size_t name_size = 3; // "(" + ")" + null
    for (size_t i = 0; i < element_count; i++) {
        if (element_types[i]->name) {
            name_size += strlen(element_types[i]->name);
        } else {
            name_size += 10; // Reserve space for anonymous types
        }
        if (i > 0) name_size += 2; // ", "
    }
    
    char *tuple_name = malloc(name_size);
    if (tuple_name) {
        strcpy(tuple_name, "(");
        for (size_t i = 0; i < element_count; i++) {
            if (i > 0) strcat(tuple_name, ", ");
            if (element_types[i]->name) {
                strcat(tuple_name, element_types[i]->name);
            } else {
                strcat(tuple_name, "?");
            }
        }
        strcat(tuple_name, ")");
        tuple_type->name = tuple_name;
    } else {
        tuple_type->name = NULL;
    }
    
    // Initialize reference count to 1
    atomic_init(&tuple_type->ref_count, 1);
    
    return tuple_type;
}

// =============================================================================
// STRUCT TYPE OPERATIONS STUBS
// =============================================================================

bool type_descriptor_add_struct_field(TypeDescriptor *struct_type, 
                                     const char *field_name, 
                                     TypeDescriptor *field_type,
                                     ASTNode *field_declaration) {
    // Stub implementation
    (void)struct_type;
    (void)field_name;
    (void)field_type;
    (void)field_declaration;
    return false;
}

SymbolEntry *type_descriptor_lookup_struct_field(TypeDescriptor *struct_type, const char *field_name) {
    // Stub implementation
    (void)struct_type;
    (void)field_name;
    return NULL;
}