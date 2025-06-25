/**
 * Asthra Programming Language Compiler
 * Linker Internal Header - Shared Functions and Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LINKER_INTERNAL_H
#define ASTHRA_LINKER_INTERNAL_H

#include "asthra_linker.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

/**
 * Get current time in milliseconds for performance measurement
 */
double linker_get_current_time_ms(void);

/**
 * Set error message in linker
 */
void linker_set_error(Asthra_Linker *linker, const char *format, ...);

/**
 * Create default executable metadata
 */
void init_executable_metadata(Asthra_ExecutableMetadata *metadata);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LINKER_INTERNAL_H