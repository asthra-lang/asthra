/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Import and Package Declaration Analysis Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Analysis of import, module, and use declarations
 */

#ifndef ASTHRA_SEMANTIC_IMPORTS_H
#define ASTHRA_SEMANTIC_IMPORTS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// IMPORT AND MODULE ANALYSIS
// =============================================================================

/**
 * Analyze an import declaration with optional alias
 */
bool analyze_import_declaration(SemanticAnalyzer *analyzer, ASTNode *import_decl);





/**
 * Analyze a visibility modifier
 */
bool analyze_visibility_modifier(SemanticAnalyzer *analyzer, ASTNode *visibility);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_IMPORTS_H 
