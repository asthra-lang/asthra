/**
 * Asthra Programming Language
 * Enhanced Diagnostics Utility Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "enhanced_diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to duplicate a string
char *duplicate_string(const char *str) {
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
char *escape_json_string(const char *str) {
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
const char *diagnostic_level_to_string(DiagnosticLevel level) {
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
const char *suggestion_type_to_string(SuggestionType type) {
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
const char *confidence_level_to_string(ConfidenceLevel confidence) {
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