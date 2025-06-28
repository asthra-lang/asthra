/**
 * Asthra Programming Language LLVM Debug Info
 * Debug information generation for LLVM backend
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_DEBUG_H
#define ASTHRA_LLVM_DEBUG_H

#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize debug info support
void initialize_debug_info(LLVMBackendData *data, const char *filename, bool is_optimized);

// Cache commonly used debug types
void cache_basic_debug_types(LLVMBackendData *data);

// Set debug location for current instruction
void set_debug_location(LLVMBackendData *data, const ASTNode *node);

// Finalize debug info
void finalize_debug_info(LLVMBackendData *data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_DEBUG_H