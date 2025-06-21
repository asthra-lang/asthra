/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Symbol Statistics
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Symbol table statistics and profiling functionality
 */

#include "semantic_symbols.h"
#include "semantic_core.h"
#include <stdlib.h>
#include <stdio.h>
#include "semantic_symbols_stats.h"

// ===============================
// SYMBOL STATISTICS
// ===============================

typedef struct {
    size_t total_symbols;
    size_t variables;
    size_t functions;
    size_t types;
    size_t unused_symbols;
} SymbolStatistics;

static bool count_symbol(const char *key, SymbolEntry *entry, void *user_data) {
    (void)key;
    SymbolStatistics *stats = (SymbolStatistics*)user_data;
    
    stats->total_symbols++;
    
    switch (entry->kind) {
        case SYMBOL_VARIABLE:
        case SYMBOL_PARAMETER:
        case SYMBOL_FIELD:
            stats->variables++;
            break;
        case SYMBOL_FUNCTION:
        case SYMBOL_METHOD:
            stats->functions++;
            break;
        case SYMBOL_TYPE:
        case SYMBOL_TYPE_PARAMETER:
            stats->types++;
            break;
        case SYMBOL_ENUM_VARIANT:
            // Count enum variants separately or as types
            stats->types++;
            break;
        case SYMBOL_CONST:
            // Count constants as variables
            stats->variables++;
            break;
        case SYMBOL_COUNT:
            // Validation case - should not occur in practice
            break;
    }
    
    if (!entry->flags.is_used) {
        stats->unused_symbols++;
    }
    
    return true; // Continue iteration
}

void semantic_print_symbol_statistics(SemanticAnalyzer *analyzer) {
    if (!analyzer) return;
    
    SymbolStatistics stats = {0};
    symbol_table_iterate(analyzer->global_scope, count_symbol, &stats);
    
    printf("Symbol Table Statistics:\n");
    printf("  Total symbols: %zu\n", stats.total_symbols);
    printf("  Variables: %zu\n", stats.variables);
    printf("  Functions: %zu\n", stats.functions);
    printf("  Types: %zu\n", stats.types);
    printf("  Unused symbols: %zu\n", stats.unused_symbols);
} 
