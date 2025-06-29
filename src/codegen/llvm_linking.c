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
#include <unistd.h>

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

    // Link with Asthra runtime library
    // Try to find the runtime library in multiple locations
    const char *lib_paths[] = {"./lib",           // From build directory (CI runs from here)
                               "../lib",          // From subdirectory of build
                               "./build/lib",     // From project root
                               "../build/lib",    // From bdd directory
                               "../../build/lib", // From deep subdirectory
                               NULL};

    // Find the first existing library path
    const char *lib_path = NULL;
    for (int i = 0; lib_paths[i] != NULL; i++) {
        char test_path[1024];
        snprintf(test_path, sizeof(test_path), "%s/libasthra_runtime.a", lib_paths[i]);
        if (access(test_path, F_OK) == 0) {
            lib_path = lib_paths[i];
            break;
        }
    }

    if (lib_path) {
        argv[argc++] = "-L";
        argv[argc++] = lib_path;
        argv[argc++] = "-lasthra_runtime";
        
        if (options->verbose) {
            fprintf(stderr, "Found Asthra runtime library at: %s\n", lib_path);
        }
    } else {
        // Still try to link with the runtime library even if we can't find it
        // The system might have it in a standard location
        argv[argc++] = "-lasthra_runtime";
        
        if (options->verbose) {
            fprintf(stderr, "Warning: Could not find Asthra runtime library in expected locations\n");
        }
    }

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