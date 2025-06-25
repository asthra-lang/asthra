/**
 * Asthra Programming Language LLVM Basic Statements
 * Header for basic statement code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#pragma once

#include "llvm_backend_internal.h"
#include "../parser/ast_types.h"
#include <llvm-c/Core.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generate code for return statements
 * @param data Backend data context
 * @param node Return statement AST node
 */
void generate_return_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for variable declarations (let statements)
 * @param data Backend data context
 * @param node Let statement AST node
 */
void generate_let_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for assignment statements
 * @param data Backend data context
 * @param node Assignment AST node
 */
void generate_assignment_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for if statements
 * @param data Backend data context
 * @param node If statement AST node
 */
void generate_if_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for expression statements
 * @param data Backend data context
 * @param node Expression statement AST node
 */
void generate_expression_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for break statements
 * @param data Backend data context
 * @param node Break statement AST node
 */
void generate_break_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for continue statements
 * @param data Backend data context
 * @param node Continue statement AST node
 */
void generate_continue_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for match statements
 * @param data Backend data context
 * @param node Match statement AST node
 */
void generate_match_statement(LLVMBackendData *data, const ASTNode *node);

/**
 * Generate code for block statements
 * @param data Backend data context
 * @param node Block AST node
 */
void generate_block_statement(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif