/**
 * Asthra Programming Language LLVM Utilities
 * Header for utility functions for LLVM tools
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_UTILITIES_H
#define ASTHRA_LLVM_UTILITIES_H

#include "llvm_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Generate output filename based on input file and format
 * @param input_file Input file path
 * @param format Desired output format
 * @return Generated filename (caller must free) or NULL on error
 */
char *asthra_llvm_get_output_filename(const char *input_file, AsthraOutputFormat format);

/**
 * Get optimization level flag for LLVM tools
 * @param level Optimization level
 * @return Flag string (static, do not free)
 */
const char *asthra_llvm_opt_level_flag(AsthraOptimizationLevel level);

/**
 * Get target triple string for architecture
 * @param arch Target architecture
 * @return Target triple string (static, do not free) or NULL for native
 */
const char *asthra_llvm_target_triple(AsthraTargetArch arch);

/**
 * Free memory allocated by tool result
 * @param result Tool result to free
 */
void asthra_llvm_tool_result_free(AsthraLLVMToolResult *result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_UTILITIES_H