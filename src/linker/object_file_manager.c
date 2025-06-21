/**
 * Asthra Programming Language Compiler
 * Object File Manager Implementation - Multiple Compilation Unit Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "object_file_manager.h"
#include "../platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

/**
 * Set error message in object file manager
 */
static void object_manager_set_error(Asthra_ObjectFileManager *manager, const char *error) {
    if (!manager) return;
    
    free(manager->last_error);
    manager->last_error = error ? strdup(error) : NULL;
    manager->error_occurred = (error != NULL);
}

/**
 * Create a new object file structure
 */
static Asthra_ObjectFile *create_object_file(const char *file_path) {
    if (!file_path) return NULL;
    
    Asthra_ObjectFile *obj_file = malloc(sizeof(Asthra_ObjectFile));
    if (!obj_file) return NULL;
    
    memset(obj_file, 0, sizeof(Asthra_ObjectFile));
    
    obj_file->file_path = strdup(file_path);
    if (!obj_file->file_path) {
        free(obj_file);
        return NULL;
    }
    
    // Extract base name
    const char *base_start = strrchr(file_path, ASTHRA_PATH_SEPARATOR);
    base_start = base_start ? base_start + 1 : file_path;
    obj_file->base_name = strdup(base_start);
    
    // Get file stats
    struct stat file_stat;
    if (stat(file_path, &file_stat) == 0) {
        obj_file->file_size = file_stat.st_size;
        obj_file->modification_time = file_stat.st_mtime;
    }
    
    // Set default format based on platform
#if ASTHRA_PLATFORM_LINUX
    obj_file->format = ASTHRA_OBJ_FORMAT_ELF;
#elif ASTHRA_PLATFORM_WINDOWS
    obj_file->format = ASTHRA_OBJ_FORMAT_COFF;
#elif ASTHRA_PLATFORM_MACOS
    obj_file->format = ASTHRA_OBJ_FORMAT_MACH_O;
#else
    obj_file->format = ASTHRA_OBJ_FORMAT_UNKNOWN;
#endif
    
    obj_file->target_architecture = "x86_64";
    obj_file->is_64bit = true;
    
    return obj_file;
}

// =============================================================================
// OBJECT FILE MANAGER API IMPLEMENTATION
// =============================================================================

Asthra_ObjectFileManager *asthra_object_manager_create(void) {
    Asthra_ObjectFileManager *manager = malloc(sizeof(Asthra_ObjectFileManager));
    if (!manager) return NULL;
    
    memset(manager, 0, sizeof(Asthra_ObjectFileManager));
    
    // Set default cache configuration
    manager->cache_config.enable_caching = true;
    manager->cache_config.cache_size_limit = 100 * 1024 * 1024; // 100MB
    manager->cache_config.cache_expiry_seconds = 3600.0; // 1 hour
    
    return manager;
}

void asthra_object_manager_destroy(Asthra_ObjectFileManager *manager) {
    if (!manager) return;
    
    // Free all loaded object files
    Asthra_ObjectFile *current = manager->object_files;
    while (current) {
        Asthra_ObjectFile *next = current->next;
        asthra_object_file_free(current);
        current = next;
    }
    
    // Free search paths
    if (manager->search_paths) {
        for (size_t i = 0; i < manager->search_path_count; i++) {
            free(manager->search_paths[i]);
        }
        free(manager->search_paths);
    }
    
    // Free working directory
    free(manager->working_directory);
    free(manager->last_error);
    
    free(manager);
}

bool asthra_object_manager_add_search_path(Asthra_ObjectFileManager *manager,
                                           const char *search_path) {
    if (!manager || !search_path) return false;
    
    // Reallocate search paths array
    char **new_paths = realloc(manager->search_paths, 
                               (manager->search_path_count + 1) * sizeof(char*));
    if (!new_paths) {
        object_manager_set_error(manager, "Failed to allocate memory for search path");
        return false;
    }
    
    manager->search_paths = new_paths;
    manager->search_paths[manager->search_path_count] = strdup(search_path);
    
    if (!manager->search_paths[manager->search_path_count]) {
        object_manager_set_error(manager, "Failed to duplicate search path string");
        return false;
    }
    
    manager->search_path_count++;
    return true;
}

bool asthra_object_manager_set_working_directory(Asthra_ObjectFileManager *manager,
                                                 const char *working_dir) {
    if (!manager || !working_dir) return false;
    
    free(manager->working_directory);
    manager->working_directory = strdup(working_dir);
    
    if (!manager->working_directory) {
        object_manager_set_error(manager, "Failed to set working directory");
        return false;
    }
    
    return true;
}

// =============================================================================
// OBJECT FILE LOADING API IMPLEMENTATION
// =============================================================================

