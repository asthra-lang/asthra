/**
 * Asthra Programming Language Compiler
 * Symbol Resolution - Cross-File Symbol Linking
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides symbol resolution capabilities for linking symbols
 * across multiple object files and resolving undefined references.
 */

#ifndef ASTHRA_SYMBOL_RESOLUTION_H
#define ASTHRA_SYMBOL_RESOLUTION_H

#include "../platform.h"
#include "../compiler.h"
#include "object_file_manager.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct Asthra_SymbolResolver Asthra_SymbolResolver;
typedef struct Asthra_SymbolEntry Asthra_SymbolEntry;
typedef struct Asthra_SymbolReference Asthra_SymbolReference;
typedef struct Asthra_ResolutionResult Asthra_ResolutionResult;
typedef struct Asthra_SymbolConflict Asthra_SymbolConflict;

// =============================================================================
// SYMBOL RESOLUTION STRUCTURES
// =============================================================================

/**
 * Symbol entry in the global symbol table
 */
struct Asthra_SymbolEntry {
    char *name;
    uint64_t address;
    size_t size;
    
    // Symbol properties
    enum {
        ASTHRA_RESOLVE_UNDEFINED,
        ASTHRA_RESOLVE_DEFINED,
        ASTHRA_RESOLVE_WEAK,
        ASTHRA_RESOLVE_COMMON
    } status;
    
    enum {
        ASTHRA_RESOLVE_FUNC,
        ASTHRA_RESOLVE_VAR,
        ASTHRA_RESOLVE_SECTION,
        ASTHRA_RESOLVE_UNKNOWN
    } type;
    
    // Source information
    char *defining_file;
    char *section_name;
    uint64_t file_offset;
    
    // Resolution state
    bool resolved;
    bool exported;
    bool imported;
    
    // References to this symbol
    Asthra_SymbolReference *references;
    size_t reference_count;
    
    struct Asthra_SymbolEntry *next;
};

/**
 * Reference to a symbol from another location
 */
struct Asthra_SymbolReference {
    char *referencing_file;
    char *section_name;
    uint64_t offset;
    enum {
        ASTHRA_REF_ABSOLUTE,
        ASTHRA_REF_RELATIVE,
        ASTHRA_REF_PLT,
        ASTHRA_REF_GOT
    } type;
    int64_t addend;
    bool resolved;
    
    struct Asthra_SymbolReference *next;
};

/**
 * Symbol conflict information
 */
struct Asthra_SymbolConflict {
    char *symbol_name;
    const char *file1;
    const char *file2;
    enum {
        ASTHRA_CONFLICT_MULTIPLE_DEFINITIONS,
        ASTHRA_CONFLICT_TYPE_MISMATCH,
        ASTHRA_CONFLICT_SIZE_MISMATCH,
        ASTHRA_CONFLICT_WEAK_STRONG
    } type;
    char *description;
    
    struct Asthra_SymbolConflict *next;
};

/**
 * Symbol resolution result
 */
struct Asthra_ResolutionResult {
    bool success;
    
    // Resolution statistics
    size_t total_symbols;
    size_t resolved_symbols;
    size_t undefined_symbols;
    size_t weak_symbols;
    
    // Error information
    Asthra_SymbolConflict *conflicts;
    size_t conflict_count;
    char **undefined_symbol_names;
    size_t undefined_count;
    
    // Performance metrics
    double resolution_time_ms;
    size_t hash_collisions;
    size_t cache_hits;
    
    // Warning information
    char **warnings;
    size_t warning_count;
};

/**
 * Main symbol resolver structure
 */
struct Asthra_SymbolResolver {
    // Global symbol table
    Asthra_SymbolEntry **symbol_table;
    size_t table_size;
    size_t symbol_count;
    
    // Symbol resolution state
    bool resolution_complete;
    Asthra_ResolutionResult last_result;
    
    // Configuration
    struct {
        bool allow_undefined_symbols;
        bool prefer_strong_over_weak;
        bool ignore_size_mismatches;
        bool case_sensitive_symbols;
        size_t max_resolution_iterations;
    } config;
    
    // Hash table performance
    struct {
        size_t hash_collisions;
        size_t max_chain_length;
        double load_factor;
        size_t resize_count;
    } hash_stats;
    
