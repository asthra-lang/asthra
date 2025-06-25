/**
 * Asthra Programming Language Compiler
 * Core token scanning functions for Asthra grammar
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer_scan_core.h"
#include "lexer_internal.h"
#include "lexer_scan_escape.h"
#include "lexer_scan_numbers.h"
#include "lexer_scan_strings.h"
#include "lexer_scan_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token scan_number(Lexer *lexer) {
    SourceLocation start_loc = current_location(lexer);
    size_t start_pos = lexer->position;

    // Check for prefix to determine literal type
    if (peek_char(lexer, 0) == '0' && lexer->position + 1 < lexer->source_length) {
        char second = peek_char(lexer, 1);

        if (second == 'x' || second == 'X') {
            // Hexadecimal literal
            advance_char(lexer);
            advance_char(lexer); // Skip "0x"
            return scan_hex_literal(lexer, start_loc, start_pos);
        } else if (second == 'b' || second == 'B') {
            // Binary literal
            advance_char(lexer);
            advance_char(lexer); // Skip "0b"
            return scan_binary_literal(lexer, start_loc, start_pos);
        } else if (second == 'o') {
            // Octal literal (explicit)
            advance_char(lexer);
            advance_char(lexer); // Skip "0o"
            return scan_octal_literal(lexer, start_loc, start_pos);
        }
    }

    // Default to decimal scanning
    return scan_decimal_literal(lexer, start_loc, start_pos);
}

Token scan_string(Lexer *lexer) {
    SourceLocation start_loc = current_location(lexer);

    // Check for multi-line string patterns first
    if (is_multiline_string_start(lexer)) {
        // Determine if it's raw or processed multi-line string
        if (peek_char(lexer, 0) == 'r' && peek_char(lexer, 1) == '"' &&
            peek_char(lexer, 2) == '"' && peek_char(lexer, 3) == '"') {
            // Raw multi-line string: r"""content"""
            return scan_multiline_raw_string(lexer);
        } else if (peek_char(lexer, 0) == '"' && peek_char(lexer, 1) == '"' &&
                   peek_char(lexer, 2) == '"') {
            // Processed multi-line string: """content"""
            return scan_multiline_processed_string(lexer);
        }
    }

    // Regular string handling (existing logic)
    advance_char(lexer); // consume opening quote

    // Regular string - process escape sequences
    char *value = malloc(LEXER_SCAN_INITIAL_STRING_BUFFER_SIZE);
    size_t value_capacity = LEXER_SCAN_INITIAL_STRING_BUFFER_SIZE;
    size_t value_length = 0;

    while (lexer->position < lexer->source_length && peek_char(lexer, 0) != '"') {
        char c = peek_char(lexer, 0);
        if (c == '\\') {
            advance_char(lexer); // consume backslash
            if (lexer->position >= lexer->source_length) {
                set_error(lexer, "Unterminated escape sequence in string literal");
                free(value);
                return token_create(TOKEN_ERROR, start_loc);
            }

            char escaped = peek_char(lexer, 0);
            advance_char(lexer); // consume escaped character

            bool error_occurred;
            char processed = process_escape_sequence(lexer, escaped, &error_occurred);
            if (error_occurred) {
                free(value);
                return token_create(TOKEN_ERROR, start_loc);
            }

            // Resize buffer if needed
            if (value_length + 1 >= value_capacity) {
                value_capacity *= 2;
                value = realloc(value, value_capacity);
            }

            value[value_length++] = processed;
        } else {
            // Resize buffer if needed
            if (value_length + 1 >= value_capacity) {
                value_capacity *= 2;
                value = realloc(value, value_capacity);
            }

            value[value_length++] = advance_char(lexer);
        }
    }

    if (peek_char(lexer, 0) != '"') {
        set_error(lexer, "Unterminated string literal");
        free(value);
        return token_create(TOKEN_ERROR, start_loc);
    }

    advance_char(lexer); // consume closing quote
    value[value_length] = '\0';

    Token token = token_create(TOKEN_STRING, start_loc);
    token.data.string.value = value;
    token.data.string.length = value_length;

    return token;
}

Token scan_identifier(Lexer *lexer) {
    SourceLocation start_loc = current_location(lexer);
    size_t start_pos = lexer->position;

    while (lexer->position < lexer->source_length && is_alnum(peek_char(lexer, 0))) {
        advance_char(lexer);
    }

    size_t length = lexer->position - start_pos;
    char *name = malloc(length + 1);
    memcpy(name, lexer->source + start_pos, length);
    name[length] = '\0';

    // Check if it's a keyword
    TokenType token_type = keyword_lookup(name, length);

    Token token = token_create(token_type, start_loc);
    if (token_type == TOKEN_IDENTIFIER) {
        token.data.identifier.name = name;
        token.data.identifier.length = length;
    } else {
        free(name); // Keywords don't need to store the name
    }

    return token;
}

Token scan_character(Lexer *lexer) {
    SourceLocation start_loc = current_location(lexer);
    advance_char(lexer); // consume opening quote

    if (lexer->position >= lexer->source_length) {
        set_error(lexer, "Unterminated character literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    char c = peek_char(lexer, 0);
    uint32_t char_value;

    if (c == '\\') {
        // Handle escape sequences
        advance_char(lexer); // consume backslash
        if (lexer->position >= lexer->source_length) {
            set_error(lexer, "Unterminated escape sequence in character literal");
            return token_create(TOKEN_ERROR, start_loc);
        }

        char escaped = peek_char(lexer, 0);
        advance_char(lexer); // consume escaped character

        bool error_occurred;
        char_value = process_escape_sequence(lexer, escaped, &error_occurred);
        if (error_occurred) {
            return token_create(TOKEN_ERROR, start_loc);
        }
    } else if (c == '\'') {
        set_error(lexer, "Empty character literal");
        return token_create(TOKEN_ERROR, start_loc);
    } else {
        char_value = (uint32_t)advance_char(lexer);
    }

    if (peek_char(lexer, 0) != '\'') {
        set_error(lexer, "Unterminated character literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    advance_char(lexer); // consume closing quote

    Token token = token_create(TOKEN_CHAR, start_loc);
    token.data.character.value = char_value;
    return token;
}

Token scan_token(Lexer *lexer) {
    if (!skip_whitespace(lexer)) {
        // Error occurred during whitespace/comment processing
        return token_create(TOKEN_ERROR, current_location(lexer));
    }

    if (lexer->position >= lexer->source_length) {
        return token_create(TOKEN_EOF, current_location(lexer));
    }

    SourceLocation start_loc = current_location(lexer);
    char c = peek_char(lexer, 0);

    // Numbers (including those starting with a decimal point)
    if (is_digit(c)) {
        return scan_number(lexer);
    }

    // Numbers starting with decimal point (like .5, .e5)
    // But NOT tuple element access (like pair.0)
    if (c == '.' && lexer->position + 1 < lexer->source_length) {
        char after_dot = peek_char(lexer, 1);

        // Check if this might be tuple element access by looking at context
        // Tuple element access would be: identifier.digit or ).digit
        bool could_be_tuple_access = false;
        if (is_digit(after_dot)) {
            // Look back to see if we have an identifier or closing paren before the dot
            // This is a heuristic - we check if the previous non-whitespace character
            // could end an expression that supports field/element access
            if (lexer->position > 0) {
                size_t check_pos = lexer->position - 1;
                // Skip back over any whitespace
                while (check_pos > 0 && is_whitespace(lexer->source[check_pos])) {
                    check_pos--;
                }
                char prev_char = lexer->source[check_pos];
                // Check if previous character could end an identifier, array access, function call,
                // or tuple index
                could_be_tuple_access =
                    is_alnum(prev_char) || prev_char == ')' || prev_char == ']' || prev_char == '}';

                // Special case: if the previous character is a digit, check if we just parsed a
                // tuple index by looking further back for a dot preceded by an identifier-like
                // character
                if (is_digit(prev_char) && check_pos > 0) {
                    // Look for pattern like "identifier.0" where we're now at ".1"
                    size_t dot_check = check_pos;
                    // Skip back over all digits
                    while (dot_check > 0 && is_digit(lexer->source[dot_check])) {
                        dot_check--;
                    }
                    // Check if we have a dot before the digits
                    if (dot_check > 0 && lexer->source[dot_check] == '.') {
                        // Found a dot, now check what's before it
                        if (dot_check > 0) {
                            size_t before_dot = dot_check - 1;
                            // Skip whitespace before the dot
                            while (before_dot > 0 && is_whitespace(lexer->source[before_dot])) {
                                before_dot--;
                            }
                            char before_dot_char = lexer->source[before_dot];
                            // If what's before the dot could be a valid expression, this is tuple
                            // access
                            if (is_alnum(before_dot_char) || before_dot_char == ')' ||
                                before_dot_char == ']' || before_dot_char == '}' ||
                                is_digit(before_dot_char)) {
                                could_be_tuple_access = true;
                            }
                        }
                    }
                }
            }

            if (!could_be_tuple_access) {
                // Definitely a decimal number like .5 or .123
                return scan_number(lexer);
            }
            // Otherwise, treat as DOT token and let parser handle the number
        } else if (after_dot == 'e' || after_dot == 'E') {
            // Check if this is actually scientific notation like .e5 or .E-3
            // Look ahead to see if there's a valid exponent after e/E
            if (lexer->position + 2 < lexer->source_length) {
                char after_e = peek_char(lexer, 2);
                // Valid scientific notation: .e5, .E-3, .e+2, etc.
                if (is_digit(after_e) ||
                    ((after_e == '+' || after_e == '-') &&
                     lexer->position + 3 < lexer->source_length && is_digit(peek_char(lexer, 3)))) {
                    return scan_number(lexer);
                }
            }
            // Otherwise, .E or .e followed by non-numeric is just DOT + identifier
            // Fall through to normal token processing
        }
    }

    // Identifiers and keywords
    if (is_alpha(c)) {
        // Check for raw multi-line strings first (r""")
        if (c == 'r' && is_multiline_string_start(lexer)) {
            return scan_string(lexer);
        }
        return scan_identifier(lexer);
    }

    // Strings (including multi-line strings starting with """)
    if (c == '"') {
        return scan_string(lexer);
    }

    // Characters
    if (c == '\'') {
        return scan_character(lexer);
    }

    // Two-character operators
    char next = peek_char(lexer, 1);
    if (c == '=' && next == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_EQUAL, start_loc);
    }
    if (c == '!' && next == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_NOT_EQUAL, start_loc);
    }
    if (c == '<' && next == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_LESS_EQUAL, start_loc);
    }
    if (c == '>' && next == '=') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_GREATER_EQUAL, start_loc);
    }
    if (c == '&' && next == '&') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_LOGICAL_AND, start_loc);
    }
    if (c == '|' && next == '|') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_LOGICAL_OR, start_loc);
    }
    if (c == '-' && next == '>') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_ARROW, start_loc);
    }
    if (c == '=' && next == '>') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_FAT_ARROW, start_loc);
    }
    if (c == '<' && next == '<') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_LEFT_SHIFT, start_loc);
    }
    if (c == '>' && next == '>') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_RIGHT_SHIFT, start_loc);
    }
    if (c == ':' && next == ':') {
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_DOUBLE_COLON, start_loc);
    }

    // Three-character operators
    if (c == '.' && next == '.' && peek_char(lexer, 2) == '.') {
        advance_char(lexer);
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_ELLIPSIS, start_loc);
    }

    // Check for orphaned comment close before single-character tokens
    if (c == '*' && next == '/') {
        // Orphaned comment close - this is an error
        set_error(lexer, "Unexpected comment close sequence '*/' outside of comment");
        advance_char(lexer);
        advance_char(lexer);
        return token_create(TOKEN_ERROR, start_loc);
    }

    // Single-character tokens
    advance_char(lexer);
    switch (c) {
    case '+':
        return token_create(TOKEN_PLUS, start_loc);
    case '-':
        return token_create(TOKEN_MINUS, start_loc);
    case '*':
        return token_create(TOKEN_MULTIPLY, start_loc);
    case '/':
        return token_create(TOKEN_DIVIDE, start_loc);
    case '%':
        return token_create(TOKEN_MODULO, start_loc);
    case '=':
        return token_create(TOKEN_ASSIGN, start_loc);
    case '<':
        return token_create(TOKEN_LESS_THAN, start_loc);
    case '>':
        return token_create(TOKEN_GREATER_THAN, start_loc);
    case '!':
        return token_create(TOKEN_LOGICAL_NOT, start_loc);
    case '&':
        return token_create(TOKEN_BITWISE_AND, start_loc);
    case '|':
        return token_create(TOKEN_BITWISE_OR, start_loc);
    case '^':
        return token_create(TOKEN_BITWISE_XOR, start_loc);
    case '~':
        return token_create(TOKEN_BITWISE_NOT, start_loc);
    case ';':
        return token_create(TOKEN_SEMICOLON, start_loc);
    case ',':
        return token_create(TOKEN_COMMA, start_loc);
    case '.':
        return token_create(TOKEN_DOT, start_loc);
    case ':':
        return token_create(TOKEN_COLON, start_loc);
    case '(':
        return token_create(TOKEN_LEFT_PAREN, start_loc);
    case ')':
        return token_create(TOKEN_RIGHT_PAREN, start_loc);
    case '{':
        return token_create(TOKEN_LEFT_BRACE, start_loc);
    case '}':
        return token_create(TOKEN_RIGHT_BRACE, start_loc);
    case '[':
        return token_create(TOKEN_LEFT_BRACKET, start_loc);
    case ']':
        return token_create(TOKEN_RIGHT_BRACKET, start_loc);
    case '#':
        return token_create(TOKEN_HASH, start_loc);
    case '@':
        return token_create(TOKEN_AT, start_loc);
    case '\n':
        return token_create(TOKEN_NEWLINE, start_loc);
    default:
        set_error(lexer, "Unexpected character");
        return token_create(TOKEN_ERROR, start_loc);
    }
}
