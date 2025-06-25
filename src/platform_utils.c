/**
 * Asthra Programming Language Compiler
 * Platform Utilities - Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "platform_utils.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// PLATFORM UTILITIES IMPLEMENTATION
// =============================================================================

char *platform_build_output_filename(const char *input_file, const char *output_file) {
    if (output_file) {
        return asthra_strdup(output_file);
    }

    // Generate default output filename
    const char *base_name = input_file;

    // Find the last path separator
    const char *last_sep = strrchr(input_file, asthra_get_path_separator());
    if (last_sep) {
        base_name = last_sep + 1;
    }

    // Find the last dot for extension
    const char *last_dot = strrchr(base_name, '.');
    size_t base_len = last_dot ? (size_t)(last_dot - base_name) : strlen(base_name);

    // Build output filename with platform-specific extension
    const char *exe_ext = asthra_get_exe_extension();
    size_t total_len = base_len + strlen(exe_ext) + 1;
    char *output = malloc(total_len);

    if (output) {
        strncpy(output, base_name, base_len);
        output[base_len] = '\0';
        strcat(output, exe_ext);
    }

    return output;
}
