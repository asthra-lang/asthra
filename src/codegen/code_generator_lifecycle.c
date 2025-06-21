/**
 * Asthra Programming Language Compiler
 * Code Generator Lifecycle Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Code generator creation, destruction, and reset functionality
 */

#include "code_generator_common.h"

// =============================================================================
// CODE GENERATOR CREATION AND DESTRUCTION
// =============================================================================

CodeGenerator *code_generator_create(TargetArchitecture arch, CallingConvention conv) {
    CodeGenerator *generator = calloc(1, sizeof(CodeGenerator));
    if (!generator) {
        return NULL;
    }
    
    // C17 designated initializer for configuration
    *generator = (CodeGenerator) {
        .target_arch = arch,
        .calling_conv = conv,
        .generate_debug_info = false,
        .optimize_code = true,
        .config = {
            .emit_comments = true,
            .position_independent_code = false,
            .stack_protection = true,
            .bounds_checking = true,
            .optimization_level = 2
        }
    };
    
    // Create core components
    generator->instruction_buffer = instruction_buffer_create(DEFAULT_INSTRUCTION_BUFFER_SIZE);
    generator->register_allocator = register_allocator_create();
    generator->label_manager = label_manager_create(DEFAULT_LABEL_MANAGER_SIZE);
    generator->symbol_table = local_symbol_table_create(DEFAULT_SYMBOL_TABLE_SIZE);
    
    // Phase 4: Create generic registry for struct monomorphization
    generator->generic_registry = generic_registry_create();
    
    if (!generator->instruction_buffer || !generator->register_allocator || 
        !generator->label_manager || !generator->symbol_table || !generator->generic_registry) {
        code_generator_destroy(generator);
        return NULL;
    }
    
    return generator;
}

void code_generator_destroy(CodeGenerator *generator) {
    if (!generator) return;
    
    instruction_buffer_destroy(generator->instruction_buffer);
    register_allocator_destroy(generator->register_allocator);
    label_manager_destroy(generator->label_manager);
    local_symbol_table_destroy(generator->symbol_table);
    
    // Phase 4: Destroy generic registry
    generic_registry_destroy(generator->generic_registry);
    
    free(generator->current_function_name);
    free(generator);
}

void code_generator_reset(CodeGenerator *generator) {
    if (!generator) return;
    
    // Reset instruction buffer
    if (generator->instruction_buffer) {
        instruction_buffer_destroy(generator->instruction_buffer);
        generator->instruction_buffer = instruction_buffer_create(DEFAULT_INSTRUCTION_BUFFER_SIZE);
    }
    
    // Reset register allocator
    if (generator->register_allocator) {
        register_allocator_destroy(generator->register_allocator);
        generator->register_allocator = register_allocator_create();
    }
    
    // Reset label manager
    if (generator->label_manager) {
        label_manager_destroy(generator->label_manager);
        generator->label_manager = label_manager_create(DEFAULT_LABEL_MANAGER_SIZE);
    }
    
    // Reset symbol table
    if (generator->symbol_table) {
        local_symbol_table_destroy(generator->symbol_table);
        generator->symbol_table = local_symbol_table_create(DEFAULT_SYMBOL_TABLE_SIZE);
    }
    
    // Reset state
    generator->current_function_stack_size = 0;
    generator->current_function_param_size = 0;
    free(generator->current_function_name);
    generator->current_function_name = NULL;
    
    // Reset statistics using atomic operations
    atomic_store(&generator->stats.instructions_generated, 0);
    atomic_store(&generator->stats.basic_blocks_generated, 0);
    atomic_store(&generator->stats.functions_generated, 0);
    atomic_store(&generator->stats.bytes_generated, 0);
}

// =============================================================================
// CODE GENERATOR SEMANTIC ANALYZER INTEGRATION
// =============================================================================

void code_generator_set_semantic_analyzer(CodeGenerator *generator, SemanticAnalyzer *analyzer) {
    if (generator) {
        generator->semantic_analyzer = analyzer;
    }
} 
