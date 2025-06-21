/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Memory Management Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Memory management for generic instantiation objects with reference counting
 */

#include "generic_instantiation_memory.h"
#include "../analysis/semantic_types.h"
#include "../analysis/semantic_symbols.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

GenericInstantiation *generic_instantiation_create(const char *concrete_name,
                                                  TypeDescriptor **type_args,
                                                  size_t type_arg_count) {
    if (!concrete_name || !type_args) return NULL;
    
    GenericInstantiation *inst = malloc(sizeof(GenericInstantiation));
    if (!inst) return NULL;
    
    inst->concrete_name = strdup(concrete_name);
    if (!inst->concrete_name) {
        free(inst);
        return NULL;
    }
    
    // Copy type arguments
    inst->type_args = malloc(type_arg_count * sizeof(TypeDescriptor*));
    if (!inst->type_args) {
        free(inst->concrete_name);
        free(inst);
        return NULL;
    }
    
    for (size_t i = 0; i < type_arg_count; i++) {
        type_descriptor_retain(type_args[i]);
        inst->type_args[i] = type_args[i];
    }
    
    inst->type_arg_count = type_arg_count;
    inst->c_struct_definition = NULL;
    inst->concrete_fields = NULL;
    inst->struct_size = 0;
    inst->struct_alignment = 0;
    inst->is_ffi_compatible = true;
    inst->next = NULL;
    
    atomic_init(&inst->ref_count, 1);
    
    return inst;
}

void generic_instantiation_retain(GenericInstantiation *instantiation) {
    if (instantiation) {
        atomic_fetch_add(&instantiation->ref_count, 1);
    }
}

void generic_instantiation_release(GenericInstantiation *instantiation) {
    if (!instantiation) return;
    
    uint32_t refs = atomic_fetch_sub(&instantiation->ref_count, 1);
    if (refs == 1) {
        // Last reference, cleanup
        free(instantiation->concrete_name);
        free(instantiation->c_struct_definition);
        
        if (instantiation->type_args) {
            for (size_t i = 0; i < instantiation->type_arg_count; i++) {
                type_descriptor_release(instantiation->type_args[i]);
            }
            free(instantiation->type_args);
        }
        
        if (instantiation->concrete_fields) {
            symbol_table_destroy(instantiation->concrete_fields);
        }
        
        free(instantiation);
    }
}

GenericStructInfo *generic_struct_info_create(const char *struct_name,
                                             ASTNode *struct_decl,
                                             TypeDescriptor *struct_type) {
    if (!struct_name || !struct_decl || !struct_type) return NULL;
    
    GenericStructInfo *info = malloc(sizeof(GenericStructInfo));
    if (!info) return NULL;
    
    info->generic_name = strdup(struct_name);
    if (!info->generic_name) {
        free(info);
        return NULL;
    }
    
    info->original_decl = struct_decl;
    info->generic_type = struct_type;
    type_descriptor_retain(struct_type);
    
    info->instantiations = NULL;
    info->instantiation_count = 0;
    info->instantiation_capacity = 0;
    
    // Extract type parameter names from AST
    ASTNodeList *type_params = struct_decl->data.struct_decl.type_params;
    if (type_params) {
        info->type_param_count = ast_node_list_size(type_params);
        info->type_param_names = malloc(info->type_param_count * sizeof(char*));
        
        if (info->type_param_names) {
            for (size_t i = 0; i < info->type_param_count; i++) {
                ASTNode *param = ast_node_list_get(type_params, i);
                if (param && param->type == AST_IDENTIFIER && param->data.identifier.name) {
                    info->type_param_names[i] = strdup(param->data.identifier.name);
                } else {
                    info->type_param_names[i] = NULL;
                }
            }
        }
    } else {
        info->type_param_count = 0;
        info->type_param_names = NULL;
    }
    
    atomic_init(&info->is_generating, false);
    atomic_init(&info->generation_id, 0);
    info->next = NULL;
    
    return info;
}

void generic_struct_info_destroy(GenericStructInfo *struct_info) {
    if (!struct_info) return;
    
    free(struct_info->generic_name);
    
    if (struct_info->generic_type) {
        type_descriptor_release(struct_info->generic_type);
    }
    
    // Destroy all instantiations
    if (struct_info->instantiations) {
        for (size_t i = 0; i < struct_info->instantiation_count; i++) {
            if (struct_info->instantiations[i]) {
                generic_instantiation_release(struct_info->instantiations[i]);
            }
        }
        free(struct_info->instantiations);
    }
    
    // Free type parameter names
    if (struct_info->type_param_names) {
        for (size_t i = 0; i < struct_info->type_param_count; i++) {
            free(struct_info->type_param_names[i]);
        }
        free(struct_info->type_param_names);
    }
    
    free(struct_info);
}

// =============================================================================
// DEBUG AND INTROSPECTION
// =============================================================================

char *generic_instantiation_debug_string(GenericInstantiation *instantiation) {
    if (!instantiation) return strdup("NULL instantiation");
    
    char *debug_str = malloc(512);
    if (!debug_str) return NULL;
    
    snprintf(debug_str, 512, "Instantiation: %s (args: %zu, size: %zu, ffi: %s)",
             instantiation->concrete_name,
             instantiation->type_arg_count,
             instantiation->struct_size,
             instantiation->is_ffi_compatible ? "yes" : "no");
    
    return debug_str;
} 
