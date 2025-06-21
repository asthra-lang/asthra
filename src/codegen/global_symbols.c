/**
 * Asthra Programming Language Compiler
 * Global Symbol Table for Const Declarations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 3: Code Generation Implementation
 * Manages global symbols for const declarations with proper visibility tracking
 */

#include "global_symbols.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// GLOBAL SYMBOL TABLE IMPLEMENTATION
// =============================================================================

// Static global symbol table for simplicity during testing
static GlobalSymbolTable* g_global_symbol_table = NULL;

/**
 * Create a new global symbol table
 */
GlobalSymbolTable* global_symbol_table_create(void) {
    GlobalSymbolTable* table = malloc(sizeof(GlobalSymbolTable));
    if (!table) {
        return NULL;
    }

    table->capacity = INITIAL_GLOBAL_SYMBOL_CAPACITY;
    table->size = 0;
    table->entries = calloc(table->capacity, sizeof(GlobalSymbolEntry*));
    if (!table->entries) {
        free(table);
        return NULL;
    }

    return table;
}

/**
 * Destroy a global symbol table
 */
void global_symbol_table_destroy(GlobalSymbolTable* table) {
    if (!table) {
        return;
    }

    for (size_t i = 0; i < table->capacity; i++) {
        GlobalSymbolEntry* entry = table->entries[i];
        while (entry) {
            GlobalSymbolEntry* next = entry->next;
            free(entry->name);
            free(entry->type);
            free(entry);
            entry = next;
        }
    }

    free(table->entries);
    free(table);
}

/**
 * Hash function for symbol names
 */
