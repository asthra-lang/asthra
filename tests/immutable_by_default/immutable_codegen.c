/**
 * Asthra Programming Language - Immutable-by-Default Code Generation
 * 
 * Code generation functions for immutable data structures and COW support
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "immutable_infrastructure.h"

// =============================================================================
// IMMUTABLE CODE GENERATION
// =============================================================================

/**
 * Generate copy-on-write support code
 */
bool immutable_generate_cow_support(ImmutableCodeGenerator* generator,
                                   EnhancedASTNode* cow_node) {
    if (!generator || !cow_node) return false;
    
    if (generator->output) {
        fprintf(generator->output, "// Copy-on-Write support\n");
        fprintf(generator->output, "if (atomic_load(&ref_count) > 1) {\n");
        fprintf(generator->output, "    // Create copy before mutation\n");
        fprintf(generator->output, "    data = immutable_cow_clone(data);\n");
        fprintf(generator->output, "    atomic_fetch_sub(&ref_count, 1);\n");
        fprintf(generator->output, "    atomic_store(&ref_count, 1);\n");
        fprintf(generator->output, "}\n\n");
        
        generator->cow_operations_generated++;
    }
    
    return true;
}

/**
 * Generate immutable structure code
 */
bool immutable_generate_structure(ImmutableCodeGenerator* generator,
                                 EnhancedASTNode* struct_node,
                                 const char* struct_name) {
    if (!generator || !struct_node || !struct_name) return false;
    
    if (generator->output) {
        fprintf(generator->output, "// Immutable structure: %s\n", struct_name);
        fprintf(generator->output, "typedef struct {\n");
        fprintf(generator->output, "    const void* data; // Immutable data\n");
        fprintf(generator->output, "    atomic_int ref_count; // Reference counting\n");
        fprintf(generator->output, "    bool is_shared; // Shared immutable reference\n");
        fprintf(generator->output, "} Immutable_%s;\n\n", struct_name);
        
        // Generate constructor
        fprintf(generator->output, "Immutable_%s* %s_create_immutable(const void* initial_data) {\n", struct_name, struct_name);
        fprintf(generator->output, "    Immutable_%s* obj = malloc(sizeof(Immutable_%s));\n", struct_name, struct_name);
        fprintf(generator->output, "    if (obj) {\n");
        fprintf(generator->output, "        obj->data = initial_data;\n");
        fprintf(generator->output, "        atomic_store(&obj->ref_count, 1);\n");
        fprintf(generator->output, "        obj->is_shared = false;\n");
        fprintf(generator->output, "    }\n");
        fprintf(generator->output, "    return obj;\n");
        fprintf(generator->output, "}\n\n");
        
        generator->immutable_structures_created++;
    }
    
    return true;
}

/**
 * Generate complete immutable program
 */
bool immutable_generate_program(ImmutableCodeGenerator* generator,
                               EnhancedASTNode* program_ast) {
    if (!generator || !program_ast) return false;
    
    // Use base code generator for standard compilation
    if (generator->base_generator) {
        ASTNode* base_ast = (ASTNode*)program_ast; // Cast for compatibility
        // Use boolean return value from code generator function
        // Note: The actual code generator functions return bool, not CodeGeneratorResult
        bool success = code_generate_program(generator->base_generator, base_ast);
        if (!success) {
            return false;
        }
    }
    
    // Generate immutability-specific code
    if (generator->output) {
        fprintf(generator->output, "// Immutable-by-default runtime support\n");
        fprintf(generator->output, "#include <stdatomic.h>\n");
        fprintf(generator->output, "#include <stdbool.h>\n\n");
    }
    
    // Generate COW support if needed
    if (program_ast->requires_copy_on_write) {
        immutable_generate_cow_support(generator, program_ast);
    }
    
    // Generate immutable structures
    immutable_generate_structure(generator, program_ast, "ExampleStruct");
    
    return true;
}