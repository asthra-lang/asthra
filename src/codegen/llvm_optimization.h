/**
 * Asthra Programming Language LLVM Optimization
 * Header for LLVM IR optimization functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_OPTIMIZATION_H
#define ASTHRA_LLVM_OPTIMIZATION_H

#include "llvm_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// OPTIMIZATION FUNCTIONS
// =============================================================================

/**
 * Optimize LLVM IR using opt tool
 * @param input_file Path to input LLVM IR file
 * @param options Optimization options
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_optimize(const char *input_file,
                                          const AsthraLLVMToolOptions *options);

/**
 * Run specific optimization passes on LLVM IR
 * @param input_file Path to input LLVM IR file
 * @param passes Pass pipeline specification
 * @param output_file Path to output file
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_run_passes(const char *input_file, const char *passes,
                                            const char *output_file);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_OPTIMIZATION_H