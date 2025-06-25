/**
 * Asthra Programming Language Compiler
 * Symbol Resolver Lifecycle Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../platform.h"
#include "symbol_resolution.h"
#include <stdlib.h>
#include <string.h>

Asthra_SymbolResolver *asthra_symbol_resolver_create(size_t initial_table_size) {
    if (initial_table_size == 0) {
        initial_table_size = 1024; // Default size
    }

    Asthra_SymbolResolver *resolver = calloc(1, sizeof(Asthra_SymbolResolver));
    if (!resolver)
        return NULL;

    resolver->symbol_table = calloc(initial_table_size, sizeof(Asthra_SymbolEntry *));
    if (!resolver->symbol_table) {
        free(resolver);
        return NULL;
    }

    resolver->table_size = initial_table_size;
    resolver->symbol_count = 0;
    resolver->resolution_complete = false;

    // Set default configuration
    resolver->config.allow_undefined_symbols = false;
    resolver->config.prefer_strong_over_weak = true;
    resolver->config.ignore_size_mismatches = false;
    resolver->config.case_sensitive_symbols = true;
    resolver->config.max_resolution_iterations = 10;

    // Initialize statistics
    memset(&resolver->statistics, 0, sizeof(resolver->statistics));
    memset(&resolver->hash_stats, 0, sizeof(resolver->hash_stats));
    memset(&resolver->last_result, 0, sizeof(resolver->last_result));

    return resolver;
}

void asthra_symbol_resolver_destroy(Asthra_SymbolResolver *resolver) {
    if (!resolver)
        return;

    // Free all symbol entries
    for (size_t i = 0; i < resolver->table_size; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry) {
            Asthra_SymbolEntry *next = entry->next;
            asthra_symbol_entry_free(entry);
            entry = next;
        }
    }

    // Free symbol table
    free(resolver->symbol_table);

    // Free last error message
    if (resolver->last_error) {
        free(resolver->last_error);
    }

    // Cleanup last result
    asthra_resolution_result_cleanup(&resolver->last_result);

    free(resolver);
}

bool asthra_symbol_resolver_configure(Asthra_SymbolResolver *resolver, bool allow_undefined,
                                      bool prefer_strong, bool case_sensitive) {
    if (!resolver)
        return false;

    resolver->config.allow_undefined_symbols = allow_undefined;
    resolver->config.prefer_strong_over_weak = prefer_strong;
    resolver->config.case_sensitive_symbols = case_sensitive;

    return true;
}

void asthra_symbol_resolver_clear_all(Asthra_SymbolResolver *resolver) {
    if (!resolver)
        return;

    // Free all symbol entries
    for (size_t i = 0; i < resolver->table_size; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry) {
            Asthra_SymbolEntry *next = entry->next;
            asthra_symbol_entry_free(entry);
            entry = next;
        }
        resolver->symbol_table[i] = NULL;
    }

    resolver->symbol_count = 0;
    resolver->resolution_complete = false;

    // Clear statistics
    memset(&resolver->statistics, 0, sizeof(resolver->statistics));
    memset(&resolver->hash_stats, 0, sizeof(resolver->hash_stats));

    // Clear last result
    asthra_resolution_result_cleanup(&resolver->last_result);
    memset(&resolver->last_result, 0, sizeof(resolver->last_result));
}