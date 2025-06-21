/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Error Handling Module Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Error reporting, management, and utilities
 */

#ifndef ASTHRA_SEMANTIC_ERRORS_H
#define ASTHRA_SEMANTIC_ERRORS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ERROR REPORTING
// =============================================================================

/**
 * Report a semantic error
 */
void semantic_report_error(SemanticAnalyzer *analyzer, 
                          SemanticErrorCode code, 
                          SourceLocation location, 
                          const char *format, ...);

/**
 * Clear all errors from the analyzer
 */
void semantic_clear_errors(SemanticAnalyzer *analyzer);

/**
 * Get the list of errors
 */
const SemanticError *semantic_get_errors(const SemanticAnalyzer *analyzer);

/**
 * Get the number of errors
 */
size_t semantic_get_error_count(const SemanticAnalyzer *analyzer);

// =============================================================================
// ERROR UTILITIES
// =============================================================================

/**
 * Get the name of an error code
 */
const char *semantic_error_code_name(SemanticErrorCode code);

/**
 * Get the description of an error code
 */
const char *semantic_error_code_description(SemanticErrorCode code);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ERRORS_H 
