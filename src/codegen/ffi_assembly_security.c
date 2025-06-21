/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Security Operation Functions
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

// Forward declarations for static functions
bool ffi_generate_secure_operation(FFIAssemblyGenerator *generator, ASTNode *operation, SecurityContext *context);

// =============================================================================
// SECURITY OPERATION GENERATION
// =============================================================================

bool ffi_generate_constant_time_operation(FFIAssemblyGenerator *generator,
                                        ASTNode *operation) {
    if (!generator || !generator->config.enable_security_features) {
        return code_generate_statement(generator->base_generator, operation);
    }
    
    atomic_fetch_add(&generator->security_operations_generated, 1);
    
    emit_comment(generator, "Constant-time operation");
    
    // Create security context
    SecurityContext *context = calloc(1, sizeof(SecurityContext));
    if (!context) return false;
    
    generator->current_security_op = context;
    context->operation = SECURITY_OP_CONSTANT_TIME;
    context->avoid_branches = true;
    context->use_cmov_instructions = true;
    context->avoid_memory_access_patterns = true;
    
    // Disable branch prediction and speculative execution
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_disable_speculation"));
    
    // Generate operation with constant-time constraints
    bool result = ffi_generate_secure_operation(generator, operation, context);
    
    // Re-enable optimizations
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_enable_speculation"));
    
    // Add memory barrier to prevent reordering
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_memory_barrier"));
    
    free(context);
    generator->current_security_op = NULL;
    
    return result;
}

bool ffi_generate_volatile_memory_access(FFIAssemblyGenerator *generator,
                                        Register memory_reg, size_t size,
                                        bool is_read) {
    if (!generator) return false;
    
    emit_comment(generator, is_read ? "Volatile memory read" : "Volatile memory write");
    
    // Add memory barrier before volatile access
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_memory_barrier"));
    
    // Generate volatile memory access with compiler barriers
    if (is_read) {
        // Volatile read with explicit memory ordering
        emit_instruction(generator, INST_MOV, 2,
                        create_register_operand(ASTHRA_REG_RAX),
                        create_memory_operand(memory_reg, ASTHRA_REG_NONE, 1, 0));
        
        // Prevent compiler from optimizing away the read
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_compiler_barrier"));
    } else {
        // Volatile write with explicit memory ordering
        emit_instruction(generator, INST_MOV, 2,
                        create_memory_operand(memory_reg, ASTHRA_REG_NONE, 1, 0),
                        create_register_operand(ASTHRA_REG_RAX));
        
        // Ensure write is committed before continuing
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_memory_fence"));
    }
    
    // Add memory barrier after volatile access
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_memory_barrier"));
    
    // For larger sizes, generate appropriate sized operations
    if (size > 8) {
        emit_comment(generator, "Multi-byte volatile access");
        
        // Generate loop for larger volatile operations
        Register counter_reg = register_allocate(generator->base_generator->register_allocator, false);
        if (counter_reg != ASTHRA_REG_NONE) {
            // Initialize counter
            emit_instruction(generator, INST_MOV, 2,
                            create_register_operand(counter_reg),
                            create_immediate_operand(0));
            
            // Loop label
            const char *loop_label = "volatile_loop";
            emit_label(generator, loop_label);
            
            // Volatile access for current byte
            if (is_read) {
                emit_instruction(generator, INST_MOV, 2,
                                create_register_operand(REG_AL),
                                create_memory_operand(memory_reg, counter_reg, 1, 0));
            } else {
                emit_instruction(generator, INST_MOV, 2,
                                create_memory_operand(memory_reg, counter_reg, 1, 0),
                                create_register_operand(REG_AL));
            }
            
            // Increment counter
            emit_instruction(generator, INST_INC, 1,
                            create_register_operand(counter_reg));
            
            // Compare with size
            emit_instruction(generator, INST_CMP, 2,
                            create_register_operand(counter_reg),
                            create_immediate_operand((int64_t)size));
            
            // Jump back if not done
            emit_instruction(generator, INST_JL, 1,
                            create_label_operand(loop_label));
            
            register_free(generator->base_generator->register_allocator, counter_reg);
        }
    }
    
    return true;
}

