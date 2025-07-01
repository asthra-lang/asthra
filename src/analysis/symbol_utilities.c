/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Symbol Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Symbol resolution and declaration utilities
 */

#include "symbol_utilities.h"
#include "semantic_errors.h"
#include "semantic_macros.h"
#include "semantic_symbols.h"
#include "semantic_symbols_core.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// SYMBOL UTILITIES
// =============================================================================

SymbolEntry *semantic_resolve_identifier(SemanticAnalyzer *analyzer, const char *name) {
    return semantic_resolve_symbol_impl(analyzer, name);
}

bool semantic_declare_symbol(SemanticAnalyzer *analyzer, const char *name, SymbolKind kind,
                             TypeDescriptor *type, ASTNode *declaration) {
    if (!analyzer || !name)
        return false;

    // Check if there's an existing symbol with this name
    SymbolEntry *existing = symbol_table_lookup_local(analyzer->current_scope, name);
    if (existing && existing->flags.is_predeclared) {
        // Remove the predeclared symbol to allow shadowing
        symbol_table_remove(analyzer->current_scope, name);
    }

    SymbolEntry *entry = symbol_entry_create(name, kind, type, declaration);
    if (!entry)
        return false;

    // Set the scope ID to the current scope
    entry->scope_id = semantic_get_current_scope_id(analyzer);

    bool success = symbol_table_insert_safe(analyzer->current_scope, name, entry);
    if (!success) {
        symbol_entry_destroy(entry);
        // Report error for duplicate symbol (only for non-predeclared symbols)
        semantic_report_error(
            analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
            declaration ? declaration->location
                        : (SourceLocation){.line = 1, .column = 1, .filename = NULL, .offset = 0},
            "Duplicate symbol declaration: %s", name);
    }

    return success;
}