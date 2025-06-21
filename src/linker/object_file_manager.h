/**
 * Asthra Programming Language Compiler
 * Object File Manager - Multiple Compilation Unit Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides object file management capabilities for handling multiple
 * compilation units, loading object file metadata, and preparing them for linking.
 */

#ifndef ASTHRA_OBJECT_FILE_MANAGER_H
#define ASTHRA_OBJECT_FILE_MANAGER_H

#include "../platform.h"
#include "../compiler.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct Asthra_ObjectFileManager Asthra_ObjectFileManager;
typedef struct Asthra_ObjectFile Asthra_ObjectFile;
typedef struct Asthra_ObjectSection Asthra_ObjectSection;
typedef struct Asthra_ObjectSymbol Asthra_ObjectSymbol;
typedef struct Asthra_ObjectRelocation Asthra_ObjectRelocation;

// =============================================================================
// OBJECT FILE STRUCTURES
// =============================================================================

/**
 * Symbol information from object file
 */
struct Asthra_ObjectSymbol {
    char *name;
    uint64_t address;
    size_t size;
    enum {
        ASTHRA_SYMBOL_LOCAL,
        ASTHRA_SYMBOL_GLOBAL,
        ASTHRA_SYMBOL_WEAK,
        ASTHRA_SYMBOL_UNDEFINED
    } binding;
    enum {
        ASTHRA_SYMBOL_FUNCTION,
        ASTHRA_SYMBOL_VARIABLE,
        ASTHRA_SYMBOL_SECTION,
        ASTHRA_SYMBOL_FILE,
        ASTHRA_SYMBOL_UNKNOWN
    } type;
    const char *section_name;
    bool is_defined;
    struct Asthra_ObjectSymbol *next;
};

/**
 * Relocation information from object file
 */
struct Asthra_ObjectRelocation {
    uint64_t offset;
    const char *symbol_name;
    enum {
        ASTHRA_RELOC_ABSOLUTE,
        ASTHRA_RELOC_RELATIVE,
        ASTHRA_RELOC_PLT,
        ASTHRA_RELOC_GOT
    } type;
    int64_t addend;
    struct Asthra_ObjectRelocation *next;
};

/**
 * Section information from object file
 */
struct Asthra_ObjectSection {
    char *name;
    uint64_t address;
    size_t size;
    uint8_t *data;
    enum {
        ASTHRA_SECTION_TEXT,
        ASTHRA_SECTION_DATA,
        ASTHRA_SECTION_BSS,
        ASTHRA_SECTION_RODATA,
        ASTHRA_SECTION_DEBUG,
        ASTHRA_SECTION_SYMBOL_TABLE,
        ASTHRA_SECTION_STRING_TABLE,
        ASTHRA_SECTION_RELOCATION,
        ASTHRA_SECTION_UNKNOWN
    } type;
    uint32_t flags;
    size_t alignment;
    
    // Relocations for this section
    Asthra_ObjectRelocation *relocations;
    size_t relocation_count;
    
    struct Asthra_ObjectSection *next;
};

/**
 * Complete object file representation
 */
struct Asthra_ObjectFile {
    // File information
    char *file_path;
    char *base_name;
    size_t file_size;
    time_t modification_time;
    
    // Object file metadata
    enum {
        ASTHRA_OBJ_FORMAT_ELF,
        ASTHRA_OBJ_FORMAT_COFF,
        ASTHRA_OBJ_FORMAT_MACH_O,
        ASTHRA_OBJ_FORMAT_UNKNOWN
    } format;
    const char *target_architecture;
    bool is_64bit;
    bool has_debug_info;
    
    // Sections and symbols
    Asthra_ObjectSection *sections;
    size_t section_count;
    Asthra_ObjectSymbol *symbols;
    size_t symbol_count;
    
    // Cross-references
    char **dependencies;
    size_t dependency_count;
    char **undefined_symbols;
    size_t undefined_symbol_count;
    
