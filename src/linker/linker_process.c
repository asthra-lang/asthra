/**
 * Asthra Programming Language Compiler
 * Linker Process Implementation - File Linking and Symbol Resolution
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_linker.h"
#include "linker_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool asthra_linker_link_files(Asthra_Linker *linker, const char **object_files, size_t file_count,
                              const char *output_executable, Asthra_LinkingResult *result) {
    if (!linker || !object_files || file_count == 0 || !output_executable || !result) {
        if (linker)
            linker_set_error(linker, "Invalid parameters for link_files");
        return false;
    }

    if (!linker->initialized) {
        linker_set_error(linker, "Linker not properly initialized");
        return false;
    }

    double start_time = linker_get_current_time_ms();

    // Initialize result
    memset(result, 0, sizeof(Asthra_LinkingResult));
    init_executable_metadata(&result->metadata);

    // Step 1: Load all object files
    size_t loaded_count;
    if (!asthra_object_manager_load_files(linker->object_manager, object_files, file_count,
                                          &loaded_count)) {
        linker_set_error(linker, "Failed to load object files: %zu/%zu loaded", loaded_count,
                         file_count);
        return false;
    }

    if (loaded_count != file_count) {
        linker_set_error(linker, "Not all object files loaded successfully: %zu/%zu", loaded_count,
                         file_count);
        return false;
    }

    // Step 2: Add symbols to resolver
    Asthra_ObjectFile *files[file_count];
    size_t actual_file_count =
        asthra_object_manager_get_all_files(linker->object_manager, files, file_count);

    for (size_t i = 0; i < actual_file_count; i++) {
        size_t symbols_added =
            asthra_symbol_resolver_add_object_file(linker->symbol_resolver, files[i]);
        result->total_symbols_processed += symbols_added;
    }

    // Step 3: Resolve all symbols
    Asthra_ResolutionResult resolution_result;
    if (!asthra_symbol_resolver_resolve_all(linker->symbol_resolver, &resolution_result)) {
        linker_set_error(linker, "Symbol resolution failed: %zu undefined symbols",
                         resolution_result.undefined_count);
        result->symbols_unresolved = resolution_result.undefined_count;
        asthra_resolution_result_cleanup(&resolution_result);
        return false;
    }

    result->symbols_resolved = resolution_result.resolved_symbols;
    result->symbols_unresolved = resolution_result.undefined_symbols;

    // Step 4: Generate executable (platform-specific)
    if (!asthra_linker_generate_executable(linker, object_files, file_count, output_executable,
                                           &result->metadata)) {
        linker_set_error(linker, "Failed to generate executable");
        asthra_resolution_result_cleanup(&resolution_result);
        return false;
    }

    // Step 5: Set executable permissions
    if (!asthra_linker_set_executable_permissions(linker, output_executable,
                                                  result->metadata.file_permissions)) {
        linker_set_error(linker, "Failed to set executable permissions");
        asthra_resolution_result_cleanup(&resolution_result);
        return false;
    }

    // Step 6: Validate generated executable
    if (!asthra_linker_validate_executable(linker, output_executable, &result->metadata)) {
        linker_set_error(linker, "Generated executable failed validation");
        asthra_resolution_result_cleanup(&resolution_result);
        return false;
    }

    // Success - populate final result
    result->success = true;
    result->executable_path = strdup(output_executable);
    result->linking_time_ms = linker_get_current_time_ms() - start_time;

    // Update linker statistics
    linker->statistics.total_objects_linked += file_count;

    asthra_resolution_result_cleanup(&resolution_result);
    return true;
}

bool asthra_linker_link_single_file(Asthra_Linker *linker, const char *object_file,
                                    const char *output_executable) {
    if (!linker || !object_file || !output_executable)
        return false;

    const char *files[] = {object_file};
    Asthra_LinkingResult result;

    bool success = asthra_linker_link_files(linker, files, 1, output_executable, &result);

    asthra_linking_result_cleanup(&result);
    return success;
}