Asthra_ObjectFile *asthra_object_manager_load_file(Asthra_ObjectFileManager *manager,
                                                   const char *file_path) {
    if (!manager || !file_path) return NULL;
    
    // Check if file is already loaded
    if (asthra_object_manager_is_file_loaded(manager, file_path)) {
        manager->statistics.cache_hits++;
        return asthra_object_manager_find_file(manager, file_path);
    }
    
    manager->statistics.cache_misses++;
    
    // Check if file exists
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        object_manager_set_error(manager, "Object file not found");
        return NULL;
    }
    
    // Create new object file
    Asthra_ObjectFile *obj_file = create_object_file(file_path);
    if (!obj_file) {
        object_manager_set_error(manager, "Failed to create object file structure");
        return NULL;
    }
    
    // For now, just mark as loaded without actually parsing the object file
    // In a full implementation, this would parse the object file format
    obj_file->loaded = true;
    obj_file->parsed = false; // Would be true after full parsing
    obj_file->validated = false; // Would be true after validation
    
    // Add to manager's list
    obj_file->next = manager->object_files;
    manager->object_files = obj_file;
    manager->object_file_count++;
    
    // Update statistics
    manager->statistics.files_loaded++;
    
    return obj_file;
}

bool asthra_object_manager_load_files(Asthra_ObjectFileManager *manager,
                                      const char **file_paths,
                                      size_t file_count,
                                      size_t *loaded_count) {
    if (!manager || !file_paths || file_count == 0) return false;
    
    size_t successful_loads = 0;
    
    for (size_t i = 0; i < file_count; i++) {
        if (asthra_object_manager_load_file(manager, file_paths[i])) {
            successful_loads++;
        }
    }
    
    if (loaded_count) *loaded_count = successful_loads;
    
    return successful_loads == file_count;
}

bool asthra_object_manager_unload_file(Asthra_ObjectFileManager *manager,
                                       const char *file_path) {
    if (!manager || !file_path) return false;
    
    Asthra_ObjectFile **current = &manager->object_files;
    
    while (*current) {
        if (strcmp((*current)->file_path, file_path) == 0) {
            Asthra_ObjectFile *to_remove = *current;
            *current = (*current)->next;
            
            asthra_object_file_free(to_remove);
            manager->object_file_count--;
            return true;
        }
        current = &(*current)->next;
    }
    
    return false;
}

bool asthra_object_manager_is_file_loaded(Asthra_ObjectFileManager *manager,
                                          const char *file_path) {
    return asthra_object_manager_find_file(manager, file_path) != NULL;
}

// =============================================================================
// OBJECT FILE QUERY API IMPLEMENTATION
// =============================================================================

