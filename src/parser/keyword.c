/**
 * Asthra Programming Language Compiler
 * Keyword handling for Asthra grammar
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer_internal.h"
#include <string.h>

// Keyword lookup table
typedef struct {
    const char *keyword;
    TokenType token_type;
} KeywordEntry;

static const KeywordEntry keywords[] = {
    {"package", TOKEN_PACKAGE},
    {"import", TOKEN_IMPORT},
    {"as", TOKEN_AS},
    {"pub", TOKEN_PUB},
    {"priv", TOKEN_PRIV},
    {"fn", TOKEN_FN},
    {"struct", TOKEN_STRUCT},
    {"extern", TOKEN_EXTERN},
    {"let", TOKEN_LET},
    {"const", TOKEN_CONST},
    {"mut", TOKEN_MUT},      // mut keyword for variable mutability
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"enum", TOKEN_ENUM},
    {"for", TOKEN_FOR},
    {"in", TOKEN_IN},
    {"return", TOKEN_RETURN},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"match", TOKEN_MATCH},
    {"spawn", TOKEN_SPAWN},
    {"unsafe", TOKEN_UNSAFE},
    {"sizeof", TOKEN_SIZEOF},
    {"impl", TOKEN_IMPL},
    {"self", TOKEN_SELF},
    {"true", TOKEN_BOOL_TRUE},
    {"false", TOKEN_BOOL_FALSE},
    
    // Types
    {"int", TOKEN_INT},
    {"float", TOKEN_FLOAT_TYPE},
    {"bool", TOKEN_BOOL},
    {"string", TOKEN_STRING_TYPE},
    {"void", TOKEN_VOID},
    {"none", TOKEN_NONE},     // structural absence marker
    {"usize", TOKEN_USIZE},
    {"isize", TOKEN_ISIZE},
    {"u8", TOKEN_U8},
    {"i8", TOKEN_I8},
    {"u16", TOKEN_U16},
    {"i16", TOKEN_I16},
    {"u32", TOKEN_U32},
    {"i32", TOKEN_I32},
    {"u64", TOKEN_U64},
    {"i64", TOKEN_I64},
    {"f32", TOKEN_F32},
    {"f64", TOKEN_F64},
    {"Result", TOKEN_RESULT},
    {"Never", TOKEN_NEVER},
    
    // Tier 1 Concurrency Features (Core & Simple)
    {"spawn_with_handle", TOKEN_SPAWN_WITH_HANDLE},
    {"await", TOKEN_AWAIT}
    
    // Note: Tier 3 concurrency keywords moved to stdlib:
    // {"channel", TOKEN_CHANNEL}, {"send", TOKEN_SEND}, {"recv", TOKEN_RECV},
    // {"select", TOKEN_SELECT}, {"default", TOKEN_DEFAULT}, 
    // {"worker_pool", TOKEN_WORKER_POOL}, {"timeout", TOKEN_TIMEOUT}, {"close", TOKEN_CLOSE}
};

static const size_t keyword_count = sizeof(keywords) / sizeof(KeywordEntry);

// =============================================================================
// KEYWORD RECOGNITION
// =============================================================================

TokenType keyword_lookup(const char *identifier, size_t length) {
    for (size_t i = 0; i < keyword_count; i++) {
        if (strlen(keywords[i].keyword) == length &&
            strncmp(keywords[i].keyword, identifier, length) == 0) {
            return keywords[i].token_type;
        }
    }
    return TOKEN_IDENTIFIER;
}

bool is_reserved_keyword(const char *identifier, size_t length) {
    return keyword_lookup(identifier, length) != TOKEN_IDENTIFIER;
} 
