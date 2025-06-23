/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Core Type Descriptor Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core type descriptor lifecycle management and operations
 */

#include "semantic_type_descriptors.h"
#include "semantic_core.h"
#include "semantic_builtins.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// CORE TYPE DESCRIPTOR FUNCTIONS
// =============================================================================

TypeDescriptor *type_descriptor_create_primitive(int primitive_kind) {
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type) return NULL;
    
    type->category = TYPE_PRIMITIVE;
    type->flags.is_mutable = false;
    type->flags.is_owned = false;
    type->flags.is_borrowed = false;
    type->flags.is_constant = false;
    type->flags.is_volatile = false;
    type->flags.is_atomic = false;
    type->flags.is_ffi_compatible = true;
    type->flags.reserved = 0;
    
    type->size = 0;
    type->alignment = 0;
    type->name = NULL;
    
    type->data.primitive.primitive_kind = primitive_kind;
    atomic_init(&type->ref_count, 1);
    
    return type;
}

void type_descriptor_retain(TypeDescriptor *type) {
    if (!type) return;
    
    // Check if this is a builtin type - builtin types don't need ref counting
    if (semantic_is_builtin_type(type)) {
        return;
    }
    
    atomic_fetch_add(&type->ref_count, 1);
}

void type_descriptor_release(TypeDescriptor *type) {
    if (!type) return;
    
    // Check for double-free attempt
    if (type->category == (TypeCategory)-1) {
        printf("ERROR: Double-free detected! Type descriptor at %p was already freed!\n", (void*)type);
        return;
    }
    
    // DEBUG: Print information about the type being released
    // // printf("DEBUG: type_descriptor_release called on type=%p, name='%s', category=%d, ref_count=%u\n", 
    //        (void*)type, type->name ? type->name : "NULL", type->category, type->ref_count);
    
    // Check if this is a builtin type - builtin types should never be freed
    bool is_builtin = semantic_is_builtin_type(type);
    // printf("DEBUG: semantic_is_builtin_type returned %s\n", is_builtin ? "true" : "false");
    if (is_builtin) {
        // printf("DEBUG: Returning early - builtin type\n");
        return;
    }
    
    // Additional safety check for high ref counts indicating builtin types
    if (type->ref_count >= 1000000) {
        // printf("DEBUG: Returning early - high ref count %u\n", type->ref_count);
        return;
    }
    
    // Check if this type has a name that matches primitive types (defensive check)
    if (type->name && type->category == TYPE_PRIMITIVE) {
        const char* primitive_names[] = {
            "void", "bool", "i8", "i16", "i32", "i64", "i128", "isize",
            "u8", "u16", "u32", "u64", "u128", "usize", "f32", "f64", 
            "char", "string", "never"
        };
        for (size_t i = 0; i < sizeof(primitive_names) / sizeof(primitive_names[0]); i++) {
            if (strcmp(type->name, primitive_names[i]) == 0) {
                // printf("DEBUG: Returning early - primitive type name match: %s\n", type->name);
                return; // Don't free primitive types
            }
        }
    }
    
    // printf("DEBUG: Proceeding with atomic_fetch_sub\n");
    
    uint32_t old_count = atomic_fetch_sub(&type->ref_count, 1);
    if (old_count == 1) {
        // Last reference - destroy the type and its nested resources
        switch (type->category) {
            case TYPE_FUNCTION:
                // Release return type
                if (type->data.function.return_type) {
                    type_descriptor_release(type->data.function.return_type);
                }
                // Release parameter types
                if (type->data.function.param_types) {
                    for (size_t i = 0; i < type->data.function.param_count; i++) {
                        if (type->data.function.param_types[i]) {
                            type_descriptor_release(type->data.function.param_types[i]);
                        }
                    }
                    free(type->data.function.param_types);
                }
                // Release FFI-specific resources
                if (type->data.function.extern_name) {
                    free((char*)type->data.function.extern_name);
                }
                break;
            case TYPE_POINTER:
                if (type->data.pointer.pointee_type) {
                    type_descriptor_release(type->data.pointer.pointee_type);
                }
                break;
            case TYPE_SLICE:
                if (type->data.slice.element_type) {
                    type_descriptor_release(type->data.slice.element_type);
                }
                break;
            case TYPE_RESULT:
                if (type->data.result.ok_type) {
                    type_descriptor_release(type->data.result.ok_type);
                }
                if (type->data.result.err_type) {
                    type_descriptor_release(type->data.result.err_type);
                }
                break;
            case TYPE_OPTION:
                if (type->data.option.value_type) {
                    type_descriptor_release(type->data.option.value_type);
                }
                break;
            case TYPE_GENERIC_INSTANCE:
                // Release base type
                if (type->data.generic_instance.base_type) {
                    type_descriptor_release(type->data.generic_instance.base_type);
                }
                // Release type arguments
                if (type->data.generic_instance.type_args) {
                    for (size_t i = 0; i < type->data.generic_instance.type_arg_count; i++) {
                        if (type->data.generic_instance.type_args[i]) {
                            type_descriptor_release(type->data.generic_instance.type_args[i]);
                        }
                    }
                    free(type->data.generic_instance.type_args);
                }
                // Free canonical name
                if (type->data.generic_instance.canonical_name) {
                    free(type->data.generic_instance.canonical_name);
                }
                break;
            case TYPE_TUPLE:
                // Release element types
                if (type->data.tuple.element_types) {
                    for (size_t i = 0; i < type->data.tuple.element_count; i++) {
                        if (type->data.tuple.element_types[i]) {
                            type_descriptor_release(type->data.tuple.element_types[i]);
                        }
                    }
                    free(type->data.tuple.element_types);
                }
                // Free element offsets
                if (type->data.tuple.element_offsets) {
                    free(type->data.tuple.element_offsets);
                }
                break;
            case TYPE_ARRAY:
                // Release element type
                if (type->data.array.element_type) {
                    type_descriptor_release(type->data.array.element_type);
                }
                break;
            default:
                // Other types don't have nested type descriptors to release
                break;
        }
        
        // Free name if allocated
        // Note: For TYPE_GENERIC_INSTANCE, name points to the same memory as canonical_name
        // which is already freed above, so we must not free it again
        if (type->name && type->category != TYPE_GENERIC_INSTANCE) {
            // printf("DEBUG: About to free type name: %s\n", type->name);
            free((char*)type->name);
        }
        
        // printf("DEBUG: About to free type descriptor at %p\n", (void*)type);
        // Mark as freed to detect double-free attempts
        type->category = (TypeCategory)-1;  // Poison the memory
        free(type);
    }
}

