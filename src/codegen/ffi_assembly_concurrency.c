/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Concurrency Operation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_assembly_generator.h"
#include "ffi_assembly_internal.h"
// Note: advanced_concurrency.h removed - Tier 3 features moved to stdlib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ffi_assembly_concurrency.h"

// Forward declarations for functions in this file
bool ffi_generate_concurrency_unsafe_block(FFIAssemblyGenerator *generator, ASTNode *unsafe_block);
bool ffi_generate_concurrency_gc_barriers(FFIAssemblyGenerator *generator, bool entering_unsafe, bool exiting_unsafe);

// =============================================================================
// CONCURRENCY GENERATION
// =============================================================================

bool ffi_generate_spawn_statement(FFIAssemblyGenerator *generator, ASTNode *spawn_stmt) {
    if (!generator || !spawn_stmt || spawn_stmt->type != AST_SPAWN_STMT) {
        return false;
    }
    
    atomic_fetch_add(&generator->spawn_statements_generated, 1);
    
    emit_comment(generator, "Spawn statement");
    
    // Create concurrency context
    ConcurrencyContext *context = calloc(1, sizeof(ConcurrencyContext));
    if (!context) return false;
    
    generator->current_concurrency_op = context;
    context->operation = CONCURRENCY_OP_SPAWN;
    context->function_name = strdup(spawn_stmt->data.spawn_stmt.function_name);
    
    // Process arguments
    ASTNodeList *args = spawn_stmt->data.spawn_stmt.args;
    context->argument_count = args ? args->count : 0;
    
    if (context->argument_count > 0) {
        context->argument_regs = calloc(context->argument_count, sizeof(Register));
        if (!context->argument_regs) {
            free(context->function_name);
            free(context);
            return false;
        }
        
        // Generate arguments
        for (size_t i = 0; i < context->argument_count; i++) {
            Register arg_reg = register_allocate(generator->base_generator->register_allocator, true);
            if (arg_reg == ASTHRA_REG_NONE) {
                // Cleanup
                for (size_t j = 0; j < i; j++) {
                    register_free(generator->base_generator->register_allocator, 
                                context->argument_regs[j]);
                }
                free(context->argument_regs);
                free(context->function_name);
                free(context);
                return false;
            }
            
            context->argument_regs[i] = arg_reg;
            
            if (!code_generate_expression(generator->base_generator, 
                                        args->nodes[i], arg_reg)) {
                // Cleanup and return error
                for (size_t j = 0; j <= i; j++) {
                    register_free(generator->base_generator->register_allocator, 
                                context->argument_regs[j]);
                }
                free(context->argument_regs);
                free(context->function_name);
                free(context);
                return false;
            }
        }
    }
    
    // Generate task creation call
    bool result = ffi_generate_task_creation(generator, context->function_name,
                                            context->argument_regs, context->argument_count,
                                            ASTHRA_REG_RAX);
    
    // Cleanup
    if (context->argument_regs) {
        for (size_t i = 0; i < context->argument_count; i++) {
            register_free(generator->base_generator->register_allocator, 
                        context->argument_regs[i]);
        }
        free(context->argument_regs);
    }
    
    free(context->function_name);
    free(context);
    generator->current_concurrency_op = NULL;
    
    return result;
}

bool ffi_generate_task_creation(FFIAssemblyGenerator *generator,
                               char *function_name, Register *arg_regs,
                               size_t arg_count, Register handle_reg) {
    (void)arg_regs; // Mark as unused to suppress warning
    
    if (!generator) return false;
    
    emit_comment(generator, "Task creation");
    
    // Set up parameters for spawn_task runtime function
    // Parameters: function pointer, arguments, argument count
    
    // Function name/pointer in RDI
    emit_instruction(generator, INST_LEA, 2,
                    create_register_operand(ASTHRA_REG_RDI),
                    create_label_operand(function_name));
    
    // Arguments array in RSI (simplified)
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(ASTHRA_REG_RSI),
                    create_immediate_operand(0)); // NULL for now
    
    // Argument count in RDX
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(ASTHRA_REG_RDX),
                    create_immediate_operand((int64_t)arg_count));
    
    // Call runtime spawn function
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand(generator->runtime_functions.spawn_task));
    
    // Move task handle to target register
    if (handle_reg != ASTHRA_REG_RAX) {
        emit_instruction(generator, INST_MOV, 2,
                        create_register_operand(handle_reg),
                        create_register_operand(ASTHRA_REG_RAX));
    }
    
    return true;
}

