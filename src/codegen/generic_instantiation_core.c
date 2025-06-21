/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Core Operations Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core instantiation operations for generic struct monomorphization
 */

#include "generic_instantiation_core.h"
#include "generic_instantiation_registry.h"
#include "generic_instantiation_memory.h"
#include "generic_instantiation_validation.h"
#include "generic_instantiation_codegen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// GENERIC INSTANTIATION MANAGEMENT
// =============================================================================

GenericInstantiation *generic_instantiate_struct(GenericRegistry *registry,
                                                const char *struct_name,
                                                TypeDescriptor **type_args,
                                                size_t type_arg_count,
                                                CodeGenerator *generator) {
    if (!registry || !struct_name || !type_args || type_arg_count == 0 || !generator) {
        return NULL;
    }
    
    // Look up the generic struct
    GenericStructInfo *struct_info = generic_registry_lookup_struct(registry, struct_name);
    if (!struct_info) {
        return NULL;
    }
    
    // Validate type argument count
    if (type_arg_count != struct_info->type_param_count) {
        return NULL;
    }
    
    // Check if instantiation already exists
    GenericInstantiation *existing = find_existing_instantiation(struct_info, type_args, type_arg_count);
    if (existing) {
        generic_instantiation_retain(existing);
        return existing;
    }
    
    // Validate instantiation arguments
    if (!validate_instantiation_args(struct_info, type_args, type_arg_count)) {
        return NULL;
    }
    
    // Check for circular dependencies
    if (!check_instantiation_cycles(registry, struct_name, type_args, type_arg_count)) {
        return NULL;
    }
    
    // Generate concrete name
    char *concrete_name = generate_concrete_struct_name(struct_name, type_args, type_arg_count);
    if (!concrete_name) {
        return NULL;
    }
    
    // Create new instantiation
    GenericInstantiation *instantiation = generic_instantiation_create(concrete_name, type_args, type_arg_count);
    if (!instantiation) {
        free(concrete_name);
        return NULL;
    }
    
    // Generate the concrete struct definition
    if (!generate_concrete_struct_definition(generator, instantiation, struct_info)) {
        generic_instantiation_release(instantiation);
        return NULL;
    }
    
    // Add to struct info's instantiation list
    if (struct_info->instantiation_count >= struct_info->instantiation_capacity) {
        size_t new_capacity = struct_info->instantiation_capacity * 2;
        if (new_capacity == 0) new_capacity = 4;
        
        GenericInstantiation **new_instantiations = realloc(struct_info->instantiations,
                                                           new_capacity * sizeof(GenericInstantiation*));
        if (!new_instantiations) {
            generic_instantiation_release(instantiation);
            return NULL;
        }
        
        struct_info->instantiations = new_instantiations;
        struct_info->instantiation_capacity = new_capacity;
    }
    
    struct_info->instantiations[struct_info->instantiation_count++] = instantiation;
    
    // Update statistics
    atomic_fetch_add(&registry->total_instantiations, 1);
    atomic_fetch_add(&registry->concrete_structs_generated, 1);
    
    return instantiation;
}

char *generate_concrete_struct_name(const char *generic_name, 
                                   TypeDescriptor **type_args, 
                                   size_t type_arg_count) {
    if (!generic_name || !type_args || type_arg_count == 0) {
        return NULL;
    }
    
    // Calculate needed buffer size
    size_t total_size = strlen(generic_name) + 1; // +1 for null terminator
    
    for (size_t i = 0; i < type_arg_count; i++) {
        if (type_args[i] && type_args[i]->name) {
            total_size += strlen(type_args[i]->name) + 1; // +1 for underscore
        }
    }
    
    char *concrete_name = malloc(total_size);
    if (!concrete_name) return NULL;
    
    // Build the name: Vec_i32, Pair_string_bool, etc.
    strcpy(concrete_name, generic_name);
    
    for (size_t i = 0; i < type_arg_count; i++) {
        if (type_args[i] && type_args[i]->name) {
            strcat(concrete_name, "_");
            strcat(concrete_name, type_args[i]->name);
        }
    }
    
    return concrete_name;
}

GenericInstantiation *find_existing_instantiation(GenericStructInfo *struct_info,
                                                 TypeDescriptor **type_args,
                                                 size_t type_arg_count) {
    if (!struct_info || !type_args) return NULL;
    
    for (size_t i = 0; i < struct_info->instantiation_count; i++) {
        GenericInstantiation *inst = struct_info->instantiations[i];
        if (!inst || inst->type_arg_count != type_arg_count) continue;
        
        // Compare type arguments
        bool matches = true;
        for (size_t j = 0; j < type_arg_count; j++) {
            if (!type_args[j] || !inst->type_args[j]) {
                matches = false;
                break;
            }
            
            // Compare type names (simplified comparison)
            if (!inst->type_args[j]->name || !type_args[j]->name ||
                strcmp(inst->type_args[j]->name, type_args[j]->name) != 0) {
                matches = false;
                break;
            }
        }
        
        if (matches) {
            return inst;
        }
    }
    
    return NULL;
} 
