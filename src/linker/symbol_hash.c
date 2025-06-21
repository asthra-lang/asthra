/**
 * Asthra Programming Language Compiler
 * Symbol Hash Table Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "symbol_resolution.h"
#include "../platform.h"
#include <stdlib.h>
#include <string.h>

/**
 * Simple hash function for symbol names
 */
size_t asthra_symbol_hash(const char *symbol_name, size_t table_size) {
    if (!symbol_name || table_size == 0) return 0;
    
    // djb2 hash algorithm
    size_t hash = 5381;
    const char *str = symbol_name;
    
    while (*str) {
        hash = ((hash << 5) + hash) + (unsigned char)*str++;
    }
    
    return hash % table_size;
}

/**
 * Resize symbol table when load factor gets too high
 */
bool resize_symbol_table(Asthra_SymbolResolver *resolver) {
    if (!resolver) return false;
    
    size_t old_size = resolver->table_size;
    size_t new_size = old_size * 2;
    
    // Allocate new table
    Asthra_SymbolEntry **new_table = calloc(new_size, sizeof(Asthra_SymbolEntry*));
    if (!new_table) {
        resolver->last_error = asthra_strdup("Failed to allocate memory for resized symbol table");
        resolver->error_occurred = true;
        return false;
    }
    
    // Rehash all existing symbols
    for (size_t i = 0; i < old_size; i++) {
        Asthra_SymbolEntry *entry = resolver->symbol_table[i];
        while (entry) {
            Asthra_SymbolEntry *next = entry->next;
            
            // Calculate new hash
            size_t new_hash = asthra_symbol_hash(entry->name, new_size);
            
            // Insert into new table
            entry->next = new_table[new_hash];
            new_table[new_hash] = entry;
            
            entry = next;
        }
    }
    
    // Replace old table
    free(resolver->symbol_table);
    resolver->symbol_table = new_table;
    resolver->table_size = new_size;
    resolver->hash_stats.resize_count++;
    
    return true;
}