// =============================================================================
// UNSAFE BLOCK GENERATION (RENAMED TO AVOID CONFLICT)
// =============================================================================

bool ffi_generate_concurrency_unsafe_block(FFIAssemblyGenerator *generator, ASTNode *unsafe_block) {
    if (!generator || !unsafe_block || unsafe_block->type != AST_UNSAFE_BLOCK) {
        return false;
    }
    
    emit_comment(generator, "Concurrency unsafe block");
    
    // Generate GC barriers for entering unsafe code
    if (!ffi_generate_concurrency_gc_barriers(generator, true, false)) {
        return false;
    }
    
    // Generate the unsafe block content
    bool result = code_generate_statement(generator->base_generator, 
                                        unsafe_block->data.unsafe_block.block);
    
    // Generate GC barriers for exiting unsafe code
    if (!ffi_generate_concurrency_gc_barriers(generator, false, true)) {
        return false;
    }
    
    return result;
}

bool ffi_generate_concurrency_gc_barriers(FFIAssemblyGenerator *generator,
                             bool entering_unsafe, bool exiting_unsafe) {
    if (!generator) return false;
    
    if (entering_unsafe) {
        emit_comment(generator, "GC barrier (concurrency): entering unsafe code");
        // Notify GC that we're entering unsafe code
        // This might involve setting thread-local flags or calling runtime functions
    }
    
    if (exiting_unsafe) {
        emit_comment(generator, "GC barrier (concurrency): exiting unsafe code");
        // Notify GC that we're exiting unsafe code
        // This might involve clearing flags or triggering GC checks
    }
    
    return true;
}

// =============================================================================
// ADVANCED CONCURRENCY FUNCTIONS
// =============================================================================

bool ffi_generate_spawn_with_handle_statement(FFIAssemblyGenerator *generator, ASTNode *spawn_stmt) {
    if (!generator || !spawn_stmt || spawn_stmt->type != AST_SPAWN_WITH_HANDLE_STMT) {
        return false;
    }
    
    atomic_fetch_add(&generator->spawn_statements_generated, 1);
    
    emit_comment(generator, "Spawn with handle statement - Tier 1 concurrency");
    
    // Enhanced Tier 1 implementation: spawn_with_handle provides deterministic
    // task creation and result retrieval without complex coordination
    
    // Get function name and handle variable name from AST
    const char *function_name = spawn_stmt->data.spawn_with_handle_stmt.function_name;
    const char *handle_variable = spawn_stmt->data.spawn_with_handle_stmt.handle_var_name;
    
    if (!function_name || !handle_variable) {
        return false;
    }
    
    // Process arguments
    ASTNodeList *args = spawn_stmt->data.spawn_with_handle_stmt.args;
    size_t arg_count = args ? args->count : 0;
    
    emit_comment(generator, "Setting up function call parameters");
    
    // Function name/pointer in RDI (System V ABI) or RCX (Windows ABI)
    Register func_reg = (generator->base_generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) ? 
                       ASTHRA_REG_RDI : ASTHRA_REG_RCX;
    
    emit_instruction(generator, INST_LEA, 2,
                    create_register_operand(func_reg),
                    create_label_operand(function_name));
    
    // Arguments array in RSI/RDX (simplified for now)
    Register args_reg = (generator->base_generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) ? 
                       ASTHRA_REG_RSI : ASTHRA_REG_RDX;
    
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(args_reg),
                    create_immediate_operand(0)); // NULL for simplified implementation
    
    // Argument count in RDX/R8
    Register count_reg = (generator->base_generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) ? 
                        ASTHRA_REG_RDX : ASTHRA_REG_R8;
    
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(count_reg),
                    create_immediate_operand((int64_t)arg_count));
    
    // Call runtime spawn_with_handle function
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_spawn_task_with_handle"));
    
    // Store handle in variable (result is in RAX)
    emit_comment(generator, "Storing task handle in variable");
    
    // For now, we'll use a simplified variable storage
    // In a full implementation, this would involve proper symbol table lookup
    // Note: For Phase 5 simplified implementation, we store to a symbolic memory location
    emit_comment(generator, "Store handle to variable (simplified implementation)");
    
    // Create a symbolic memory operand - using RBP (frame pointer) with offset
    // This is a simplified approach; production code would use proper symbol resolution
    emit_instruction(generator, INST_MOV, 2,
                    create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, -16), // Store at [rbp-16]
                    create_register_operand(ASTHRA_REG_RAX));
    
    return true;
}

