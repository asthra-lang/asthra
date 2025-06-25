/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Loop Statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of loop statements (for, while)
 */

#include "semantic_loops.h"
#include "../parser/ast_node_list.h"
#include "semantic_basic_statements.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// LOOP STATEMENTS
// =============================================================================

bool analyze_for_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_FOR_STMT) {
        return false;
    }

    char *iterator_var = stmt->data.for_stmt.variable;
    ASTNode *iterable_expr = stmt->data.for_stmt.iterable;
    ASTNode *body = stmt->data.for_stmt.body;

    if (!iterator_var || !iterable_expr || !body) {
        semantic_report_error(
            analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
            "For statement missing iterator variable, iterable expression, or body");
        return false;
    }

    // Analyze iterable expression
    if (!semantic_analyze_expression(analyzer, iterable_expr)) {
        return false;
    }

    TypeDescriptor *iterable_type = semantic_get_expression_type(analyzer, iterable_expr);
    if (!iterable_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED,
                              iterable_expr->location,
                              "Could not infer type of iterable expression in for loop");
        return false;
    }

    // Check if iterable type is an array or slice (simplified for now)
    TypeDescriptor *element_type = NULL;
    if (iterable_type->category == TYPE_SLICE) {
        element_type = iterable_type->data.slice.element_type;
    } else {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, iterable_expr->location,
                              "For loop iterable must be a slice, but found %s",
                              iterable_type->name);
        type_descriptor_release(iterable_type);
        return false;
    }

    // Enter new scope for loop variables
    semantic_enter_scope(analyzer);

    // Declare iterator variable in the loop scope
    if (!semantic_declare_symbol(analyzer, iterator_var, SYMBOL_VARIABLE, element_type, stmt)) {
        semantic_exit_scope(analyzer); // Exit scope on error
        type_descriptor_release(iterable_type);
        return false;
    }

    // Increment loop depth to track that we're in a loop
    analyzer->loop_depth++;

    // Analyze loop body
    bool body_analyzed = analyze_block_statement(analyzer, body);

    // Decrement loop depth when exiting the loop
    analyzer->loop_depth--;

    semantic_exit_scope(analyzer); // Exit loop variable scope
    type_descriptor_release(iterable_type);

    return body_analyzed;
}

bool analyze_while_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    // While statements are not currently supported in the AST
    // This is a placeholder for future implementation
    (void)analyzer;
    (void)stmt;

    semantic_report_error(analyzer, SEMANTIC_ERROR_UNSUPPORTED_OPERATION, stmt->location,
                          "While statements are not yet implemented");
    return false;
}

// =============================================================================
// LOOP CONTROL FLOW STATEMENTS
// =============================================================================

bool analyze_break_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_BREAK_STMT) {
        return false;
    }

    // Check if we're inside a loop
    if (analyzer->loop_depth == 0) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Break statement can only be used inside a loop");
        return false;
    }

    // Break statements are valid within loops
    return true;
}

bool analyze_continue_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_CONTINUE_STMT) {
        return false;
    }

    // Check if we're inside a loop
    if (analyzer->loop_depth == 0) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Continue statement can only be used inside a loop");
        return false;
    }

    // Continue statements are valid within loops
    return true;
}