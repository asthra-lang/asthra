/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Marshaling Implementations
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
// SPECIFIC MARSHALING IMPLEMENTATIONS
// =============================================================================

bool ffi_marshal_slice_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg) {
    if (!generator || !param) return false;
    
    emit_comment(generator, "Marshal slice parameter to pointer+length");
    
    // Allocate registers for slice components
    Register slice_reg = register_allocate(generator->base_generator->register_allocator, true);
    Register ptr_reg = register_allocate(generator->base_generator->register_allocator, true);
    Register len_reg = register_allocate(generator->base_generator->register_allocator, true);
    
    if (slice_reg == REG_NONE || ptr_reg == REG_NONE || len_reg == REG_NONE) {
        if (slice_reg != REG_NONE) register_free(generator->base_generator->register_allocator, slice_reg);
        if (ptr_reg != REG_NONE) register_free(generator->base_generator->register_allocator, ptr_reg);
        if (len_reg != REG_NONE) register_free(generator->base_generator->register_allocator, len_reg);
        return false;
    }
    
    // Generate code to load the slice
    if (!code_generate_expression(generator->base_generator, param, slice_reg)) {
        register_free(generator->base_generator->register_allocator, slice_reg);
        register_free(generator->base_generator->register_allocator, ptr_reg);
        register_free(generator->base_generator->register_allocator, len_reg);
        return false;
    }
    
    // Extract pointer from slice (offset 0)
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(ptr_reg),
                    create_memory_operand(slice_reg, REG_NONE, 1, 0));
    
    // Extract length from slice (offset 8)
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(len_reg),
                    create_memory_operand(slice_reg, REG_NONE, 1, 8));
    
    // Move pointer to target register (first parameter)
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(target_reg),
                    create_register_operand(ptr_reg));
    
    // Length will be passed in the next parameter register
    // This is handled by the calling function
    
    // Cleanup
    register_free(generator->base_generator->register_allocator, slice_reg);
    register_free(generator->base_generator->register_allocator, ptr_reg);
    register_free(generator->base_generator->register_allocator, len_reg);
    
    return true;
}

bool ffi_marshal_string_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg) {
    if (!generator || !param) return false;
    
    emit_comment(generator, "Marshal string parameter to C string");
    
    // Allocate register for string
    Register string_reg = register_allocate(generator->base_generator->register_allocator, true);
    if (string_reg == REG_NONE) return false;
    
    // Generate code to load the string
    if (!code_generate_expression(generator->base_generator, param, string_reg)) {
        register_free(generator->base_generator->register_allocator, string_reg);
        return false;
    }
    
    // Call runtime function to convert Asthra string to C string
    // Set up parameter for runtime call
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(REG_RDI),
                    create_register_operand(string_reg));
    
    // Call asthra_string_to_cstr
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_string_to_cstr"));
    
    // Move result to target register
    if (target_reg != REG_RAX) {
        emit_instruction(generator, INST_MOV, 2,
                        create_register_operand(target_reg),
                        create_register_operand(REG_RAX));
    }
    
    register_free(generator->base_generator->register_allocator, string_reg);
    return true;
}

bool ffi_marshal_result_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg) {
    if (!generator || !param) return false;
    
    emit_comment(generator, "Marshal Result<T,E> parameter");
    
    // Allocate register for result
    Register result_reg = register_allocate(generator->base_generator->register_allocator, true);
    if (result_reg == REG_NONE) return false;
    
    // Generate code to load the result
    if (!code_generate_expression(generator->base_generator, param, result_reg)) {
        register_free(generator->base_generator->register_allocator, result_reg);
        return false;
    }
    
    // Result types are passed as tagged unions
    // For FFI, we might need to convert to a specific C representation
    // For now, pass the result structure directly
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(target_reg),
                    create_register_operand(result_reg));
    
    register_free(generator->base_generator->register_allocator, result_reg);
    return true;
}

bool ffi_marshal_option_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg) {
    if (!generator || !param) return false;
    
    emit_comment(generator, "Marshal Option<T> parameter");
    
    // Allocate register for option
    Register option_reg = register_allocate(generator->base_generator->register_allocator, true);
    if (option_reg == REG_NONE) return false;
    
    // Generate code to load the option
    if (!code_generate_expression(generator->base_generator, param, option_reg)) {
        register_free(generator->base_generator->register_allocator, option_reg);
        return false;
    }
    
    // Option types are passed as tagged unions (similar to Result)
    // For FFI, we represent Option<T> as:
    // - NULL pointer for None
    // - Valid pointer to T for Some(value)
    // This matches common C idioms for optional values
    
    // Check if this is None (tag == 1) or Some (tag == 0)
    Register tag_reg = register_allocate(generator->base_generator->register_allocator, true);
    if (tag_reg == REG_NONE) {
        register_free(generator->base_generator->register_allocator, option_reg);
        return false;
    }
    
    // Load tag from option struct (offset 0)
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(tag_reg),
                    create_memory_operand(option_reg, REG_NONE, 1, 0));
    
    // Create labels for branching
    char *none_label = generate_unique_label(generator, "option_none");
    char *end_label = generate_unique_label(generator, "option_end");
    
    // Compare tag with 1 (None)
    emit_instruction(generator, INST_CMP, 2,
                    create_register_operand(tag_reg),
                    create_immediate_operand(1));
    
    // Jump if equal (is None)
    emit_instruction(generator, INST_JE, 1,
                    create_label_operand(none_label));
    
    // Some case: Load pointer to value
    emit_instruction(generator, INST_LEA, 2,
                    create_register_operand(target_reg),
                    create_memory_operand(option_reg, REG_NONE, 1, 8)); // Value at offset 8
    emit_instruction(generator, INST_JMP, 1,
                    create_label_operand(end_label));
    
    // None case: Set to NULL
    emit_label(generator, none_label);
    emit_instruction(generator, INST_XOR, 2,
                    create_register_operand(target_reg),
                    create_register_operand(target_reg));
    
    emit_label(generator, end_label);
    
    // Cleanup
    register_free(generator->base_generator->register_allocator, option_reg);
    register_free(generator->base_generator->register_allocator, tag_reg);
    free(none_label);
    free(end_label);
    
    return true;
} 
