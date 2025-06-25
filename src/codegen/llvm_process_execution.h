/**
 * Asthra Programming Language LLVM Process Execution
 * Header for external process execution functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_PROCESS_EXECUTION_H
#define ASTHRA_LLVM_PROCESS_EXECUTION_H

#include "llvm_tools.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PROCESS EXECUTION FUNCTIONS
// =============================================================================

/**
 * Execute external command and capture output
 * @param argv Command and arguments array (NULL-terminated)
 * @param capture_output Whether to capture stdout/stderr
 * @return Tool execution result with output and timing
 */
AsthraLLVMToolResult execute_command(const char **argv, bool capture_output);

/**
 * Find executable in PATH
 * @param name Executable name to find
 * @return Full path to executable or NULL if not found (caller must free)
 */
char *find_executable(const char *name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_PROCESS_EXECUTION_H