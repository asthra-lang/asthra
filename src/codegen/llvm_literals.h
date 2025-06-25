/**
 * Asthra Programming Language LLVM Literals
 * Literal value code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_LITERALS_H
#define ASTHRA_LLVM_LITERALS_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Literal value generators
LLVMValueRef generate_integer_literal(LLVMBackendData *data, const ASTNode *node);
LLVMValueRef generate_float_literal(LLVMBackendData *data, const ASTNode *node);
LLVMValueRef generate_string_literal(LLVMBackendData *data, const ASTNode *node);
LLVMValueRef generate_bool_literal(LLVMBackendData *data, const ASTNode *node);
LLVMValueRef generate_char_literal(LLVMBackendData *data, const ASTNode *node);
LLVMValueRef generate_unit_literal(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_LITERALS_H