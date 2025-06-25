/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Concurrency Statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of concurrency statements (spawn, spawn_with_handle, await)
 */

#include "semantic_concurrency.h"
#include "../parser/ast_node_list.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CONCURRENCY STATEMENTS (STUB IMPLEMENTATIONS)
// =============================================================================

bool analyze_spawn_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_SPAWN_STMT) {
        return false;
    }

    // Tier 1 concurrency: Basic spawn is deterministic and doesn't require annotation
    // Validate the spawned function exists and is callable
    const char *function_name = stmt->data.spawn_stmt.function_name;
    ASTNodeList *args = stmt->data.spawn_stmt.args;

    if (!function_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "spawn statement missing function name");
        return false;
    }

    SymbolEntry *function_symbol = semantic_resolve_identifier(analyzer, function_name);
    if (!function_symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, stmt->location,
                              "Undefined function '%s' in spawn statement", function_name);
        return false;
    }

    if (function_symbol->kind != SYMBOL_FUNCTION) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, stmt->location,
                              "'%s' is not a function", function_name);
        return false;
    }

    // Validate argument types match function parameters
    TypeDescriptor *function_type = function_symbol->type;

    if (function_type && function_type->category == TYPE_FUNCTION) {
        size_t expected_param_count = function_type->data.function.param_count;
        size_t actual_arg_count = ast_node_list_size(args);

        if (expected_param_count != actual_arg_count) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, stmt->location,
                                  "Function '%s' expects %zu arguments, got %zu", function_name,
                                  expected_param_count, actual_arg_count);
            return false;
        }

        // Validate each argument type
        for (size_t i = 0; i < expected_param_count; i++) {
            ASTNode *arg = ast_node_list_get(args, i);
            if (!arg)
                break;

            TypeDescriptor *expected_type = function_type->data.function.param_types[i];
            TypeDescriptor *actual_type = semantic_get_expression_type(analyzer, arg);

            if (expected_type && actual_type) {
                if (!semantic_check_type_compatibility(analyzer, expected_type, actual_type)) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, stmt->location,
                                          "Argument %zu type mismatch: expected %s, got %s", i + 1,
                                          expected_type->name ? expected_type->name : "unknown",
                                          actual_type->name ? actual_type->name : "unknown");
                    if (actual_type != expected_type) {
                        type_descriptor_release(actual_type);
                    }
                    return false;
                }
                if (actual_type != expected_type) {
                    type_descriptor_release(actual_type);
                }
            }
        }
    }

    return true;
}

bool analyze_spawn_with_handle_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_SPAWN_WITH_HANDLE_STMT) {
        return false;
    }

    char *function_name = stmt->data.spawn_with_handle_stmt.function_name;
    char *handle_var_name = stmt->data.spawn_with_handle_stmt.handle_var_name;
    ASTNodeList *args = stmt->data.spawn_with_handle_stmt.args;

    if (!function_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "spawn_with_handle statement missing function name");
        return false;
    }

    if (!handle_var_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "spawn_with_handle statement missing handle variable name");
        return false;
    }

    // Validate that the function exists and is callable
    SymbolEntry *function_symbol = symbol_table_lookup_safe(analyzer->current_scope, function_name);
    if (!function_symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, stmt->location,
                              "Undefined function '%s' in spawn_with_handle", function_name);
        return false;
    }

    // TODO: Add proper function type checking when symbol types are fully implemented
    // For now, we just validate that the symbol exists

    // Check if handle variable already exists in current scope
    SymbolEntry *existing_handle =
        symbol_table_lookup_local(analyzer->current_scope, handle_var_name);
    if (existing_handle) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, stmt->location,
                              "Handle variable '%s' already defined in current scope",
                              handle_var_name);
        return false;
    }

    // Create a symbol entry for the handle variable
    SymbolEntry *handle_symbol =
        symbol_entry_create(handle_var_name, SYMBOL_VARIABLE,
                            NULL, // TODO: Set to TaskHandle type when available
                            stmt);
    if (!handle_symbol) {
        return false;
    }

    // Insert the handle variable into the current scope
    if (!symbol_table_insert_safe(analyzer->current_scope, handle_var_name, handle_symbol)) {
        symbol_entry_destroy(handle_symbol);
        semantic_report_error(analyzer, SEMANTIC_ERROR_SYMBOL_TABLE, stmt->location,
                              "Failed to register handle variable '%s'", handle_var_name);
        return false;
    }

    // Analyze function arguments
    if (args && args->count > 0) {
        for (size_t i = 0; i < args->count; i++) {
            if (!semantic_analyze_expression(analyzer, args->nodes[i])) {
                return false;
            }
        }
    }

    return true;
}

bool analyze_await_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    (void)analyzer;
    (void)stmt;
    // TODO: Implement await statement analysis
    // Tier 1 concurrency: Basic await is deterministic and doesn't require annotation
    return true;
}