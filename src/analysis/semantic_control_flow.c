/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Control Flow Statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of control flow statements (return, if, if-let, match)
 */

#include "semantic_control_flow.h"
#include "../parser/ast_node_list.h"
#include "semantic_basic_statements.h"
#include "semantic_builtins.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_inference.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CONTROL FLOW STATEMENTS
// =============================================================================

bool analyze_return_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_RETURN_STMT) {
        return false;
    }

    ASTNode *expression = stmt->data.return_stmt.expression;

    // Expression is now always required
    if (!expression) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Return statement missing required expression. "
                              "Use 'return ();' for void returns");
        return false;
    }

    // Set expected type context for return expression analysis
    TypeDescriptor *old_expected_type = analyzer->expected_type;
    SymbolEntry *current_function = semantic_get_current_function(analyzer);
    if (current_function && current_function->type &&
        current_function->type->category == TYPE_FUNCTION) {
        analyzer->expected_type = current_function->type->data.function.return_type;
    }

    // Analyze the return expression
    bool analyze_success = semantic_analyze_expression(analyzer, expression);

    // Restore previous expected type context
    analyzer->expected_type = old_expected_type;

    if (!analyze_success) {
        return false;
    }

    // Validate return type matches function signature
    // Use the current function we already retrieved above
    if (current_function && current_function->type &&
        current_function->type->category == TYPE_FUNCTION) {
        TypeDescriptor *expected_return_type = current_function->type->data.function.return_type;
        TypeDescriptor *actual_return_type = semantic_get_expression_type(analyzer, expression);

        if (expected_return_type && actual_return_type) {
            // Special case: Functions returning Never can have "return ();" as a placeholder
            // This is technically unreachable code but allowed for development/testing
            bool is_never_function =
                (expected_return_type->category == TYPE_PRIMITIVE &&
                 expected_return_type->data.primitive.primitive_kind == PRIMITIVE_NEVER);
            bool is_unit_return =
                (actual_return_type->category == TYPE_PRIMITIVE &&
                 actual_return_type->data.primitive.primitive_kind == PRIMITIVE_VOID);

            if (is_never_function && is_unit_return) {
                // Allow "return ();" in Never-returning functions as placeholder
                // This is technically unreachable code but useful for development
                if (actual_return_type != expected_return_type) {
                    type_descriptor_release(actual_return_type);
                }
            } else if (!semantic_check_type_compatibility(analyzer, actual_return_type,
                                                          expected_return_type)) {
                semantic_report_error(
                    analyzer, SEMANTIC_ERROR_INVALID_RETURN, stmt->location,
                    "Return type mismatch: expected %s, got %s",
                    expected_return_type->name ? expected_return_type->name : "unknown",
                    actual_return_type->name ? actual_return_type->name : "unknown");
                if (actual_return_type != expected_return_type) {
                    type_descriptor_release(actual_return_type);
                }
                return false;
            } else {
                if (actual_return_type != expected_return_type) {
                    type_descriptor_release(actual_return_type);
                }
            }
            if (actual_return_type != expected_return_type) {
                type_descriptor_release(actual_return_type);
            }
        }
    }

    return true;
}

bool analyze_if_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_IF_STMT) {
        return false;
    }

    ASTNode *condition = stmt->data.if_stmt.condition;
    ASTNode *then_block = stmt->data.if_stmt.then_block;
    ASTNode *else_block = stmt->data.if_stmt.else_block;

    if (!condition) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "If statement missing condition");
        return false;
    }
    if (!then_block) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "If statement missing then block");
        return false;
    }

    // Analyze condition expression
    if (!semantic_analyze_expression(analyzer, condition)) {
        return false;
    }

    // Condition must be a boolean type
    TypeDescriptor *condition_type = semantic_get_expression_type(analyzer, condition);
    if (!condition_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, condition->location,
                              "If statement condition type could not be determined");
        return false;
    }

    // Check if it's a boolean type (primitive bool)
    bool is_bool = false;
    if (condition_type->category == TYPE_PRIMITIVE &&
        condition_type->data.primitive.primitive_kind == PRIMITIVE_BOOL) {
        is_bool = true;
    } else if (condition_type->category == TYPE_BOOL) {
        // Also accept TYPE_BOOL for compatibility
        is_bool = true;
    }

    if (!is_bool) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, condition->location,
                              "If statement condition must be a boolean expression, got %s",
                              condition_type->name ? condition_type->name : "unknown");
        type_descriptor_release(condition_type);
        return false;
    }
    type_descriptor_release(condition_type);

    // Analyze then block (enters new scope implicitly within analyze_block_statement)
    if (!analyze_block_statement(analyzer, then_block)) {
        return false;
    }

    // Analyze else block if present
    if (else_block) {
        if (!analyze_block_statement(analyzer, else_block)) {
            return false;
        }
    }

    // Check if both branches return Never
    // This is used for unreachable code detection in the parent block
    // Store this information in the analyzer's state or a separate tracking structure
    // For now, we'll use a simpler approach

    // For if statements used as statements (not expressions),
    // we don't assign a type to the statement node
    // This prevents type mismatch errors in void contexts

    return true;
}

