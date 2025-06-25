/**
 * Asthra Programming Language
 * Enhanced Diagnostics Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "enhanced_diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SPAN_CAPACITY 4
#define INITIAL_SUGGESTION_CAPACITY 4
#define INITIAL_RELATED_CAPACITY 4

// Helper function to duplicate a string
static char *duplicate_string(const char *str) {
    if (!str)
        return NULL;
    size_t len = strlen(str);
    char *dup = malloc(len + 1);
    if (dup) {
        strcpy(dup, str);
    }
    return dup;
}

// Helper function to escape JSON strings
static char *escape_json_string(const char *str) {
    if (!str)
        return duplicate_string("null");

    size_t len = strlen(str);
    size_t escaped_len = len * 2 + 3; // Worst case: every char escaped + quotes + null
    char *escaped = malloc(escaped_len);
    if (!escaped)
        return NULL;

    char *p = escaped;
    *p++ = '"';

    for (const char *s = str; *s; s++) {
        switch (*s) {
        case '"':
            *p++ = '\\';
            *p++ = '"';
            break;
        case '\\':
            *p++ = '\\';
            *p++ = '\\';
            break;
        case '\n':
            *p++ = '\\';
            *p++ = 'n';
            break;
        case '\r':
            *p++ = '\\';
            *p++ = 'r';
            break;
        case '\t':
            *p++ = '\\';
            *p++ = 't';
            break;
        default:
            *p++ = *s;
            break;
        }
    }

    *p++ = '"';
    *p = '\0';

    return escaped;
}

// Helper function to convert diagnostic level to string
static const char *diagnostic_level_to_string(DiagnosticLevel level) {
    switch (level) {
    case DIAGNOSTIC_ERROR:
        return "error";
    case DIAGNOSTIC_WARNING:
        return "warning";
    case DIAGNOSTIC_HELP:
        return "help";
    case DIAGNOSTIC_NOTE:
        return "note";
    default:
        return "unknown";
    }
}

// Helper function to convert suggestion type to string
static const char *suggestion_type_to_string(SuggestionType type) {
    switch (type) {
    case SUGGESTION_INSERT:
        return "insert";
    case SUGGESTION_DELETE:
        return "delete";
    case SUGGESTION_REPLACE:
        return "replace";
    default:
        return "unknown";
    }
}

// Helper function to convert confidence level to string
static const char *confidence_level_to_string(ConfidenceLevel confidence) {
    switch (confidence) {
    case CONFIDENCE_HIGH:
        return "high";
    case CONFIDENCE_MEDIUM:
        return "medium";
    case CONFIDENCE_LOW:
        return "low";
    default:
        return "unknown";
    }
}

// Helper function to ensure capacity for dynamic arrays
static bool ensure_span_capacity(EnhancedDiagnostic *diagnostic) {
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

static bool ensure_suggestion_capacity(EnhancedDiagnostic *diagnostic) {
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

char *enhanced_diagnostic_to_json(EnhancedDiagnostic *diagnostic) {
    if (!diagnostic)
        return NULL;

    // Estimate buffer size (conservative)
    size_t buffer_size = 4096;
    char *json = malloc(buffer_size);
    if (!json)
        return NULL;

    char *p = json;
    size_t remaining = buffer_size;

// Helper macro for safe string appending
#define APPEND_STR(str)                                                                            \
    do {                                                                                           \
        size_t len = strlen(str);                                                                  \
        if (len >= remaining) {                                                                    \
            buffer_size *= 2;                                                                      \
            size_t offset = (size_t)(p - json);                                                    \
            char *new_json = realloc(json, buffer_size);                                           \
            if (!new_json) {                                                                       \
                free(json);                                                                        \
                return NULL;                                                                       \
            }                                                                                      \
            json = new_json;                                                                       \
            p = json + offset;                                                                     \
            remaining = buffer_size - offset;                                                      \
        }                                                                                          \
        strcpy(p, str);                                                                            \
        p += len;                                                                                  \
        remaining -= len;                                                                          \
    } while (0)

    // Start JSON object
    APPEND_STR("{\n");

    // Basic diagnostic information
    char *escaped_code = escape_json_string(diagnostic->code);
    char *escaped_message = escape_json_string(diagnostic->message);

    char temp[512];
    snprintf(temp, sizeof(temp), "  \"code\": %s,\n", escaped_code ? escaped_code : "null");
    APPEND_STR(temp);

    snprintf(temp, sizeof(temp), "  \"level\": \"%s\",\n",
             diagnostic_level_to_string(diagnostic->level));
    APPEND_STR(temp);

    snprintf(temp, sizeof(temp), "  \"message\": %s", escaped_message ? escaped_message : "null");
    APPEND_STR(temp);

    free(escaped_code);
    free(escaped_message);

    // Add spans if present
    if (diagnostic->span_count > 0) {
        APPEND_STR(",\n  \"spans\": [\n");
        for (size_t i = 0; i < diagnostic->span_count; i++) {
            const DiagnosticSpan *span = &diagnostic->spans[i];

            APPEND_STR("    {\n");
            snprintf(temp, sizeof(temp), "      \"start_line\": %zu,\n", span->start_line);
            APPEND_STR(temp);
            snprintf(temp, sizeof(temp), "      \"start_column\": %zu,\n", span->start_column);
            APPEND_STR(temp);
            snprintf(temp, sizeof(temp), "      \"end_line\": %zu,\n", span->end_line);
            APPEND_STR(temp);
            snprintf(temp, sizeof(temp), "      \"end_column\": %zu", span->end_column);
            APPEND_STR(temp);

            if (span->file_path) {
                char *escaped_path = escape_json_string(span->file_path);
                snprintf(temp, sizeof(temp), ",\n      \"file_path\": %s",
                         escaped_path ? escaped_path : "null");
                APPEND_STR(temp);
                free(escaped_path);
            }

            if (span->label) {
                char *escaped_label = escape_json_string(span->label);
                snprintf(temp, sizeof(temp), ",\n      \"label\": %s",
                         escaped_label ? escaped_label : "null");
                APPEND_STR(temp);
                free(escaped_label);
            }

            APPEND_STR("\n    }");
            if (i < diagnostic->span_count - 1) {
                APPEND_STR(",");
            }
            APPEND_STR("\n");
        }
        APPEND_STR("  ]");
    }

    // Add suggestions if present
    if (diagnostic->suggestion_count > 0) {
        APPEND_STR(",\n  \"suggestions\": [\n");
        for (size_t i = 0; i < diagnostic->suggestion_count; i++) {
            const DiagnosticSuggestion *suggestion = &diagnostic->suggestions[i];

            APPEND_STR("    {\n");
            snprintf(temp, sizeof(temp), "      \"type\": \"%s\",\n",
                     suggestion_type_to_string(suggestion->type));
            APPEND_STR(temp);

            char *escaped_text = escape_json_string(suggestion->text);
            snprintf(temp, sizeof(temp), "      \"text\": %s,\n",
                     escaped_text ? escaped_text : "null");
            APPEND_STR(temp);
            free(escaped_text);

            snprintf(temp, sizeof(temp), "      \"confidence\": \"%s\"",
                     confidence_level_to_string(suggestion->confidence));
            APPEND_STR(temp);

            if (suggestion->rationale) {
                char *escaped_rationale = escape_json_string(suggestion->rationale);
                snprintf(temp, sizeof(temp), ",\n      \"rationale\": %s",
                         escaped_rationale ? escaped_rationale : "null");
                APPEND_STR(temp);
                free(escaped_rationale);
            }

            APPEND_STR("\n    }");
            if (i < diagnostic->suggestion_count - 1) {
                APPEND_STR(",");
            }
            APPEND_STR("\n");
        }
        APPEND_STR("  ]");
    }

    // Add metadata if present
    if (diagnostic->metadata) {
        APPEND_STR(",\n  \"metadata\": {\n");

        if (diagnostic->metadata->error_category) {
            char *escaped_category = escape_json_string(diagnostic->metadata->error_category);
            snprintf(temp, sizeof(temp), "    \"error_category\": %s",
                     escaped_category ? escaped_category : "null");
            APPEND_STR(temp);
            free(escaped_category);
        }

        if (diagnostic->metadata->symbol_count > 0) {
            if (diagnostic->metadata->error_category) {
                APPEND_STR(",\n");
            }
            APPEND_STR("    \"similar_symbols\": [");
            for (size_t i = 0; i < diagnostic->metadata->symbol_count; i++) {
                char *escaped_symbol = escape_json_string(diagnostic->metadata->similar_symbols[i]);
                snprintf(temp, sizeof(temp), "%s%s", i > 0 ? ", " : "",
                         escaped_symbol ? escaped_symbol : "null");
                APPEND_STR(temp);
                free(escaped_symbol);
            }
            APPEND_STR("]");
        }

        APPEND_STR("\n  }");
    }

    APPEND_STR("\n}");

#undef APPEND_STR

    return json;
}
