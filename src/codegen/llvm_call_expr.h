/**
 * Asthra Programming Language LLVM Function Calls
 * Function call expression code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_CALL_EXPR_H
#define ASTHRA_LLVM_CALL_EXPR_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function calls
LLVMValueRef generate_call_expr(LLVMBackendData *data, const ASTNode *node);

// Helper for Option.Some/None function calls
LLVMValueRef generate_option_function_call(LLVMBackendData *data, const ASTNode *node,
                                           const char *func_name);

// Helper for Result.Ok/Err function calls
LLVMValueRef generate_result_function_call(LLVMBackendData *data, const ASTNode *node,
                                           const char *func_name);

// Helper for len() predeclared function calls
LLVMValueRef generate_len_function_call(LLVMBackendData *data, const ASTNode *node);

// Helper for log() predeclared function calls
LLVMValueRef generate_log_function_call(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_CALL_EXPR_H