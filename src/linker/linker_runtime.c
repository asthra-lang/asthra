/**
 * Asthra Programming Language Compiler
 * Linker Runtime Integration - Runtime Library Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../platform.h"
#include "asthra_linker.h"
#include "linker_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// =============================================================================
// RUNTIME LIBRARY INTEGRATION
// =============================================================================

bool asthra_linker_add_runtime_library(Asthra_Linker *linker, const char *library_path,
                                       bool required) {
    if (!linker || !library_path)
        return false;

    // For now, just validate that the library exists
    struct stat lib_stat;
    if (stat(library_path, &lib_stat) != 0) {
        if (required) {
            linker_set_error(linker, "Required runtime library not found: %s", library_path);
            return false;
        }
        // Optional library not found - continue
        return true;
    }

    return true;
}

bool asthra_linker_link_asthra_runtime(Asthra_Linker *linker, const char *runtime_path) {
    if (!linker || !runtime_path)
        return false;

    // Link with standard Asthra runtime libraries
    char runtime_lib[512];

    // Core runtime library
    snprintf(runtime_lib, sizeof(runtime_lib), "%s/libasthra_runtime" ASTHRA_LIB_EXT, runtime_path);
    if (!asthra_linker_add_runtime_library(linker, runtime_lib, true)) {
        return false;
    }

    // Concurrency runtime
    snprintf(runtime_lib, sizeof(runtime_lib), "%s/libasthra_concurrency" ASTHRA_LIB_EXT,
             runtime_path);
    asthra_linker_add_runtime_library(linker, runtime_lib, false); // Optional

    return true;
}