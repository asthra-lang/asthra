/**
 * Asthra Programming Language Compiler
 * Main Linker Module - Object File Combination and Executable Generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides the main linking interface for combining multiple object files
 * into single executable binaries, supporting cross-file symbol resolution and
 * runtime library integration as required by the pipeline orchestrator.
 */

#ifndef ASTHRA_LINKER_H
#define ASTHRA_LINKER_H

#include "../compiler.h"
#include "../platform.h"
#include "object_file_manager.h"
#include "symbol_resolution.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct Asthra_Linker Asthra_Linker;
typedef struct Asthra_LinkingRequest Asthra_LinkingRequest;
typedef struct Asthra_LinkingResult Asthra_LinkingResult;
typedef struct Asthra_ExecutableMetadata Asthra_ExecutableMetadata;

// =============================================================================
// LINKING REQUEST CONFIGURATION
// =============================================================================

/**
 * Configuration for a linking request
 */
struct Asthra_LinkingRequest {
    // Input files
    const char **object_files;
    size_t object_file_count;
    const char **library_paths;
    size_t library_path_count;
    const char **runtime_libraries;
    size_t runtime_library_count;

    // Output configuration
    const char *output_executable_path;
    const char *intermediate_directory;

    // Platform-specific settings
    bool generate_debug_info;
    bool strip_symbols;
    bool optimize_size;
    bool static_linking;

    // Symbol resolution settings
    bool allow_undefined_symbols;
    bool weak_symbol_resolution;
    const char *entry_point_symbol;

    // Performance settings
    bool parallel_linking;
    size_t max_worker_threads;
    double timeout_seconds;
};

/**
 * Executable metadata for generated binaries
 */
struct Asthra_ExecutableMetadata {
    const char *target_platform;
    const char *target_architecture;
    const char *executable_format; // "ELF", "PE", "Mach-O"
    size_t executable_size;
    size_t symbol_count;
    size_t section_count;
    bool has_debug_info;
    bool is_stripped;
    const char *entry_point;
    uint64_t base_address;
    uint32_t file_permissions;
};

/**
 * Comprehensive linking result
 */
struct Asthra_LinkingResult {
    bool success;

    // Output information
    char *executable_path;
    Asthra_ExecutableMetadata metadata;

    // Performance metrics
    double linking_time_ms;
    size_t total_symbols_processed;
    size_t symbols_resolved;
    size_t symbols_unresolved;

    // Error reporting
    char *error_message;
    char **warning_messages;
    size_t warning_count;

    // Debug information
    char *linker_command_used;
    char **intermediate_files;
    size_t intermediate_file_count;
};

/**
 * Main linker structure
 */
struct Asthra_Linker {
    // Configuration
    Asthra_LinkingRequest current_request;

    // Component managers
    Asthra_ObjectFileManager *object_manager;
    Asthra_SymbolResolver *symbol_resolver;

    // State management
    bool initialized;
    bool linking_in_progress;

    // Resource tracking
    char **allocated_paths;
    size_t allocated_path_count;
    void **allocated_buffers;
    size_t allocated_buffer_count;

    // Statistics
    struct {
        size_t total_objects_linked;
        size_t total_executables_generated;
        double total_linking_time_ms;
        size_t cache_hits;
        size_t cache_misses;
    } statistics;

    // Error handling
    char *last_error;
    bool error_occurred;
};

// =============================================================================
// MAIN LINKER API
// =============================================================================

/**
 * Create a new Asthra linker instance
 *
 * @return New linker instance, or NULL on failure
 */
Asthra_Linker *asthra_linker_create(void);

/**
 * Destroy a linker instance and free all resources
 *
 * @param linker Linker instance to destroy
 */
void asthra_linker_destroy(Asthra_Linker *linker);

/**
 * Configure linking request parameters
 *
 * @param linker Linker instance
 * @param request Linking configuration
 * @return true on success, false on failure
 */
bool asthra_linker_configure(Asthra_Linker *linker, const Asthra_LinkingRequest *request);

/**
 * Execute the main linking process
 *
 * @param linker Configured linker instance
 * @param result Output linking result
 * @return true on success, false on failure
 */
bool asthra_linker_execute(Asthra_Linker *linker, Asthra_LinkingResult *result);

