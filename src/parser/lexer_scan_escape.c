/**
 * Asthra Programming Language Compiler
 * Escape sequence processing implementation for Asthra grammar
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer_scan_escape.h"
#include "lexer_internal.h"

// =============================================================================
// ESCAPE SEQUENCE PROCESSING IMPLEMENTATION
// =============================================================================

/**
 * Helper function to process escape sequences
 * Returns the processed character, or sets error on invalid sequence
 */
char process_escape_sequence(Lexer *lexer, char escaped_char, bool *error_occurred) {
    *error_occurred = false;
    switch (escaped_char) {
    case 'n':
        return '\n';
    case 't':
        return '\t';
    case 'r':
        return '\r';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    case '"':
        return '"'; // Add quote escaping for strings
    case '0':
        return '\0';
    case '{':
        return '{'; // Allow escaping curly braces for templates
    case '}':
        return '}'; // Allow escaping curly braces for templates
    default:
        set_error(lexer, "Invalid escape sequence in string literal");
        *error_occurred = true;
        return '\0';
    }
}
