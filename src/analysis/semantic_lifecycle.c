/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Lifecycle Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Semantic analyzer lifecycle: creation, destruction, and reset
 */

#include "semantic_builtins.h"
#include "semantic_core.h"
#include "semantic_errors.h"
#include "semantic_scopes.h"
#include "semantic_statistics.h"
#include "semantic_symbols.h"
#include "semantic_utilities.h"
#include <stdlib.h>
#include <string.h>

// C17 feature detection and compatibility
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
#define ASTHRA_HAS_C17 1
#else
#define ASTHRA_HAS_C17 0
#endif

SemanticAnalyzer *semantic_analyzer_create(void) {
    SemanticAnalyzer *analyzer = malloc(sizeof(SemanticAnalyzer));
    if (!analyzer)
        return NULL;

    // Initialize with C17 designated initializers
    *analyzer = (SemanticAnalyzer){.global_scope = symbol_table_create(64),
                                   .current_scope = NULL,
                                   .builtin_types = NULL,
                                   .builtin_type_count = 0,
                                   .errors = NULL,
                                   .last_error = NULL,
                                   .error_count = 0,
                                   .max_errors = 100,
                                   .stats = {.nodes_analyzed = 0,
                                             .types_checked = 0,
                                             .symbols_resolved = 0,
                                             .errors_found = 0,
                                             .warnings_issued = 0,
                                             .max_scope_depth = 0,
                                             .current_scope_depth = 0},
                                   .in_unsafe_context = false,
                                   .current_function = NULL,
                                   .loop_depth = 0,
                                   .in_unreachable_code = false,
                                   .config = {.strict_mode = true,
                                              .allow_unsafe = false,
                                              .check_ownership = true,
                                              .validate_ffi = true,
                                              .enable_warnings = true,
                                              .test_mode = false}};

    if (!analyzer->global_scope) {
        free(analyzer);
        return NULL;
    }

    analyzer->current_scope = analyzer->global_scope;

    // Initialize builtin types
    semantic_init_builtin_types(analyzer);

    // Initialize predeclared identifiers
    semantic_init_predeclared_identifiers(analyzer);

    // Initialize builtin generic types (Option, Result, etc.)
    semantic_init_builtin_generic_types(analyzer);

    return analyzer;
}

void semantic_analyzer_destroy(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    // Clear errors
    semantic_clear_errors(analyzer);

    // Destroy symbol tables
    symbol_table_destroy(analyzer->global_scope);

    // Free builtin types array (but not the types themselves - they're static)
    free(analyzer->builtin_types);

    // Free predeclared identifiers
    free(analyzer->predeclared_identifiers);

    free(analyzer);
}

void semantic_analyzer_reset(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    // Clear errors
    semantic_clear_errors(analyzer);

    // Reset statistics
    semantic_reset_statistics(analyzer);

    // Reset current scope to global
    analyzer->current_scope = analyzer->global_scope;

    // Reset current function
    analyzer->current_function = NULL;

    // Reset loop depth
    analyzer->loop_depth = 0;
}

void semantic_analyzer_set_test_mode(SemanticAnalyzer *analyzer, bool enable) {
    if (!analyzer)
        return;

    analyzer->config.test_mode = enable;

    // In test mode, relax certain restrictions
    if (enable) {
        analyzer->config.strict_mode = false; // Relax type annotation requirements
        // Note: warnings remain enabled even in test mode so we can test warning generation
    }
}