    // Statistics
    struct {
        size_t total_resolutions;
        size_t successful_resolutions;
        double total_resolution_time_ms;
        size_t symbols_processed;
    } statistics;
    
    // Error handling
    char *last_error;
    bool error_occurred;
};

// =============================================================================
// SYMBOL RESOLVER API
// =============================================================================

/**
 * Create a new symbol resolver
 * 
 * @param initial_table_size Initial size of symbol hash table
 * @return New symbol resolver, or NULL on failure
 */
Asthra_SymbolResolver *asthra_symbol_resolver_create(size_t initial_table_size);

/**
 * Destroy symbol resolver and free all resources
 * 
 * @param resolver Symbol resolver to destroy
 */
void asthra_symbol_resolver_destroy(Asthra_SymbolResolver *resolver);

/**
 * Configure symbol resolution behavior
 * 
 * @param resolver Symbol resolver
 * @param allow_undefined Whether to allow undefined symbols
 * @param prefer_strong Whether to prefer strong symbols over weak
 * @param case_sensitive Whether symbol names are case sensitive
 * @return true on success, false on failure
 */
bool asthra_symbol_resolver_configure(Asthra_SymbolResolver *resolver,
                                      bool allow_undefined,
                                      bool prefer_strong,
                                      bool case_sensitive);

// =============================================================================
// SYMBOL REGISTRATION API
// =============================================================================

/**
 * Add symbol from object file to global symbol table
 * 
 * @param resolver Symbol resolver
 * @param symbol Symbol from object file
 * @param source_file File containing the symbol
 * @return true on success, false on failure
 */
bool asthra_symbol_resolver_add_symbol(Asthra_SymbolResolver *resolver,
                                       const Asthra_ObjectSymbol *symbol,
                                       const char *source_file);

/**
 * Add all symbols from an object file
 * 
 * @param resolver Symbol resolver
 * @param object_file Object file to process
 * @return Number of symbols successfully added
 */
size_t asthra_symbol_resolver_add_object_file(Asthra_SymbolResolver *resolver,
                                              const Asthra_ObjectFile *object_file);

/**
 * Add symbol reference (use of a symbol)
 * 
 * @param resolver Symbol resolver
 * @param symbol_name Name of referenced symbol
 * @param referencing_file File making the reference
 * @param section_name Section containing the reference
 * @param offset Offset within the section
 * @param reference_type Type of reference
 * @param addend Reference addend value
 * @return true on success, false on failure
 */
bool asthra_symbol_resolver_add_reference(Asthra_SymbolResolver *resolver,
                                          const char *symbol_name,
                                          const char *referencing_file,
                                          const char *section_name,
                                          uint64_t offset,
                                          int reference_type,
                                          int64_t addend);

// =============================================================================
// SYMBOL RESOLUTION API
// =============================================================================

/**
 * Resolve all symbols in the global symbol table
 * 
 * @param resolver Symbol resolver
 * @param result Resolution result output
 * @return true if resolution successful, false on errors
 */
bool asthra_symbol_resolver_resolve_all(Asthra_SymbolResolver *resolver,
                                        Asthra_ResolutionResult *result);

/**
 * Resolve a specific symbol by name
 * 
 * @param resolver Symbol resolver
 * @param symbol_name Name of symbol to resolve
 * @return Resolved symbol entry, or NULL if not found
 */
Asthra_SymbolEntry *asthra_symbol_resolver_resolve_symbol(Asthra_SymbolResolver *resolver,
                                                          const char *symbol_name);

/**
 * Check if all symbols are resolved
 * 
 * @param resolver Symbol resolver
 * @param undefined_count Output: number of undefined symbols
 * @return true if all symbols resolved, false otherwise
 */
bool asthra_symbol_resolver_is_complete(Asthra_SymbolResolver *resolver,
                                        size_t *undefined_count);

/**
 * Apply symbol resolution to object file relocations
 * 
 * @param resolver Symbol resolver
 * @param object_file Object file to update
 * @return true on success, false on failure
 */
bool asthra_symbol_resolver_apply_relocations(Asthra_SymbolResolver *resolver,
                                              Asthra_ObjectFile *object_file);

