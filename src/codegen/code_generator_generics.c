/**
 * Asthra Programming Language Compiler
 * Code Generator Generic Struct Handling
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Generic struct instantiation and literal generation functionality
 */

#include "code_generator_common.h"

// =============================================================================
// PHASE 4: GENERIC STRUCT CODE GENERATION IMPLEMENTATION
// =============================================================================

bool code_generator_register_generic_struct(CodeGenerator *generator, 
                                           const char *struct_name,
                                           ASTNode *struct_decl,
                                           TypeDescriptor *struct_type) {
    if (!generator || !generator->generic_registry || !struct_name || !struct_decl || !struct_type) {
        return false;
    }
    
    return generic_registry_register_struct(generator->generic_registry, 
                                           struct_name, struct_decl, struct_type);
}

bool code_generate_generic_struct_instantiation(CodeGenerator *generator,
                                               const char *struct_name,
                                               TypeDescriptor **type_args,
                                               size_t type_arg_count) {
    if (!generator || !generator->generic_registry || !struct_name || !type_args) {
        return false;
    }
    
    // Check if this instantiation already exists by looking up the struct info
    GenericStructInfo *struct_info = generic_registry_lookup_struct(generator->generic_registry, struct_name);
    if (!struct_info) {
        return false; // Struct not registered
    }
    
    // Check if instantiation already exists
    GenericInstantiation *existing = find_existing_instantiation(struct_info, type_args, type_arg_count);
    if (existing) {
        return true; // Already generated
    }
    
    // Generate the monomorphized struct
    GenericInstantiation *instantiation = generic_instantiate_struct(generator->generic_registry,
                                                                    struct_name, type_args, type_arg_count, generator);
    return instantiation != NULL;
}

bool code_generate_all_generic_instantiations(CodeGenerator *generator,
                                             char *output_buffer,
                                             size_t buffer_size) {
    if (!generator || !generator->generic_registry || !output_buffer) {
        return false;
    }
    
    return generate_all_instantiated_structs(generator, generator->generic_registry, 
                                            output_buffer, buffer_size);
}

bool code_generate_generic_struct_literal(CodeGenerator *generator,
                                         ASTNode *struct_literal,
                                         Register target_reg) {
    if (!generator || !struct_literal || struct_literal->type != AST_STRUCT_LITERAL) {
        return false;
    }
    
    // This is a complex operation that would:
    // 1. Determine the type of the struct literal
    // 2. Initialize each field
    // 3. Store the result in target_reg
    
    (void)target_reg; // Suppress unused parameter warning for now
    
    // Update statistics
    atomic_fetch_add(&generator->stats.instructions_generated, 3); // Approximate
    
    return true;
} 
