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
#include "semantic_expression_utils.h"
#include "semantic_expressions.h"
#include "semantic_symbols.h"
#include "semantic_type_creation.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "symbol_utilities.h"
#include "type_info.h"
#include "type_info_integration.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CONCURRENCY STATEMENTS (STUB IMPLEMENTATIONS)
// =============================================================================

bool analyze_spawn_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_SPAWN_STMT) {
        return false;
    }

    // Check if we have the new call_expr field (supports method calls)
    if (stmt->data.spawn_stmt.call_expr) {
        // New path: Analyze the full call expression
        ASTNode *call_expr = stmt->data.spawn_stmt.call_expr;

        // Analyze the call expression
        bool call_result = semantic_analyze_expression(analyzer, call_expr);
        if (!call_result) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                                  "Invalid expression in spawn statement");
            return false;
        }

        // Verify it's a function call that returns void
        if (!call_expr->type_info) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, stmt->location,
                                  "Spawned function call has no type information");
            return false;
        }

        TypeInfo *call_type_info = call_expr->type_info;
        TypeDescriptor *call_type = type_descriptor_from_type_info(call_type_info);

        // Check if it's void type
        bool is_void = false;
        if (call_type && call_type->category == TYPE_PRIMITIVE &&
            call_type->data.primitive.primitive_kind == PRIMITIVE_VOID) {
            is_void = true;
        }

        if (!is_void) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, stmt->location,
                                  "Spawned function must return void");
            if (call_type) {
                type_descriptor_release(call_type);
            }
            return false;
        }

        if (call_type) {
            type_descriptor_release(call_type);
        }

        // Set the statement type info to void
        TypeDescriptor *void_type = get_builtin_type("void");
        stmt->type_info = type_info_from_type_descriptor(void_type);
        return true;
    }

    // Legacy path: Use function_name and args
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

    char *handle_var_name = stmt->data.spawn_with_handle_stmt.handle_var_name;

    if (!handle_var_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "spawn_with_handle statement missing handle variable name");
        return false;
    }

    // Check if we have the new call_expr field (supports method calls)
    if (stmt->data.spawn_with_handle_stmt.call_expr) {
        // New path: Analyze the full call expression
        ASTNode *call_expr = stmt->data.spawn_with_handle_stmt.call_expr;

        // Analyze the call expression
        bool call_result = semantic_analyze_expression(analyzer, call_expr);
        if (!call_result) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                                  "Invalid expression in spawn_with_handle statement");
            return false;
        }

        // Get the function's return type to create TaskHandle<T>
        TypeDescriptor *return_type = NULL;
        if (call_expr->type_info) {
            return_type = type_descriptor_from_type_info(call_expr->type_info);
        }

        TypeDescriptor *handle_type = NULL;
        if (return_type) {
            // Create proper TaskHandle<return_type> type
            handle_type = type_descriptor_create_task_handle(return_type);
            type_descriptor_release(return_type);
        }

        if (!handle_type) {
            TypeDescriptor *void_type = get_builtin_type("void");
            handle_type = type_descriptor_create_task_handle(void_type);
        }

        // Check if handle variable already exists in current scope
        SymbolEntry *existing_handle =
            symbol_table_lookup_local(analyzer->current_scope, handle_var_name);
        if (existing_handle) {
            // Variable already exists - verify it has the correct type
            if (!semantic_check_type_compatibility(analyzer, handle_type, existing_handle->type)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, stmt->location,
                                      "Handle variable '%s' exists but has incompatible type",
                                      handle_var_name);
                type_descriptor_release(handle_type);
                return false;
            }
            // Variable exists with correct type - just use it
            type_descriptor_release(handle_type);
        } else {
            // Create a new variable for the handle
            bool declare_result = semantic_declare_symbol(analyzer, handle_var_name,
                                                          SYMBOL_VARIABLE, handle_type, stmt);
            if (!declare_result) {
                type_descriptor_release(handle_type);
                return false;
            }
            type_descriptor_release(handle_type);
        }

        // Set the statement type info to void
        TypeDescriptor *void_type = get_builtin_type("void");
        stmt->type_info = type_info_from_type_descriptor(void_type);
        return true;
    }

    // Legacy path: Use function_name and args
    char *function_name = stmt->data.spawn_with_handle_stmt.function_name;
    ASTNodeList *args = stmt->data.spawn_with_handle_stmt.args;

    if (!function_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "spawn_with_handle statement missing function name");
        return false;
    }

    // Validate that the function exists and is callable
    SymbolEntry *function_symbol = symbol_table_lookup_safe(analyzer->current_scope, function_name);
    if (!function_symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, stmt->location,
                              "Undefined function '%s' in spawn_with_handle", function_name);
        return false;
    }

    // Get the function's return type to create TaskHandle<T>
    TypeDescriptor *handle_type = NULL;
    if (function_symbol->type && function_symbol->type->category == TYPE_FUNCTION) {
        TypeDescriptor *return_type = function_symbol->type->data.function.return_type;
        if (return_type) {
            // Create proper TaskHandle<return_type> type
            handle_type = type_descriptor_create_task_handle(return_type);
        }
    }

    // Check if handle variable already exists in current scope
    SymbolEntry *existing_handle =
        symbol_table_lookup_local(analyzer->current_scope, handle_var_name);
    if (existing_handle) {
        // Variable already exists - verify it has the correct type
        if (!handle_type) {
            // Create default TaskHandle<void> type if we couldn't determine the function's return
            // type
            TypeDescriptor *void_type = get_builtin_type("void");
            handle_type = type_descriptor_create_task_handle(void_type);
        }

        if (!semantic_check_type_compatibility(analyzer, handle_type, existing_handle->type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, stmt->location,
                                  "Handle variable '%s' exists but has incompatible type",
                                  handle_var_name);
            if (handle_type) {
                type_descriptor_release(handle_type);
            }
            return false;
        }
        // Variable exists with correct type - just use it
        if (handle_type) {
            type_descriptor_release(handle_type);
        }

        // Still need to analyze the arguments
        if (args) {
            for (size_t i = 0; i < ast_node_list_size(args); i++) {
                ASTNode *arg = ast_node_list_get(args, i);
                if (!semantic_analyze_expression(analyzer, arg)) {
                    return false;
                }
            }
        }

        // Set the statement type info to void
        TypeDescriptor *void_type = get_builtin_type("void");
        stmt->type_info = type_info_from_type_descriptor(void_type);
        return true;
    }

    // Create a symbol entry for the handle variable
    SymbolEntry *handle_symbol = symbol_entry_create(handle_var_name, SYMBOL_VARIABLE,
                                                     handle_type, // Use the TaskHandle<T> type
                                                     stmt);
    if (!handle_symbol) {
        if (handle_type) {
            type_descriptor_release(handle_type);
        }
        return false;
    }

    // Insert the handle variable into the current scope
    if (!symbol_table_insert_safe(analyzer->current_scope, handle_var_name, handle_symbol)) {
        symbol_entry_destroy(handle_symbol);
        if (handle_type) {
            type_descriptor_release(handle_type);
        }
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
    if (!analyzer || !stmt) {
        return false;
    }

    // An await statement is an await expression used in statement context
    // We delegate to the expression analysis which handles all the validation
    if (stmt->type != AST_AWAIT_EXPR) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Expected await expression in await statement");
        return false;
    }

    // Analyze the await expression - this handles all the semantic validation
    // including checking that the handle is a TaskHandle<T> type
    bool result = analyze_await_expression(analyzer, stmt);

    if (result) {
        // For statement context, we don't need to do anything special with the result type
        // The value returned by await is simply discarded in statement context
        // This is similar to how function calls can be used as statements

        // Note: Tier 1 concurrency features like await are deterministic and don't require
        // additional annotations or special handling beyond type checking
    }

    return result;
}

