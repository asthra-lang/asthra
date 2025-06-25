/**
 * Asthra Programming Language Compiler
 * Linker Internal Helper Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "linker_internal.h"
#include "../platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

/**
 * Get current time in milliseconds for performance measurement
 */
double linker_get_current_time_ms(void) {
    return asthra_get_elapsed_seconds(0, asthra_get_high_resolution_time()) * 1000.0;
}

/**
 * Set error message in linker
 */
void linker_set_error(Asthra_Linker *linker, const char *format, ...) {
    if (!linker)
        return;

    // Free existing error message
    free(linker->last_error);

    // Allocate new error message
    va_list args;
    va_start(args, format);

    // Calculate required buffer size
    int size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    if (size > 0) {
        linker->last_error = malloc(size + 1);
        if (linker->last_error) {
            va_start(args, format);
            vsnprintf(linker->last_error, size + 1, format, args);
            va_end(args);
        }
    }

    linker->error_occurred = true;
}

// TODO: Platform-specific executable name generation will be implemented
// when advanced linker features are added

/**
 * Create default executable metadata
 */
void init_executable_metadata(Asthra_ExecutableMetadata *metadata) {
    if (!metadata)
        return;

    memset(metadata, 0, sizeof(Asthra_ExecutableMetadata));

    metadata->target_platform = ASTHRA_PLATFORM_NAME;
    metadata->target_architecture = "x86_64";

#if ASTHRA_PLATFORM_LINUX
    metadata->executable_format = "ELF";
#elif ASTHRA_PLATFORM_WINDOWS
    metadata->executable_format = "PE";
#elif ASTHRA_PLATFORM_MACOS
    metadata->executable_format = "Mach-O";
#else
    metadata->executable_format = "Unknown";
#endif

    metadata->entry_point = "main";
    metadata->base_address = 0x400000; // Standard Linux base address
    metadata->file_permissions = 0755; // rwxr-xr-x
}