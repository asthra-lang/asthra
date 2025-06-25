/**
 * Asthra Programming Language LLVM Pipeline
 * Header for integrated compilation pipeline
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_PIPELINE_H
#define ASTHRA_LLVM_PIPELINE_H

#include "llvm_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PIPELINE FUNCTIONS
// =============================================================================

/**
 * Execute complete compilation pipeline from IR to target format
 * Handles optimization, compilation, and linking as needed
 * @param ir_file Path to input LLVM IR file
 * @param output_file Path to output file
 * @param format Target output format
 * @param options Compiler options
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_compile_pipeline(const char *ir_file, const char *output_file,
                                                  AsthraOutputFormat format,
                                                  const AsthraCompilerOptions *options);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_PIPELINE_H