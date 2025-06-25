/**
 * Asthra Programming Language LLVM Cast Expressions
 * Type casting code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_CAST_EXPR_H
#define ASTHRA_LLVM_CAST_EXPR_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Cast expressions
LLVMValueRef generate_cast_expr(LLVMBackendData *data, const ASTNode *node);

// Helper functions for specific cast types
LLVMValueRef cast_from_integer(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                              LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind);

LLVMValueRef cast_from_float(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                            LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind);

LLVMValueRef cast_from_pointer(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                              LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind);

LLVMValueRef cast_from_struct(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                             LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind);

LLVMValueRef cast_from_array(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                            LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_CAST_EXPR_H