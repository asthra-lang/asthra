/**
 * Asthra Programming Language
 * Enhanced Diagnostics JSON Serialization
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "enhanced_diagnostics.h"
#include "enhanced_diagnostics_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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