/**
 * Generate await expression for task synchronization
 */
bool ffi_generate_await_expression(FFIAssemblyGenerator *generator,
                                 ASTNode *await_expr, Register result_reg) {
    if (!generator || !await_expr || await_expr->type != AST_AWAIT_EXPR || result_reg == REGISTER_NONE) {
        return false;
    }
    
    emit_comment(generator, "Generating await expression");
    
    // Extract handle expression
    ASTNode *handle_expr = await_expr->data.await_expr.task_handle_expr;
    
    if (!handle_expr) {
        return false;
    }
    
    // Generate code to load the handle
    Register handle_reg = ASTHRA_REG_RBX; // Use RBX directly instead of allocation
    if (!ffi_generate_expression_impl(generator, handle_expr, handle_reg)) {
        return false;
    }
    
    // Optimization: Check if we can use direct register-to-register transfer
    if (handle_reg == result_reg) {
        emit_comment(generator, "Handle already in result register - no await needed (optimization)");
        return true;
    }
    
    // Call the runtime await function
    const char *await_function = "asthra_await_task";
    
    // Create a new concurrency context
    ConcurrencyContext *context = calloc(1, sizeof(ConcurrencyContext));
    if (!context) {
        return false;
    }
    
    context->operation = CONCURRENCY_OP_AWAIT;
    context->task_handle_reg = handle_reg;
    
    // Store the context
    generator->current_concurrency_op = context;
    
    // Emit function call to await runtime function
    emit_comment(generator, "Call runtime await function");
    
    // Save the handle register to the stack if needed
    if (generator->base_generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) {
        // Handle goes in RDI for System V ABI
        if (handle_reg != ASTHRA_REG_RDI) {
            emit_instruction(generator, INST_MOV, 2,
                create_register_operand(ASTHRA_REG_RDI),
                create_register_operand(handle_reg));
        }
    } else {
        // Handle goes in RCX for Windows ABI
        if (handle_reg != ASTHRA_REG_RCX) {
            emit_instruction(generator, INST_MOV, 2,
                create_register_operand(ASTHRA_REG_RCX),
                create_register_operand(handle_reg));
        }
    }
    
    // Call the await function
    emit_instruction(generator, INST_CALL, 1,
        create_label_operand(await_function));
    
    // Result is in RAX, move it to the result register if needed
    if (result_reg != ASTHRA_REG_RAX) {
        emit_instruction(generator, INST_MOV, 2,
            create_register_operand(result_reg),
            create_register_operand(ASTHRA_REG_RAX));
    }
    
    // Update statistics
    atomic_fetch_add(&generator->spawn_statements_generated, 1);
    
    // Clean up context
    free(context);
    generator->current_concurrency_op = NULL;
    
    return true;
}

