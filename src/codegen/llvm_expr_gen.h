/**
 * Asthra Programming Language LLVM Expression Generation
 * Expression code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_EXPR_GEN_H
#define ASTHRA_LLVM_EXPR_GEN_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Main expression generation dispatcher
LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node);

// Binary operations
LLVMValueRef generate_binary_op(LLVMBackendData *data, const ASTNode *node);

// Unary operations
LLVMValueRef generate_unary_op(LLVMBackendData *data, const ASTNode *node);

// Identifier references (variables/functions)
LLVMValueRef generate_identifier(LLVMBackendData *data, const ASTNode *node);

// Function calls
LLVMValueRef generate_call_expr(LLVMBackendData *data, const ASTNode *node);

// Array/slice indexing
LLVMValueRef generate_index_expr(LLVMBackendData *data, const ASTNode *node);

// Field access
LLVMValueRef generate_field_access(LLVMBackendData *data, const ASTNode *node);

// Array literals
LLVMValueRef generate_array_literal(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_EXPR_GEN_H