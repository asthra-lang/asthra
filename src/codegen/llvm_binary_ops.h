/**
 * Asthra Programming Language LLVM Binary Operations
 * Binary operation code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_BINARY_OPS_H
#define ASTHRA_LLVM_BINARY_OPS_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Binary operations
LLVMValueRef generate_binary_op(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_BINARY_OPS_H