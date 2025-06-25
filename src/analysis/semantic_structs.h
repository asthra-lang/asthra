/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Struct Declaration Analysis Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of struct declarations and field management
 */

#ifndef ASTHRA_SEMANTIC_STRUCTS_H
#define ASTHRA_SEMANTIC_STRUCTS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STRUCT DECLARATION ANALYSIS
// =============================================================================

/**
 * Analyze a struct declaration and create the corresponding type
 */
bool analyze_struct_declaration(SemanticAnalyzer *analyzer, ASTNode *struct_decl);

/**
 * Check visibility rules for field access
 */
bool check_field_visibility(SemanticAnalyzer *analyzer, const char *struct_name,
                            const char *field_name, SourceLocation location);

// =============================================================================
// PHASE 3: GENERIC STRUCT TYPE PARAMETER VALIDATION
// =============================================================================

/**
 * Validate type parameters for generic structs
 */
bool validate_struct_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params,
                                     SourceLocation location);

/**
 * Register type parameters in the current scope for field validation
 */
bool register_struct_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params,
                                     SourceLocation location);

/**
 * Validate that field types in generic structs properly reference type parameters
 */
bool validate_field_type_parameters(SemanticAnalyzer *analyzer, ASTNode *field_type_node,
                                    ASTNodeList *type_params, SourceLocation location);

/**
 * Check if a name conflicts with built-in types
 */
bool is_builtin_type_name(const char *name);

/**
 * Analyze struct literal expressions with generic type support
 */
bool analyze_struct_literal_expression(SemanticAnalyzer *analyzer, ASTNode *struct_literal);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_STRUCTS_H
