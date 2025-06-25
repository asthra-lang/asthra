/**
 * Asthra Programming Language Compiler
 * Lexical analyzer implementation for v1.25 grammar
 * Modular version with separate files for components
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

/**
 * This file is a thin wrapper around the modularized lexer implementation.
 * The original lexer.c has been split into the following files:
 * - token.c: Token creation, management and utilities
 * - keyword.c: Keyword lookup and management
 * - lexer_util.c: Utility functions for character handling and locations
 * - lexer_scan.c: Token scanning logic for different token types
 * - lexer_core.c: Core lexer functionality (init, destroy, next token)
 */

#include "lexer.h"

// Functions from token.c
Token token_create(TokenType type, SourceLocation location);
void token_free(Token *token);
Token token_clone(const Token *token);
const char *token_type_name(TokenType type);
bool token_is_keyword(TokenType type);
bool token_is_operator(TokenType type);
bool token_is_literal(TokenType type);
bool token_is_type(TokenType type);

// Functions from keyword.c
TokenType keyword_lookup(const char *identifier, size_t length);
bool is_reserved_keyword(const char *identifier, size_t length);

// Functions from lexer_util.c
bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);
bool is_whitespace(char c);
char peek_char(Lexer *lexer, size_t offset);
char advance_char(Lexer *lexer);
SourceLocation current_location(Lexer *lexer);
void set_error(Lexer *lexer, const char *message);
void skip_whitespace(Lexer *lexer);

// Functions from lexer_scan.c
Token scan_number(Lexer *lexer);
Token scan_string(Lexer *lexer);
Token scan_identifier(Lexer *lexer);
Token scan_character(Lexer *lexer);
Token scan_token(Lexer *lexer);

// Functions from lexer_core.c (already have full declarations in lexer.h)
// Lexer *lexer_create(const char *source, size_t source_length, const char *filename);
// void lexer_destroy(Lexer *lexer);
// Token lexer_next_token(Lexer *lexer);
// Token lexer_peek_token(Lexer *lexer);
// bool lexer_match(Lexer *lexer, TokenType expected);
// bool lexer_consume(Lexer *lexer, TokenType expected);
// Token lexer_current_token(Lexer *lexer);
// bool lexer_at_eof(Lexer *lexer);
// const char *lexer_get_error(Lexer *lexer);
