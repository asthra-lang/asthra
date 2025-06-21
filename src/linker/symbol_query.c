/**
 * Asthra Programming Language Compiler
 * Symbol Query and Inspection Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "symbol_resolution.h"
#include "../platform.h"
#include <stdlib.h>
#include <stdio.h>

size_t asthra_symbol_resolver_get_undefined_symbols(Asthra_SymbolResolver *resolver,
                                                   char **undefined_symbols,
                                                   size_t max_symbols) {
    if (!resolver || !undefined_symbols) return 0;
    
    size_t count = 0;
    
    for (size_t i = 0; i < resolver->table_size && count < max_symbols; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry && count < max_symbols) {
            if (entry->status == ASTHRA_RESOLVE_UNDEFINED) {
                undefined_symbols[count] = asthra_strdup(entry->name);
                count++;
            }
            entry = entry->next;
        }
    }
    
    return count;
}

size_t asthra_symbol_resolver_get_conflicts(Asthra_SymbolResolver *resolver,
                                           Asthra_SymbolConflict **conflicts,
                                           size_t max_conflicts) {
    if (!resolver || !conflicts) return 0;
    
    // For now, return 0 conflicts
    // In a real implementation, this would detect and return actual conflicts
    return 0;
}

bool asthra_symbol_resolver_has_conflict(Asthra_SymbolResolver *resolver,
                                        const char *symbol_name,
                                        Asthra_SymbolConflict **conflict) {
    if (!resolver || !symbol_name) return false;
    
    // For now, assume no conflicts
    if (conflict) {
        *conflict = NULL;
    }
    return false;
}

void asthra_symbol_resolver_get_statistics(Asthra_SymbolResolver *resolver,
                                          size_t *total_symbols,
                                          size_t *resolved_symbols,
                                          double *resolution_time_ms,
                                          double *hash_efficiency) {
    if (!resolver) return;
    
    if (total_symbols) {
        *total_symbols = resolver->symbol_count;
    }
    
    if (resolved_symbols) {
        size_t resolved = 0;
        for (size_t i = 0; i < resolver->table_size; i++) {
            Asthra_SymbolEntry *entry = resolver->symbol_table[i];
            while (entry) {
                if (entry->resolved) {
                    resolved++;
                }
                entry = entry->next;
            }
        }
        *resolved_symbols = resolved;
    }
    
    if (resolution_time_ms) {
        *resolution_time_ms = resolver->statistics.total_resolution_time_ms;
    }
    
    if (hash_efficiency) {
        if (resolver->symbol_count > 0) {
            double collisions_per_symbol = (double)resolver->hash_stats.hash_collisions / (double)resolver->symbol_count;
            *hash_efficiency = 1.0 - collisions_per_symbol;
        } else {
            *hash_efficiency = 1.0;
        }
    }
}

bool asthra_symbol_resolver_dump_symbols(Asthra_SymbolResolver *resolver,
                                        FILE *output_file) {
    if (!resolver || !output_file) return false;
    
    fprintf(output_file, "Symbol Resolver Dump:\n");
    fprintf(output_file, "Total symbols: %zu\n", resolver->symbol_count);
    fprintf(output_file, "Table size: %zu\n", resolver->table_size);
    fprintf(output_file, "Hash collisions: %zu\n", resolver->hash_stats.hash_collisions);
    fprintf(output_file, "\nSymbols:\n");
    
    for (size_t i = 0; i < resolver->table_size; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry) {
            fprintf(output_file, "  %s: addr=0x%016llx size=%zu status=%d type=%d refs=%zu\n",
                    entry->name, entry->address, entry->size, entry->status, 
                    entry->type, entry->reference_count);
            entry = entry->next;
        }
    }
    
    return true;
}