/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Symbol Entry Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Symbol entry lifecycle and operations
 */

#include "semantic_symbols_entries.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include <stdlib.h>
#include <string.h>

// ===============================
// SYMBOL ENTRY OPERATIONS
// ===============================

SymbolEntry *symbol_entry_create(const char *name, SymbolKind kind, TypeDescriptor *type,
                                 ASTNode *declaration) {
    if (!name)
        return NULL;

    SymbolEntry *entry = malloc(sizeof(SymbolEntry));
    if (!entry)
        return NULL;

    entry->name = strdup(name);
    if (!entry->name) {
        free(entry);
        return NULL;
    }

    entry->kind = kind;
    entry->type = type;
    if (type) {
        type_descriptor_retain(type);
    }
    entry->declaration = declaration;
    entry->scope_id = 0;
    entry->flags.is_used = false;
    entry->flags.is_exported = false;
    entry->flags.is_mutable = false;
    entry->flags.is_initialized = false;
    entry->flags.is_predeclared = false;
    entry->flags.reserved = 0;
    entry->next = NULL;

    return entry;
}

void symbol_entry_destroy(SymbolEntry *entry) {
    if (!entry)
        return;

    free(entry->name);
    // Release the type descriptor reference
    if (entry->type) {
        type_descriptor_release(entry->type);
    }
    // Note: We don't free declaration as it's owned by the AST
    free(entry);
}

SymbolEntry *symbol_entry_copy(const SymbolEntry *entry) {
    if (!entry)
        return NULL;

    SymbolEntry *copy =
        symbol_entry_create(entry->name, entry->kind, entry->type, entry->declaration);
    if (!copy)
        return NULL;

    copy->flags = entry->flags;
    copy->scope_id = entry->scope_id;

    return copy;
}

// ===============================
// SYMBOL ITERATION
// ===============================

void symbol_table_iterate(SymbolTable *table, SymbolIteratorFunc func, void *user_data) {
    if (!table || !func)
        return;

    pthread_rwlock_rdlock(&table->rwlock);

    for (size_t i = 0; i < table->bucket_count; i++) {
        SymbolEntry *entry = table->buckets[i];
        while (entry) {
            if (!func(entry->name, entry, user_data)) {
                pthread_rwlock_unlock(&table->rwlock);
                return;
            }
            entry = entry->next;
        }
    }

    pthread_rwlock_unlock(&table->rwlock);
}

// ===============================
// IMPLEMENTATION HELPERS
// ===============================

SymbolEntry *semantic_resolve_symbol_impl(SemanticAnalyzer *analyzer, const char *name) {
    if (!analyzer || !name)
        return NULL;

    // Use the existing thread-safe lookup function
    SymbolEntry *entry = symbol_table_lookup_safe(analyzer->current_scope, name);

    // If symbol not found, report an error
    if (!entry) {
        semantic_report_error(
            analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
            (SourceLocation){.line = 1, .column = 1, .filename = NULL, .offset = 0},
            "Undefined symbol: %s", name);
    }

    return entry;
}
