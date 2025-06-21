/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Const Declaration Analysis Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 2: Semantic Analysis Implementation
 * Interface for analyzing const declarations and compile-time evaluation
 */

#ifndef ASTHRA_SEMANTIC_CONST_DECLARATIONS_H
#define ASTHRA_SEMANTIC_CONST_DECLARATIONS_H

#include "semantic_analyzer_core.h"
#include "const_evaluator.h"
#include "semantic_types_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONST DECLARATION ANALYSIS
// =============================================================================

/**
 * Analyze a const declaration and add it to the symbol table
 * @param analyzer The semantic analyzer instance
 * @param const_decl The const declaration AST node to analyze
 * @return true if analysis succeeded, false otherwise
 */
bool analyze_const_declaration(SemanticAnalyzer *analyzer, ASTNode *const_decl);

/**
 * Validate that the evaluated const value is compatible with the declared type
 * @param analyzer The semantic analyzer instance
 * @param declared_type The declared type of the const
 * @param const_value The evaluated const value
 * @param location Source location for error reporting
 * @return true if types are compatible, false otherwise
 */
bool validate_const_type_compatibility(SemanticAnalyzer *analyzer, 
                                      TypeDescriptor *declared_type, 
                                      ConstValue *const_value, 
                                      SourceLocation location);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_CONST_DECLARATIONS_H 
