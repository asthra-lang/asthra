/**
 * Asthra Programming Language Compiler
 * Type checking helpers and validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "parser.h"
#include <string.h>

// =============================================================================
// TYPE CHECKING HELPERS
// =============================================================================

bool is_valid_type_name(const char *name) {
    if (!name)
        return false;

    // Check against known type names
    const char *valid_types[] = {"int", "float", "bool", "string", "void", "usize", "isize",
                                 "u8",  "i8",    "u16",  "i16",    "u32",  "i32",   "u64",
                                 "i64", "u128",  "i128", "f32",    "f64"};

    size_t type_count = sizeof(valid_types) / sizeof(valid_types[0]);
    for (size_t i = 0; i < type_count; i++) {
        if (strcmp(name, valid_types[i]) == 0) {
            return true;
        }
    }

    return false;
}

bool is_primitive_type(TokenType token_type) {
    switch (token_type) {
    case TOKEN_INT:
    case TOKEN_FLOAT_TYPE:
    case TOKEN_BOOL:
    case TOKEN_STRING_TYPE:
    case TOKEN_VOID:
    case TOKEN_USIZE:
    case TOKEN_ISIZE:
    case TOKEN_U8:
    case TOKEN_I8:
    case TOKEN_U16:
    case TOKEN_I16:
    case TOKEN_U32:
    case TOKEN_I32:
    case TOKEN_U64:
    case TOKEN_I64:
    case TOKEN_U128:
    case TOKEN_I128:
    case TOKEN_F32:
    case TOKEN_F64:
        return true;
    default:
        return false;
    }
}

bool is_pointer_type_start(Parser *parser) {
    if (!parser)
        return false;
    return parser->current_token.type == TOKEN_MULTIPLY;
}

bool is_slice_type_start(Parser *parser) {
    if (!parser)
        return false;
    return parser->current_token.type == TOKEN_LEFT_BRACKET;
}

bool is_result_type_start(Parser *parser) {
    if (!parser)
        return false;
    return parser->current_token.type == TOKEN_RESULT;
}
