/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Statement Analysis Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of statement nodes (let, blocks, if, match, etc.)
 */

#ifndef ASTHRA_SEMANTIC_STATEMENTS_H
#define ASTHRA_SEMANTIC_STATEMENTS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// VARIABLE DECLARATION STATEMENTS
// =============================================================================

/**
 * Analyze a let statement with type inference
 */
bool analyze_let_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// =============================================================================
// BLOCK STATEMENTS
// =============================================================================

/**
 * Analyze a block statement with scope management
 */
bool analyze_block_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// =============================================================================
// EXPRESSION STATEMENTS
// =============================================================================

/**
 * Analyze an expression statement
 */
bool analyze_expression_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// =============================================================================
// CONTROL FLOW STATEMENTS
// =============================================================================

/**
 * Analyze a return statement
 */
bool analyze_return_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze an if statement
 */
bool analyze_if_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze an if-let statement with pattern matching
 */
bool analyze_if_let_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze a match statement with pattern exhaustiveness
 */
bool analyze_match_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// =============================================================================
// LOOP STATEMENTS
// =============================================================================

/**
 * Analyze a for loop statement
 */
bool analyze_for_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze a while loop statement
 */
bool analyze_while_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// =============================================================================
// ASSIGNMENT STATEMENTS
// =============================================================================

/**
 * Analyze an assignment statement
 */
bool analyze_assignment_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Validate assignment expressions to ensure immutable variables aren't modified
 */
bool analyze_assignment_validation(SemanticAnalyzer *analyzer, ASTNode *assignment);

// =============================================================================
// CONCURRENCY STATEMENTS (TIER 1)
// =============================================================================

/**
 * Analyze a spawn statement (Tier 1 - deterministic, no annotation required)
 */
bool analyze_spawn_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze a spawn_with_handle statement (Tier 1 - deterministic, no annotation required)
 */
bool analyze_spawn_with_handle_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze an await statement (Tier 1 - deterministic, no annotation required)
 */
bool analyze_await_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// =============================================================================
// UNSAFE BLOCKS
// =============================================================================

/**
 * Analyze an unsafe block statement with safety validation
 */
bool analyze_unsafe_block_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_STATEMENTS_H
