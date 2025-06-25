/**
 * Asthra Programming Language LLVM Unary Operations
 * Unary operation code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_UNARY_OPS_H
#define ASTHRA_LLVM_UNARY_OPS_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Unary operations
LLVMValueRef generate_unary_op(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_UNARY_OPS_H