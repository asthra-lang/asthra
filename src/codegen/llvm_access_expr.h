/**
 * Asthra Programming Language LLVM Access Expressions
 * Field access, indexing, and array literal code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_ACCESS_EXPR_H
#define ASTHRA_LLVM_ACCESS_EXPR_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Array/slice indexing
LLVMValueRef generate_index_expr(LLVMBackendData *data, const ASTNode *node);

// Field access
LLVMValueRef generate_field_access(LLVMBackendData *data, const ASTNode *node);

// Array literals
LLVMValueRef generate_array_literal(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_ACCESS_EXPR_H