/**
 * Asthra Programming Language LLVM Utilities
 * Implementation of utility functions for LLVM tools
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

char *asthra_llvm_get_output_filename(const char *input_file, AsthraOutputFormat format) {
    if (!input_file)
        return NULL;

    // Find the last dot to get extension
    const char *last_dot = strrchr(input_file, '.');
    size_t base_len = last_dot ? (size_t)(last_dot - input_file) : strlen(input_file);

    const char *extension;
    switch (format) {
    case ASTHRA_FORMAT_LLVM_IR:
        extension = ".ll";
        break;
    case ASTHRA_FORMAT_LLVM_BC:
        extension = ".bc";
        break;
    case ASTHRA_FORMAT_ASSEMBLY:
        extension = ".s";
        break;
    case ASTHRA_FORMAT_OBJECT:
        extension = ".o";
        break;
    case ASTHRA_FORMAT_EXECUTABLE:
        extension = ""; // No extension for executables
        break;
    default:
        extension = ".out";
    }

    size_t output_len = base_len + strlen(extension) + 1;
    char *output_file = malloc(output_len);
    if (output_file) {
        strncpy(output_file, input_file, base_len);
        output_file[base_len] = '\0';
        strcat(output_file, extension);
    }

    return output_file;
}

const char *asthra_llvm_opt_level_flag(AsthraOptimizationLevel level) {
    switch (level) {
    case ASTHRA_OPT_NONE:
        return "-O0";
    case ASTHRA_OPT_BASIC:
        return "-O1";
    case ASTHRA_OPT_STANDARD:
        return "-O2";
    case ASTHRA_OPT_AGGRESSIVE:
        return "-O3";
    default:
        return "-O2";
    }
}

const char *asthra_llvm_target_triple(AsthraTargetArch arch) {
    switch (arch) {
    case ASTHRA_TARGET_X86_64:
#ifdef __APPLE__
        return "x86_64-apple-darwin";
#elif defined(__linux__)
        return "x86_64-pc-linux-gnu";
#else
        return "x86_64-unknown-unknown";
#endif

    case ASTHRA_TARGET_ARM64:
#ifdef __APPLE__
        return "arm64-apple-darwin";
#elif defined(__linux__)
        return "aarch64-unknown-linux-gnu";
#else
        return "aarch64-unknown-unknown";
#endif

    case ASTHRA_TARGET_WASM32:
        return "wasm32-unknown-unknown";

    case ASTHRA_TARGET_NATIVE:
    default:
        return NULL; // Let LLVM detect the host triple
    }
}

void asthra_llvm_tool_result_free(AsthraLLVMToolResult *result) {
    if (!result)
        return;

    if (result->stdout_output) {
        free(result->stdout_output);
        result->stdout_output = NULL;
    }

    if (result->stderr_output) {
        free(result->stderr_output);
        result->stderr_output = NULL;
    }
}