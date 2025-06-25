/**
 * Asthra Programming Language Compiler
 * Lexical analyzer utility functions for Asthra grammar
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer_internal.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Additional character checking for numeric literals (Phase 2)
bool is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_binary_digit(char c) {
    return c == '0' || c == '1';
}

bool is_octal_digit(char c) {
    return c >= '0' && c <= '7';
}

char peek_char(Lexer *lexer, size_t offset) {
    size_t pos = lexer->position + offset;
    if (pos >= lexer->source_length) {
        return '\0';
    }
    return lexer->source[pos];
}

char advance_char(Lexer *lexer) {
    if (lexer->position >= lexer->source_length) {
        return '\0';
    }

    char c = lexer->source[lexer->position++];
    if (c == '\n') {
        // Unix line ending (LF) or second part of Windows line ending (CRLF)
        lexer->line++;
        lexer->column = 1;
    } else if (c == '\r') {
        // Check if this is Windows line ending (CRLF) or Mac line ending (CR)
        if (lexer->position < lexer->source_length && lexer->source[lexer->position] == '\n') {
            // Windows line ending (CRLF) - the \n will be handled in the next call
            // Don't increment line here, let the \n handle it
            lexer->column++;
        } else {
            // Mac classic line ending (CR only)
            lexer->line++;
            lexer->column = 1;
        }
    } else {
        lexer->column++;
    }
    return c;
}

SourceLocation current_location(Lexer *lexer) {
    SourceLocation loc = {.filename = (char *)lexer->filename,
                          .line = (int)lexer->line,
                          .column = (int)lexer->column,
                          .offset = (int)lexer->position};
    return loc;
}

void set_error(Lexer *lexer, const char *message) {
    if (lexer->error_message) {
        free(lexer->error_message);
    }
    lexer->error_message = strdup(message);
}

bool skip_whitespace(Lexer *lexer) {
    while (lexer->position < lexer->source_length) {
        char c = peek_char(lexer, 0);
        if (is_whitespace(c)) {
            advance_char(lexer);
        } else if (c == '/' && peek_char(lexer, 1) == '/') {
            // Skip line comment
            while (lexer->position < lexer->source_length && peek_char(lexer, 0) != '\n') {
                advance_char(lexer);
            }
        } else if (c == '/' && peek_char(lexer, 1) == '*') {
            // Skip block comment with nested comment support
            advance_char(lexer); // consume '/'
            advance_char(lexer); // consume '*'

            int nesting_level = 1;
            while (lexer->position < lexer->source_length && nesting_level > 0) {
                char current = peek_char(lexer, 0);
                char next = peek_char(lexer, 1);

                if (current == '/' && next == '*') {
                    // Found nested comment start
                    nesting_level++;
                    advance_char(lexer); // consume '/'
                    advance_char(lexer); // consume '*'
                } else if (current == '*' && next == '/') {
                    // Found comment end
                    nesting_level--;
                    advance_char(lexer); // consume '*'
                    advance_char(lexer); // consume '/'
                } else {
                    advance_char(lexer);
                }
            }

            // Check for unterminated comment
            if (nesting_level > 0) {
                set_error(lexer, "Unterminated multi-line comment");
                return false; // Error occurred
            }
        } else {
            break;
        }
    }
    return true; // Success
}
