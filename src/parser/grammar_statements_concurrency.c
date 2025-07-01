/**
 * Asthra Programming Language Compiler - Concurrency Statement Grammar Productions
 * Concurrency statement parsing including spawn and log statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include "grammar_statements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CONCURRENCY STATEMENTS
// =============================================================================

ASTNode *parse_spawn_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Check if this is spawn or spawn_with_handle
    bool is_spawn_with_handle = match_token(parser, TOKEN_SPAWN_WITH_HANDLE);

    if (!is_spawn_with_handle && !expect_token(parser, TOKEN_SPAWN)) {
        return NULL;
    }

    if (is_spawn_with_handle) {
        advance_token(parser); // consume TOKEN_SPAWN_WITH_HANDLE
    }

    char *handle_var_name = NULL;
    char *function_name = NULL;
    ASTNode **args = NULL;
    size_t arg_count = 0;
    ASTNode *stored_call_expr = NULL;  // Store the complete call expression

    if (is_spawn_with_handle) {
        // spawn_with_handle syntax: spawn_with_handle variable = function(args);
        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected variable name after spawn_with_handle");
            return NULL;
        }

        handle_var_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);

        if (!expect_token(parser, TOKEN_ASSIGN)) {
            free(handle_var_name);
            return NULL;
        }

        // Parse a complete call expression (same as regular spawn)
        ASTNode *call_expr = parse_postfix_expr(parser);
        if (!call_expr) {
            report_error(parser, "Expected function call after '=' in spawn_with_handle");
            free(handle_var_name);
            return NULL;
        }

        // Verify that the parsed expression is a call expression
        if (call_expr->type != AST_CALL_EXPR && call_expr->type != AST_ASSOCIATED_FUNC_CALL) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Expected function call after '=' in spawn_with_handle, got AST type %d", call_expr->type);
            report_error(parser, error_msg);
            ast_free_node(call_expr);
            free(handle_var_name);
            return NULL;
        }

        // Store the complete call expression for method call support
        stored_call_expr = call_expr;
        
        // Extract function name for backward compatibility
        // TODO: Remove this once all code uses call_expr instead of function_name
        ASTNodeList *arg_list = NULL;
        
        if (call_expr->type == AST_CALL_EXPR) {
            // For regular function calls, including method calls
            if (call_expr->data.call_expr.function) {
                if (call_expr->data.call_expr.function->type == AST_IDENTIFIER) {
                    function_name = strdup(call_expr->data.call_expr.function->data.identifier.name);
                } else if (call_expr->data.call_expr.function->type == AST_FIELD_ACCESS) {
                    // Method call: obj.method - generate a descriptive name
                    ASTNode *field_access = call_expr->data.call_expr.function;
                    function_name = strdup(field_access->data.field_access.field_name);
                } else {
                    // Other complex expressions
                    function_name = strdup("complex_call");
                }
            }
            // Extract arguments for backward compatibility
            arg_list = call_expr->data.call_expr.args;
        } else if (call_expr->type == AST_ASSOCIATED_FUNC_CALL) {
            // For associated function calls (Type::method)
            char *struct_name = call_expr->data.associated_func_call.struct_name;
            char *method_name = call_expr->data.associated_func_call.function_name;
            if (struct_name && method_name) {
                size_t name_len = strlen(struct_name) + strlen(method_name) + 3; // type + "::" + method + null
                function_name = malloc(name_len);
                if (function_name) {
                    snprintf(function_name, name_len, "%s::%s", struct_name, method_name);
                }
            } else {
                function_name = strdup("associated_func_call");
            }
            // Extract arguments for backward compatibility
            arg_list = call_expr->data.associated_func_call.args;
        }

        if (!function_name) {
            // Default name if extraction fails
            function_name = strdup("unknown_call");
        }

        // Convert ASTNodeList to args array for backward compatibility
        if (arg_list && arg_list->count > 0) {
            arg_count = arg_list->count;
            args = malloc(arg_count * sizeof(ASTNode *));
            if (args) {
                for (size_t i = 0; i < arg_count; i++) {
                    args[i] = arg_list->nodes[i];
                    // Retain nodes to prevent them from being freed with call_expr
                    ast_retain_node(args[i]);
                }
            }
        }
    } else {
        // Regular spawn syntax: spawn call_expression;
        // Parse a complete call expression (supports method calls)
        ASTNode *call_expr = parse_postfix_expr(parser);
        if (!call_expr) {
            report_error(parser, "Expected function call after spawn");
            if (handle_var_name)
                free(handle_var_name);
            return NULL;
        }

        // Verify that the parsed expression is a call expression
        if (call_expr->type != AST_CALL_EXPR && call_expr->type != AST_ASSOCIATED_FUNC_CALL) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Expected function call after spawn, got AST type %d", call_expr->type);
            report_error(parser, error_msg);
            ast_free_node(call_expr);
            if (handle_var_name)
                free(handle_var_name);
            return NULL;
        }

        // Store the complete call expression for method call support
        stored_call_expr = call_expr;
        
        // Extract function name for backward compatibility
        // TODO: Remove this once all code uses call_expr instead of function_name
        ASTNodeList *arg_list = NULL;
        
        if (call_expr->type == AST_CALL_EXPR) {
            // For regular function calls, including method calls
            if (call_expr->data.call_expr.function) {
                if (call_expr->data.call_expr.function->type == AST_IDENTIFIER) {
                    function_name = strdup(call_expr->data.call_expr.function->data.identifier.name);
                } else if (call_expr->data.call_expr.function->type == AST_FIELD_ACCESS) {
                    // Method call: obj.method - generate a descriptive name
                    ASTNode *field_access = call_expr->data.call_expr.function;
                    function_name = strdup(field_access->data.field_access.field_name);
                } else {
                    // Other complex expressions
                    function_name = strdup("complex_call");
                }
            }
            // Extract arguments for backward compatibility
            arg_list = call_expr->data.call_expr.args;
        } else if (call_expr->type == AST_ASSOCIATED_FUNC_CALL) {
            // For associated function calls (Type::method)
            char *struct_name = call_expr->data.associated_func_call.struct_name;
            char *method_name = call_expr->data.associated_func_call.function_name;
            if (struct_name && method_name) {
                size_t name_len = strlen(struct_name) + strlen(method_name) + 3; // type + "::" + method + null
                function_name = malloc(name_len);
                if (function_name) {
                    snprintf(function_name, name_len, "%s::%s", struct_name, method_name);
                }
            } else {
                function_name = strdup("associated_func_call");
            }
            // Extract arguments for backward compatibility
            arg_list = call_expr->data.associated_func_call.args;
        }

        if (!function_name) {
            // Default name if extraction fails
            function_name = strdup("unknown_call");
        }

        // Convert ASTNodeList to args array for backward compatibility
        if (arg_list && arg_list->count > 0) {
            arg_count = arg_list->count;
            args = malloc(arg_count * sizeof(ASTNode *));
            if (args) {
                for (size_t i = 0; i < arg_count; i++) {
                    args[i] = arg_list->nodes[i];
                    // Retain nodes to prevent them from being freed with call_expr
                    ast_retain_node(args[i]);
                }
            }
        }
    }

    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        for (size_t i = 0; i < arg_count; i++) {
            ast_free_node(args[i]);
        }
        free(args);
        free(function_name);
        if (handle_var_name)
            free(handle_var_name);
        return NULL;
    }

    // Create appropriate AST node based on spawn type
    ASTNodeType node_type = is_spawn_with_handle ? AST_SPAWN_WITH_HANDLE_STMT : AST_SPAWN_STMT;
    ASTNode *node = ast_create_node(node_type, start_loc);
    if (!node) {
        for (size_t i = 0; i < arg_count; i++) {
            ast_free_node(args[i]);
        }
        free(args);
        free(function_name);
        if (handle_var_name)
            free(handle_var_name);
        return NULL;
    }

    if (is_spawn_with_handle) {
        node->data.spawn_with_handle_stmt.call_expr = stored_call_expr;
        node->data.spawn_with_handle_stmt.function_name = function_name;
        node->data.spawn_with_handle_stmt.handle_var_name = handle_var_name;
    } else {
        node->data.spawn_stmt.call_expr = stored_call_expr;
        node->data.spawn_stmt.function_name = function_name;
    }

    // Convert args array to ASTNodeList for both spawn types
    if (args && arg_count > 0) {
        ASTNodeList *arg_list = ast_node_list_create(arg_count);
        if (arg_list) {
            for (size_t i = 0; i < arg_count; i++) {
                ast_node_list_add(&arg_list, args[i]);
            }
        }

        if (is_spawn_with_handle) {
            node->data.spawn_with_handle_stmt.args = arg_list;
        } else {
            node->data.spawn_stmt.args = arg_list;
        }
        free(args);
    } else {
        if (is_spawn_with_handle) {
            node->data.spawn_with_handle_stmt.args = NULL;
        } else {
            node->data.spawn_stmt.args = NULL;
        }
    }

    return node;
}
