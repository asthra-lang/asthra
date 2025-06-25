/**
 * Asthra Programming Language LLVM Linking
 * Implementation of linking and executable generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_linking.h"
#include "llvm_process_execution.h"
#include "llvm_tool_detection.h"
#include "llvm_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// LINKING AND EXECUTABLE GENERATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_link(const char **object_files, size_t num_objects,
                                      const AsthraLLVMToolOptions *options) {
    const char *clang_path = asthra_llvm_tool_path("clang");
    if (!clang_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("clang tool not found in PATH");
        return result;
    }

    // Build command line arguments
    const char **argv = malloc((num_objects + 16) * sizeof(char *));
    int argc = 0;

    argv[argc++] = clang_path;

    // Add object files
    for (size_t i = 0; i < num_objects; i++) {
        argv[argc++] = object_files[i];
    }

    argv[argc++] = "-o";
    argv[argc++] = options->output_file;

    // Add optimization level
    argv[argc++] = asthra_llvm_opt_level_flag(options->opt_level);

    // Add target triple if specified
    if (options->target_triple) {
        argv[argc++] = "-target";
        argv[argc++] = options->target_triple;
    }

    // Add coverage flags if enabled
    if (options->coverage) {
        argv[argc++] = "-fprofile-instr-generate";
        argv[argc++] = "-fcoverage-mapping";
    }

    argv[argc] = NULL;

    AsthraLLVMToolResult result = execute_command(argv, options->verbose);
    free(argv);

    return result;
}

AsthraLLVMToolResult asthra_llvm_to_executable(const char *input_file, const char *output_file,
                                               const char **libraries, size_t num_libraries) {
    const char *clang_path = asthra_llvm_tool_path("clang");
    if (!clang_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("clang tool not found in PATH");
        return result;
    }

    // Build command line arguments
    const char **argv = malloc((num_libraries + 16) * sizeof(char *));
    int argc = 0;

    argv[argc++] = clang_path;
    argv[argc++] = input_file;
    argv[argc++] = "-o";
    argv[argc++] = output_file;

    // Add libraries
    for (size_t i = 0; i < num_libraries; i++) {
        argv[argc++] = "-l";
        argv[argc++] = libraries[i];
    }

    argv[argc] = NULL;

    AsthraLLVMToolResult result = execute_command(argv, false);
    free(argv);

    return result;
}