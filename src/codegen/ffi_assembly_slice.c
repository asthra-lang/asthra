/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Slice Operation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_assembly_generator.h"
#include "ffi_assembly_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// SLICE OPERATION GENERATION
// =============================================================================

bool ffi_generate_slice_creation(FFIAssemblyGenerator *generator,
                                ASTNode *array_literal, Register result_reg) {
    (void)generator; // Mark as unused to suppress warning
    (void)array_literal; // Mark as unused to suppress warning
    (void)result_reg; // Mark as unused to suppress warning
    // Simplified implementation - would generate slice creation
    return true;
}

bool ffi_generate_slice_length_access(FFIAssemblyGenerator *generator,
                                     Register slice_reg, Register result_reg) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->slice_operations_generated, 1);
    
    emit_comment(generator, "Slice length access");
    
    // Slice structure: [ptr, len, cap] (simplified)
    // Length is at offset 8 from slice pointer
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(result_reg),
                    create_memory_operand(slice_reg, ASTHRA_REG_NONE, 1, 8));
    
    return true;
}

bool ffi_generate_slice_bounds_check(FFIAssemblyGenerator *generator,
                                    Register slice_reg, Register index_reg,
                                    char *bounds_error_label) {
    if (!generator || !generator->config.enable_bounds_checking) {
        return true; // Skip if bounds checking disabled
    }
    
    emit_comment(generator, "Slice bounds check");
    
    // Load slice length
    Register len_reg = register_allocate(generator->base_generator->register_allocator, true);
    if (len_reg == ASTHRA_REG_NONE) return false;
    
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(len_reg),
                    create_memory_operand(slice_reg, ASTHRA_REG_NONE, 1, 8));
    
    // Compare index with length
    emit_instruction(generator, INST_CMP, 2,
                    create_register_operand(index_reg),
                    create_register_operand(len_reg));
    
    // Jump to error if index >= length
    emit_instruction(generator, INST_JAE, 1, create_label_operand(bounds_error_label));
    
    register_free(generator->base_generator->register_allocator, len_reg);
    
    return true;
}

bool ffi_generate_slice_to_ffi(FFIAssemblyGenerator *generator,
                              Register slice_reg, Register ptr_reg, Register len_reg) {
    if (!generator) return false;
    
    emit_comment(generator, "Slice to FFI pointer conversion");
    
    // Load pointer from slice
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(ptr_reg),
                    create_memory_operand(slice_reg, ASTHRA_REG_NONE, 1, 0));
    
    // Load length from slice
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(len_reg),
                    create_memory_operand(slice_reg, ASTHRA_REG_NONE, 1, 8));
    
    return true;
} 