static uint32_t hash_symbol_name(const char* name) {
    if (!name) {
        return 0;
    }

    uint32_t hash = 5381;
    int c;
    while ((c = *name++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c; // hash * 33 + c
    }
    return hash;
}

/**
 * Resize the global symbol table
 */
static bool global_symbol_table_resize(GlobalSymbolTable* table) {
    if (!table) {
        return false;
    }

    size_t old_capacity = table->capacity;
    GlobalSymbolEntry** old_entries = table->entries;

    table->capacity *= 2;
    table->entries = calloc(table->capacity, sizeof(GlobalSymbolEntry*));
    if (!table->entries) {
        table->capacity = old_capacity;
        table->entries = old_entries;
        return false;
    }

    table->size = 0;

    // Rehash all entries
    for (size_t i = 0; i < old_capacity; i++) {
        GlobalSymbolEntry* entry = old_entries[i];
        while (entry) {
            GlobalSymbolEntry* next = entry->next;
            
            // Re-insert entry
            uint32_t hash = hash_symbol_name(entry->name);
            size_t index = hash % table->capacity;
            
            entry->next = table->entries[index];
            table->entries[index] = entry;
            table->size++;
            
            entry = next;
        }
    }

    free(old_entries);
    return true;
}

/**
 * Add a symbol to the global symbol table
 */
bool global_symbol_table_add(GlobalSymbolTable* table, const char* name, const char* type, 
                             GlobalSymbolVisibility visibility, ConstValueType value_type) {
    if (!table || !name || !type) {
        return false;
    }

    // Check if symbol already exists
    if (global_symbol_table_lookup(table, name)) {
        return false; // Duplicate symbol
    }

    // Check if resize is needed
    if ((double)table->size >= (double)table->capacity * LOAD_FACTOR_THRESHOLD) {
        if (!global_symbol_table_resize(table)) {
            return false;
        }
    }

    // Create new entry
    GlobalSymbolEntry* entry = malloc(sizeof(GlobalSymbolEntry));
    if (!entry) {
        return false;
    }

    entry->name = malloc(strlen(name) + 1);
    entry->type = malloc(strlen(type) + 1);
    if (!entry->name || !entry->type) {
        free(entry->name);
        free(entry->type);
        free(entry);
        return false;
    }

    strcpy(entry->name, name);
    strcpy(entry->type, type);
    entry->visibility = visibility;
    entry->value_type = value_type;

    // Insert into hash table
    uint32_t hash = hash_symbol_name(name);
    size_t index = hash % table->capacity;
    
    entry->next = table->entries[index];
    table->entries[index] = entry;
    table->size++;

    return true;
}

/**
 * Look up a symbol in the global symbol table
 */
GlobalSymbolEntry* global_symbol_table_lookup(GlobalSymbolTable* table, const char* name) {
    if (!table || !name) {
        return NULL;
    }

    uint32_t hash = hash_symbol_name(name);
    size_t index = hash % table->capacity;
    
    GlobalSymbolEntry* entry = table->entries[index];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    return NULL;
}

/**
 * Get all public symbols from the global symbol table
 */
GlobalSymbolEntry** global_symbol_table_get_public_symbols(GlobalSymbolTable* table, size_t* count) {
    if (!table || !count) {
        return NULL;
    }

    // Count public symbols
    size_t public_count = 0;
    for (size_t i = 0; i < table->capacity; i++) {
        GlobalSymbolEntry* entry = table->entries[i];
        while (entry) {
            if (entry->visibility == GLOBAL_SYMBOL_PUBLIC) {
                public_count++;
            }
            entry = entry->next;
        }
    }

    if (public_count == 0) {
        *count = 0;
        return NULL;
    }

    // Allocate array for public symbols
    GlobalSymbolEntry** public_symbols = malloc(public_count * sizeof(GlobalSymbolEntry*));
    if (!public_symbols) {
        *count = 0;
        return NULL;
    }

    // Collect public symbols
    size_t index = 0;
    for (size_t i = 0; i < table->capacity; i++) {
        GlobalSymbolEntry* entry = table->entries[i];
        while (entry) {
            if (entry->visibility == GLOBAL_SYMBOL_PUBLIC) {
                public_symbols[index++] = entry;
            }
            entry = entry->next;
        }
    }

    *count = public_count;
    return public_symbols;
}

/**
 * Create an iterator for the global symbol table
 */
GlobalSymbolIterator global_symbol_table_iterator_create(GlobalSymbolTable* table) {
    GlobalSymbolIterator iterator;
    iterator.table = table;
    iterator.bucket_index = 0;
    iterator.current_entry = NULL;
    iterator.finished = false;

    if (!table || table->size == 0) {
        iterator.finished = true;
        return iterator;
    }

    // Find first non-empty bucket
    while (iterator.bucket_index < table->capacity && !table->entries[iterator.bucket_index]) {
        iterator.bucket_index++;
    }

    if (iterator.bucket_index < table->capacity) {
        iterator.current_entry = table->entries[iterator.bucket_index];
    } else {
        iterator.finished = true;
    }

    return iterator;
}

/**
 * Get the next symbol from the iterator
 */
GlobalSymbolEntry* global_symbol_table_iterator_next(GlobalSymbolIterator* iterator) {
    if (!iterator || iterator->finished || !iterator->table) {
        return NULL;
    }

    GlobalSymbolEntry* current = iterator->current_entry;
    if (!current) {
        iterator->finished = true;
        return NULL;
    }

    // Move to next entry
    if (current->next) {
        iterator->current_entry = current->next;
    } else {
        // Move to next bucket
        iterator->bucket_index++;
        iterator->current_entry = NULL;

        // Find next non-empty bucket
        while (iterator->bucket_index < iterator->table->capacity && 
               !iterator->table->entries[iterator->bucket_index]) {
            iterator->bucket_index++;
        }

        if (iterator->bucket_index < iterator->table->capacity) {
            iterator->current_entry = iterator->table->entries[iterator->bucket_index];
        } else {
            iterator->finished = true;
        }
    }

    return current;
}

/**
 * Check if the iterator has finished
 */
bool global_symbol_table_iterator_finished(GlobalSymbolIterator* iterator) {
    return !iterator || iterator->finished;
}

// =============================================================================
// FFI ASSEMBLY GENERATOR INTEGRATION (SIMPLIFIED)
// =============================================================================

/**
 * Add a const symbol to the global table
 */
bool ffi_add_global_const_symbol(FFIAssemblyGenerator* generator, const char* symbol_name, 
                                 const char* symbol_type, bool is_public) {
    // For testing, ignore the generator and use global table
    (void)generator; // Suppress unused parameter warning
    
    // Initialize global table if needed
    if (!g_global_symbol_table) {
        g_global_symbol_table = global_symbol_table_create();
        if (!g_global_symbol_table) {
            return false;
        }
    }

    GlobalSymbolVisibility visibility = is_public ? GLOBAL_SYMBOL_PUBLIC : GLOBAL_SYMBOL_PRIVATE;
    return global_symbol_table_add(g_global_symbol_table, symbol_name, symbol_type, 
                                  visibility, CONST_VALUE_INTEGER);
}

/**
 * Check if a const symbol exists
 */
bool ffi_const_symbol_exists(FFIAssemblyGenerator* generator, const char* symbol_name) {
    (void)generator; // Suppress unused parameter warning
    
    if (!g_global_symbol_table || !symbol_name) {
        return false;
    }

    GlobalSymbolEntry* entry = global_symbol_table_lookup(g_global_symbol_table, symbol_name);
    return entry != NULL;
}

/**
 * Check if a const symbol is public
 */
bool ffi_const_symbol_is_public(FFIAssemblyGenerator* generator, const char* symbol_name) {
    (void)generator; // Suppress unused parameter warning
    
    if (!g_global_symbol_table || !symbol_name) {
        return false;
    }

    GlobalSymbolEntry* entry = global_symbol_table_lookup(g_global_symbol_table, symbol_name);
    return entry && (entry->visibility == GLOBAL_SYMBOL_PUBLIC);
}

/**
 * Cleanup global symbol table (for testing)
 */
void global_symbols_cleanup(void) {
    if (g_global_symbol_table) {
        global_symbol_table_destroy(g_global_symbol_table);
        g_global_symbol_table = NULL;
    }
} 