bool analyze_await_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_AWAIT_EXPR) {
        return false;
    }

    ASTNode *handle_expr = expr->data.await_expr.task_handle_expr;
    if (!handle_expr) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, expr->location,
                              "await expression missing handle");
        return false;
    }

    // Analyze the handle expression
    if (!semantic_analyze_expression(analyzer, handle_expr)) {
        return false;
    }

    // Get the type of the handle expression
    TypeDescriptor *handle_type = semantic_get_expression_type(analyzer, handle_expr);
    if (!handle_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, handle_expr->location,
                              "Cannot determine type of await handle");
        return false;
    }

    // Check that the handle expression is a TaskHandle<T> and extract T
    if (handle_expr->type == AST_IDENTIFIER) {
        // Look up the handle variable
        const char *handle_name = handle_expr->data.identifier.name;
        SymbolEntry *handle_symbol = semantic_resolve_identifier(analyzer, handle_name);

        if (!handle_symbol) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, handle_expr->location,
                                  "Undefined handle '%s'", handle_name);
            type_descriptor_release(handle_type);
            return false;
        }

        // Check if this is actually a task handle type
        if (handle_symbol->kind != SYMBOL_VARIABLE || !is_task_handle_type(handle_symbol->type)) {
            const char *type_name = "unknown";
            if (handle_symbol->type) {
                if (handle_symbol->type->name) {
                    type_name = handle_symbol->type->name;
                } else {
                    // Type exists but name is NULL - show category
                    static char type_desc[128];
                    snprintf(type_desc, sizeof(type_desc), "(unnamed type, category=%d)",
                             handle_symbol->type->category);
                    type_name = type_desc;
                }
            }
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, handle_expr->location,
                                  "await expected TaskHandle<T> from spawn_with_handle, got %s",
                                  type_name);
            type_descriptor_release(handle_type);
            return false;
        }

        // Extract the result type T from TaskHandle<T>
        TypeDescriptor *result_type = get_task_handle_result_type(handle_symbol->type);
        if (result_type) {
            // The await expression has the type T from TaskHandle<T>
            expr->type_info = type_info_from_type_descriptor(result_type);
            if (!expr->type_info) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                      "Failed to create type info for await expression");
                type_descriptor_release(handle_type);
                return false;
            }
        } else {
            // This shouldn't happen if the TaskHandle<T> was created properly
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                  "Invalid TaskHandle<T> type - missing result type");
            type_descriptor_release(handle_type);
            return false;
        }
    } else {
        // For non-identifier expressions, we should reject them as invalid await targets
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, handle_expr->location,
                              "await expected TaskHandle<T> identifier");
        type_descriptor_release(handle_type);
        return false;
    }

    type_descriptor_release(handle_type);
    return true;
}