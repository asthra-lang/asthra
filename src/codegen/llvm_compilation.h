/**
 * Asthra Programming Language LLVM Compilation
 * Header for native code generation from LLVM IR
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_COMPILATION_H
#define ASTHRA_LLVM_COMPILATION_H

#include "llvm_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// COMPILATION FUNCTIONS
// =============================================================================

/**
 * Compile LLVM IR to native code using llc
 * @param input_file Path to input LLVM IR file
 * @param options Compilation options
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_compile(const char *input_file,
                                         const AsthraLLVMToolOptions *options);


#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_COMPILATION_H