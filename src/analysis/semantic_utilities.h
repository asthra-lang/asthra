/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Utilities Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Utility functions and helpers for semantic analysis
 */

#ifndef ASTHRA_SEMANTIC_UTILITIES_H
#define ASTHRA_SEMANTIC_UTILITIES_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE CHECKING UTILITIES
// =============================================================================

/**
 * Check if two types are compatible (can type1 be used where type2 is expected)
 */
bool semantic_check_type_compatibility(SemanticAnalyzer *analyzer, TypeDescriptor *type1,
                                       TypeDescriptor *type2);

/**
 * Check if a cast from one type to another is valid
 */
bool semantic_can_cast(SemanticAnalyzer *analyzer, TypeDescriptor *from, TypeDescriptor *to);

/**
 * Get the type of an expression
 */
TypeDescriptor *semantic_get_expression_type(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Promote arithmetic types according to type promotion rules
 */
TypeDescriptor *semantic_promote_arithmetic_types(SemanticAnalyzer *analyzer,
                                                  TypeDescriptor *left_type,
                                                  TypeDescriptor *right_type);

/**
 * Check if a type is a boolean type
 */
bool semantic_is_bool_type(TypeDescriptor *type);

/**
 * Set the type of an expression
 */
void semantic_set_expression_type(SemanticAnalyzer *analyzer, ASTNode *expr, TypeDescriptor *type);

/**
 * Check if two types are exactly equal
 */
bool semantic_types_equal(TypeDescriptor *type1, TypeDescriptor *type2);

// =============================================================================
// SYMBOL UTILITIES
// =============================================================================

/**
 * Resolve an identifier to a symbol
 */
SymbolEntry *semantic_resolve_identifier(SemanticAnalyzer *analyzer, const char *name);

/**
 * Get the current function being analyzed
 */
SymbolEntry *semantic_get_current_function(SemanticAnalyzer *analyzer);

/**
 * Declare a new symbol in the current scope
 */
bool semantic_declare_symbol(SemanticAnalyzer *analyzer, const char *name, SymbolKind kind,
                             TypeDescriptor *type, ASTNode *declaration);

// =============================================================================
// C17 GENERIC IMPLEMENTATION FUNCTIONS
// =============================================================================

/**
 * Implementation function for semantic_analyze_node generic
 */
bool semantic_analyze_node_impl(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Implementation function for semantic_analyze_node generic (const version)
 */
bool semantic_analyze_node_const_impl(SemanticAnalyzer *analyzer, const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_UTILITIES_H
