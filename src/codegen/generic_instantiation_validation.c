/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Validation Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Validation and error handling for generic struct instantiations
 */

#include "generic_instantiation_validation.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// VALIDATION AND ERROR HANDLING
// =============================================================================

bool validate_instantiation_args(GenericStructInfo *struct_info,
                                TypeDescriptor **type_args,
                                size_t type_arg_count) {
    if (!struct_info || !type_args) return false;
    
    // Check argument count
    if (type_arg_count != struct_info->type_param_count) {
        return false;
    }
    
    // Validate each type argument
    for (size_t i = 0; i < type_arg_count; i++) {
        if (!type_args[i] || !type_args[i]->name) {
            return false;
        }
        
        // Additional validation could check type constraints here
    }
    
    return true;
}

bool check_instantiation_cycles(GenericRegistry *registry,
                               const char *struct_name,
                               TypeDescriptor **type_args,
                               size_t type_arg_count) {
    (void)registry;     // Suppress unused parameter warning
    (void)struct_name;  // Suppress unused parameter warning
    (void)type_args;    // Suppress unused parameter warning
    (void)type_arg_count; // Suppress unused parameter warning
    
    // Simplified cycle detection - in a full implementation, this would
    // check for recursive generic dependencies
    return true;
}

bool validate_generic_struct_constraints(GenericStructInfo *struct_info,
                                        TypeDescriptor **type_args,
                                        size_t type_arg_count) {
    if (!struct_info || !type_args) return false;
    
    // Basic validation - could be extended with constraint checking
    return validate_instantiation_args(struct_info, type_args, type_arg_count);
}

bool validate_ffi_compatibility(GenericStructInfo *struct_info,
                               TypeDescriptor **type_args,
                               size_t type_arg_count) {
    if (!struct_info || !type_args) return false;
    
    // Check if all type arguments are FFI-compatible
    for (size_t i = 0; i < type_arg_count; i++) {
        if (!type_args[i] || !type_args[i]->name) {
            return false;
        }
        
        // Check for FFI-incompatible types
        const char *type_name = type_args[i]->name;
        
        // These types are generally FFI-compatible
        if (strcmp(type_name, "i32") == 0 ||
            strcmp(type_name, "i64") == 0 ||
            strcmp(type_name, "u32") == 0 ||
            strcmp(type_name, "u64") == 0 ||
            strcmp(type_name, "f32") == 0 ||
            strcmp(type_name, "f64") == 0 ||
            strcmp(type_name, "bool") == 0 ||
            strcmp(type_name, "usize") == 0 ||
            strcmp(type_name, "isize") == 0) {
            continue;
        }
        
        // String types need special handling for FFI
        if (strcmp(type_name, "string") == 0) {
            continue; // Assume handled as char*
        }
        
        // Other types might not be FFI-compatible
        // This is a simplified check - real implementation would be more thorough
    }
    
    return true;
} 