bool ffi_generate_secure_zero(FFIAssemblyGenerator *generator,
                             Register memory_reg, Register size_reg) {
    if (!generator) return false;
    
    emit_comment(generator, "Secure memory zeroing");
    
    // Call runtime secure zero function
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(ASTHRA_REG_RDI),
                    create_register_operand(memory_reg));
    
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(ASTHRA_REG_RSI),
                    create_register_operand(size_reg));
    
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_secure_zero"));
    
    // Add memory barrier to ensure zeroing is complete
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_memory_barrier"));
    
    return true;
}

bool ffi_generate_ffi_security_boundary(FFIAssemblyGenerator *generator,
                                       ASTNode *extern_call, bool entering_ffi) {
    if (!generator || !extern_call) return false;
    
    if (entering_ffi) {
        emit_comment(generator, "Entering FFI security boundary");
        
        // Validate security annotations on the extern function
        if (extern_call->type == AST_CALL_EXPR) {
            ASTNode *function = extern_call->data.call_expr.function;
            if (function && function->type == AST_IDENTIFIER) {
                const char *func_name = function->data.identifier.name;
                
                // Check if this is a security-sensitive FFI call
                if (strstr(func_name, "crypto") || strstr(func_name, "secure") || 
                    strstr(func_name, "constant_time")) {
                    
                    // Add security barriers for crypto functions
                    emit_instruction(generator, INST_CALL, 1,
                                    create_label_operand("asthra_crypto_barrier_enter"));
                    
                    // Clear sensitive registers
                    emit_instruction(generator, INST_XOR, 2,
                                    create_register_operand(ASTHRA_REG_RAX),
                                    create_register_operand(ASTHRA_REG_RAX));
                    emit_instruction(generator, INST_XOR, 2,
                                    create_register_operand(ASTHRA_REG_RDX),
                                    create_register_operand(ASTHRA_REG_RDX));
                }
            }
        }
        
        // Save security context
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_save_security_context"));
        
    } else {
        emit_comment(generator, "Exiting FFI security boundary");
        
        // Restore security context
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_restore_security_context"));
        
        // Clear any sensitive data that might have been returned
        if (extern_call->type == AST_CALL_EXPR) {
            ASTNode *function = extern_call->data.call_expr.function;
            if (function && function->type == AST_IDENTIFIER) {
                const char *func_name = function->data.identifier.name;
                
                if (strstr(func_name, "crypto") || strstr(func_name, "secure")) {
                    // Add security barriers for crypto functions
                    emit_instruction(generator, INST_CALL, 1,
                                    create_label_operand("asthra_crypto_barrier_exit"));
                }
            }
        }
        
        // Memory barrier to prevent sensitive data leakage
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_memory_barrier"));
    }
    
    return true;
}

