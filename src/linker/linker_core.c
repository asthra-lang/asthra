/**
 * Asthra Programming Language Compiler
 * Linker Core Implementation - Main API and Lifecycle Management
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

// =============================================================================
// MAIN LINKER API IMPLEMENTATION
// =============================================================================

Asthra_Linker *asthra_linker_create(void) {
    Asthra_Linker *linker = malloc(sizeof(Asthra_Linker));
    if (!linker)
        return NULL;

    // Initialize structure
    memset(linker, 0, sizeof(Asthra_Linker));

    // Create component managers
    linker->object_manager = asthra_object_manager_create();
    if (!linker->object_manager) {
        free(linker);
        return NULL;
    }

    linker->symbol_resolver =
        asthra_symbol_resolver_create(1024); // Start with 1024 symbol table size
    if (!linker->symbol_resolver) {
        asthra_object_manager_destroy(linker->object_manager);
        free(linker);
        return NULL;
    }

    // Initialize default configuration
    asthra_linking_request_init(&linker->current_request);

    // Set default symbol resolution configuration
    asthra_symbol_resolver_configure(linker->symbol_resolver, false, true, true);

    linker->initialized = true;
    return linker;
}

void asthra_linker_destroy(Asthra_Linker *linker) {
    if (!linker)
        return;

    // Destroy component managers
    if (linker->object_manager) {
        asthra_object_manager_destroy(linker->object_manager);
    }

    if (linker->symbol_resolver) {
        asthra_symbol_resolver_destroy(linker->symbol_resolver);
    }

    // Free allocated resources
    if (linker->allocated_paths) {
        for (size_t i = 0; i < linker->allocated_path_count; i++) {
            free(linker->allocated_paths[i]);
        }
        free(linker->allocated_paths);
    }

    if (linker->allocated_buffers) {
        for (size_t i = 0; i < linker->allocated_buffer_count; i++) {
            free(linker->allocated_buffers[i]);
        }
        free(linker->allocated_buffers);
    }

    // Free error message
    free(linker->last_error);

    // Clean up linking request
    asthra_linking_request_cleanup(&linker->current_request);

    free(linker);
}

bool asthra_linker_configure(Asthra_Linker *linker, const Asthra_LinkingRequest *request) {
    if (!linker || !request)
        return false;

    if (!linker->initialized) {
        linker_set_error(linker, "Linker not properly initialized");
        return false;
    }

    // Clean up previous request
    asthra_linking_request_cleanup(&linker->current_request);

    // Copy new request (deep copy)
    memcpy(&linker->current_request, request, sizeof(Asthra_LinkingRequest));

    // Configure symbol resolver based on request settings
    asthra_symbol_resolver_configure(linker->symbol_resolver, request->allow_undefined_symbols,
                                     !request->weak_symbol_resolution,
                                     true); // Always case sensitive

    return true;
}

bool asthra_linker_execute(Asthra_Linker *linker, Asthra_LinkingResult *result) {
    if (!linker || !result)
        return false;

    if (!linker->initialized) {
        linker_set_error(linker, "Linker not properly initialized");
        return false;
    }

    // Initialize result structure
    memset(result, 0, sizeof(Asthra_LinkingResult));

    double start_time = linker_get_current_time_ms();
    linker->linking_in_progress = true;

    // Main linking process
    bool success = asthra_linker_link_files(linker, linker->current_request.object_files,
                                            linker->current_request.object_file_count,
                                            linker->current_request.output_executable_path, result);

    double end_time = linker_get_current_time_ms();

    // Update timing information
    result->linking_time_ms = end_time - start_time;
    linker->linking_in_progress = false;

    // Update statistics
    linker->statistics.total_linking_time_ms += result->linking_time_ms;
    if (success) {
        linker->statistics.total_executables_generated++;
    }

    return success;
}