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

// Include specialized expression generators
#include "llvm_access_expr.h"
#include "llvm_binary_ops.h"
#include "llvm_call_expr.h"
#include "llvm_cast_expr.h"
#include "llvm_literals.h"
#include "llvm_unary_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

// Main expression generation dispatcher
LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node);

// Identifier references (variables/functions)
LLVMValueRef generate_identifier(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_EXPR_GEN_H