bool ffi_validate_security_annotations_at_boundary(FFIAssemblyGenerator *generator,
                                                  ASTNode *extern_decl) {
    if (!generator || !extern_decl || extern_decl->type != AST_EXTERN_DECL) {
        return false;
    }
    
    emit_comment(generator, "Validating FFI security annotations");
    
    // Check for security annotations on the extern function
    ASTNodeList *annotations = extern_decl->data.extern_decl.annotations;
    bool has_constant_time = false;
    bool has_volatile_memory = false;
    
    if (annotations) {
        for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
            ASTNode *annotation = ast_node_list_get(annotations, i);
            if (annotation && annotation->type == AST_SECURITY_TAG) {
                SecurityType security_type = annotation->data.security_tag.security_type;
                
                switch (security_type) {
                    case SECURITY_CONSTANT_TIME:
                        has_constant_time = true;
                        break;
                    case SECURITY_VOLATILE_MEMORY:
                        has_volatile_memory = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    // Generate runtime validation for security properties
    if (has_constant_time) {
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_validate_constant_time_ffi"));
    }
    
    if (has_volatile_memory) {
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_validate_volatile_memory_ffi"));
    }
    
    // Validate parameter transfer annotations
    ASTNodeList *params = extern_decl->data.extern_decl.params;
    if (params) {
        for (size_t i = 0; i < ast_node_list_size(params); i++) {
            ASTNode *param = ast_node_list_get(params, i);
            if (param && param->data.param_decl.annotations) {
                // Generate validation for parameter transfer semantics
                emit_instruction(generator, INST_PUSH, 1,
                                create_immediate_operand((int64_t)i));
                emit_instruction(generator, INST_CALL, 1,
                                create_label_operand("asthra_validate_ffi_parameter"));
                emit_instruction(generator, INST_ADD, 2,
                                create_register_operand(ASTHRA_REG_RSP),
                                create_immediate_operand(8));
            }
        }
    }
    
    return true;
}

bool ffi_generate_ownership_transfer(FFIAssemblyGenerator *generator,
                                   Register source_reg, Register target_reg,
                                   FFIOwnershipTransferType transfer_type) {
    (void)generator; // Mark as unused to suppress warning
    (void)source_reg; // Mark as unused to suppress warning
    (void)target_reg; // Mark as unused to suppress warning
    (void)transfer_type; // Mark as unused to suppress warning
    // Simplified implementation - would generate ownership transfer code
    return true;
}

// =============================================================================
// HELPER FUNCTION IMPLEMENTATIONS
// =============================================================================

bool ffi_generate_secure_operation(FFIAssemblyGenerator *generator, ASTNode *operation, SecurityContext *context) {
    if (!generator || !operation) return false;
    
    // Check if security features are enabled
    if (!generator->config.enable_security_features) {
        // Fall back to normal code generation
        return code_generate_statement(generator->base_generator, operation);
    }
    
    emit_comment(generator, "Secure operation with constant-time guarantees");
    
    // Generate secure code based on context
    if (context && context->operation == SECURITY_OP_CONSTANT_TIME) {
        // Implement constant-time operation
        emit_comment(generator, "Constant-time operation");
        
        // Disable branch prediction optimizations
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_disable_branch_prediction"));
        
        // Generate the operation
        bool result = code_generate_statement(generator->base_generator, operation);
        
        // Re-enable optimizations
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_enable_branch_prediction"));
        
        return result;
    }
    
    if (context && context->operation == SECURITY_OP_VOLATILE_MEMORY) {
        // Implement volatile memory access
        emit_comment(generator, "Volatile memory access");
        
        // Use memory barriers
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_memory_barrier"));
        
        bool result = code_generate_statement(generator->base_generator, operation);
        
        emit_instruction(generator, INST_CALL, 1,
                        create_label_operand("asthra_memory_barrier"));
        
        return result;
    }
    
    // Default secure operation
    return code_generate_statement(generator->base_generator, operation);
}

bool ffi_generate_unsafe_block(FFIAssemblyGenerator *generator, ASTNode *unsafe_block) {
    if (!generator || !unsafe_block || unsafe_block->type != AST_UNSAFE_BLOCK) {
        return false;
    }
    
    emit_comment(generator, "Unsafe block");
    
    // Generate GC barriers for entering unsafe code
    if (!ffi_generate_gc_barriers(generator, true, false)) {
        return false;
    }
    
    // Generate the unsafe block content
    bool result = code_generate_statement(generator->base_generator, 
                                        unsafe_block->data.unsafe_block.block);
    
    // Generate GC barriers for exiting unsafe code
    if (!ffi_generate_gc_barriers(generator, false, true)) {
        return false;
    }
    
    return result;
}

bool ffi_generate_gc_barriers(FFIAssemblyGenerator *generator,
                             bool entering_unsafe, bool exiting_unsafe) {
    if (!generator) return false;
    
    if (entering_unsafe) {
        emit_comment(generator, "GC barrier: entering unsafe code");
        // Notify GC that we're entering unsafe code
        // This might involve setting thread-local flags or calling runtime functions
    }
    
    if (exiting_unsafe) {
        emit_comment(generator, "GC barrier: exiting unsafe code");
        // Notify GC that we're exiting unsafe code
        // This might involve clearing flags or triggering GC checks
    }
    
    return true;
}

bool ffi_optimize_zero_cost_abstractions(FFIAssemblyGenerator *generator) {
    (void)generator; // Mark as unused to suppress warning
    // Simplified implementation - would optimize zero-cost abstractions
    return true;
} 
