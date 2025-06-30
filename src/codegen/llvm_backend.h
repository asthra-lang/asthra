/**
 * Asthra Programming Language LLVM Backend
 * Direct LLVM code generation interface
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_BACKEND_H
#define ASTHRA_LLVM_BACKEND_H

#include "../compiler.h"
#include "../parser/ast_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generate LLVM IR code directly from AST
 * @param ctx Compiler context with options
 * @param ast Root AST node
 * @param output_file Output file path
 * @return 0 on success, negative on error
 */
int asthra_generate_llvm_code(AsthraCompilerContext *ctx, const ASTNode *ast, const char *output_file);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_BACKEND_H