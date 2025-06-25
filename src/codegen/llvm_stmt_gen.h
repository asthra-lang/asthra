/**
 * Asthra Programming Language LLVM Statement Generation
 * Statement code generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_STMT_GEN_H
#define ASTHRA_LLVM_STMT_GEN_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Main statement generation dispatcher
void generate_statement(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_STMT_GEN_H