/*
 * =============================================================================
 * TIER 3 CONCURRENCY FUNCTIONS (REMOVED - MOVED TO STDLIB)
 * =============================================================================
 * 
 * The following functions have been removed as part of Phase 5 of the 
 * Pragmatic Concurrency Balance Plan. These features are now available
 * through the standard library and require #[non_deterministic] annotation:
 * 
 * - ffi_generate_channel_declaration()    -> stdlib/concurrent/channels.asthra
 * - ffi_generate_send_statement()         -> stdlib/concurrent/channels.asthra  
 * - ffi_generate_recv_expression()        -> stdlib/concurrent/channels.asthra
 * - ffi_generate_select_statement()       -> stdlib/concurrent/coordination.asthra
 * - ffi_generate_worker_pool_declaration() -> stdlib/concurrent/patterns.asthra
 * - ffi_generate_close_statement()        -> stdlib/concurrent/channels.asthra
 * - ffi_generate_channel_send()           -> stdlib/concurrent/channels.asthra
 * - ffi_generate_channel_recv()           -> stdlib/concurrent/channels.asthra
 * 
 * These features are complex concurrency patterns that are better served by
 * mature standard library implementations using FFI to proven concurrency
 * libraries like Tokio, Go's runtime, or custom C implementations.
 * 
 * Tier 1 concurrency (spawn, spawn_with_handle, await) remains in the core
 * language as it provides deterministic, simple task creation and coordination
 * that is AI-friendly and predictable.
 */

// =============================================================================
// ENHANCED TIER 1 HELPER FUNCTIONS (PHASE 5 ADDITIONS)
// =============================================================================

/**
 * Enhanced helper function for task creation with handle (Phase 5 addition)
 * This provides optimized, cross-platform task creation for Tier 1 concurrency
 */
bool ffi_generate_task_creation_with_handle(FFIAssemblyGenerator *generator,
                                           const char *function_name, Register *arg_regs,
                                           size_t arg_count, Register handle_reg) {
    (void)arg_regs; // Mark as unused to suppress warning
    
    if (!generator || !function_name) return false;
    
    emit_comment(generator, "Enhanced task creation with handle - Tier 1 optimized");
    
    // Set up parameters for spawn_task_with_handle runtime function
    // This is an enhanced version that properly handles cross-platform calling conventions
    
    // Function name/pointer setup
    Register func_ptr_reg = (generator->base_generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) ? 
                           ASTHRA_REG_RDI : ASTHRA_REG_RCX;
    
    emit_instruction(generator, INST_LEA, 2,
                    create_register_operand(func_ptr_reg),
                    create_label_operand(function_name));
    
    // Arguments array setup (enhanced for future argument passing)
    Register args_array_reg = (generator->base_generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) ? 
                             ASTHRA_REG_RSI : ASTHRA_REG_RDX;
    
    if (arg_count > 0) {
        // For future implementation: properly pack arguments
        emit_comment(generator, "Argument packing (future enhancement)");
        emit_instruction(generator, INST_MOV, 2,
                        create_register_operand(args_array_reg),
                        create_immediate_operand(0)); // NULL for now, to be enhanced
    } else {
        emit_instruction(generator, INST_MOV, 2,
                        create_register_operand(args_array_reg),
                        create_immediate_operand(0)); // NULL for no arguments
    }
    
    // Argument count
    Register count_reg = (generator->base_generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64) ? 
                        ASTHRA_REG_RDX : ASTHRA_REG_R8;
    
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(count_reg),
                    create_immediate_operand((int64_t)arg_count));
    
    // Call enhanced runtime spawn_with_handle function
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand("asthra_spawn_task_with_handle_enhanced"));
    
    // Move task handle to target register (result is in RAX)
    if (handle_reg != ASTHRA_REG_RAX) {
        emit_instruction(generator, INST_MOV, 2,
                        create_register_operand(handle_reg),
                        create_register_operand(ASTHRA_REG_RAX));
    }
    
    return true;
}

/**
 * Enhanced annotation validation for Tier 2 concurrency features (Phase 5 addition)
 * This function validates that functions using non-deterministic concurrency
 * patterns have the required #[non_deterministic] annotation
 */
bool ffi_validate_concurrency_annotations(FFIAssemblyGenerator *generator, ASTNode *node) {
    if (!generator || !node) return false;
    
    // This function would be called when generating code for functions that
    // use stdlib concurrency features (Tier 2) to ensure they have the
    // #[non_deterministic] annotation
    
    emit_comment(generator, "Validating concurrency annotations for Tier 2 features");
    
    // For now, this is a placeholder that returns true
    // In a full implementation, this would:
    // 1. Check if the current function has #[non_deterministic] annotation
    // 2. If using Tier 2 features without annotation, emit compile error
    // 3. Generate appropriate runtime checks if needed
    
    return true;
}
