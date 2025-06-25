/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Module Alias Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Module alias and import management functionality
 */

#include "semantic_symbols_aliases.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// Local hash function to avoid dependency issues
static uint32_t symbol_hash(const char *key) {
    if (!key)
        return 0;

    // FNV-1a hash
    uint32_t hash = 2166136261u;
    while (*key) {
        hash ^= (uint8_t)*key++;
        hash *= 16777619u;
    }
    return hash;
}

// ===============================
// MODULE ALIAS MANAGEMENT
// ===============================

static SymbolTable *g_module_aliases = NULL;
static pthread_once_t g_module_init;
static bool g_module_init_initialized = false;

static void init_module_aliases(void) {
    g_module_aliases = symbol_table_create(32);
}

bool semantic_register_module_alias(const char *alias, const char *module_name) {
    if (!alias || !module_name)
        return false;

    if (!g_module_init_initialized) {
        static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&init_mutex);
        if (!g_module_init_initialized) {
            init_module_aliases();
            g_module_init_initialized = true;
        }
        pthread_mutex_unlock(&init_mutex);
    }
    if (!g_module_aliases)
        return false;

    // Create a simple string entry for the module name
    SymbolEntry *entry = symbol_entry_create(module_name, SYMBOL_TYPE, NULL, NULL);
    if (!entry)
        return false;

    return symbol_table_insert_safe(g_module_aliases, alias, entry);
}

const char *semantic_resolve_module_alias(const char *alias) {
    if (!alias)
        return NULL;

    if (!g_module_init_initialized) {
        static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&init_mutex);
        if (!g_module_init_initialized) {
            init_module_aliases();
            g_module_init_initialized = true;
        }
        pthread_mutex_unlock(&init_mutex);
    }
    if (!g_module_aliases)
        return NULL;

    SymbolEntry *entry = symbol_table_lookup_safe(g_module_aliases, alias);
    return entry ? entry->name : NULL;
}

bool semantic_has_module_alias(const char *alias) {
    return semantic_resolve_module_alias(alias) != NULL;
}

bool symbol_table_add_alias(SymbolTable *table, const char *alias_name, const char *module_path,
                            SymbolTable *module_symbols) {
    if (!table || !alias_name || !module_path)
        return false;

    // First check for conflicts with existing symbols without holding the lock
    // This is safe because we'll re-check under the write lock
    if (symbol_table_lookup_safe(table, alias_name)) {
        return false; // Alias conflicts with existing symbol
    }

    pthread_rwlock_wrlock(&table->rwlock);

    // Re-check for conflicts with existing symbols using local lookup only
    // to avoid deadlock from recursive locking
    uint32_t hash = symbol_hash(alias_name);
    size_t bucket = hash % table->bucket_count;
    SymbolEntry *entry = table->buckets[bucket];
    while (entry) {
        if (strcmp(entry->name, alias_name) == 0) {
            pthread_rwlock_unlock(&table->rwlock);
            return false; // Alias conflicts with existing symbol
        }
        entry = entry->next;
    }

    // Check for conflicts with existing aliases
    for (size_t i = 0; i < table->alias_count; i++) {
        if (strcmp(table->aliases[i].alias_name, alias_name) == 0) {
            pthread_rwlock_unlock(&table->rwlock);
            return false; // Alias conflicts with existing alias
        }
    }

    // Expand alias array if needed
    if (table->alias_count >= table->alias_capacity) {
        size_t new_capacity = table->alias_capacity == 0 ? 4 : table->alias_capacity * 2;
        ModuleAlias *new_aliases = realloc(table->aliases, new_capacity * sizeof(ModuleAlias));
        if (!new_aliases) {
            pthread_rwlock_unlock(&table->rwlock);
            return false;
        }
        table->aliases = new_aliases;
        table->alias_capacity = new_capacity;
    }

    // Add alias
    ModuleAlias *alias = &table->aliases[table->alias_count++];
    alias->alias_name = strdup(alias_name);
    alias->module_path = strdup(module_path);
    alias->module_symbols = module_symbols;

    if (!alias->alias_name || !alias->module_path) {
        // Cleanup on failure
        free(alias->alias_name);
        free(alias->module_path);
        table->alias_count--;
        pthread_rwlock_unlock(&table->rwlock);
        return false;
    }

    pthread_rwlock_unlock(&table->rwlock);
    return true;
}

SymbolTable *symbol_table_resolve_alias(SymbolTable *table, const char *alias_name) {
    if (!table || !alias_name)
        return NULL;

    pthread_rwlock_rdlock(&table->rwlock);

    for (size_t i = 0; i < table->alias_count; i++) {
        if (strcmp(table->aliases[i].alias_name, alias_name) == 0) {
            SymbolTable *result = table->aliases[i].module_symbols;
            pthread_rwlock_unlock(&table->rwlock);
            return result;
        }
    }

    pthread_rwlock_unlock(&table->rwlock);
    return NULL;
}

void symbol_table_clear_aliases(SymbolTable *table) {
    if (!table)
        return;

    pthread_rwlock_wrlock(&table->rwlock);

    for (size_t i = 0; i < table->alias_count; i++) {
        free(table->aliases[i].alias_name);
        free(table->aliases[i].module_path);
        // Note: We don't destroy module_symbols as they may be shared
    }

    free(table->aliases);
    table->aliases = NULL;
    table->alias_count = 0;
    table->alias_capacity = 0;

    pthread_rwlock_unlock(&table->rwlock);
}

// ===============================
// CLEANUP
// ===============================

void semantic_cleanup_module_aliases(void) {
    if (g_module_aliases) {
        symbol_table_destroy(g_module_aliases);
        g_module_aliases = NULL;
    }
}