bool analyze_if_let_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_IF_LET_STMT) {
        return false;
    }

    ASTNode *pattern = stmt->data.if_let_stmt.pattern;
    ASTNode *expression = stmt->data.if_let_stmt.expression;
    ASTNode *then_block = stmt->data.if_let_stmt.then_block;
    ASTNode *else_block = stmt->data.if_let_stmt.else_block;

    // Analyze the expression first
    if (!semantic_analyze_expression(analyzer, expression)) {
        return false;
    }

    // Get expression type for pattern compatibility checking
    TypeDescriptor *expr_type = semantic_get_expression_type(analyzer, expression);
    if (!expr_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expression->location,
                              "Cannot determine type of if-let expression");
        return false;
    }

    // Create new scope for pattern variables
    SymbolTable *pattern_scope = symbol_table_create_child(analyzer->current_scope);
    if (!pattern_scope) {
        type_descriptor_release(expr_type);
        return false;
    }

    SymbolTable *previous_scope = analyzer->current_scope;
    analyzer->current_scope = pattern_scope;

    // Analyze pattern with expression type context
    bool pattern_valid = semantic_validate_pattern_types(analyzer, pattern, expr_type);

    if (pattern_valid) {
        // Analyze then block with pattern variables in scope
        pattern_valid = semantic_analyze_statement(analyzer, then_block);
    }

    // Restore previous scope
    analyzer->current_scope = previous_scope;

    if (!pattern_valid) {
        symbol_table_destroy(pattern_scope);
        type_descriptor_release(expr_type);
        return false;
    }

    // Analyze else block (if present) in original scope
    if (else_block) {
        if (!semantic_analyze_statement(analyzer, else_block)) {
            symbol_table_destroy(pattern_scope);
            type_descriptor_release(expr_type);
            return false;
        }
    }

    type_descriptor_release(expr_type);
    return true;
}

bool analyze_match_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_MATCH_STMT) {
        return false;
    }

    ASTNode *expression = stmt->data.match_stmt.expression;
    ASTNodeList *arms = stmt->data.match_stmt.arms;

    if (!expression) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Match statement missing expression");
        return false;
    }
    if (!arms || arms->count == 0) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Match statement missing match arms");
        return false;
    }

    // Analyze match expression
    if (!semantic_analyze_expression(analyzer, expression)) {
        return false;
    }

    TypeDescriptor *expr_type = semantic_get_expression_type(analyzer, expression);
    if (!expr_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, expression->location,
                              "Could not infer type of match expression");
        return false;
    }

    bool has_catch_all = false;
    for (size_t i = 0; i < arms->count; i++) {
        ASTNode *arm = arms->nodes[i];
        if (arm->type != AST_MATCH_ARM) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, arm->location,
                                  "Invalid node type in match arms list");
            type_descriptor_release(expr_type);
            return false;
        }

        ASTNode *pattern = arm->data.match_arm.pattern;
        ASTNode *body = arm->data.match_arm.body;

        if (!pattern || !body) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, arm->location,
                                  "Match arm missing pattern or body");
            type_descriptor_release(expr_type);
            return false;
        }

        // Enter new scope for pattern variables
        semantic_enter_scope(analyzer);

        // Validate pattern against expression type
        if (!semantic_validate_pattern_types(analyzer, pattern, expr_type)) {
            semantic_exit_scope(analyzer); // Exit scope on error
            type_descriptor_release(expr_type);
            return false;
        }

        // Check for wildcard pattern
        if (pattern->type == AST_WILDCARD_PATTERN) {
            has_catch_all = true;
        }

        // Analyze arm body
        bool body_analyzed = semantic_analyze_statement(analyzer, body);

        semantic_exit_scope(analyzer); // Exit pattern scope

        if (!body_analyzed) {
            type_descriptor_release(expr_type);
            return false;
        }
    }

    // Basic exhaustiveness check: ensure a catch-all pattern exists
    if (!has_catch_all) {
        // This is a simplified check. Full exhaustiveness requires more complex analysis,
        // especially for enums or sealed types. For now, a missing wildcard is a warning.
        semantic_report_warning(
            analyzer, stmt->location,
            "Match statement is not exhaustive. Consider adding a wildcard pattern '_ => ...'");
    }

    type_descriptor_release(expr_type);
    return true;
}

// =============================================================================
// CONTROL FLOW HELPERS
// =============================================================================

bool block_returns_never(SemanticAnalyzer *analyzer, ASTNode *block) {
    if (!analyzer || !block || block->type != AST_BLOCK) {
        return false;
    }

    ASTNodeList *statements = block->data.block.statements;
    if (!statements) {
        return false; // Empty block doesn't return Never
    }

    // Check each statement in the block
    for (size_t i = 0; i < ast_node_list_size(statements); i++) {
        ASTNode *statement = ast_node_list_get(statements, i);
        if (!statement)
            continue;

        // Check if this statement returns Never
        switch (statement->type) {
        case AST_RETURN_STMT:
            // All return statements make the block Never-returning
            return true;

        case AST_EXPR_STMT: {
            // Check if the expression returns Never
            ASTNode *expr = statement->data.expr_stmt.expression;
            if (expr) {
                TypeDescriptor *expr_type = semantic_get_expression_type(analyzer, expr);
                if (expr_type) {
                    bool is_never = type_is_never(expr_type);
                    type_descriptor_release(expr_type);
                    if (is_never) {
                        return true;
                    }
                }
            }
            break;
        }

        case AST_IF_STMT: {
            // Check if both branches return Never
            ASTNode *then_block = statement->data.if_stmt.then_block;
            ASTNode *else_block = statement->data.if_stmt.else_block;

            // Only if both branches exist and both return Never
            if (then_block && else_block && block_returns_never(analyzer, then_block) &&
                block_returns_never(analyzer, else_block)) {
                return true;
            }
            break;
        }

        case AST_BLOCK: {
            // Recursively check nested blocks
            if (block_returns_never(analyzer, statement)) {
                return true;
            }
            break;
        }

        default:
            // Other statement types don't cause Never return
            break;
        }
    }

    return false; // No Never-returning statement found
}