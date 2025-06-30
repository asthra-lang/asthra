/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Primitive Types Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Primitive type definitions and management
 */

#include "semantic_primitive_types.h"
#include "semantic_symbols.h"
#include "semantic_symbols_entries.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C17 feature detection and compatibility
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
#define ASTHRA_HAS_C17 1
#include <stdatomic.h>
#else
#define ASTHRA_HAS_C17 0
// Fallbacks for older C standards
#define _Alignof(type) __alignof__(type)
#define atomic_init(ptr, val) (*(ptr) = (val))
#endif

// =============================================================================
// PRIMITIVE TYPE DEFINITIONS WITH C17 DESIGNATED INITIALIZERS
// =============================================================================

// Builtin type descriptors with C17 designated initializers
static TypeDescriptor g_builtin_types[] = {
    [PRIMITIVE_VOID] = {.category = TYPE_PRIMITIVE,
                        .flags = {.is_constant = true},
                        .size = 0,
                        .alignment = 1,
                        .name = "void",
                        .data.primitive = {.primitive_kind = PRIMITIVE_VOID},
                        .ref_count = 1000000},
    [PRIMITIVE_BOOL] = {.category = TYPE_PRIMITIVE,
                        .flags = {.is_constant = false},
                        .size = sizeof(bool),
                        .alignment = _Alignof(bool),
                        .name = "bool",
                        .data.primitive = {.primitive_kind = PRIMITIVE_BOOL},
                        .ref_count = 1000000},
    [PRIMITIVE_I8] = {.category = TYPE_PRIMITIVE,
                      .flags = {.is_constant = false},
                      .size = sizeof(int8_t),
                      .alignment = _Alignof(int8_t),
                      .name = "i8",
                      .data.primitive = {.primitive_kind = PRIMITIVE_I8},
                      .ref_count = 1000000},
    [PRIMITIVE_I16] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(int16_t),
                       .alignment = _Alignof(int16_t),
                       .name = "i16",
                       .data.primitive = {.primitive_kind = PRIMITIVE_I16},
                       .ref_count = 1000000},
    [PRIMITIVE_I32] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(int32_t),
                       .alignment = _Alignof(int32_t),
                       .name = "i32",
                       .data.primitive = {.primitive_kind = PRIMITIVE_I32},
                       .ref_count = 1000000},
    [PRIMITIVE_I64] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(int64_t),
                       .alignment = _Alignof(int64_t),
                       .name = "i64",
                       .data.primitive = {.primitive_kind = PRIMITIVE_I64},
                       .ref_count = 1000000},
    [PRIMITIVE_I128] = {.category = TYPE_PRIMITIVE,
                        .flags = {.is_constant = false},
                        .size = 16,
                        .alignment = 16,
                        .name = "i128",
                        .data.primitive = {.primitive_kind = PRIMITIVE_I128},
                        .ref_count = 1000000},
    [PRIMITIVE_U8] = {.category = TYPE_PRIMITIVE,
                      .flags = {.is_constant = false},
                      .size = sizeof(uint8_t),
                      .alignment = _Alignof(uint8_t),
                      .name = "u8",
                      .data.primitive = {.primitive_kind = PRIMITIVE_U8},
                      .ref_count = 1000000},
    [PRIMITIVE_U16] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(uint16_t),
                       .alignment = _Alignof(uint16_t),
                       .name = "u16",
                       .data.primitive = {.primitive_kind = PRIMITIVE_U16},
                       .ref_count = 1000000},
    [PRIMITIVE_U32] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(uint32_t),
                       .alignment = _Alignof(uint32_t),
                       .name = "u32",
                       .data.primitive = {.primitive_kind = PRIMITIVE_U32},
                       .ref_count = 1000000},
    [PRIMITIVE_U64] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(uint64_t),
                       .alignment = _Alignof(uint64_t),
                       .name = "u64",
                       .data.primitive = {.primitive_kind = PRIMITIVE_U64},
                       .ref_count = 1000000},
    [PRIMITIVE_U128] = {.category = TYPE_PRIMITIVE,
                        .flags = {.is_constant = false},
                        .size = 16,
                        .alignment = 16,
                        .name = "u128",
                        .data.primitive = {.primitive_kind = PRIMITIVE_U128},
                        .ref_count = 1000000},
    [PRIMITIVE_USIZE] = {.category = TYPE_PRIMITIVE,
                         .flags = {.is_constant = false},
                         .size = sizeof(size_t),
                         .alignment = _Alignof(size_t),
                         .name = "usize",
                         .data.primitive = {.primitive_kind = PRIMITIVE_USIZE},
                         .ref_count = 1000000},
    [PRIMITIVE_ISIZE] = {.category = TYPE_PRIMITIVE,
                         .flags = {.is_constant = false},
                         .size = sizeof(ptrdiff_t),
                         .alignment = _Alignof(ptrdiff_t),
                         .name = "isize",
                         .data.primitive = {.primitive_kind = PRIMITIVE_ISIZE},
                         .ref_count = 1000000},
    [PRIMITIVE_F32] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(float),
                       .alignment = _Alignof(float),
                       .name = "f32",
                       .data.primitive = {.primitive_kind = PRIMITIVE_F32},
                       .ref_count = 1000000},
    [PRIMITIVE_F64] = {.category = TYPE_PRIMITIVE,
                       .flags = {.is_constant = false},
                       .size = sizeof(double),
                       .alignment = _Alignof(double),
                       .name = "f64",
                       .data.primitive = {.primitive_kind = PRIMITIVE_F64},
                       .ref_count = 1000000},
    [PRIMITIVE_CHAR] = {.category = TYPE_PRIMITIVE,
                        .flags = {.is_constant = false},
                        .size = sizeof(char),
                        .alignment = _Alignof(char),
                        .name = "char",
                        .data.primitive = {.primitive_kind = PRIMITIVE_CHAR},
                        .ref_count = 1000000},
    [PRIMITIVE_STRING] = {.category = TYPE_PRIMITIVE,
                          .flags = {.is_constant = false},
                          .size = sizeof(void *), // String is a pointer type
                          .alignment = _Alignof(void *),
                          .name = "string",
                          .data.primitive = {.primitive_kind = PRIMITIVE_STRING},
                          .ref_count = 1000000},
    [PRIMITIVE_NEVER] = {.category = TYPE_PRIMITIVE,
                         .flags = {.is_constant = true},
                         .size = 0, // Never type has no values
                         .alignment = 1,
                         .name = "Never",
                         .data.primitive = {.primitive_kind = PRIMITIVE_NEVER},
                         .ref_count = 1000000}};

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