// =============================================================================
// SYMBOL QUERY API
// =============================================================================

/**
 * Find symbol entry by name
 * 
 * @param resolver Symbol resolver
 * @param symbol_name Name to search for
 * @return Symbol entry if found, NULL otherwise
 */
Asthra_SymbolEntry *asthra_symbol_resolver_find_symbol(Asthra_SymbolResolver *resolver,
                                                       const char *symbol_name);

/**
 * Get all undefined symbols
 * 
 * @param resolver Symbol resolver
 * @param undefined_symbols Output array of undefined symbol names
 * @param max_symbols Maximum number of symbols to return
 * @return Number of undefined symbols found
 */
size_t asthra_symbol_resolver_get_undefined_symbols(Asthra_SymbolResolver *resolver,
                                                    char **undefined_symbols,
                                                    size_t max_symbols);

/**
 * Get all symbol conflicts
 * 
 * @param resolver Symbol resolver
 * @param conflicts Output array of symbol conflicts
 * @param max_conflicts Maximum number of conflicts to return
 * @return Number of conflicts found
 */
size_t asthra_symbol_resolver_get_conflicts(Asthra_SymbolResolver *resolver,
                                            Asthra_SymbolConflict **conflicts,
                                            size_t max_conflicts);

/**
 * Check for symbol conflicts
 * 
 * @param resolver Symbol resolver
 * @param symbol_name Symbol to check for conflicts
 * @param conflict Output: conflict information if found
 * @return true if conflict exists, false otherwise
 */
bool asthra_symbol_resolver_has_conflict(Asthra_SymbolResolver *resolver,
                                         const char *symbol_name,
                                         Asthra_SymbolConflict **conflict);

// =============================================================================
// UTILITY AND MANAGEMENT API
// =============================================================================

/**
 * Get symbol resolution statistics
 * 
 * @param resolver Symbol resolver
 * @param total_symbols Output: total symbols processed
 * @param resolved_symbols Output: symbols successfully resolved
 * @param resolution_time_ms Output: total resolution time
 * @param hash_efficiency Output: hash table efficiency (0.0-1.0)
 */
void asthra_symbol_resolver_get_statistics(Asthra_SymbolResolver *resolver,
                                           size_t *total_symbols,
                                           size_t *resolved_symbols,
                                           double *resolution_time_ms,
                                           double *hash_efficiency);

/**
 * Clear all symbol data and reset resolver state
 * 
 * @param resolver Symbol resolver
 */
void asthra_symbol_resolver_clear_all(Asthra_SymbolResolver *resolver);

/**
 * Get last error message from symbol resolver
 * 
 * @param resolver Symbol resolver
 * @return Error message string, or NULL if no error
 */
const char *asthra_symbol_resolver_get_last_error(Asthra_SymbolResolver *resolver);

/**
 * Dump symbol table for debugging
 * 
 * @param resolver Symbol resolver
 * @param output_file File to write symbol dump to
 * @return true on success, false on failure
 */
bool asthra_symbol_resolver_dump_symbols(Asthra_SymbolResolver *resolver,
                                         FILE *output_file);

// =============================================================================
// SYMBOL UTILITIES
// =============================================================================

/**
 * Free symbol entry and all associated resources
 * 
 * @param entry Symbol entry to free
 */
void asthra_symbol_entry_free(Asthra_SymbolEntry *entry);

/**
 * Free symbol conflict structure
 * 
 * @param conflict Symbol conflict to free
 */
void asthra_symbol_conflict_free(Asthra_SymbolConflict *conflict);

/**
 * Free resolution result and all associated resources
 * 
 * @param result Resolution result to free
 */
void asthra_resolution_result_cleanup(Asthra_ResolutionResult *result);

/**
 * Calculate hash for symbol name
 * 
 * @param symbol_name Symbol name to hash
 * @param table_size Size of hash table
 * @return Hash value for the symbol name
 */
size_t asthra_symbol_hash(const char *symbol_name, size_t table_size);

/**
 * Resize symbol table when load factor gets too high
 * 
 * @param resolver Symbol resolver with table to resize
 * @return true on success, false on failure
 */
bool resize_symbol_table(Asthra_SymbolResolver *resolver);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SYMBOL_RESOLUTION_H 