bool type_descriptor_equals(const TypeDescriptor *type1, const TypeDescriptor *type2) {
    if (!type1 || !type2) return false;
    if (type1 == type2) return true;
    
    if (type1->category != type2->category) return false;
    
    switch (type1->category) {
        case TYPE_PRIMITIVE:
            return type1->data.primitive.primitive_kind == type2->data.primitive.primitive_kind;
        case TYPE_POINTER:
            return type_descriptor_equals(type1->data.pointer.pointee_type, 
                                        type2->data.pointer.pointee_type);
        case TYPE_SLICE:
            return type_descriptor_equals(type1->data.slice.element_type, 
                                        type2->data.slice.element_type);
        case TYPE_RESULT:
            return type_descriptor_equals(type1->data.result.ok_type, type2->data.result.ok_type) &&
                   type_descriptor_equals(type1->data.result.err_type, type2->data.result.err_type);
        case TYPE_FUNCTION:
            // Simple comparison - could be more sophisticated
            return type1->data.function.param_count == type2->data.function.param_count &&
                   type_descriptor_equals(type1->data.function.return_type, 
                                        type2->data.function.return_type);
        case TYPE_GENERIC_INSTANCE: {
            // Compare base types and type arguments
            if (!type_descriptor_equals(type1->data.generic_instance.base_type,
                                       type2->data.generic_instance.base_type)) {
                return false;
            }
            
            if (type1->data.generic_instance.type_arg_count != 
                type2->data.generic_instance.type_arg_count) {
                return false;
            }
            
            // Compare each type argument
            for (size_t i = 0; i < type1->data.generic_instance.type_arg_count; i++) {
                if (!type_descriptor_equals(type1->data.generic_instance.type_args[i],
                                           type2->data.generic_instance.type_args[i])) {
                    return false;
                }
            }
            
            return true;
        }
        case TYPE_TUPLE: {
            // Compare element counts
            if (type1->data.tuple.element_count != type2->data.tuple.element_count) {
                return false;
            }
            
            // Compare each element type
            for (size_t i = 0; i < type1->data.tuple.element_count; i++) {
                if (!type_descriptor_equals(type1->data.tuple.element_types[i],
                                           type2->data.tuple.element_types[i])) {
                    return false;
                }
            }
            
            return true;
        }
        case TYPE_ARRAY:
            // Arrays are equal if they have the same element type and size
            return type1->data.array.size == type2->data.array.size &&
                   type_descriptor_equals(type1->data.array.element_type,
                                        type2->data.array.element_type);
        case TYPE_ENUM:
            // Enums are equal if they have the same name
            // This handles the case where builtin types like Result may be different instances
            if (type1->name && type2->name) {
                return strcmp(type1->name, type2->name) == 0;
            }
            return type1 == type2; // Same object reference
        case TYPE_STRUCT:
            // Structs are equal if they have the same name
            // This handles the case where struct types may be different instances
            if (type1->name && type2->name) {
                return strcmp(type1->name, type2->name) == 0;
            }
            return type1 == type2; // Same object reference
        default:
            return false;
    }
}

size_t type_descriptor_hash(const TypeDescriptor *type) {
    if (!type) return 0;
    
    // Simple hash based on category and primitive kind
    size_t hash = (size_t)type->category;
    if (type->category == TYPE_PRIMITIVE) {
        hash ^= (size_t)type->data.primitive.primitive_kind << 8;
    }
    return hash;
}

bool type_is_never(const TypeDescriptor *type) {
    if (!type) return false;
    
    return type->category == TYPE_PRIMITIVE && 
           type->data.primitive.primitive_kind == PRIMITIVE_NEVER;
}