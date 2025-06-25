/**
 * Asthra Programming Language LLVM Pattern Matching
 * Header for pattern matching in match statements
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
 * Generate pattern check for match statement arms
 * @param data Backend data context
 * @param pattern Pattern AST node to check
 * @param value LLVM value to match against
 * @param match_block Basic block to jump to on match
 * @param next_block Basic block to jump to on no match
 * @return true if pattern check was generated successfully
 */
bool generate_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                           LLVMValueRef value, LLVMBasicBlockRef match_block,
                           LLVMBasicBlockRef next_block);

/**
 * Generate identifier pattern binding for matched values
 * @param data Backend data context
 * @param pattern Pattern AST node
 * @param value LLVM value to bind
 * @return true if binding was successful
 */
bool generate_identifier_pattern_binding(LLVMBackendData *data, const ASTNode *pattern,
                                        LLVMValueRef value);

#ifdef __cplusplus
}
#endif