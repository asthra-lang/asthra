/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Parameter Marshaling
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

// Forward declarations from other files
extern bool ffi_marshal_slice_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg);
extern bool ffi_marshal_string_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg);
extern bool ffi_marshal_result_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg);

// =============================================================================
// FFI PARAMETER MARSHALING COORDINATION
// =============================================================================

bool ffi_generate_parameter_marshaling(FFIAssemblyGenerator *generator,
                                     ASTNode *param, FFIMarshalingType marshal_type,
                                     FFIOwnershipTransferType transfer_type, Register target_reg) {
    if (!generator || !param) return false;
    
    // Phase 4: FFI Annotation Ambiguity Fix - Use annotation-aware parameter handling
    // First check if this is a parameter declaration with FFI annotations
    if (param->type == AST_PARAM_DECL) {
        return ffi_generate_annotated_parameter(generator, param, REG_NONE, target_reg);
    }
    
    // For expressions, use the provided transfer type or extract from context
    FFIOwnershipTransferType actual_transfer_type = transfer_type;
    if (transfer_type == FFI_OWNERSHIP_NONE) {
        // Try to extract annotation from the parameter expression context
        ffi_extract_annotation(generator, param, &actual_transfer_type);
    }
    
    switch (marshal_type) {
        case FFI_MARSHAL_SLICE:
            return ffi_marshal_slice_parameter(generator, param, target_reg);
            
        case FFI_MARSHAL_STRING:
            return ffi_marshal_string_parameter(generator, param, target_reg);
            
        case FFI_MARSHAL_RESULT:
            return ffi_marshal_result_parameter(generator, param, target_reg);
            
        case FFI_MARSHAL_VARIADIC:
            {
                emit_comment(generator, "Marshal variadic parameter with FFI annotation support");
                
                // Allocate register for the argument
                Register arg_reg = register_allocate(generator->base_generator->register_allocator, true);
                if (arg_reg == REG_NONE) return false;
                
                // Generate code to load the argument
                if (!code_generate_expression(generator->base_generator, param, arg_reg)) {
                    register_free(generator->base_generator->register_allocator, arg_reg);
                    return false;
                }
                
                // Apply FFI annotation-based transfer before pushing to stack
                if (!ffi_generate_annotation_based_transfer(generator, arg_reg, arg_reg, actual_transfer_type)) {
                    register_free(generator->base_generator->register_allocator, arg_reg);
                    return false;
                }
                
                // Push variadic argument onto stack
                emit_instruction(generator, INST_PUSH, 1,
                                create_register_operand(arg_reg));
                
                register_free(generator->base_generator->register_allocator, arg_reg);
                return true;
            }
            
        case FFI_MARSHAL_DIRECT:
        default:
            {
                emit_comment(generator, "Marshal direct parameter with FFI annotation support");
                
                // Direct marshaling with FFI annotation support
                if (target_reg == REG_NONE) {
                    // If no target register, push onto stack
                    Register temp_reg = register_allocate(generator->base_generator->register_allocator, true);
                    if (temp_reg == REG_NONE) return false;
                    
                    if (!code_generate_expression(generator->base_generator, param, temp_reg)) {
                        register_free(generator->base_generator->register_allocator, temp_reg);
                        return false;
                    }
                    
                    // Apply FFI annotation-based transfer before pushing
                    Register transfer_reg = register_allocate(generator->base_generator->register_allocator, true);
                    if (transfer_reg == REG_NONE) {
                        register_free(generator->base_generator->register_allocator, temp_reg);
                        return false;
                    }
                    
                    if (!ffi_generate_annotation_based_transfer(generator, temp_reg, transfer_reg, actual_transfer_type)) {
                        register_free(generator->base_generator->register_allocator, temp_reg);
                        register_free(generator->base_generator->register_allocator, transfer_reg);
                        return false;
                    }
                    
                    emit_instruction(generator, INST_PUSH, 1,
                                    create_register_operand(transfer_reg));
                    
                    register_free(generator->base_generator->register_allocator, temp_reg);
                    register_free(generator->base_generator->register_allocator, transfer_reg);
                    return true;
                } else {
                    // Generate expression and apply FFI annotation-based transfer
                    Register temp_reg = register_allocate(generator->base_generator->register_allocator, true);
                    if (temp_reg == REG_NONE) return false;
                    
                    if (!code_generate_expression(generator->base_generator, param, temp_reg)) {
                        register_free(generator->base_generator->register_allocator, temp_reg);
                        return false;
                    }
                    
                    bool success = ffi_generate_annotation_based_transfer(generator, temp_reg, target_reg, actual_transfer_type);
                    register_free(generator->base_generator->register_allocator, temp_reg);
                    return success;
                }
            }
    }
}

bool ffi_generate_return_unmarshaling(FFIAssemblyGenerator *generator,
                                    FFIMarshalingType marshal_type,
                                    FFIOwnershipTransferType transfer_type,
                                    Register source_reg, Register target_reg) {
    (void)marshal_type; // Mark as unused to suppress warning
    (void)transfer_type; // Mark as unused to suppress warning
    
    if (!generator) return false;
    
    // Simplified implementation - just move from source to target
    if (source_reg != target_reg) {
        return emit_instruction(generator, INST_MOV, 2,
                              create_register_operand(target_reg),
                              create_register_operand(source_reg));
    }
    return true;
} 
