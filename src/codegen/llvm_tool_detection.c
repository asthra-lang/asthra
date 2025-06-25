/**
 * Asthra Programming Language LLVM Tool Detection
 * Implementation of LLVM tool discovery and validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_tool_detection.h"
#include "llvm_process_execution.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// LLVM TOOL DETECTION AND VALIDATION
// =============================================================================

bool asthra_llvm_tools_available(void) {
    const char *required_tools[] = {"llc", "opt", "clang", NULL};

    for (int i = 0; required_tools[i]; i++) {
        char *path = find_executable(required_tools[i]);
        if (!path) {
            return false;
        }
        free(path);
    }

    return true;
}

const char *asthra_llvm_tool_path(const char *tool_name) {
    static char path_buffer[4096];
    char *path = find_executable(tool_name);

    if (path) {
        strncpy(path_buffer, path, sizeof(path_buffer) - 1);
        path_buffer[sizeof(path_buffer) - 1] = '\0';
        free(path);
        return path_buffer;
    }

    return NULL;
}

const char *asthra_llvm_version(void) {
    static char version_buffer[256];
    const char *llc_path = asthra_llvm_tool_path("llc");
    if (!llc_path)
        return NULL;

    const char *argv[] = {llc_path, "--version", NULL};
    AsthraLLVMToolResult result = execute_command(argv, true);

    if (result.success && result.stdout_output) {
        // Parse version from output
        char *version_line = strstr(result.stdout_output, "LLVM version");
        if (version_line) {
            sscanf(version_line, "LLVM version %255s", version_buffer);
            asthra_llvm_tool_result_free(&result);
            return version_buffer;
        }
    }

    asthra_llvm_tool_result_free(&result);
    return NULL;
}