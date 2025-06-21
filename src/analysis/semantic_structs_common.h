/*
 * Common Header for Semantic Structs Analysis
 * Shared definitions, types, and function declarations for struct semantic analysis
 * 
 * Part of semantic_structs.c split (578 lines -> 5 focused modules)
 * Provides comprehensive struct declaration and literal analysis infrastructure
 */

#ifndef SEMANTIC_STRUCTS_COMMON_H
#define SEMANTIC_STRUCTS_COMMON_H

#include "semantic_structs.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// STRUCT DECLARATION ANALYSIS FUNCTIONS
// ============================================================================

/**
 * Analyze struct declarations with generic type support
 */
bool analyze_struct_declaration(SemanticAnalyzer *analyzer, ASTNode *struct_decl);

// ============================================================================
// FIELD VISIBILITY AND ACCESS FUNCTIONS
// ============================================================================

/**
 * Check field visibility and access permissions
 */
bool check_field_visibility(SemanticAnalyzer *analyzer, const char *struct_name, 
                           const char *field_name, SourceLocation location);

// ============================================================================
// GENERIC STRUCT TYPE PARAMETER FUNCTIONS
// ============================================================================

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

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if a name conflicts with built-in types
 */
bool is_builtin_type_name(const char *name);

// ============================================================================
// STRUCT LITERAL ANALYSIS FUNCTIONS
// ============================================================================

/**
 * Analyze struct literal expressions with generic type support
 */
bool analyze_struct_literal_expression(SemanticAnalyzer *analyzer, ASTNode *struct_literal);

#endif // SEMANTIC_STRUCTS_COMMON_H 
