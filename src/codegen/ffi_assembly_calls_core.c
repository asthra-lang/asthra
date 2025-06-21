/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Core Call Functions
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

// Helper functions for type checking
static bool is_void_type(const TypeInfo *type_info) {
    return type_info && 
           type_info->category == TYPE_INFO_PRIMITIVE &&
           type_info->data.primitive.kind == PRIMITIVE_INFO_VOID;
}

static bool is_float_type(const TypeInfo *type_info) {
    return type_info &&
           type_info->category == TYPE_INFO_PRIMITIVE &&
           type_info->data.primitive.kind == PRIMITIVE_INFO_F32;
}

static bool is_double_type(const TypeInfo *type_info) {
    return type_info &&
           type_info->category == TYPE_INFO_PRIMITIVE &&
           type_info->data.primitive.kind == PRIMITIVE_INFO_F64;
}

// Forward declarations from other files
extern FFIMarshalingType ffi_determine_marshaling_type(FFIAssemblyGenerator *generator, ASTNode *arg);
extern FFIMarshalingType ffi_determine_return_marshaling_type(FFIAssemblyGenerator *generator, ASTNode *call);
extern bool ffi_generate_parameter_marshaling(FFIAssemblyGenerator *generator,
                                              ASTNode *param, FFIMarshalingType marshal_type,
                                              FFIOwnershipTransferType transfer_type, Register target_reg);

// =============================================================================
// FFI CALL GENERATION - CORE FUNCTIONS
// =============================================================================

bool ffi_generate_extern_call(FFIAssemblyGenerator *generator, ASTNode *call_expr) {
    if (!generator || !call_expr || call_expr->type != AST_CALL_EXPR) {
        return false;
    }
    
    atomic_fetch_add(&generator->ffi_calls_generated, 1);
    
    // Create FFI call context
    FFICallContext *context = calloc(1, sizeof(FFICallContext));
    if (!context) return false;
    
    generator->current_ffi_call = context;
    
    // Extract function name and arguments
    ASTNode *function = call_expr->data.call_expr.function;
    ASTNodeList *args = call_expr->data.call_expr.args;
    
    if (function->type != AST_IDENTIFIER) {
        free(context);
        return false;
    }
    
    context->function_name = strdup(function->data.identifier.name);
    context->calling_conv = FFI_CONV_SYSV_AMD64;
    context->parameter_count = args ? args->count : 0;
    
    emit_comment(generator, "FFI function call");
    
    // Allocate parameter information
    if (context->parameter_count > 0) {
        context->parameters = calloc(context->parameter_count, 
                                   sizeof(*context->parameters));
        if (!context->parameters) {
            free(context->function_name);
            free(context);
            return false;
        }
    }
    
    // Process parameters and determine marshaling
    size_t int_param_index = 0;
    size_t float_param_index = 0;
    (void)float_param_index; // Suppress unused variable warning
    size_t stack_offset = 0;
    
    for (size_t i = 0; i < context->parameter_count; i++) {
        ASTNode *arg = args->nodes[i];
        
        // Determine marshaling type based on argument type
        FFIMarshalingType marshal_type = ffi_determine_marshaling_type(generator, arg);
        context->parameters[i].marshal_type = marshal_type;
        
        // Allocate register or stack space
        Register param_reg = ASTHRA_REG_NONE;
        
        switch (marshal_type) {
            case FFI_MARSHAL_DIRECT:
                if (int_param_index < SYSV_INT_PARAM_COUNT) {
                    param_reg = SYSV_INT_PARAM_REGS[int_param_index++];
                } else {
                    context->parameters[i].stack_offset = stack_offset;
                    stack_offset += 8; // 8-byte alignment
                }
                break;
                
            case FFI_MARSHAL_SLICE:
                // Slices need two registers (pointer + length)
                if (int_param_index + 1 < SYSV_INT_PARAM_COUNT) {
                    param_reg = SYSV_INT_PARAM_REGS[int_param_index];
                    int_param_index += 2; // Use two registers
                } else {
                    context->parameters[i].stack_offset = stack_offset;
                    stack_offset += 16; // Two 8-byte values
                }
                break;
                
            case FFI_MARSHAL_STRING:
                if (int_param_index < SYSV_INT_PARAM_COUNT) {
                    param_reg = SYSV_INT_PARAM_REGS[int_param_index++];
                } else {
                    context->parameters[i].stack_offset = stack_offset;
                    stack_offset += 8;
                }
                break;
                
            default:
                // Handle other marshaling types
                break;
        }
        
        context->parameters[i].allocated_reg = param_reg;
        
        // Generate parameter marshaling code
        if (!ffi_generate_parameter_marshaling(generator, arg, marshal_type, 
                                              FFI_OWNERSHIP_NONE, param_reg)) {
            // Cleanup and return error
            free(context->parameters);
            free(context->function_name);
            free(context);
            return false;
        }
    }
    
    context->total_stack_size = stack_offset;
    
    // Align stack to 16-byte boundary (System V AMD64 requirement)
    if (context->total_stack_size % 16 != 0) {
        context->alignment_padding = 16 - (context->total_stack_size % 16);
        context->total_stack_size += context->alignment_padding;
    }
    
    // Adjust stack pointer if needed
    if (context->total_stack_size > 0) {
        emit_instruction(generator, INST_SUB, 2,
                        create_register_operand(ASTHRA_REG_RSP),
                        create_immediate_operand((int64_t)context->total_stack_size));
    }
    
    // Generate the actual call
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand(context->function_name));
    
    // Restore stack pointer
    if (context->total_stack_size > 0) {
        emit_instruction(generator, INST_ADD, 2,
                        create_register_operand(ASTHRA_REG_RSP),
                        create_immediate_operand((int64_t)context->total_stack_size));
    }
    
    // Handle return value if needed
    if (call_expr->type_info && !is_void_type(call_expr->type_info)) {
        // Return value is in RAX for integers, XMM0 for floats
        Register return_reg = (is_float_type(call_expr->type_info) || 
                              is_double_type(call_expr->type_info)) ? 
                              ASTHRA_REG_XMM0 : ASTHRA_REG_RAX;
        
        context->return_reg = return_reg;
        context->return_marshal_type = ffi_determine_return_marshaling_type(generator, call_expr);
    }
    
    // Cleanup context
    free(context->parameters);
    free(context->function_name);
    free(context);
    generator->current_ffi_call = NULL;
    
    return true;
}