TypeDescriptor *semantic_get_primitive_types_array(void) {
    return g_builtin_types;
}

bool semantic_is_primitive_type(const TypeDescriptor *type) {
    if (!type)
        return false;

    // Check if the type pointer is within the builtin types array
    const TypeDescriptor *start = &g_builtin_types[0];
    const TypeDescriptor *end = &g_builtin_types[PRIMITIVE_COUNT];

    return (type >= start && type < end);
}

void semantic_init_primitive_types(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    analyzer->builtin_type_count = PRIMITIVE_COUNT;
    analyzer->builtin_types = malloc(sizeof(TypeDescriptor *) * PRIMITIVE_COUNT);
    if (!analyzer->builtin_types)
        return;

    // Point to static builtin type descriptors
    for (size_t i = 0; i < PRIMITIVE_COUNT; i++) {
        analyzer->builtin_types[i] = &g_builtin_types[i];
    }

    // Register builtin types in the global symbol table
    for (size_t i = 0; i < PRIMITIVE_COUNT; i++) {
        TypeDescriptor *type = &g_builtin_types[i];
        if (type->name) {
            SymbolEntry *type_symbol =
                symbol_entry_create(type->name, SYMBOL_TYPE, type,
                                    NULL // No AST declaration for builtin types
                );

            if (type_symbol) {
                // Mark primitive types as predeclared to allow user-defined types to shadow them
                type_symbol->flags.is_predeclared = true;
                symbol_table_insert_safe(analyzer->global_scope, type->name, type_symbol);
            }
        }
    }

    // Register type aliases for compatibility
    // 'int' is an alias for 'i32'
    TypeDescriptor *i32_type = &g_builtin_types[PRIMITIVE_I32];
    SymbolEntry *int_alias = symbol_entry_create("int", SYMBOL_TYPE, i32_type, NULL);
    if (int_alias) {
        int_alias->flags.is_predeclared = true;
        symbol_table_insert_safe(analyzer->global_scope, "int", int_alias);
    }

    // 'float' is an alias for 'f32'
    TypeDescriptor *f32_type = &g_builtin_types[PRIMITIVE_F32];
    SymbolEntry *float_alias = symbol_entry_create("float", SYMBOL_TYPE, f32_type, NULL);
    if (float_alias) {
        float_alias->flags.is_predeclared = true;
        symbol_table_insert_safe(analyzer->global_scope, "float", float_alias);
    }

    // 'usize' is an alias for 'u64' (on 64-bit systems)
    TypeDescriptor *u64_type = &g_builtin_types[PRIMITIVE_U64];
    SymbolEntry *usize_alias = symbol_entry_create("usize", SYMBOL_TYPE, u64_type, NULL);
    if (usize_alias) {
        usize_alias->flags.is_predeclared = true;
        symbol_table_insert_safe(analyzer->global_scope, "usize", usize_alias);
    }

    // 'isize' is an alias for 'i64' (on 64-bit systems)
    TypeDescriptor *i64_type = &g_builtin_types[PRIMITIVE_I64];
    SymbolEntry *isize_alias = symbol_entry_create("isize", SYMBOL_TYPE, i64_type, NULL);
    if (isize_alias) {
        isize_alias->flags.is_predeclared = true;
        symbol_table_insert_safe(analyzer->global_scope, "isize", isize_alias);
    }
}

// Simple helper to get builtin type without analyzer
TypeDescriptor *get_builtin_type(const char *name) {
    if (!name)
        return NULL;

    // Check primitive types
    for (int i = 0; i < PRIMITIVE_COUNT; i++) {
        if (g_builtin_types[i].name && strcmp(g_builtin_types[i].name, name) == 0) {
            return &g_builtin_types[i];
        }
    }

    // Check type aliases
    if (strcmp(name, "int") == 0) {
        return &g_builtin_types[PRIMITIVE_I32];
    } else if (strcmp(name, "float") == 0) {
        return &g_builtin_types[PRIMITIVE_F32];
    } else if (strcmp(name, "usize") == 0) {
        return &g_builtin_types[PRIMITIVE_U64];
    } else if (strcmp(name, "isize") == 0) {
        return &g_builtin_types[PRIMITIVE_I64];
    }

    return NULL;
}