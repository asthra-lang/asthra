/**
 * Asthra Programming Language
 * Enhanced Diagnostics Core Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "enhanced_diagnostics.h"
#include "enhanced_diagnostics_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

EnhancedDiagnostic *enhanced_diagnostic_create(const char *code, DiagnosticLevel level,
                                               const char *message) {
    if (!code || !message)
        return NULL;

    EnhancedDiagnostic *diagnostic = malloc(sizeof(EnhancedDiagnostic));
    if (!diagnostic)
        return NULL;

    // Initialize basic fields
    diagnostic->code = duplicate_string(code);
    diagnostic->level = level;
    diagnostic->message = duplicate_string(message);

    // Initialize dynamic arrays
    diagnostic->spans = NULL;
    diagnostic->span_count = 0;
    diagnostic->suggestions = NULL;
    diagnostic->suggestion_count = 0;
    diagnostic->metadata = NULL;
    diagnostic->related_info = NULL;
    diagnostic->related_count = 0;

    // Check for allocation failures
    if (!diagnostic->code || !diagnostic->message) {
        enhanced_diagnostic_destroy(diagnostic);
        return NULL;
    }

    return diagnostic;
}

void enhanced_diagnostic_destroy(EnhancedDiagnostic *diagnostic) {
    if (!diagnostic)
        return;

    // Free basic fields
    free(diagnostic->code);
    free(diagnostic->message);

    // Free spans
    if (diagnostic->spans) {
        for (size_t i = 0; i < diagnostic->span_count; i++) {
            free(diagnostic->spans[i].file_path);
            free(diagnostic->spans[i].label);
            free(diagnostic->spans[i].snippet);
        }
        free(diagnostic->spans);
    }

    // Free suggestions
    if (diagnostic->suggestions) {
        for (size_t i = 0; i < diagnostic->suggestion_count; i++) {
            free(diagnostic->suggestions[i].text);
            free(diagnostic->suggestions[i].rationale);
            free(diagnostic->suggestions[i].span.file_path);
            free(diagnostic->suggestions[i].span.label);
            free(diagnostic->suggestions[i].span.snippet);
        }
        free(diagnostic->suggestions);
    }

    // Free metadata
    if (diagnostic->metadata) {
        if (diagnostic->metadata->inferred_types) {
            for (size_t i = 0; i < diagnostic->metadata->type_count; i++) {
                free(diagnostic->metadata->inferred_types[i]);
            }
            free(diagnostic->metadata->inferred_types);
        }

        if (diagnostic->metadata->available_methods) {
            for (size_t i = 0; i < diagnostic->metadata->method_count; i++) {
                free(diagnostic->metadata->available_methods[i]);
            }
            free(diagnostic->metadata->available_methods);
        }

        if (diagnostic->metadata->similar_symbols) {
            for (size_t i = 0; i < diagnostic->metadata->symbol_count; i++) {
                free(diagnostic->metadata->similar_symbols[i]);
            }
            free(diagnostic->metadata->similar_symbols);
        }

        free(diagnostic->metadata->error_category);
        free(diagnostic->metadata);
    }

    // Free related info
    if (diagnostic->related_info) {
        for (size_t i = 0; i < diagnostic->related_count; i++) {
            free(diagnostic->related_info[i]);
        }
        free(diagnostic->related_info);
    }

    free(diagnostic);
}

bool enhanced_diagnostic_add_span(EnhancedDiagnostic *diagnostic, const DiagnosticSpan *span) {
    if (!diagnostic || !span)
        return false;

    if (!ensure_span_capacity(diagnostic))
        return false;

    DiagnosticSpan *new_span = &diagnostic->spans[diagnostic->span_count];
    new_span->start_line = span->start_line;
    new_span->start_column = span->start_column;
    new_span->end_line = span->end_line;
    new_span->end_column = span->end_column;
    new_span->file_path = duplicate_string(span->file_path);
    new_span->label = duplicate_string(span->label);
    new_span->snippet = duplicate_string(span->snippet);

    diagnostic->span_count++;
    return true;
}

bool enhanced_diagnostic_add_suggestion(EnhancedDiagnostic *diagnostic,
                                        const DiagnosticSuggestion *suggestion) {
    if (!diagnostic || !suggestion)
        return false;

    if (!ensure_suggestion_capacity(diagnostic))
        return false;

    DiagnosticSuggestion *new_suggestion = &diagnostic->suggestions[diagnostic->suggestion_count];
    new_suggestion->type = suggestion->type;
    new_suggestion->span = suggestion->span;
    new_suggestion->text = duplicate_string(suggestion->text);
    new_suggestion->confidence = suggestion->confidence;
    new_suggestion->rationale = duplicate_string(suggestion->rationale);

    // Duplicate span strings
    new_suggestion->span.file_path = duplicate_string(suggestion->span.file_path);
    new_suggestion->span.label = duplicate_string(suggestion->span.label);
    new_suggestion->span.snippet = duplicate_string(suggestion->span.snippet);

    diagnostic->suggestion_count++;
    return true;
}

bool enhanced_diagnostic_set_metadata(EnhancedDiagnostic *diagnostic,
                                      const DiagnosticMetadata *metadata) {
    if (!diagnostic || !metadata)
        return false;

    // Free existing metadata if any
    if (diagnostic->metadata) {
        // Free existing metadata arrays
        if (diagnostic->metadata->inferred_types) {
            for (size_t i = 0; i < diagnostic->metadata->type_count; i++) {
                free(diagnostic->metadata->inferred_types[i]);
            }
            free(diagnostic->metadata->inferred_types);
        }

        if (diagnostic->metadata->available_methods) {
            for (size_t i = 0; i < diagnostic->metadata->method_count; i++) {
                free(diagnostic->metadata->available_methods[i]);
            }
            free(diagnostic->metadata->available_methods);
        }

        if (diagnostic->metadata->similar_symbols) {
            for (size_t i = 0; i < diagnostic->metadata->symbol_count; i++) {
                free(diagnostic->metadata->similar_symbols[i]);
            }
            free(diagnostic->metadata->similar_symbols);
        }

        free(diagnostic->metadata->error_category);
        free(diagnostic->metadata);
    }

    // Allocate new metadata
    diagnostic->metadata = malloc(sizeof(DiagnosticMetadata));
    if (!diagnostic->metadata)
        return false;

    // Copy metadata
    diagnostic->metadata->type_count = metadata->type_count;
    diagnostic->metadata->method_count = metadata->method_count;
    diagnostic->metadata->symbol_count = metadata->symbol_count;
    diagnostic->metadata->error_category = duplicate_string(metadata->error_category);

    // Copy inferred types
    if (metadata->type_count > 0 && metadata->inferred_types) {
        diagnostic->metadata->inferred_types = malloc(sizeof(char *) * metadata->type_count);
        if (!diagnostic->metadata->inferred_types)
            return false;

        for (size_t i = 0; i < metadata->type_count; i++) {
            diagnostic->metadata->inferred_types[i] = duplicate_string(metadata->inferred_types[i]);
        }
    } else {
        diagnostic->metadata->inferred_types = NULL;
    }

    // Copy available methods
    if (metadata->method_count > 0 && metadata->available_methods) {
        diagnostic->metadata->available_methods = malloc(sizeof(char *) * metadata->method_count);
        if (!diagnostic->metadata->available_methods)
            return false;

        for (size_t i = 0; i < metadata->method_count; i++) {
            diagnostic->metadata->available_methods[i] =
                duplicate_string(metadata->available_methods[i]);
        }
    } else {
        diagnostic->metadata->available_methods = NULL;
    }

    // Copy similar symbols
    if (metadata->symbol_count > 0 && metadata->similar_symbols) {
        diagnostic->metadata->similar_symbols = malloc(sizeof(char *) * metadata->symbol_count);
        if (!diagnostic->metadata->similar_symbols)
            return false;

        for (size_t i = 0; i < metadata->symbol_count; i++) {
            diagnostic->metadata->similar_symbols[i] =
                duplicate_string(metadata->similar_symbols[i]);
        }
    } else {
        diagnostic->metadata->similar_symbols = NULL;
    }

    return true;
}