/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Logic
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../platform.h"
#include "symbol_resolution.h"
#include <stdlib.h>
#include <string.h>

bool asthra_symbol_resolver_resolve_all(Asthra_SymbolResolver *resolver,
                                        Asthra_ResolutionResult *result) {
    if (!resolver || !result)
        return false;

    uint64_t start_time = asthra_get_high_resolution_time();

    // Initialize result
    memset(result, 0, sizeof(Asthra_ResolutionResult));
    result->total_symbols = resolver->symbol_count;

    // Count symbol types
    size_t resolved_count = 0;
    size_t undefined_count = 0;
    size_t weak_count = 0;

    for (size_t i = 0; i < resolver->table_size; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry) {
            switch (entry->status) {
            case ASTHRA_RESOLVE_DEFINED:
                resolved_count++;
                break;
            case ASTHRA_RESOLVE_UNDEFINED:
                undefined_count++;
                break;
            case ASTHRA_RESOLVE_WEAK:
                weak_count++;
                resolved_count++;
                break;
            case ASTHRA_RESOLVE_COMMON:
                // Common symbols need special handling
                resolved_count++;
                break;
            }
            entry = entry->next;
        }
    }

    result->resolved_symbols = resolved_count;
    result->undefined_symbols = undefined_count;
    result->weak_symbols = weak_count;

    // Check if resolution is successful
    if (undefined_count > 0 && !resolver->config.allow_undefined_symbols) {
        result->success = false;

        // Collect undefined symbol names
        result->undefined_symbol_names = calloc(undefined_count, sizeof(char *));
        if (result->undefined_symbol_names) {
            result->undefined_count = 0;

            for (size_t i = 0;
                 i < resolver->table_size && result->undefined_count < undefined_count; i++) {
                Asthra_SymbolEntry *entry = resolver->symbol_table[i];
                while (entry && result->undefined_count < undefined_count) {
                    if (entry->status == ASTHRA_RESOLVE_UNDEFINED) {
                        result->undefined_symbol_names[result->undefined_count] =
                            asthra_strdup(entry->name);
                        result->undefined_count++;
                    }
                    entry = entry->next;
                }
            }
        }
    } else {
        result->success = true;
        resolver->resolution_complete = true;
    }

    // Calculate performance metrics
    uint64_t end_time = asthra_get_high_resolution_time();
    result->resolution_time_ms = asthra_get_elapsed_seconds(start_time, end_time) * 1000.0;
    result->hash_collisions = resolver->hash_stats.hash_collisions;

    // Update statistics
    resolver->statistics.total_resolutions++;
    if (result->success) {
        resolver->statistics.successful_resolutions++;
    }
    resolver->statistics.total_resolution_time_ms += result->resolution_time_ms;
    resolver->statistics.symbols_processed += result->total_symbols;

    // Store result for future reference
    resolver->last_result = *result;

    return result->success;
}

bool asthra_symbol_resolver_is_complete(Asthra_SymbolResolver *resolver, size_t *undefined_count) {
    if (!resolver)
        return false;

    size_t undefined = 0;

    for (size_t i = 0; i < resolver->table_size; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry) {
            if (entry->status == ASTHRA_RESOLVE_UNDEFINED) {
                undefined++;
            }
            entry = entry->next;
        }
    }

    if (undefined_count) {
        *undefined_count = undefined;
    }

    return (undefined == 0) || resolver->config.allow_undefined_symbols;
}

bool asthra_symbol_resolver_apply_relocations(Asthra_SymbolResolver *resolver,
                                              Asthra_ObjectFile *object_file) {
    if (!resolver || !object_file)
        return false;

    // This is a placeholder for relocation application
    // In a real implementation, this would apply relocations based on resolved symbols

    size_t relocations_applied = 0;

    // Process each symbol's references
    for (size_t i = 0; i < resolver->table_size; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry) {
            if (entry->resolved) {
                Asthra_SymbolReference *ref = entry->references;
                while (ref) {
                    // Apply relocation (placeholder logic)
                    ref->resolved = true;
                    relocations_applied++;
                    ref = ref->next;
                }
            }
            entry = entry->next;
        }
    }

    return relocations_applied > 0;
}