bool ffi_generate_variadic_call(FFIAssemblyGenerator *generator, 
                               ASTNode *call_expr, size_t fixed_args) {
    if (!generator || !call_expr || call_expr->type != AST_CALL_EXPR) {
        return false;
    }
    
    atomic_fetch_add(&generator->ffi_calls_generated, 1);
    
    // Create FFI call context for variadic function
    FFICallContext *context = calloc(1, sizeof(FFICallContext));
    if (!context) return false;
    
    generator->current_ffi_call = context;
    
    // Extract function name and arguments
    ASTNode *function = call_expr->data.call_expr.function;
    ASTNodeList *args = call_expr->data.call_expr.args;
    
    if (function->type != AST_IDENTIFIER) {
        free(context);
        return false;
    }
    
    context->function_name = strdup(function->data.identifier.name);
    context->calling_conv = FFI_CONV_SYSV_AMD64;
    context->parameter_count = args ? args->count : 0;
    
    if (context->parameter_count < fixed_args) {
        free(context->function_name);
        free(context);
        return false;
    }
    
    emit_comment(generator, "Variadic FFI function call");
    
    // Allocate parameter information
    if (context->parameter_count > 0) {
        context->parameters = calloc(context->parameter_count, 
                                   sizeof(*context->parameters));
        if (!context->parameters) {
            free(context->function_name);
            free(context);
            return false;
        }
    }
    
    // Process fixed parameters first
    size_t int_param_index = 0;
    size_t float_param_index = 0;
    size_t stack_offset = 0;
    
    (void)float_param_index; // Suppress unused variable warning
    
    for (size_t i = 0; i < fixed_args && i < context->parameter_count; i++) {
        ASTNode *arg = args->nodes[i];
        
        // Determine marshaling type
        FFIMarshalingType marshal_type = ffi_determine_marshaling_type(generator, arg);
        context->parameters[i].marshal_type = marshal_type;
        context->parameters[i].is_variadic = false;
        
        // Allocate register or stack space
        Register param_reg = ASTHRA_REG_NONE;
        
        switch (marshal_type) {
            case FFI_MARSHAL_DIRECT:
                if (int_param_index < SYSV_INT_PARAM_COUNT) {
                    param_reg = SYSV_INT_PARAM_REGS[int_param_index++];
                } else {
                    context->parameters[i].stack_offset = stack_offset;
                    stack_offset += 8;
                }
                break;
                
            case FFI_MARSHAL_SLICE:
                if (int_param_index + 1 < SYSV_INT_PARAM_COUNT) {
                    param_reg = SYSV_INT_PARAM_REGS[int_param_index];
                    int_param_index += 2;
                } else {
                    context->parameters[i].stack_offset = stack_offset;
                    stack_offset += 16;
                }
                break;
                
            default:
                if (int_param_index < SYSV_INT_PARAM_COUNT) {
                    param_reg = SYSV_INT_PARAM_REGS[int_param_index++];
                } else {
                    context->parameters[i].stack_offset = stack_offset;
                    stack_offset += 8;
                }
                break;
        }
        
        context->parameters[i].allocated_reg = param_reg;
        
        // Generate parameter marshaling
        if (!ffi_generate_parameter_marshaling(generator, arg, marshal_type, 
                                              FFI_OWNERSHIP_NONE, param_reg)) {
            // Cleanup and return error
            free(context->parameters);
            free(context->function_name);
            free(context);
            return false;
        }
    }
    
    // Process variadic parameters
    for (size_t i = fixed_args; i < context->parameter_count; i++) {
        ASTNode *arg = args->nodes[i];
        
        context->parameters[i].marshal_type = FFI_MARSHAL_VARIADIC;
        context->parameters[i].is_variadic = true;
        
        // Variadic arguments are always passed on stack in System V AMD64
        context->parameters[i].stack_offset = stack_offset;
        context->parameters[i].allocated_reg = ASTHRA_REG_NONE;
        
        // Generate variadic argument marshaling
        if (!ffi_generate_parameter_marshaling(generator, arg, FFI_MARSHAL_VARIADIC, 
                                              FFI_OWNERSHIP_NONE, ASTHRA_REG_NONE)) {
            free(context->parameters);
            free(context->function_name);
            free(context);
            return false;
        }
        
        // Variadic arguments are 8-byte aligned
        stack_offset += 8;
    }
    
    context->total_stack_size = stack_offset;
    
    // Set AL register to number of vector registers used (required for variadic calls)
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(REG_AL),
                    create_immediate_operand(0)); // No vector registers for now
    
    // Align stack to 16-byte boundary
    if (context->total_stack_size % 16 != 0) {
        context->alignment_padding = 16 - (context->total_stack_size % 16);
        context->total_stack_size += context->alignment_padding;
    }
    
    // Adjust stack pointer if needed
    if (context->total_stack_size > 0) {
        emit_instruction(generator, INST_SUB, 2,
                        create_register_operand(ASTHRA_REG_RSP),
                        create_immediate_operand((int64_t)context->total_stack_size));
    }
    
    // Generate the actual call
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand(context->function_name));
    
    // Restore stack pointer
    if (context->total_stack_size > 0) {
        emit_instruction(generator, INST_ADD, 2,
                        create_register_operand(ASTHRA_REG_RSP),
                        create_immediate_operand((int64_t)context->total_stack_size));
    }
    
    // Handle return value
    if (call_expr->type_info && !is_void_type(call_expr->type_info)) {
        context->return_reg = ASTHRA_REG_RAX;
        context->return_marshal_type = ffi_determine_return_marshaling_type(generator, call_expr);
    }
    
    // Cleanup context
    free(context->parameters);
    free(context->function_name);
    free(context);
    generator->current_ffi_call = NULL;
    
    return true;
} 
