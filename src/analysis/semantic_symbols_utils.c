/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Symbol Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Utility functions and validation for symbols
 */

#include "semantic_symbols_utils.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include <stdlib.h>
#include <string.h>

// ===============================
// UTILITY FUNCTIONS
// ===============================

const char *symbol_kind_name(SymbolKind kind) {
    switch (kind) {
    case SYMBOL_VARIABLE:
        return "variable";
    case SYMBOL_FUNCTION:
        return "function";
    case SYMBOL_TYPE:
        return "type";
    case SYMBOL_PARAMETER:
        return "parameter";
    case SYMBOL_FIELD:
        return "field";
    default:
        return "unknown";
    }
}

bool symbol_kind_is_callable(SymbolKind kind) {
    return kind == SYMBOL_FUNCTION;
}

bool symbol_kind_is_type(SymbolKind kind) {
    return kind == SYMBOL_TYPE;
}

bool symbol_kind_is_value(SymbolKind kind) {
    return kind == SYMBOL_VARIABLE || kind == SYMBOL_PARAMETER || kind == SYMBOL_FIELD;
}

// ===============================
// SYMBOL VALIDATION
// ===============================

bool semantic_validate_symbol_name(const char *name) {
    if (!name || *name == '\0')
        return false;

    // Check first character (must be letter or underscore)
    if (!((name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z') ||
          name[0] == '_')) {
        return false;
    }

    // Check remaining characters (letters, digits, underscores)
    for (size_t i = 1; name[i] != '\0'; i++) {
        if (!((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= '0' && name[i] <= '9') || name[i] == '_')) {
            return false;
        }
    }

    return true;
}

bool semantic_is_reserved_keyword(const char *name) {
    if (!name)
        return false;

    static const char *keywords[] = {
        "let",  "fn",    "struct", "enum",   "impl",   "trait",    "if",     "else",  "match",
        "for",  "while", "loop",   "return", "break",  "continue", "true",   "false", "null",
        "pub",  "mod",   "use",    "extern", "unsafe", "const",    "static", "mut",   "i8",
        "i16",  "i32",   "i64",    "u8",     "u16",    "u32",      "u64",    "f32",   "f64",
        "bool", "char",  "string", "void",   "never",  NULL};

    for (size_t i = 0; keywords[i]; i++) {
        if (strcmp(name, keywords[i]) == 0) {
            return true;
        }
    }

    return false;
}

bool semantic_check_symbol_shadowing(SemanticAnalyzer *analyzer, const char *name) {
    if (!analyzer || !name)
        return false;

    // Check if symbol exists in parent scopes
    SymbolTable *current = analyzer->current_scope;
    if (current && current->parent) {
        return symbol_table_lookup_safe(current->parent, name) != NULL;
    }

    return false;
}
