/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Enhanced Diagnostics Module
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Enhanced error reporting with context, suggestions, and fixes
 */

#ifndef ASTHRA_SEMANTIC_DIAGNOSTICS_H
#define ASTHRA_SEMANTIC_DIAGNOSTICS_H

#include "semantic_analyzer_core.h"
#include "semantic_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DIAGNOSTIC TYPES
// =============================================================================

typedef enum {
    DIAG_LEVEL_ERROR,
    DIAG_LEVEL_WARNING,
    DIAG_LEVEL_NOTE,
    DIAG_LEVEL_HELP
} DiagnosticLevel;

typedef struct DiagnosticSuggestion {
    char *text;                    // Suggestion text
    char *code_snippet;            // Example code showing the fix
    struct DiagnosticSuggestion *next;
} DiagnosticSuggestion;

typedef struct DiagnosticContext {
    SourceLocation primary_location;   // Primary error location
    char *primary_label;              // Label for primary location
    
    SourceLocation *secondary_locations; // Related locations
    char **secondary_labels;           // Labels for secondary locations
    size_t secondary_count;
    
    char *source_line;               // The actual source line
    char *highlight_line;            // Underline/highlight markers
} DiagnosticContext;

typedef struct EnhancedDiagnostic {
    DiagnosticLevel level;
    SemanticErrorCode code;
    char *message;
    
    DiagnosticContext *context;
    DiagnosticSuggestion *suggestions;
    
    char *note;                      // Additional explanation
    char *help;                      // How to fix the issue
    
    struct EnhancedDiagnostic *next;
} EnhancedDiagnostic;

// =============================================================================
// ENHANCED ERROR REPORTING
// =============================================================================

/**
 * Report an error with enhanced context
 */
void semantic_report_error_enhanced(
    SemanticAnalyzer *analyzer,
    SemanticErrorCode code,
    SourceLocation location,
    const char *message,
    const char *suggestion,
    const char *note
);

/**
 * Report an error with did-you-mean suggestion
 */
void semantic_report_error_with_suggestion(
    SemanticAnalyzer *analyzer,
    SemanticErrorCode code,
    SourceLocation location,
    const char *message,
    const char *incorrect_name,
    const char **suggestions,
    size_t suggestion_count
);

/**
 * Report a type mismatch with detailed information
 */
void semantic_report_type_mismatch_detailed(
    SemanticAnalyzer *analyzer,
    SourceLocation location,
    TypeDescriptor *expected_type,
    TypeDescriptor *actual_type,
    const char *context
);

/**
 * Report undefined symbol with similar symbols
 */
void semantic_report_undefined_with_similar(
    SemanticAnalyzer *analyzer,
    SourceLocation location,
    const char *symbol_name,
    SymbolTable *scope
);

// =============================================================================
// SUGGESTION UTILITIES
// =============================================================================

/**
 * Find similar identifiers using edit distance
 */
char **find_similar_identifiers(
    const char *name,
    SymbolTable *scope,
    size_t max_suggestions,
    size_t *count
);

/**
 * Calculate Levenshtein distance between two strings
 */
size_t levenshtein_distance(const char *s1, const char *s2);

/**
 * Generate a fix-it suggestion for common mistakes
 */
char *generate_fixit_suggestion(
    SemanticErrorCode code,
    const char *context
);

// =============================================================================
// CONTEXT EXTRACTION
// =============================================================================

/**
 * Extract source line context around an error
 */
DiagnosticContext *extract_diagnostic_context(
    SourceLocation location,
    const char *label
);

/**
 * Add a secondary location to diagnostic context
 */
void add_secondary_location(
    DiagnosticContext *context,
    SourceLocation location,
    const char *label
);

// =============================================================================
// DIAGNOSTIC FORMATTING
// =============================================================================

/**
 * Format and print enhanced diagnostic
 */
void print_enhanced_diagnostic(
    const EnhancedDiagnostic *diagnostic,
    bool use_color
);

/**
 * Get ANSI color code for diagnostic level
 */
const char *get_diagnostic_color(DiagnosticLevel level);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_DIAGNOSTICS_H