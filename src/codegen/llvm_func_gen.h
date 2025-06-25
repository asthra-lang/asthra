/**
 * Asthra Programming Language LLVM Function Generation
 * Function and top-level code generation for LLVM backend
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_FUNC_GEN_H
#define ASTHRA_LLVM_FUNC_GEN_H

#include "llvm_backend_internal.h"
#include "../parser/ast_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Generate code for function definitions
void generate_function(LLVMBackendData *data, const ASTNode *node);

// Generate code for top-level declarations
void generate_top_level(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_FUNC_GEN_H