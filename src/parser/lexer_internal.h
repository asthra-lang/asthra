/**
 * Asthra Programming Language Compiler
 * Internal lexical analyzer header for v1.25 grammar
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LEXER_INTERNAL_H
#define ASTHRA_LEXER_INTERNAL_H

#include "lexer.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Character handling
bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);
bool is_whitespace(char c);

// Additional character checking for numeric literals (Phase 2)
bool is_hex_digit(char c);
bool is_binary_digit(char c);
bool is_octal_digit(char c);

// Lexer position and content access
char peek_char(Lexer *lexer, size_t offset);
char advance_char(Lexer *lexer);
SourceLocation current_location(Lexer *lexer);
void set_error(Lexer *lexer, const char *message);
bool skip_whitespace(Lexer *lexer); // Returns false if error occurred

// =============================================================================
// TOKEN SCANNING FUNCTIONS
// =============================================================================

// Token scanning for specific types
Token scan_number(Lexer *lexer);
Token scan_string(Lexer *lexer);
Token scan_identifier(Lexer *lexer);
Token scan_character(Lexer *lexer);
Token scan_token(Lexer *lexer);

// Specialized numeric literal scanners (Phase 2)
Token scan_hex_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);
Token scan_binary_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);
Token scan_octal_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);
Token scan_decimal_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);

// Multi-line string literal scanners (Phase 2)
bool is_multiline_string_start(Lexer *lexer);
Token scan_multiline_raw_string(Lexer *lexer);
Token scan_multiline_processed_string(Lexer *lexer);

// =============================================================================
// KEYWORD RECOGNITION
// =============================================================================

TokenType keyword_lookup(const char *identifier, size_t length);
bool is_reserved_keyword(const char *identifier, size_t length);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LEXER_INTERNAL_H 
