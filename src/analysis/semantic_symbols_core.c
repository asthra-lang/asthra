/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Symbol Table Core Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core symbol table implementation with thread-safe operations
 */

#include "semantic_symbols.h"
#include "semantic_core.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include "semantic_symbols_core.h"

// ===============================
// HASH FUNCTION
// ===============================

static uint32_t symbol_hash(const char *key) {
    if (!key) return 0;
    
    // FNV-1a hash
    uint32_t hash = 2166136261u;
    while (*key) {
        hash ^= (uint8_t)*key++;
        hash *= 16777619u;
    }
    return hash;
}

// ===============================
// CORE SYMBOL TABLE FUNCTIONS
// ===============================

SymbolTable *symbol_table_create(size_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = 16;
    
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table) return NULL;
    
    table->buckets = calloc(initial_capacity, sizeof(SymbolEntry*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    
    table->bucket_count = initial_capacity;
    atomic_init(&table->entry_count, 0);
    atomic_init(&table->scope_counter, 0);
    table->current_scope = 0;
    
    if (pthread_rwlock_init(&table->rwlock, NULL) != 0) {
        free(table->buckets);
        free(table);
        return NULL;
    }
    
    table->parent = NULL;
    table->aliases = NULL;
    table->alias_count = 0;
    table->alias_capacity = 0;
    
    return table;
}

void symbol_table_destroy(SymbolTable *table) {
    if (!table) return;
    
    // Free all entries
    for (size_t i = 0; i < table->bucket_count; i++) {
        SymbolEntry *entry = table->buckets[i];
        while (entry) {
            SymbolEntry *next = entry->next;
            symbol_entry_destroy(entry);
            entry = next;
        }
    }
    
    // Free aliases
    for (size_t i = 0; i < table->alias_count; i++) {
        free(table->aliases[i].alias_name);
        free(table->aliases[i].module_path);
    }
    free(table->aliases);
    
    pthread_rwlock_destroy(&table->rwlock);
    free(table->buckets);
    free(table);
}

bool symbol_table_insert_impl(SymbolTable *table, const char *name, SymbolEntry *entry) {
    if (!table || !name || !entry) return false;
    
    uint32_t hash = symbol_hash(name);
    size_t bucket = hash % table->bucket_count;
    
    pthread_rwlock_wrlock(&table->rwlock);
    
    // Check if entry already exists
    SymbolEntry *existing = table->buckets[bucket];
    while (existing) {
        if (strcmp(existing->name, name) == 0) {
            pthread_rwlock_unlock(&table->rwlock);
            return false; // Duplicate entry
        }
        existing = existing->next;
    }
    
    // Insert at head of bucket
    entry->next = table->buckets[bucket];
    table->buckets[bucket] = entry;
    atomic_fetch_add(&table->entry_count, 1);
    
    pthread_rwlock_unlock(&table->rwlock);
    return true;
}

SymbolEntry *symbol_table_lookup_impl(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;
    
    uint32_t hash = symbol_hash(name);
    size_t bucket = hash % table->bucket_count;
    
    pthread_rwlock_rdlock(&table->rwlock);
    
    SymbolEntry *entry = table->buckets[bucket];
    while (entry) {
        if (strcmp(entry->name, name) == 0) {
            pthread_rwlock_unlock(&table->rwlock);
            return entry;
        }
        entry = entry->next;
    }
    
    pthread_rwlock_unlock(&table->rwlock);
    
    // Check parent scope
    if (table->parent) {
        return symbol_table_lookup_impl(table->parent, name);
    }
    
    return NULL;
}

bool symbol_table_insert_safe(SymbolTable *table, const char *name, SymbolEntry *entry) {
    return symbol_table_insert_impl(table, name, entry);
}

SymbolEntry *symbol_table_lookup_safe(SymbolTable *table, const char *name) {
    return symbol_table_lookup_impl(table, name);
}

// ===============================
// ADDITIONAL SYMBOL TABLE OPERATIONS
// ===============================

SymbolTable *symbol_table_create_child(SymbolTable *parent) {
    SymbolTable *child = symbol_table_create(16);  // Use existing function
    if (!child) return NULL;
    
    child->parent = parent;
    child->current_scope = parent ? parent->current_scope + 1 : 0;
    
    return child;
}

SymbolEntry *symbol_table_lookup_local(SymbolTable *table, const char *key) {
    if (!table || !key) return NULL;
    
    uint32_t hash = symbol_hash(key);
    size_t bucket = hash % table->bucket_count;
    
    pthread_rwlock_rdlock(&table->rwlock);
    
    SymbolEntry *entry = table->buckets[bucket];
    while (entry) {
        if (strcmp(entry->name, key) == 0) {
            pthread_rwlock_unlock(&table->rwlock);
            return entry;
        }
        entry = entry->next;
    }
    
    pthread_rwlock_unlock(&table->rwlock);
    return NULL;
}

bool symbol_table_contains(SymbolTable *table, const char *key) {
    return symbol_table_lookup_local(table, key) != NULL;
}

size_t symbol_table_size(const SymbolTable *table) {
    if (!table) return 0;
    return atomic_load(&table->entry_count);
}

size_t symbol_table_capacity(SymbolTable *table) {
    return table ? table->bucket_count : 0;
}

bool symbol_table_remove(SymbolTable *table, const char *key) {
    if (!table || !key) return false;
    
    uint32_t hash = symbol_hash(key);
    size_t bucket = hash % table->bucket_count;
    
    pthread_rwlock_wrlock(&table->rwlock);
    
    SymbolEntry **entry_ptr = &table->buckets[bucket];
    while (*entry_ptr) {
        if (strcmp((*entry_ptr)->name, key) == 0) {
            SymbolEntry *to_remove = *entry_ptr;
            *entry_ptr = to_remove->next;
            symbol_entry_destroy(to_remove);
            atomic_fetch_sub(&table->entry_count, 1);
            pthread_rwlock_unlock(&table->rwlock);
            return true;
        }
        entry_ptr = &(*entry_ptr)->next;
    }
    
    pthread_rwlock_unlock(&table->rwlock);
    return false;
} 
