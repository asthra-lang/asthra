/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Expression Analysis Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of expression nodes and field/module access
 */

#ifndef ASTHRA_SEMANTIC_EXPRESSIONS_H
#define ASTHRA_SEMANTIC_EXPRESSIONS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FIELD AND MODULE ACCESS
// =============================================================================

/**
 * Analyze field access expressions
 */
bool analyze_field_access(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Analyze regular field access on structs
 */
ASTNode *analyze_regular_field_access(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Analyze module access via aliases
 */
ASTNode *analyze_module_access(SemanticAnalyzer *analyzer, ASTNode *node);

// =============================================================================
// EXPRESSION TYPE ANALYSIS
// =============================================================================

/**
 * Analyze binary expressions
 */
bool analyze_binary_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze unary expressions
 */
bool analyze_unary_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze function call expressions
 */
bool analyze_call_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze assignment expressions
 */
bool analyze_assignment_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

// =============================================================================
// LITERAL ANALYSIS
// =============================================================================

/**
 * Analyze identifier expressions
 */
bool analyze_identifier_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze literal expressions (int, float, string, bool, char)
 */
bool analyze_literal_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze character literal expressions with type annotation enforcement
 */
bool analyze_char_literal(SemanticAnalyzer *analyzer, ASTNode *literal);

/**
 * Analyze string literal expressions including multi-line strings (Phase 4)
 */
bool analyze_string_literal(SemanticAnalyzer *analyzer, ASTNode *literal);

// =============================================================================
// ARRAY AND SLICE EXPRESSIONS
// =============================================================================

/**
 * Analyze array literal expressions
 */
bool analyze_array_literal(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze slice expressions
 */
bool analyze_slice_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze index access expressions
 */
bool analyze_index_access(SemanticAnalyzer *analyzer, ASTNode *expr);

// =============================================================================
// STRUCT AND ENUM EXPRESSIONS
// =============================================================================

/**
 * Analyze struct instantiation expressions
 */
bool analyze_struct_instantiation(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze enum variant expressions
 */
bool analyze_enum_variant(SemanticAnalyzer *analyzer, ASTNode *expr);

// =============================================================================
// TYPE CHECKING UTILITIES
// =============================================================================

/**
 * Check if an expression is an lvalue
 */
bool semantic_is_lvalue_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Check if an expression is constant
 */
bool semantic_is_constant_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Check if an expression has side effects
 */
bool semantic_has_side_effects(SemanticAnalyzer *analyzer, ASTNode *expr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_EXPRESSIONS_H
