/**
 * Asthra Programming Language LLVM For Loop Iteration
 * Header for for loop code generation with different iterable types
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#pragma once

#include "llvm_backend_internal.h"
#include "../parser/ast_types.h"
#include <llvm-c/Core.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generate code for for loop iteration over different iterable types
 * Handles ranges, arrays, and slices
 * @param data Backend data context
 * @param node For statement AST node
 * @return true if loop generation was successful
 */
bool generate_for_loop_iteration(LLVMBackendData *data, const ASTNode *node);

#ifdef __cplusplus
}
#endif