/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Basic Statements
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Analysis of fundamental statement types (blocks, expression statements)
 */

#include "semantic_basic_statements.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "semantic_expressions.h"
#include "semantic_errors.h"
#include "../parser/ast_node_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// BLOCK STATEMENTS
// =============================================================================

bool analyze_block_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_BLOCK) {
        return false;
    }
    
    ASTNodeList *statements = stmt->data.block.statements;
    if (!statements) {
        return true; // Empty block is valid
    }
    
    // Enter new scope for block
    semantic_enter_scope(analyzer);
    
    bool success = true;
    bool found_never_statement = false;
    
    // Analyze each statement in the block
    for (size_t i = 0; i < ast_node_list_size(statements); i++) {
        ASTNode *statement = ast_node_list_get(statements, i);
        if (!statement) continue;
        
        // Check if we're in unreachable code
        if (found_never_statement && analyzer->config.enable_warnings) {
            semantic_report_warning(analyzer, statement->location,
                                  "Unreachable code detected");
        }
        
        if (!semantic_analyze_statement(analyzer, statement)) {
            success = false;
            break;
        }
        
        // Check if this statement returns Never type
        TypeDescriptor *stmt_type = NULL;
        
        // Handle different statement types that might return Never
        switch (statement->type) {
            case AST_RETURN_STMT:
                // Return statements make subsequent code unreachable
                found_never_statement = true;
                break;
                
            case AST_EXPR_STMT:
                // Check if the expression returns Never
                if (statement->data.expr_stmt.expression) {
                    stmt_type = semantic_get_expression_type(analyzer, 
                                                           statement->data.expr_stmt.expression);
                    if (stmt_type && type_is_never(stmt_type)) {
                        found_never_statement = true;
                    }
                }
                break;
                
            case AST_IF_STMT: {
                // For if statements, we don't track them as Never here
                // The unreachable code detection within each branch
                // happens when those blocks are analyzed separately
                break;
            }
                
            default:
                // Other statement types don't directly cause unreachability
                break;
        }
    }
    
    // Exit scope when leaving block
    semantic_exit_scope(analyzer);
    
    return success;
}

// =============================================================================
// EXPRESSION STATEMENTS
// =============================================================================

bool analyze_expression_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_EXPR_STMT) {
        return false;
    }
    
    ASTNode *expression = stmt->data.expr_stmt.expression;
    if (!expression) {
        return true; // Empty expression statement is valid
    }
    
    // Analyze the expression - its value will be discarded
    bool result = semantic_analyze_expression(analyzer, expression);
    
    // For expression statements, we don't need to check if the expression type
    // matches the function return type - the value is simply discarded
    return result;
}