Asthra_ObjectFile *asthra_object_manager_find_file(Asthra_ObjectFileManager *manager,
                                                   const char *file_path) {
    if (!manager || !file_path) return NULL;
    
    Asthra_ObjectFile *current = manager->object_files;
    while (current) {
        if (strcmp(current->file_path, file_path) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

size_t asthra_object_manager_get_all_files(Asthra_ObjectFileManager *manager,
                                           Asthra_ObjectFile **files,
                                           size_t max_files) {
    if (!manager || !files) return 0;
    
    size_t count = 0;
    Asthra_ObjectFile *current = manager->object_files;
    
    while (current && count < max_files) {
        files[count] = current;
        count++;
        current = current->next;
    }
    
    return count;
}

Asthra_ObjectSymbol *asthra_object_manager_find_symbol(Asthra_ObjectFileManager *manager,
                                                       const char *symbol_name,
                                                       Asthra_ObjectFile **file_containing_symbol) {
    if (!manager || !symbol_name) return NULL;
    
    Asthra_ObjectFile *current = manager->object_files;
    while (current) {
        Asthra_ObjectSymbol *symbol = asthra_object_file_get_symbol(current, symbol_name);
        if (symbol) {
            if (file_containing_symbol) *file_containing_symbol = current;
            return symbol;
        }
        current = current->next;
    }
    
    return NULL;
}

size_t asthra_object_manager_get_undefined_symbols(Asthra_ObjectFileManager *manager,
                                                   char **undefined_symbols,
                                                   size_t max_symbols) {
    if (!manager || !undefined_symbols) return 0;
    
    // For stub implementation, return empty list
    return 0;
}

// =============================================================================
// OBJECT FILE VALIDATION API IMPLEMENTATION
// =============================================================================

bool asthra_object_manager_validate_file(Asthra_ObjectFileManager *manager,
                                         Asthra_ObjectFile *object_file) {
    if (!manager || !object_file) return false;
    
    // Basic validation: file exists and is loaded
    if (!object_file->loaded) {
        object_manager_set_error(manager, "Object file not loaded");
        return false;
    }
    
    // For stub implementation, assume all loaded files are valid
    object_file->validated = true;
    return true;
}

bool asthra_object_manager_check_compatibility(Asthra_ObjectFileManager *manager,
                                               Asthra_ObjectFile *file1,
                                               Asthra_ObjectFile *file2,
                                               char **error_message) {
    if (!manager || !file1 || !file2) return false;
    
    // Check architecture compatibility
    if (strcmp(file1->target_architecture, file2->target_architecture) != 0) {
        if (error_message) {
            *error_message = strdup("Architecture mismatch between object files");
        }
        return false;
    }
    
    // Check format compatibility
    if (file1->format != file2->format) {
        if (error_message) {
            *error_message = strdup("Object file format mismatch");
        }
        return false;
    }
    
    return true;
}

bool asthra_object_manager_validate_all_files(Asthra_ObjectFileManager *manager,
                                              size_t *error_count) {
    if (!manager) return false;
    
    size_t errors = 0;
    Asthra_ObjectFile *current = manager->object_files;
    
    while (current) {
        if (!asthra_object_manager_validate_file(manager, current)) {
            errors++;
        }
        current = current->next;
    }
    
    if (error_count) *error_count = errors;
    
    return errors == 0;
}

// =============================================================================
// UTILITY AND MANAGEMENT API IMPLEMENTATION
// =============================================================================

void asthra_object_manager_get_statistics(Asthra_ObjectFileManager *manager,
                                          size_t *files_loaded,
                                          double *cache_hit_rate,
                                          double *total_load_time_ms,
                                          size_t *symbols_processed) {
    if (!manager) return;
    
    if (files_loaded) *files_loaded = manager->statistics.files_loaded;
    if (total_load_time_ms) *total_load_time_ms = manager->statistics.total_load_time_ms;
    if (symbols_processed) *symbols_processed = manager->statistics.total_symbols_processed;
    
    if (cache_hit_rate) {
        size_t total_ops = manager->statistics.cache_hits + manager->statistics.cache_misses;
        *cache_hit_rate = total_ops > 0 ? (double)manager->statistics.cache_hits / (double)total_ops : 0.0;
    }
}

void asthra_object_manager_clear_all(Asthra_ObjectFileManager *manager) {
    if (!manager) return;
    
    // Free all loaded object files
    Asthra_ObjectFile *current = manager->object_files;
    while (current) {
        Asthra_ObjectFile *next = current->next;
        asthra_object_file_free(current);
        current = next;
    }
    
    manager->object_files = NULL;
    manager->object_file_count = 0;
    
    // Reset statistics
    memset(&manager->statistics, 0, sizeof(manager->statistics));
}

const char *asthra_object_manager_get_last_error(Asthra_ObjectFileManager *manager) {
    if (!manager) return "Invalid object file manager";
    return manager->last_error;
}

// =============================================================================
// OBJECT FILE UTILITIES IMPLEMENTATION
// =============================================================================

void asthra_object_file_free(Asthra_ObjectFile *object_file) {
    if (!object_file) return;
    
    // Free basic information
    free(object_file->file_path);
    free(object_file->base_name);
    free(object_file->load_error);
    
    // Free sections
    Asthra_ObjectSection *section = object_file->sections;
    while (section) {
        Asthra_ObjectSection *next_section = section->next;
        
        free(section->name);
        free(section->data);
        
        // Free relocations
        Asthra_ObjectRelocation *reloc = section->relocations;
        while (reloc) {
            Asthra_ObjectRelocation *next_reloc = reloc->next;
            free(reloc);
            reloc = next_reloc;
        }
        
        free(section);
        section = next_section;
    }
    
    // Free symbols
    Asthra_ObjectSymbol *symbol = object_file->symbols;
    while (symbol) {
        Asthra_ObjectSymbol *next_symbol = symbol->next;
        
        free(symbol->name);
        free(symbol);
        
        symbol = next_symbol;
    }
    
    // Free dependencies and undefined symbols
    if (object_file->dependencies) {
        for (size_t i = 0; i < object_file->dependency_count; i++) {
            free(object_file->dependencies[i]);
        }
        free(object_file->dependencies);
    }
    
    if (object_file->undefined_symbols) {
        for (size_t i = 0; i < object_file->undefined_symbol_count; i++) {
            free(object_file->undefined_symbols[i]);
        }
        free(object_file->undefined_symbols);
    }
    
    free(object_file);
}

Asthra_ObjectSection *asthra_object_file_get_section(Asthra_ObjectFile *object_file,
                                                     const char *section_name) {
    if (!object_file || !section_name) return NULL;
    
    Asthra_ObjectSection *section = object_file->sections;
    while (section) {
        if (section->name && strcmp(section->name, section_name) == 0) {
            return section;
        }
        section = section->next;
    }
    
    return NULL;
}

Asthra_ObjectSymbol *asthra_object_file_get_symbol(Asthra_ObjectFile *object_file,
                                                   const char *symbol_name) {
    if (!object_file || !symbol_name) return NULL;
    
    Asthra_ObjectSymbol *symbol = object_file->symbols;
    while (symbol) {
        if (symbol->name && strcmp(symbol->name, symbol_name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    
    return NULL;
} 