    // Loading state
    bool loaded;
    bool parsed;
    bool validated;
    char *load_error;
    
    struct Asthra_ObjectFile *next;
};

/**
 * Object file manager main structure
 */
struct Asthra_ObjectFileManager {
    // Managed object files
    Asthra_ObjectFile *object_files;
    size_t object_file_count;
    
    // Working directory and paths
    char *working_directory;
    char **search_paths;
    size_t search_path_count;
    
    // Caching and performance
    struct {
        bool enable_caching;
        size_t cache_size_limit;
        double cache_expiry_seconds;
    } cache_config;
    
    // Statistics
    struct {
        size_t files_loaded;
        size_t cache_hits;
        size_t cache_misses;
        double total_load_time_ms;
        size_t total_symbols_processed;
    } statistics;
    
    // Error handling
    char *last_error;
    bool error_occurred;
};

// =============================================================================
// OBJECT FILE MANAGER API
// =============================================================================

/**
 * Create a new object file manager
 * 
 * @return New object file manager, or NULL on failure
 */
Asthra_ObjectFileManager *asthra_object_manager_create(void);

/**
 * Destroy object file manager and free all resources
 * 
 * @param manager Object file manager to destroy
 */
void asthra_object_manager_destroy(Asthra_ObjectFileManager *manager);

/**
 * Add search path for object files
 * 
 * @param manager Object file manager
 * @param search_path Directory path to search for object files
 * @return true on success, false on failure
 */
bool asthra_object_manager_add_search_path(Asthra_ObjectFileManager *manager,
                                           const char *search_path);

/**
 * Set working directory for relative paths
 * 
 * @param manager Object file manager
 * @param working_dir Working directory path
 * @return true on success, false on failure
 */
bool asthra_object_manager_set_working_directory(Asthra_ObjectFileManager *manager,
                                                 const char *working_dir);

// =============================================================================
// OBJECT FILE LOADING API
// =============================================================================

/**
 * Load object file from disk
 * 
 * @param manager Object file manager
 * @param file_path Path to object file
 * @return Loaded object file, or NULL on failure
 */
Asthra_ObjectFile *asthra_object_manager_load_file(Asthra_ObjectFileManager *manager,
                                                   const char *file_path);

/**
 * Load multiple object files
 * 
 * @param manager Object file manager
 * @param file_paths Array of object file paths
 * @param file_count Number of files to load
 * @param loaded_count Output: number of successfully loaded files
 * @return true if all files loaded successfully, false otherwise
 */
bool asthra_object_manager_load_files(Asthra_ObjectFileManager *manager,
                                      const char **file_paths,
                                      size_t file_count,
                                      size_t *loaded_count);

/**
 * Unload object file and free its resources
 * 
 * @param manager Object file manager
 * @param file_path Path of file to unload
 * @return true on success, false on failure
 */
bool asthra_object_manager_unload_file(Asthra_ObjectFileManager *manager,
                                       const char *file_path);

/**
 * Check if object file is already loaded
 * 
 * @param manager Object file manager
 * @param file_path Path to check
 * @return true if loaded, false otherwise
 */
bool asthra_object_manager_is_file_loaded(Asthra_ObjectFileManager *manager,
                                          const char *file_path);

// =============================================================================
// OBJECT FILE QUERY API
// =============================================================================

/**
 * Find object file by path
 * 
 * @param manager Object file manager
 * @param file_path Path to find
 * @return Object file if found, NULL otherwise
 */
Asthra_ObjectFile *asthra_object_manager_find_file(Asthra_ObjectFileManager *manager,
                                                   const char *file_path);

/**
 * Get all loaded object files
 * 
 * @param manager Object file manager
 * @param files Output array of object files
 * @param max_files Maximum number of files to return
 * @return Number of files returned
 */
size_t asthra_object_manager_get_all_files(Asthra_ObjectFileManager *manager,
                                           Asthra_ObjectFile **files,
                                           size_t max_files);

