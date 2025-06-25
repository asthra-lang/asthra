/**
 * Asthra Programming Language Compiler
 * Numeric literal scanning implementation for Asthra grammar
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer_scan_numbers.h"
#include "lexer_internal.h"
#include <stdlib.h>
#include <string.h>

Token scan_hex_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos) {
    if (!is_hex_digit(peek_char(lexer, 0))) {
        set_error(lexer, "Incomplete hexadecimal literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    while (lexer->position < lexer->source_length && is_hex_digit(peek_char(lexer, 0))) {
        advance_char(lexer);
    }

    // Check if there's an invalid character immediately following the hex digits
    char next_char = peek_char(lexer, 0);
    if (next_char != '\0' && (is_alnum(next_char) || next_char == '.')) {
        // Invalid character in hex literal (like 0xG or 0x.5)
        set_error(lexer, "Invalid character in hexadecimal literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    size_t length = lexer->position - start_pos;
    char *hex_str = malloc(length + 1);
    memcpy(hex_str, lexer->source + start_pos, length);
    hex_str[length] = '\0';

    Token token = token_create(TOKEN_INTEGER, start_loc);
    token.data.integer.value = strtoll(hex_str + 2, NULL, 16); // Skip "0x"

    free(hex_str);
    return token;
}

Token scan_binary_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos) {
    if (!is_binary_digit(peek_char(lexer, 0))) {
        set_error(lexer, "Incomplete binary literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    while (lexer->position < lexer->source_length && is_binary_digit(peek_char(lexer, 0))) {
        advance_char(lexer);
    }

    // Check if there's an invalid character immediately following the binary digits
    char next_char = peek_char(lexer, 0);
    if (next_char != '\0' && (is_alnum(next_char) || next_char == '.')) {
        // Invalid character in binary literal (like 0b2 or 0b1.0)
        set_error(lexer, "Invalid character in binary literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    size_t length = lexer->position - start_pos;
    char *bin_str = malloc(length + 1);
    memcpy(bin_str, lexer->source + start_pos, length);
    bin_str[length] = '\0';

    Token token = token_create(TOKEN_INTEGER, start_loc);
    token.data.integer.value = strtoll(bin_str + 2, NULL, 2); // Skip "0b"

    free(bin_str);
    return token;
}

Token scan_octal_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos) {
    if (!is_octal_digit(peek_char(lexer, 0))) {
        set_error(lexer, "Incomplete octal literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    while (lexer->position < lexer->source_length && is_octal_digit(peek_char(lexer, 0))) {
        advance_char(lexer);
    }

    // Check if there's an invalid character immediately following the octal digits
    char next_char = peek_char(lexer, 0);
    if (next_char != '\0' && (is_alnum(next_char) || next_char == '.')) {
        // Invalid character in octal literal (like 0o8 or 0o7.8)
        set_error(lexer, "Invalid character in octal literal");
        return token_create(TOKEN_ERROR, start_loc);
    }

    size_t length = lexer->position - start_pos;
    char *oct_str = malloc(length + 1);
    memcpy(oct_str, lexer->source + start_pos, length);
    oct_str[length] = '\0';

    Token token = token_create(TOKEN_INTEGER, start_loc);
    token.data.integer.value = strtoll(oct_str + 2, NULL, 8); // Skip "0o"

    free(oct_str);
    return token;
}

Token scan_decimal_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos) {
    bool is_float = false;
    bool has_decimal_point = false;
    bool has_exponent = false;
    bool has_integer_part = false;
    bool has_fractional_part = false;

    // Check if we're starting with a decimal point
    if (lexer->position < lexer->source_length && peek_char(lexer, 0) == '.') {
        is_float = true;
        has_decimal_point = true;
        advance_char(lexer); // consume '.'

        // Must have digits after decimal point for numbers starting with '.'
        if (lexer->position >= lexer->source_length || !is_digit(peek_char(lexer, 0))) {
            set_error(lexer, "Invalid decimal number starting with '.'");
            return token_create(TOKEN_ERROR, start_loc);
        }

        // Scan fractional part
        while (lexer->position < lexer->source_length && is_digit(peek_char(lexer, 0))) {
            advance_char(lexer);
        }
        has_fractional_part = true;
    } else {
        // Scan integer part
        while (lexer->position < lexer->source_length && is_digit(peek_char(lexer, 0))) {
            advance_char(lexer);
            has_integer_part = true;
        }

        // Check for decimal point
        if (lexer->position < lexer->source_length && peek_char(lexer, 0) == '.') {
            // Check for double decimal point (like 1..2)
            if (lexer->position + 1 < lexer->source_length && peek_char(lexer, 1) == '.') {
                set_error(lexer, "Invalid decimal point sequence");
                return token_create(TOKEN_ERROR, start_loc);
            }

            is_float = true;
            has_decimal_point = true;
            advance_char(lexer); // consume '.'

            // Scan fractional part (if present)
            while (lexer->position < lexer->source_length && is_digit(peek_char(lexer, 0))) {
                advance_char(lexer);
                has_fractional_part = true;
            }

            // Check for additional decimal points (like 1.2.3)
            if (lexer->position < lexer->source_length && peek_char(lexer, 0) == '.') {
                set_error(lexer, "Multiple decimal points in number");
                return token_create(TOKEN_ERROR, start_loc);
            }
        }
    }

    // Check for scientific notation
    if (lexer->position < lexer->source_length &&
        (peek_char(lexer, 0) == 'e' || peek_char(lexer, 0) == 'E')) {
        // For scientific notation, we need either an integer part or fractional part
        if (!has_integer_part && !has_fractional_part) {
            set_error(lexer, "Invalid number format before exponent");
            return token_create(TOKEN_ERROR, start_loc);
        }

        advance_char(lexer); // consume 'e' or 'E'
        has_exponent = true;
        is_float = true;

        // Check for sign
        if (lexer->position < lexer->source_length &&
            (peek_char(lexer, 0) == '+' || peek_char(lexer, 0) == '-')) {
            advance_char(lexer);
        }

        // Must have at least one digit after exponent
        if (lexer->position >= lexer->source_length || !is_digit(peek_char(lexer, 0))) {
            set_error(lexer, "Incomplete exponent in number");
            return token_create(TOKEN_ERROR, start_loc);
        }

        // Scan exponent digits
        while (lexer->position < lexer->source_length && is_digit(peek_char(lexer, 0))) {
            advance_char(lexer);
        }
    }

    size_t length = lexer->position - start_pos;
    char *number_str = malloc(length + 1);
    memcpy(number_str, lexer->source + start_pos, length);
    number_str[length] = '\0';

    Token token = token_create(is_float ? TOKEN_FLOAT : TOKEN_INTEGER, start_loc);

    if (is_float) {
        token.data.float_val.value = strtod(number_str, NULL);
    } else {
        token.data.integer.value = strtoll(number_str, NULL, 10);
    }

    free(number_str);
    return token;
}
