/**
 * Asthra Programming Language Compiler
 * Platform Utilities - Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PLATFORM_UTILS_H
#define ASTHRA_PLATFORM_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PLATFORM UTILITIES
// =============================================================================

/**
 * Build output filename from input filename
 * Returns allocated string that must be freed by caller
 * If output_file is provided, returns a copy of it
 * Otherwise generates default filename based on input_file
 */
char* platform_build_output_filename(const char* input_file, const char* output_file);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PLATFORM_UTILS_H 
