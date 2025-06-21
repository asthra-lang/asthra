/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Scope Management Module Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Scope creation, management, and navigation
 */

#ifndef ASTHRA_SEMANTIC_SCOPES_H
#define ASTHRA_SEMANTIC_SCOPES_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SCOPE MANAGEMENT
// =============================================================================

/**
 * Enter a new scope
 */
void semantic_enter_scope(SemanticAnalyzer *analyzer);

/**
 * Exit the current scope
 */
void semantic_exit_scope(SemanticAnalyzer *analyzer);

/**
 * Get the current scope ID
 */
uint32_t semantic_get_current_scope_id(SemanticAnalyzer *analyzer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_SCOPES_H 
