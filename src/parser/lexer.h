/**
 * Asthra Programming Language Compiler
 * Lexical analyzer (tokenizer) for Asthra grammar
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LEXER_H
#define ASTHRA_LEXER_H

#include "common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct Token Token;
typedef struct Lexer Lexer;

// Token types for Asthra grammar
typedef enum {
    // End of file
    TOKEN_EOF,

    // Literals
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_BOOL_TRUE,
    TOKEN_BOOL_FALSE,
    TOKEN_CHAR, // Character literals like 'a', '\n'
    TOKEN_IDENTIFIER,

    // Keywords
    TOKEN_PACKAGE,
    TOKEN_IMPORT,
    TOKEN_AS,   // for import aliases
    TOKEN_PUB,  // for public visibility
    TOKEN_PRIV, // for private visibility (v1.18 explicit visibility)
    TOKEN_FN,
    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_EXTERN,
    TOKEN_LET,
    TOKEN_CONST, // const keyword for compile-time constants
    TOKEN_MUT,   // mut keyword for variable mutability (v1.25: immutable by default)
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_MATCH,
    TOKEN_SPAWN,
    TOKEN_UNSAFE,
    TOKEN_SIZEOF,
    TOKEN_IMPL, // for struct method implementations
    TOKEN_SELF, // for instance methods

    // Types
    TOKEN_INT,
    TOKEN_FLOAT_TYPE,
    TOKEN_BOOL,
    TOKEN_STRING_TYPE,
    TOKEN_VOID,
    TOKEN_NONE, // v1.19: structural absence marker
    TOKEN_USIZE,
    TOKEN_ISIZE,
    TOKEN_U8,
    TOKEN_I8,
    TOKEN_U16,
    TOKEN_I16,
    TOKEN_U32,
    TOKEN_I32,
    TOKEN_U64,
    TOKEN_I64,
    TOKEN_U128,
    TOKEN_I128,
    TOKEN_F32,
    TOKEN_F64,
    TOKEN_RESULT,
    TOKEN_OPTION,     // Option<T> built-in type
    TOKEN_TASKHANDLE, // TaskHandle<T> built-in type for concurrency
    TOKEN_NEVER,      // Never type for functions that don't return

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS_THAN,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_THAN,
    TOKEN_GREATER_EQUAL,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_LOGICAL_NOT,
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_XOR,
    TOKEN_BITWISE_NOT,
    TOKEN_LEFT_SHIFT,
    TOKEN_RIGHT_SHIFT,
    TOKEN_ARROW,     // ->
    TOKEN_FAT_ARROW, // =>

    // Punctuation
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_DOUBLE_COLON, // ::
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_ANGLE,
    TOKEN_RIGHT_ANGLE,

    // Special tokens for annotations
    TOKEN_HASH,     // #
    TOKEN_AT,       // @
    TOKEN_ELLIPSIS, // ... (for variadic functions)

    // Memory and FFI keywords
    TOKEN_STAR,      // * (for pointers)
    TOKEN_AMPERSAND, // & (for references)

    // Error token
    TOKEN_ERROR,

    // Whitespace and comments (usually skipped)
    TOKEN_WHITESPACE,
    TOKEN_COMMENT,
    TOKEN_NEWLINE,

    // New token types for advanced concurrency features
    TOKEN_SPAWN_WITH_HANDLE,
    TOKEN_AWAIT,
    // Note: Tier 3 tokens (TOKEN_CHANNEL, TOKEN_SEND, TOKEN_RECV, TOKEN_SELECT,
    // TOKEN_DEFAULT, TOKEN_WORKER_POOL, TOKEN_TIMEOUT, TOKEN_CLOSE) moved to stdlib
} TokenType;

// Token structure
struct Token {
    TokenType type;
    SourceLocation location;
    union {
        struct {
            char *value;
            size_t length;
        } string;
        struct {
            int64_t value;
        } integer;
        struct {
            double value;
        } float_val;
        struct {
            char *name;
            size_t length;
        } identifier;
        struct {
            uint32_t value; // Unicode code point
        } character;
    } data;
};

// Lexer state
struct Lexer {
    const char *source;
    size_t source_length;
    size_t position;
    size_t line;
    size_t column;
    const char *filename;
    Token current_token;
    Token peek_token;
    bool has_peek;
    char *error_message;
};

// =============================================================================
// LEXER INTERFACE
// =============================================================================

// Create and initialize lexer
Lexer *lexer_create(const char *source, size_t source_length, const char *filename);

// Destroy lexer and free resources
void lexer_destroy(Lexer *lexer);

// Get next token
Token lexer_next_token(Lexer *lexer);

// Peek at next token without consuming it
Token lexer_peek_token(Lexer *lexer);

// Check if current token matches expected type
bool lexer_match(Lexer *lexer, TokenType expected);

// Consume token if it matches expected type
bool lexer_consume(Lexer *lexer, TokenType expected);

// Get current token
Token lexer_current_token(Lexer *lexer);

// Check if at end of file
bool lexer_at_eof(Lexer *lexer);

// Get error message (if any)
const char *lexer_get_error(Lexer *lexer);

// =============================================================================
// TOKEN UTILITIES
// =============================================================================

// Create token
Token token_create(TokenType type, SourceLocation location);

// Free token resources
void token_free(Token *token);

// Clone token
Token token_clone(const Token *token);

// Get token type name (for debugging)
const char *token_type_name(TokenType type);

// Get user-friendly token display name
const char *token_type_display_name(TokenType type);

// Check if token is a keyword
bool token_is_keyword(TokenType type);

// Check if token is an operator
bool token_is_operator(TokenType type);

// Check if token is a literal
bool token_is_literal(TokenType type);

// Check if token is a type keyword
bool token_is_type(TokenType type);

// =============================================================================
// KEYWORD RECOGNITION
// =============================================================================

// Convert identifier to keyword token type (if applicable)
TokenType keyword_lookup(const char *identifier, size_t length);

// Check if string is a reserved keyword
bool is_reserved_keyword(const char *identifier, size_t length);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LEXER_H