/**
 * Link multiple object files into a single executable
 *
 * @param linker Linker instance
 * @param object_files Array of object file paths
 * @param file_count Number of object files
 * @param output_executable Path for output executable
 * @param result Detailed linking result
 * @return true on success, false on failure
 */
bool asthra_linker_link_files(Asthra_Linker *linker, const char **object_files, size_t file_count,
                              const char *output_executable, Asthra_LinkingResult *result);

/**
 * Quick single-file linking for simple cases
 *
 * @param linker Linker instance
 * @param object_file Single object file path
 * @param output_executable Output executable path
 * @return true on success, false on failure
 */
bool asthra_linker_link_single_file(Asthra_Linker *linker, const char *object_file,
                                    const char *output_executable);

// =============================================================================
// EXECUTABLE GENERATION API
// =============================================================================

/**
 * Generate platform-specific executable from object files
 *
 * @param linker Linker instance
 * @param object_files Object file paths
 * @param file_count Number of files
 * @param output_path Output executable path
 * @param metadata Executable metadata output
 * @return true on success, false on failure
 */
bool asthra_linker_generate_executable(Asthra_Linker *linker, const char **object_files,
                                       size_t file_count, const char *output_path,
                                       Asthra_ExecutableMetadata *metadata);

/**
 * Set executable permissions and metadata
 *
 * @param linker Linker instance
 * @param executable_path Path to executable
 * @param permissions Platform-specific permissions
 * @return true on success, false on failure
 */
bool asthra_linker_set_executable_permissions(Asthra_Linker *linker, const char *executable_path,
                                              uint32_t permissions);

/**
 * Validate generated executable format and structure
 *
 * @param linker Linker instance
 * @param executable_path Path to executable
 * @param metadata Validation metadata output
 * @return true if valid, false on validation failure
 */
bool asthra_linker_validate_executable(Asthra_Linker *linker, const char *executable_path,
                                       Asthra_ExecutableMetadata *metadata);

// =============================================================================
// RUNTIME LIBRARY INTEGRATION
// =============================================================================

/**
 * Add runtime library to linking request
 *
 * @param linker Linker instance
 * @param library_path Path to runtime library
 * @param required Whether library is required for linking
 * @return true on success, false on failure
 */
bool asthra_linker_add_runtime_library(Asthra_Linker *linker, const char *library_path,
                                       bool required);

/**
 * Automatically link with Asthra runtime libraries
 *
 * @param linker Linker instance
 * @param runtime_path Path to Asthra runtime directory
 * @return true on success, false on failure
 */
bool asthra_linker_link_asthra_runtime(Asthra_Linker *linker, const char *runtime_path);

// =============================================================================
// UTILITY AND MANAGEMENT API
// =============================================================================

/**
 * Get linker statistics and performance metrics
 *
 * @param linker Linker instance
 * @param objects_linked Output: total objects linked
 * @param executables_generated Output: total executables generated
 * @param total_time_ms Output: total linking time
 * @param cache_hit_rate Output: symbol cache hit rate
 */
void asthra_linker_get_statistics(Asthra_Linker *linker, size_t *objects_linked,
                                  size_t *executables_generated, double *total_time_ms,
                                  double *cache_hit_rate);

/**
 * Clear all cached data and reset linker state
 *
 * @param linker Linker instance
 */
void asthra_linker_clear_cache(Asthra_Linker *linker);

/**
 * Get last error message from linker
 *
 * @param linker Linker instance
 * @return Error message string, or NULL if no error
 */
const char *asthra_linker_get_last_error(Asthra_Linker *linker);

/**
 * Check if linker is ready for operation
 *
 * @param linker Linker instance
 * @return true if ready, false if not initialized or in error state
 */
bool asthra_linker_is_ready(Asthra_Linker *linker);

// =============================================================================
// LINKING REQUEST UTILITIES
// =============================================================================

/**
 * Initialize a linking request with default values
 *
 * @param request Request structure to initialize
 */
void asthra_linking_request_init(Asthra_LinkingRequest *request);

/**
 * Free resources associated with a linking request
 *
 * @param request Request structure to clean up
 */
void asthra_linking_request_cleanup(Asthra_LinkingRequest *request);

/**
 * Free resources associated with a linking result
 *
 * @param result Result structure to clean up
 */
void asthra_linking_result_cleanup(Asthra_LinkingResult *result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LINKER_H
