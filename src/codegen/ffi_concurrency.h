/**
 * Asthra Programming Language Compiler
 * FFI Concurrency Operation Generation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_CONCURRENCY_H
#define ASTHRA_FFI_CONCURRENCY_H

#include "ffi_types.h"
#include "ffi_contexts.h"
#include "../parser/ast.h"
#include "code_generator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct FFIAssemblyGenerator FFIAssemblyGenerator;

// =============================================================================
// CONCURRENCY GENERATION
// =============================================================================

/**
 * Generate spawn statement
 */
bool ffi_generate_spawn_statement(FFIAssemblyGenerator *generator, ASTNode *spawn_stmt);

/**
 * Generate spawn with handle statement
 */
bool ffi_generate_spawn_with_handle_statement(FFIAssemblyGenerator *generator, ASTNode *spawn_stmt);

/**
 * Generate await expression
 */
bool ffi_generate_await_expression(FFIAssemblyGenerator *generator, ASTNode *await_expr, Register result_reg);

/**
 * Generate task creation and scheduling
 */
bool ffi_generate_task_creation(FFIAssemblyGenerator *generator,
                               char *function_name, Register *arg_regs,
                               size_t arg_count, Register handle_reg);

/**
 * Generate channel declaration
 */
bool ffi_generate_channel_declaration(FFIAssemblyGenerator *generator, ASTNode *channel_decl);

/**
 * Generate send statement
 */
bool ffi_generate_send_statement(FFIAssemblyGenerator *generator, ASTNode *send_stmt);

/**
 * Generate recv expression
 */
bool ffi_generate_recv_expression(FFIAssemblyGenerator *generator, ASTNode *recv_expr, Register result_reg);

/**
 * Generate select statement
 */
bool ffi_generate_select_statement(FFIAssemblyGenerator *generator, ASTNode *select_stmt);

/**
 * Generate worker pool declaration
 */
bool ffi_generate_worker_pool_declaration(FFIAssemblyGenerator *generator, ASTNode *worker_pool_decl);

/**
 * Generate close statement
 */
bool ffi_generate_close_statement(FFIAssemblyGenerator *generator, ASTNode *close_stmt);

/**
 * Generate channel send operation
 */
bool ffi_generate_channel_send(FFIAssemblyGenerator *generator, Register channel_reg, Register value_reg);

/**
 * Generate channel recv operation
 */
bool ffi_generate_channel_recv(FFIAssemblyGenerator *generator, Register channel_reg, Register result_reg, ASTNode *timeout_expr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_CONCURRENCY_H 
