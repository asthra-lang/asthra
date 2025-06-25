/**
 * Asthra Programming Language Compiler
 * Linker Utilities - Management API and Request/Result Handling
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_linker.h"
#include "linker_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// UTILITY AND MANAGEMENT API
// =============================================================================

void asthra_linker_get_statistics(Asthra_Linker *linker, size_t *objects_linked,
                                  size_t *executables_generated, double *total_time_ms,
                                  double *cache_hit_rate) {
    if (!linker)
        return;

    if (objects_linked)
        *objects_linked = linker->statistics.total_objects_linked;
    if (executables_generated)
        *executables_generated = linker->statistics.total_executables_generated;
    if (total_time_ms)
        *total_time_ms = linker->statistics.total_linking_time_ms;

    if (cache_hit_rate) {
        size_t total_ops = linker->statistics.cache_hits + linker->statistics.cache_misses;
        *cache_hit_rate =
            total_ops > 0 ? (double)linker->statistics.cache_hits / (double)total_ops : 0.0;
    }
}

void asthra_linker_clear_cache(Asthra_Linker *linker) {
    if (!linker)
        return;

    // Clear object manager cache
    asthra_object_manager_clear_all(linker->object_manager);

    // Clear symbol resolver cache
    asthra_symbol_resolver_clear_all(linker->symbol_resolver);

    // Reset statistics
    linker->statistics.cache_hits = 0;
    linker->statistics.cache_misses = 0;
}

const char *asthra_linker_get_last_error(Asthra_Linker *linker) {
    if (!linker)
        return "Invalid linker instance";
    return linker->last_error;
}

bool asthra_linker_is_ready(Asthra_Linker *linker) {
    if (!linker)
        return false;
    return linker->initialized && !linker->error_occurred && !linker->linking_in_progress;
}

// =============================================================================
// LINKING REQUEST UTILITIES
// =============================================================================

void asthra_linking_request_init(Asthra_LinkingRequest *request) {
    if (!request)
        return;

    memset(request, 0, sizeof(Asthra_LinkingRequest));

    // Set defaults
    request->generate_debug_info = false;
    request->strip_symbols = false;
    request->optimize_size = false;
    request->static_linking = false;
    request->allow_undefined_symbols = false;
    request->weak_symbol_resolution = false;
    request->entry_point_symbol = "main";
    request->parallel_linking = false;
    request->max_worker_threads = 1;
    request->timeout_seconds = 300.0; // 5 minutes default timeout
}

void asthra_linking_request_cleanup(Asthra_LinkingRequest *request) {
    if (!request)
        return;

    // Note: This function assumes the request owns no dynamically allocated memory
    // If we add dynamic allocation to requests, this function should free those resources
    memset(request, 0, sizeof(Asthra_LinkingRequest));
}

void asthra_linking_result_cleanup(Asthra_LinkingResult *result) {
    if (!result)
        return;

    // Free dynamically allocated strings
    free(result->executable_path);
    free(result->error_message);
    free(result->linker_command_used);

    // Free warning messages array
    if (result->warning_messages) {
        for (size_t i = 0; i < result->warning_count; i++) {
            free(result->warning_messages[i]);
        }
        free(result->warning_messages);
    }

    // Free intermediate files array
    if (result->intermediate_files) {
        for (size_t i = 0; i < result->intermediate_file_count; i++) {
            free(result->intermediate_files[i]);
        }
        free(result->intermediate_files);
    }

    memset(result, 0, sizeof(Asthra_LinkingResult));
}