/**
 * Find symbol across all loaded object files
 * 
 * @param manager Object file manager
 * @param symbol_name Symbol name to find
 * @param file_containing_symbol Output: file containing the symbol
 * @return Symbol if found, NULL otherwise
 */
Asthra_ObjectSymbol *asthra_object_manager_find_symbol(Asthra_ObjectFileManager *manager,
                                                       const char *symbol_name,
                                                       Asthra_ObjectFile **file_containing_symbol);

/**
 * Get all undefined symbols across loaded files
 * 
 * @param manager Object file manager
 * @param undefined_symbols Output array of undefined symbol names
 * @param max_symbols Maximum number of symbols to return
 * @return Number of undefined symbols found
 */
size_t asthra_object_manager_get_undefined_symbols(Asthra_ObjectFileManager *manager,
                                                   char **undefined_symbols,
                                                   size_t max_symbols);

// =============================================================================
// OBJECT FILE VALIDATION API
// =============================================================================

/**
 * Validate object file format and structure
 * 
 * @param manager Object file manager
 * @param object_file Object file to validate
 * @return true if valid, false on validation failure
 */
bool asthra_object_manager_validate_file(Asthra_ObjectFileManager *manager,
                                         Asthra_ObjectFile *object_file);

/**
 * Check compatibility between object files
 * 
 * @param manager Object file manager
 * @param file1 First object file
 * @param file2 Second object file
 * @param error_message Output: compatibility error description
 * @return true if compatible, false otherwise
 */
bool asthra_object_manager_check_compatibility(Asthra_ObjectFileManager *manager,
                                               Asthra_ObjectFile *file1,
                                               Asthra_ObjectFile *file2,
                                               char **error_message);

/**
 * Validate all loaded object files for consistency
 * 
 * @param manager Object file manager
 * @param error_count Output: number of validation errors
 * @return true if all files are valid, false otherwise
 */
bool asthra_object_manager_validate_all_files(Asthra_ObjectFileManager *manager,
                                              size_t *error_count);

// =============================================================================
// UTILITY AND MANAGEMENT API
// =============================================================================

/**
 * Get object file manager statistics
 * 
 * @param manager Object file manager
 * @param files_loaded Output: total files loaded
 * @param cache_hit_rate Output: cache hit rate
 * @param total_load_time_ms Output: total loading time
 * @param symbols_processed Output: total symbols processed
 */
void asthra_object_manager_get_statistics(Asthra_ObjectFileManager *manager,
                                          size_t *files_loaded,
                                          double *cache_hit_rate,
                                          double *total_load_time_ms,
                                          size_t *symbols_processed);

/**
 * Clear all loaded files and reset manager state
 * 
 * @param manager Object file manager
 */
void asthra_object_manager_clear_all(Asthra_ObjectFileManager *manager);

/**
 * Get last error message from object file manager
 * 
 * @param manager Object file manager
 * @return Error message string, or NULL if no error
 */
const char *asthra_object_manager_get_last_error(Asthra_ObjectFileManager *manager);

// =============================================================================
// OBJECT FILE UTILITIES
// =============================================================================

/**
 * Free object file structure and all associated resources
 * 
 * @param object_file Object file to free
 */
void asthra_object_file_free(Asthra_ObjectFile *object_file);

/**
 * Get section by name from object file
 * 
 * @param object_file Object file to search
 * @param section_name Name of section to find
 * @return Section if found, NULL otherwise
 */
Asthra_ObjectSection *asthra_object_file_get_section(Asthra_ObjectFile *object_file,
                                                     const char *section_name);

/**
 * Get symbol by name from object file
 * 
 * @param object_file Object file to search
 * @param symbol_name Name of symbol to find
 * @return Symbol if found, NULL otherwise
 */
Asthra_ObjectSymbol *asthra_object_file_get_symbol(Asthra_ObjectFile *object_file,
                                                   const char *symbol_name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OBJECT_FILE_MANAGER_H 
