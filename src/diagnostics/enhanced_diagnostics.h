#ifndef ASTHRA_ENHANCED_DIAGNOSTICS_H
#define ASTHRA_ENHANCED_DIAGNOSTICS_H

#include "../parser/common.h"
#include <stdbool.h>
#include <stddef.h>

// Enhanced diagnostic system
typedef enum {
    DIAGNOSTIC_ERROR,
    DIAGNOSTIC_WARNING,
    DIAGNOSTIC_HELP,
    DIAGNOSTIC_NOTE
} DiagnosticLevel;

typedef enum { SUGGESTION_INSERT, SUGGESTION_DELETE, SUGGESTION_REPLACE } SuggestionType;

typedef enum { CONFIDENCE_HIGH, CONFIDENCE_MEDIUM, CONFIDENCE_LOW } ConfidenceLevel;

typedef struct {
    size_t start_line, start_column;
    size_t end_line, end_column;
    char *file_path;
    char *label;
    char *snippet;
} DiagnosticSpan;

typedef struct {
    SuggestionType type;
    DiagnosticSpan span;
    char *text;
    ConfidenceLevel confidence;
    char *rationale;
} DiagnosticSuggestion;

typedef struct {
    char **inferred_types;
    size_t type_count;
    char **available_methods;
    size_t method_count;
    char **similar_symbols;
    size_t symbol_count;
    char *error_category;
} DiagnosticMetadata;

typedef struct {
    char *code;
    DiagnosticLevel level;
    char *message;
    DiagnosticSpan *spans;
    size_t span_count;
    DiagnosticSuggestion *suggestions;
    size_t suggestion_count;
    DiagnosticMetadata *metadata;
    char **related_info;
    size_t related_count;
} EnhancedDiagnostic;

// Core API functions
EnhancedDiagnostic *enhanced_diagnostic_create(const char *code, DiagnosticLevel level,
                                               const char *message);
void enhanced_diagnostic_destroy(EnhancedDiagnostic *diagnostic);
bool enhanced_diagnostic_add_span(EnhancedDiagnostic *diagnostic, const DiagnosticSpan *span);
bool enhanced_diagnostic_add_suggestion(EnhancedDiagnostic *diagnostic,
                                        const DiagnosticSuggestion *suggestion);
bool enhanced_diagnostic_set_metadata(EnhancedDiagnostic *diagnostic,
                                      const DiagnosticMetadata *metadata);
char *enhanced_diagnostic_to_json(EnhancedDiagnostic *diagnostic);

#endif
