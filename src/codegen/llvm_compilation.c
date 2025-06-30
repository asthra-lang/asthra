/**
 * Asthra Programming Language LLVM Compilation
 * Implementation of native code generation from LLVM IR
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_compilation.h"
#include "llvm_process_execution.h"
#include "llvm_tool_detection.h"
#include "llvm_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// NATIVE CODE GENERATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_compile(const char *input_file,
                                         const AsthraLLVMToolOptions *options) {
    const char *llc_path = asthra_llvm_tool_path("llc");
    if (!llc_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("llc tool not found in PATH");
        return result;
    }

    // Build command line arguments
    const char *argv[32];
    int argc = 0;

    argv[argc++] = llc_path;
    argv[argc++] = input_file;
    argv[argc++] = "-o";
    argv[argc++] = options->output_file;

    // Add optimization level
    argv[argc++] = asthra_llvm_opt_level_flag(options->opt_level);

    // Add file type based on output format
    if (options->output_format == ASTHRA_FORMAT_ASSEMBLY) {
        argv[argc++] = "-filetype=asm";
    } else if (options->output_format == ASTHRA_FORMAT_OBJECT) {
        argv[argc++] = "-filetype=obj";
    }

    // Add target triple if specified
    if (options->target_triple) {
        argv[argc++] = "-mtriple";
        argv[argc++] = options->target_triple;
    }

    // Add CPU type if specified
    if (options->cpu_type) {
        argv[argc++] = "-mcpu";
        argv[argc++] = options->cpu_type;
    }

    // Add CPU features if specified
    if (options->features) {
        argv[argc++] = "-mattr";
        argv[argc++] = options->features;
    }

    // Add relocation model based on PIE mode
    if (options->pie_mode == ASTHRA_PIE_FORCE_ENABLED ||
        (options->pie_mode == ASTHRA_PIE_DEFAULT &&
         options->output_format == ASTHRA_FORMAT_OBJECT)) {
        argv[argc++] = "-relocation-model=pic";
    } else if (options->pie_mode == ASTHRA_PIE_FORCE_DISABLED) {
        argv[argc++] = "-relocation-model=static";
    }
    // For ASTHRA_PIE_DEFAULT with non-object output, let llc use its default

    argv[argc] = NULL;

    return execute_command(argv, options->verbose);
}

