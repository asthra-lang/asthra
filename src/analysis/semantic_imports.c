/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Import and Package Declaration Analysis
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of import, module, and use declarations
 */

#include "semantic_imports.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// IMPORT DECLARATION ANALYSIS
// =============================================================================

bool analyze_import_declaration(SemanticAnalyzer *analyzer, ASTNode *import_decl) {
    if (!analyzer || !import_decl)
        return false;

    // Handle import declarations with aliases
    const char *path = import_decl->data.import_decl.path;
    const char *alias = import_decl->data.import_decl.alias;

    // Check for duplicate imports
    for (size_t i = 0; i < analyzer->imported_count; i++) {
        if (strcmp(analyzer->imported_modules[i].path, path) == 0) {
            semantic_report_error(
                analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, import_decl->location,
                "Duplicate import: Module '%s' has already been imported at line %d, column %d",
                path, analyzer->imported_modules[i].location.line,
                analyzer->imported_modules[i].location.column);
            return false;
        }
    }

    // Add import to tracking list
    if (analyzer->imported_count >= analyzer->imported_capacity) {
        size_t new_capacity =
            analyzer->imported_capacity == 0 ? 16 : analyzer->imported_capacity * 2;
        ImportedModule *new_modules =
            realloc(analyzer->imported_modules, new_capacity * sizeof(ImportedModule));
        if (!new_modules) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, import_decl->location,
                                  "Failed to allocate memory for import tracking");
            return false;
        }
        analyzer->imported_modules = new_modules;
        analyzer->imported_capacity = new_capacity;
    }

    // Store the import
    analyzer->imported_modules[analyzer->imported_count].path = strdup(path);
    analyzer->imported_modules[analyzer->imported_count].location = import_decl->location;
    analyzer->imported_count++;

    if (alias) {
        // TODO: In a real implementation, we would load the module
        // and get its symbol table. For now, we'll create a placeholder.
        SymbolTable *module_symbols = symbol_table_create(16);
        if (!module_symbols) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, import_decl->location,
                                  "Failed to create symbol table for module '%s'", path);
            return false;
        }

        // Register the alias
        if (!symbol_table_add_alias(analyzer->current_scope, alias, path, module_symbols)) {
            symbol_table_destroy(module_symbols);
            semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, import_decl->location,
                                  "Alias '%s' conflicts with existing symbol", alias);
            return false;
        }
    }

    return true;
}

// =============================================================================
// VISIBILITY MODIFIER ANALYSIS
// =============================================================================

bool analyze_visibility_modifier(SemanticAnalyzer *analyzer, ASTNode *visibility) {
    (void)analyzer;
    (void)visibility;
    // TODO: Implement visibility modifier analysis
    return true;
}
