/**
 * Asthra Programming Language
 * Enhanced Diagnostics Capacity Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "enhanced_diagnostics.h"
#include <stdlib.h>

#define INITIAL_SPAN_CAPACITY 4
#define INITIAL_SUGGESTION_CAPACITY 4

// Helper function to ensure capacity for dynamic arrays
bool ensure_span_capacity(EnhancedDiagnostic *diagnostic) {
    if (!diagnostic->spans) {
        // First allocation
        diagnostic->spans = malloc(sizeof(DiagnosticSpan) * INITIAL_SPAN_CAPACITY);
        return diagnostic->spans != NULL;
    }

    // Check if we need to expand
    size_t current_capacity = INITIAL_SPAN_CAPACITY;
    // Find current capacity by doubling from initial until we exceed current count
    while (current_capacity <= diagnostic->span_count) {
        current_capacity *= 2;
    }

    if (diagnostic->span_count >= current_capacity) {
        size_t new_capacity = current_capacity * 2;
        DiagnosticSpan *new_spans =
            realloc(diagnostic->spans, sizeof(DiagnosticSpan) * new_capacity);
        if (!new_spans)
            return false;

        diagnostic->spans = new_spans;
    }
    return true;
}

bool ensure_suggestion_capacity(EnhancedDiagnostic *diagnostic) {
    if (!diagnostic->suggestions) {
        // First allocation
        diagnostic->suggestions =
            malloc(sizeof(DiagnosticSuggestion) * INITIAL_SUGGESTION_CAPACITY);
        return diagnostic->suggestions != NULL;
    }

    // Check if we need to expand
    size_t current_capacity = INITIAL_SUGGESTION_CAPACITY;
    // Find current capacity by doubling from initial until we exceed current count
    while (current_capacity <= diagnostic->suggestion_count) {
        current_capacity *= 2;
    }

    if (diagnostic->suggestion_count >= current_capacity) {
        size_t new_capacity = current_capacity * 2;
        DiagnosticSuggestion *new_suggestions =
            realloc(diagnostic->suggestions, sizeof(DiagnosticSuggestion) * new_capacity);
        if (!new_suggestions)
            return false;

        diagnostic->suggestions = new_suggestions;
    }
    return true;
}