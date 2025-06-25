/**
 * Asthra Programming Language LLVM Linking
 * Header for linking and executable generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_LINKING_H
#define ASTHRA_LLVM_LINKING_H

#include "llvm_tools.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// LINKING FUNCTIONS
// =============================================================================

/**
 * Link object files into executable using clang
 * @param object_files Array of object file paths
 * @param num_objects Number of object files
 * @param options Linking options
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_link(const char **object_files, size_t num_objects,
                                      const AsthraLLVMToolOptions *options);

/**
 * Create executable from input file with libraries
 * @param input_file Path to input file (object or IR)
 * @param output_file Path to output executable
 * @param libraries Array of library names (without -l prefix)
 * @param num_libraries Number of libraries
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_to_executable(const char *input_file, const char *output_file,
                                               const char **libraries, size_t num_libraries);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_LINKING_H