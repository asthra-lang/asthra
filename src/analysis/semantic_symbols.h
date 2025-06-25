/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Symbol Table Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Symbol table operations and symbol management
 */

#ifndef ASTHRA_SEMANTIC_SYMBOLS_H
#define ASTHRA_SEMANTIC_SYMBOLS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {

// semantic_register_module_alias
;

// semantic_resolve_module_alias
;

// semantic_has_module_alias
;

// semantic_cleanup_module_aliases
;

// symbol_table_insert_impl
;

// symbol_table_lookup_impl
;

// symbol_table_contains
;

// symbol_table_capacity
;

// symbol_entry_copy
;

// semantic_resolve_symbol_impl
;

// semantic_print_symbol_statistics
;

// symbol_kind_name
;

// symbol_kind_is_callable
;

// symbol_kind_is_type
;

// symbol_kind_is_value
;

// semantic_validate_symbol_name
;

// semantic_is_reserved_keyword
;

// semantic_check_symbol_shadowing
;

#endif

// =============================================================================
// SYMBOL TABLE LIFECYCLE
// =============================================================================

/**
 * Create a new symbol table
 */
SymbolTable *symbol_table_create(size_t initial_capacity);

/**
 * Destroy a symbol table and free all resources
 */
void symbol_table_destroy(SymbolTable *table);

/**
 * Create a child symbol table with the given parent
 */
SymbolTable *symbol_table_create_child(SymbolTable *parent);

// =============================================================================
// SYMBOL OPERATIONS
// =============================================================================

/**
 * Insert a symbol into the table (thread-safe)
 */
bool symbol_table_insert_safe(SymbolTable *table, const char *name, SymbolEntry *entry);

/**
 * Look up a symbol in the table (thread-safe)
 */
SymbolEntry *symbol_table_lookup_safe(SymbolTable *table, const char *name);

/**
 * Look up a symbol in the local table only (not parent scopes)
 */
SymbolEntry *symbol_table_lookup_local(SymbolTable *table, const char *name);

/**
 * Remove a symbol from the table (thread-safe)
 */
bool symbol_table_remove_safe(SymbolTable *table, const char *name);

// =============================================================================
// SYMBOL ENTRY MANAGEMENT
// =============================================================================

/**
 * Create a new symbol entry
 */
SymbolEntry *symbol_entry_create(const char *name, SymbolKind kind, TypeDescriptor *type,
                                 ASTNode *declaration);

/**
 * Destroy a symbol entry and free resources
 */
void symbol_entry_destroy(SymbolEntry *entry);

// =============================================================================
// MODULE ALIAS MANAGEMENT
// =============================================================================

/**
 * Add a module alias to the symbol table
 */
bool symbol_table_add_alias(SymbolTable *table, const char *alias_name, const char *module_path,
                            SymbolTable *module_symbols);

/**
 * Resolve a module alias to its symbol table
 */
SymbolTable *symbol_table_resolve_alias(SymbolTable *table, const char *alias_name);

/**
 * Clear all module aliases from the symbol table
 */
void symbol_table_clear_aliases(SymbolTable *table);

// =============================================================================
// SYMBOL TABLE UTILITIES
// =============================================================================

/**
 * Get the number of entries in the symbol table
 */
size_t symbol_table_size(const SymbolTable *table);

/**
 * Print symbol table contents for debugging
 */
void symbol_table_print(const SymbolTable *table);

/**
 * Check if the symbol table is empty
 */
bool symbol_table_is_empty(const SymbolTable *table);

/**
 * Iterate over all symbols in the table
 */
void symbol_table_iterate(SymbolTable *table, SymbolIteratorFunc func, void *user_data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_SYMBOLS_H
