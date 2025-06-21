/**
 * Asthra Programming Language Compiler
 * Token management and utilities for v1.25 grammar
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TOKEN CREATION AND MANAGEMENT
// =============================================================================

Token token_create(TokenType type, SourceLocation location) {
    Token token = {0};
    token.type = type;
    token.location = location;
    return token;
}

void token_free(Token *token) {
    if (!token) return;
    
    switch (token->type) {
        case TOKEN_STRING:
            if (token->data.string.value) {
                free(token->data.string.value);
                token->data.string.value = NULL;
            }
            break;
        case TOKEN_IDENTIFIER:
            if (token->data.identifier.name) {
                free(token->data.identifier.name);
                token->data.identifier.name = NULL;
            }
            break;

        default:
            break;
    }
}

Token token_clone(const Token *token) {
    if (!token) {
        Token empty = {0};
        return empty;
    }
    
    // Create clean token with only basic fields copied
    Token clone = {0};
    clone.type = token->type;
    clone.location = token->location;
    
    // Deep copy only the token types that have dynamically allocated data
    switch (token->type) {
        case TOKEN_STRING:
            if (token->data.string.value) {
                clone.data.string.value = malloc(token->data.string.length + 1);
                if (clone.data.string.value) {
                    memcpy(clone.data.string.value, token->data.string.value, token->data.string.length + 1);
                    clone.data.string.length = token->data.string.length;
                }
            } else {
                clone.data.string.value = NULL;
                clone.data.string.length = 0;
            }
            break;
        case TOKEN_IDENTIFIER:
            if (token->data.identifier.name) {
                clone.data.identifier.name = malloc(token->data.identifier.length + 1);
                if (clone.data.identifier.name) {
                    memcpy(clone.data.identifier.name, token->data.identifier.name, token->data.identifier.length + 1);
                    clone.data.identifier.length = token->data.identifier.length;
                }
            } else {
                clone.data.identifier.name = NULL;
                clone.data.identifier.length = 0;
            }
            break;

        default:
            // For all other token types (INTEGER, FLOAT, CHAR, etc.), copy the data directly
            // These don't use dynamically allocated memory, so direct copy is safe
            clone.data = token->data;
            break;
    }
    
    return clone;
}

// =============================================================================
// TOKEN TYPE UTILITIES
// =============================================================================

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_STRING: return "STRING";

        case TOKEN_BOOL_TRUE: return "TRUE";
        case TOKEN_BOOL_FALSE: return "FALSE";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_PACKAGE: return "PACKAGE";
        case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_AS: return "AS";
        case TOKEN_PUB: return "PUB";
        case TOKEN_PRIV: return "PRIV";
        case TOKEN_FN: return "FN";
        case TOKEN_STRUCT: return "STRUCT";
        case TOKEN_ENUM: return "ENUM";
        case TOKEN_EXTERN: return "EXTERN";
        case TOKEN_LET: return "LET";
        case TOKEN_CONST: return "CONST";
        case TOKEN_MUT: return "MUT";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_IN: return "IN";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_MATCH: return "MATCH";
        case TOKEN_SPAWN: return "SPAWN";
        case TOKEN_UNSAFE: return "UNSAFE";
        case TOKEN_SIZEOF: return "SIZEOF";
        case TOKEN_IMPL: return "IMPL";
        case TOKEN_SELF: return "SELF";
        
        // Types
        case TOKEN_INT: return "INT";
        case TOKEN_FLOAT_TYPE: return "FLOAT_TYPE";
        case TOKEN_BOOL: return "BOOL";
        case TOKEN_STRING_TYPE: return "STRING_TYPE";
        case TOKEN_VOID: return "VOID";
        case TOKEN_NONE: return "NONE";
        case TOKEN_USIZE: return "USIZE";
        case TOKEN_ISIZE: return "ISIZE";
        case TOKEN_U8: return "U8";
        case TOKEN_I8: return "I8";
        case TOKEN_U16: return "U16";
        case TOKEN_I16: return "I16";
        case TOKEN_U32: return "U32";
        case TOKEN_I32: return "I32";
        case TOKEN_U64: return "U64";
        case TOKEN_I64: return "I64";
        case TOKEN_F32: return "F32";
        case TOKEN_F64: return "F64";
        case TOKEN_RESULT: return "RESULT";
        case TOKEN_OPTION: return "OPTION";
        case TOKEN_NEVER: return "NEVER";
        
        // Operators
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_MODULO: return "MODULO";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
        case TOKEN_LESS_THAN: return "LESS_THAN";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_GREATER_THAN: return "GREATER_THAN";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_LOGICAL_AND: return "LOGICAL_AND";
        case TOKEN_LOGICAL_OR: return "LOGICAL_OR";
        case TOKEN_LOGICAL_NOT: return "LOGICAL_NOT";
        case TOKEN_BITWISE_AND: return "BITWISE_AND";
        case TOKEN_BITWISE_OR: return "BITWISE_OR";
        case TOKEN_BITWISE_XOR: return "BITWISE_XOR";
        case TOKEN_BITWISE_NOT: return "BITWISE_NOT";
        case TOKEN_LEFT_SHIFT: return "LEFT_SHIFT";
        case TOKEN_RIGHT_SHIFT: return "RIGHT_SHIFT";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_FAT_ARROW: return "FAT_ARROW";
        
        // Punctuation
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_DOT: return "DOT";
        case TOKEN_COLON: return "COLON";
        case TOKEN_DOUBLE_COLON: return "DOUBLE_COLON";
        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TOKEN_LEFT_ANGLE: return "LEFT_ANGLE";
        case TOKEN_RIGHT_ANGLE: return "RIGHT_ANGLE";
        
        // Special tokens
        case TOKEN_HASH: return "HASH";
        case TOKEN_AT: return "AT";
        case TOKEN_ELLIPSIS: return "ELLIPSIS";
        
        // Memory and FFI
        case TOKEN_STAR: return "STAR";
        
        // Special tokens
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_WHITESPACE: return "WHITESPACE";
        case TOKEN_COMMENT: return "COMMENT";
        case TOKEN_NEWLINE: return "NEWLINE";
        
        // Tier 1 Concurrency Features (Core & Simple)
        case TOKEN_SPAWN_WITH_HANDLE: return "SPAWN_WITH_HANDLE";
        case TOKEN_AWAIT: return "AWAIT";
        // Note: Tier 3 tokens moved to stdlib
        
        default: return "UNKNOWN";
    }
}

bool token_is_keyword(TokenType type) {
    // Boolean literals are keywords (reserved words)
    if (type == TOKEN_BOOL_TRUE || type == TOKEN_BOOL_FALSE) {
        return true;
    }
    
    // Regular keywords and type keywords
    return (type >= TOKEN_PACKAGE && type <= TOKEN_RESULT) ||
           (type >= TOKEN_SPAWN_WITH_HANDLE && type <= TOKEN_AWAIT);
}

bool token_is_operator(TokenType type) {
    return type >= TOKEN_PLUS && type <= TOKEN_FAT_ARROW;
}

bool token_is_literal(TokenType type) {
    return type >= TOKEN_INTEGER && type <= TOKEN_CHAR;
}

bool token_is_type(TokenType type) {
    return type >= TOKEN_INT && type <= TOKEN_RESULT;
} 
