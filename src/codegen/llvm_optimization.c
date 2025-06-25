/**
 * Asthra Programming Language LLVM Optimization
 * Implementation of LLVM IR optimization functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_optimization.h"
#include "llvm_process_execution.h"
#include "llvm_tool_detection.h"
#include "llvm_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// LLVM IR OPTIMIZATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_optimize(const char *input_file,
                                          const AsthraLLVMToolOptions *options) {
    const char *opt_path = asthra_llvm_tool_path("opt");
    if (!opt_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("opt tool not found in PATH");
        return result;
    }

    // Build command line arguments
    const char *argv[32];
    int argc = 0;

    argv[argc++] = opt_path;
    argv[argc++] = input_file;
    argv[argc++] = "-o";
    argv[argc++] = options->output_file;

    // Add optimization level
    argv[argc++] = asthra_llvm_opt_level_flag(options->opt_level);

    // Add debug info preservation
    if (options->debug_info) {
        argv[argc++] = "-debugify";
    }

    // Add custom pass pipeline if specified
    if (options->pass_pipeline) {
        argv[argc++] = "-passes";
        argv[argc++] = options->pass_pipeline;
    }

    // Add S flag to emit text IR if requested
    if (options->emit_llvm_after_opt) {
        argv[argc++] = "-S";
    }

    argv[argc] = NULL;

    return execute_command(argv, options->verbose);
}

AsthraLLVMToolResult asthra_llvm_run_passes(const char *input_file, const char *passes,
                                            const char *output_file) {
    const char *opt_path = asthra_llvm_tool_path("opt");
    if (!opt_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("opt tool not found in PATH");
        return result;
    }

    const char *argv[] = {opt_path, input_file, "-o", output_file, "-passes", passes,
                          "-S", // Emit text IR
                          NULL};

    return execute_command(argv, false);
}