/**
 * Asthra Programming Language LLVM Tool Detection
 * Header for LLVM tool discovery and validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_TOOL_DETECTION_H
#define ASTHRA_LLVM_TOOL_DETECTION_H

#include "llvm_tools.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TOOL DETECTION FUNCTIONS
// =============================================================================

/**
 * Check if required LLVM tools are available
 * @return true if all required tools (llc, opt, clang) are found in PATH
 */
bool asthra_llvm_tools_available(void);

/**
 * Get path to a specific LLVM tool
 * @param tool_name Name of the tool to find
 * @return Path to tool or NULL if not found (static buffer, do not free)
 */
const char *asthra_llvm_tool_path(const char *tool_name);

/**
 * Get LLVM version string
 * @return LLVM version or NULL if not available (static buffer, do not free)
 */
const char *asthra_llvm_version(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_TOOL_DETECTION_H