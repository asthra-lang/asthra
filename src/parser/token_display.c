/**
 * Asthra Programming Language Compiler
 * User-friendly token display names
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer.h"

// Forward declaration for fallback
const char *token_type_name(TokenType type);

// =============================================================================
// USER-FRIENDLY TOKEN DISPLAY NAMES
// =============================================================================

const char *token_type_display_name(TokenType type) {
    switch (type) {
    // Punctuation - use actual symbols
    case TOKEN_SEMICOLON:
        return ";";
    case TOKEN_COMMA:
        return ",";
    case TOKEN_DOT:
        return ".";
    case TOKEN_COLON:
        return ":";
    case TOKEN_DOUBLE_COLON:
        return "::";
    case TOKEN_LEFT_PAREN:
        return "(";
    case TOKEN_RIGHT_PAREN:
        return ")";
    case TOKEN_LEFT_BRACE:
        return "{";
    case TOKEN_RIGHT_BRACE:
        return "}";
    case TOKEN_LEFT_BRACKET:
        return "[";
    case TOKEN_RIGHT_BRACKET:
        return "]";
    case TOKEN_LEFT_ANGLE:
        return "<";
    case TOKEN_RIGHT_ANGLE:
        return ">";

    // Operators - use actual symbols
    case TOKEN_PLUS:
        return "+";
    case TOKEN_MINUS:
        return "-";
    case TOKEN_MULTIPLY:
        return "*";
    case TOKEN_DIVIDE:
        return "/";
    case TOKEN_MODULO:
        return "%";
    case TOKEN_ASSIGN:
        return "=";
    case TOKEN_EQUAL:
        return "==";
    case TOKEN_NOT_EQUAL:
        return "!=";
    case TOKEN_LESS_THAN:
        return "<";
    case TOKEN_LESS_EQUAL:
        return "<=";
    case TOKEN_GREATER_THAN:
        return ">";
    case TOKEN_GREATER_EQUAL:
        return ">=";
    case TOKEN_LOGICAL_AND:
        return "&&";
    case TOKEN_LOGICAL_OR:
        return "||";
    case TOKEN_LOGICAL_NOT:
        return "!";
    case TOKEN_BITWISE_AND:
        return "&";
    case TOKEN_BITWISE_OR:
        return "|";
    case TOKEN_BITWISE_XOR:
        return "^";
    case TOKEN_BITWISE_NOT:
        return "~";
    case TOKEN_LEFT_SHIFT:
        return "<<";
    case TOKEN_RIGHT_SHIFT:
        return ">>";
    case TOKEN_ARROW:
        return "->";
    case TOKEN_FAT_ARROW:
        return "=>";
    case TOKEN_STAR:
        return "*";
    case TOKEN_HASH:
        return "#";
    case TOKEN_AT:
        return "@";
    case TOKEN_ELLIPSIS:
        return "...";

    // Keywords and identifiers - use lowercase for readability
    case TOKEN_PACKAGE:
        return "package";
    case TOKEN_IMPORT:
        return "import";
    case TOKEN_AS:
        return "as";
    case TOKEN_PUB:
        return "pub";
    case TOKEN_PRIV:
        return "priv";
    case TOKEN_FN:
        return "fn";
    case TOKEN_STRUCT:
        return "struct";
    case TOKEN_ENUM:
        return "enum";
    case TOKEN_EXTERN:
        return "extern";
    case TOKEN_LET:
        return "let";
    case TOKEN_CONST:
        return "const";
    case TOKEN_MUT:
        return "mut";
    case TOKEN_IF:
        return "if";
    case TOKEN_ELSE:
        return "else";
    case TOKEN_FOR:
        return "for";
    case TOKEN_IN:
        return "in";
    case TOKEN_RETURN:
        return "return";
    case TOKEN_MATCH:
        return "match";
    case TOKEN_SPAWN:
        return "spawn";
    case TOKEN_UNSAFE:
        return "unsafe";
    case TOKEN_SIZEOF:
        return "sizeof";
    case TOKEN_IMPL:
        return "impl";
    case TOKEN_SELF:
        return "self";
    case TOKEN_BOOL_TRUE:
        return "true";
    case TOKEN_BOOL_FALSE:
        return "false";
    case TOKEN_NONE:
        return "none";
    case TOKEN_VOID:
        return "void";

    // Type names
    case TOKEN_INT:
        return "int";
    case TOKEN_FLOAT_TYPE:
        return "float";
    case TOKEN_BOOL:
        return "bool";
    case TOKEN_STRING_TYPE:
        return "string";
    case TOKEN_USIZE:
        return "usize";
    case TOKEN_ISIZE:
        return "isize";
    case TOKEN_U8:
        return "u8";
    case TOKEN_I8:
        return "i8";
    case TOKEN_U16:
        return "u16";
    case TOKEN_I16:
        return "i16";
    case TOKEN_U32:
        return "u32";
    case TOKEN_I32:
        return "i32";
    case TOKEN_U64:
        return "u64";
    case TOKEN_I64:
        return "i64";
    case TOKEN_U128:
        return "u128";
    case TOKEN_I128:
        return "i128";
    case TOKEN_F32:
        return "f32";
    case TOKEN_F64:
        return "f64";
    case TOKEN_RESULT:
        return "Result";
    case TOKEN_OPTION:
        return "Option";
    case TOKEN_TASKHANDLE:
        return "TaskHandle";
    case TOKEN_NEVER:
        return "never";

    // Literals
    case TOKEN_INTEGER:
        return "integer literal";
    case TOKEN_FLOAT:
        return "float literal";
    case TOKEN_STRING:
        return "string literal";
    case TOKEN_CHAR:
        return "character literal";
    case TOKEN_IDENTIFIER:
        return "identifier";

    // Special cases
    case TOKEN_EOF:
        return "end of file";
    case TOKEN_ERROR:
        return "error";
    case TOKEN_WHITESPACE:
        return "whitespace";
    case TOKEN_COMMENT:
        return "comment";
    case TOKEN_NEWLINE:
        return "newline";

    // Concurrency
    case TOKEN_SPAWN_WITH_HANDLE:
        return "spawn_with_handle";
    case TOKEN_AWAIT:
        return "await";

    default:
        return token_type_name(type); // Fallback to uppercase name
    }
}