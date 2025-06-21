/**
 * Asthra Programming Language Compiler
 * Parser token manipulation and lookahead functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_TOKEN_H
#define ASTHRA_PARSER_TOKEN_H

#include "lexer.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// TOKEN MANIPULATION FUNCTIONS
// =============================================================================

// Token consumption and checking
bool expect_token(Parser *parser, TokenType expected);
bool match_token(Parser *parser, TokenType expected);
bool consume_token(Parser *parser, TokenType expected);
Token advance_token(Parser *parser);
Token peek_token(Parser *parser);
bool check_token(Parser *parser, TokenType expected);
bool at_end(Parser *parser);

// Enhanced token operations with lookahead
Token peek_token_ahead(Parser *parser, size_t offset);
bool match_token_sequence(Parser *parser, const TokenType *sequence, size_t count);
void push_token_back(Parser *parser, Token token);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_TOKEN_H 
