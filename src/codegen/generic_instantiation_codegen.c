/**
 * Asthra Programming Language Compiler
 * Generic Instantiation Code Generation Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Code generation for concrete struct definitions from generic instantiations
 */

#include "generic_instantiation_codegen.h"
#include "../analysis/semantic_types.h"
#include "../analysis/semantic_symbols.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// CODE GENERATION FOR INSTANTIATIONS
// =============================================================================

bool generate_concrete_struct_definition(CodeGenerator *generator,
                                        GenericInstantiation *instantiation,
                                        GenericStructInfo *struct_info) {
    if (!generator || !instantiation || !struct_info) {
        return false;
    }
    
    ASTNode *original_decl = struct_info->original_decl;
    if (!original_decl || original_decl->type != AST_STRUCT_DECL) {
        return false;
    }
    
    // Build C struct definition
    char *definition = malloc(4096); // Reasonable starting size
    if (!definition) return false;
    
    // Start struct definition
    snprintf(definition, 4096, "// Generated from generic struct %s\ntypedef struct {\n", 
             struct_info->generic_name);
    
    // Process fields with type substitution
    ASTNodeList *fields = original_decl->data.struct_decl.fields;
    if (fields) {
        size_t field_count = ast_node_list_size(fields);
        
        for (size_t i = 0; i < field_count; i++) {
            ASTNode *field = ast_node_list_get(fields, i);
            if (!field || field->type != AST_PARAM_DECL) continue;
            
            const char *field_name = field->data.param_decl.name;
            ASTNode *field_type_node = field->data.param_decl.type;
            
            if (!field_name || !field_type_node) continue;
            
            // Get field type and substitute type parameters
            TypeDescriptor *field_type = substitute_type_parameters(
                struct_info->generic_type, // This needs to be the field's original type
                struct_info->type_param_names,
                instantiation->type_args,
                instantiation->type_arg_count);
            
            if (!field_type) continue;
            
            // Convert to C type string
            char *c_type_str = type_descriptor_to_c_string(field_type);
            if (c_type_str) {
                // Append field to definition
                char field_line[256];
                snprintf(field_line, sizeof(field_line), "    %s %s;\n", c_type_str, field_name);
                strcat(definition, field_line);
                free(c_type_str);
            }
            
            type_descriptor_release(field_type);
        }
    }
    
    // Close struct definition
    char closing[128];
    snprintf(closing, sizeof(closing), "} %s;\n\n", instantiation->concrete_name);
    strcat(definition, closing);
    
    // Store the generated definition
    instantiation->c_struct_definition = definition;
    
    // Update statistics
    atomic_fetch_add(&generator->stats.bytes_generated, strlen(definition));
    
    return true;
}

bool generate_all_instantiated_structs(CodeGenerator *generator, 
                                      GenericRegistry *registry,
                                      char *output_buffer,
                                      size_t buffer_size) {
    if (!generator || !registry || !output_buffer || buffer_size == 0) {
        return false;
    }
    
    output_buffer[0] = '\0'; // Clear output buffer
    
    pthread_rwlock_rdlock(&registry->rwlock);
    
    for (size_t i = 0; i < registry->struct_count; i++) {
        GenericStructInfo *struct_info = registry->structs[i];
        if (!struct_info) continue;
        
        for (size_t j = 0; j < struct_info->instantiation_count; j++) {
            GenericInstantiation *inst = struct_info->instantiations[j];
            if (!inst || !inst->c_struct_definition) continue;
            
            // Check if we have space
            size_t current_len = strlen(output_buffer);
            size_t def_len = strlen(inst->c_struct_definition);
            
            if (current_len + def_len + 1 >= buffer_size) {
                pthread_rwlock_unlock(&registry->rwlock);
                return false; // Buffer too small
            }
            
            // Append definition
            strcat(output_buffer, inst->c_struct_definition);
        }
    }
    
    pthread_rwlock_unlock(&registry->rwlock);
    return true;
}

bool generate_struct_field_accessors(CodeGenerator *generator,
                                    GenericInstantiation *instantiation,
                                    GenericStructInfo *struct_info) {
    // Placeholder for optional field accessor generation
    (void)generator;
    (void)instantiation;
    (void)struct_info;
    return true;
}

// =============================================================================
// TYPE SUBSTITUTION UTILITIES
// =============================================================================

TypeDescriptor *substitute_type_parameters(TypeDescriptor *field_type,
                                          char **type_param_names,
                                          TypeDescriptor **type_args,
                                          size_t type_param_count) {
    if (!field_type) return NULL;
    
    // If this is a type parameter, substitute it
    if (field_type->name) {
        for (size_t i = 0; i < type_param_count; i++) {
            if (type_param_names[i] && strcmp(field_type->name, type_param_names[i]) == 0) {
                // Return the concrete type argument
                type_descriptor_retain(type_args[i]);
                return type_args[i];
            }
        }
    }
    
    // For other types, return as-is (more complex substitution would handle nested generics)
    type_descriptor_retain(field_type);
    return field_type;
}

bool type_contains_type_parameters(TypeDescriptor *type,
                                  char **type_param_names,
                                  size_t type_param_count) {
    if (!type || !type->name) return false;
    
    for (size_t i = 0; i < type_param_count; i++) {
        if (type_param_names[i] && strcmp(type->name, type_param_names[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

char *type_descriptor_to_c_string(TypeDescriptor *type) {
    if (!type || !type->name) return strdup("void");
    
    // Map Asthra types to C types
    if (strcmp(type->name, "i32") == 0) return strdup("int32_t");
    if (strcmp(type->name, "i64") == 0) return strdup("int64_t");
    if (strcmp(type->name, "u32") == 0) return strdup("uint32_t");
    if (strcmp(type->name, "u64") == 0) return strdup("uint64_t");
    if (strcmp(type->name, "f32") == 0) return strdup("float");
    if (strcmp(type->name, "f64") == 0) return strdup("double");
    if (strcmp(type->name, "bool") == 0) return strdup("bool");
    if (strcmp(type->name, "string") == 0) return strdup("char*");
    if (strcmp(type->name, "usize") == 0) return strdup("size_t");
    if (strcmp(type->name, "isize") == 0) return strdup("ssize_t");
    
    // For other types, use the name as-is
    return strdup(type->name);
} 
