/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Enum Declaration Analysis Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Analysis of enum declarations and variant management
 */

#ifndef ASTHRA_SEMANTIC_ENUMS_H
#define ASTHRA_SEMANTIC_ENUMS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ENUM DECLARATION ANALYSIS
// =============================================================================

/**
 * Analyze an enum declaration and create the corresponding type
 */
bool analyze_enum_declaration(SemanticAnalyzer *analyzer, ASTNode *enum_decl);

/**
 * Analyze an enum variant declaration within an enum
 */
bool analyze_enum_variant_declaration(SemanticAnalyzer *analyzer, ASTNode *variant_decl, const char *enum_name, TypeDescriptor *enum_type);

/**
 * Check visibility rules for enum variant access
 */
bool check_variant_visibility(SemanticAnalyzer *analyzer, const char *enum_name, const char *variant_name, SourceLocation location);

/**
 * Validate enum variant value (explicit integer values for C-style enums)
 */
bool validate_enum_variant_value(SemanticAnalyzer *analyzer, ASTNode *value_expr, int64_t *out_value);

/**
 * Check for duplicate variant names within an enum
 */
bool check_duplicate_variants(SemanticAnalyzer *analyzer, ASTNodeList *variants, SourceLocation enum_location);

/**
 * Validate enum type parameters (for generic enums like Option<T>)
 */
bool validate_enum_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params, SourceLocation location);

/**
 * Register type parameters in the current scope for variant type validation
 */
bool register_enum_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params, SourceLocation location);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ENUMS_H
