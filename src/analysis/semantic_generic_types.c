/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Generic Types Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Builtin generic type definitions (Option, Result)
 */

#include "semantic_generic_types.h"
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
// GENERIC TYPE INITIALIZATION
// =============================================================================

void semantic_init_builtin_generic_types(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    // Get primitive types for variant types
    TypeDescriptor *primitive_types = semantic_get_primitive_types_array();

    // Register Option<T> as a builtin generic enum type
    // Create the Option enum type descriptor
    TypeDescriptor *option_type = malloc(sizeof(TypeDescriptor));
    if (!option_type)
        return;

    // Initialize Option type descriptor
    option_type->category = TYPE_ENUM;
    option_type->flags = (TypeFlags){0};
    option_type->flags.is_ffi_compatible = true;
    option_type->size = sizeof(int); // Enum size
    option_type->alignment = _Alignof(int);
    option_type->name = strdup("Option");
    atomic_init(&option_type->ref_count, 1);

    // Create variant symbol table
    option_type->data.enum_type.variants = symbol_table_create(4);
    option_type->data.enum_type.variant_count = 2;

    // Create Option symbol entry as generic
    SymbolEntry *option_symbol = symbol_entry_create("Option", SYMBOL_TYPE, option_type,
                                                     NULL // No AST declaration for builtin
    );

    if (option_symbol) {
        // Mark as generic with 1 type parameter
        option_symbol->is_generic = true;
        option_symbol->type_param_count = 1;
        option_symbol->flags.is_predeclared = true;

        // Add to global scope
        symbol_table_insert_safe(analyzer->global_scope, "Option", option_symbol);

        // Add Some and None variants
        // Some variant
        TypeDescriptor *some_type = malloc(sizeof(TypeDescriptor));
        if (some_type) {
            some_type->category = TYPE_PRIMITIVE;
            some_type->flags = (TypeFlags){0};
            some_type->flags.is_constant = true;
            some_type->flags.is_ffi_compatible = true;
            some_type->size = sizeof(int);
            some_type->alignment = _Alignof(int);
            some_type->name = strdup("Option.Some");
            some_type->data.primitive.primitive_kind = PRIMITIVE_I32;
            atomic_init(&some_type->ref_count, 1);

            SymbolEntry *some_symbol =
                symbol_entry_create("Some", SYMBOL_ENUM_VARIANT, some_type, NULL);

            if (some_symbol) {
                symbol_table_insert_safe(option_type->data.enum_type.variants, "Some", some_symbol);

                // Also add qualified name to global scope
                SymbolEntry *qualified_some = symbol_entry_copy(some_symbol);
                if (qualified_some) {
                    free(qualified_some->name);
                    qualified_some->name = strdup("Option.Some");
                    symbol_table_insert_safe(analyzer->global_scope, "Option.Some", qualified_some);
                }
            }
        }

        // None variant
        TypeDescriptor *none_type = malloc(sizeof(TypeDescriptor));
        if (none_type) {
            none_type->category = TYPE_PRIMITIVE;
            none_type->flags = (TypeFlags){0};
            none_type->flags.is_constant = true;
            none_type->flags.is_ffi_compatible = true;
            none_type->size = sizeof(int);
            none_type->alignment = _Alignof(int);
            none_type->name = strdup("Option.None");
            none_type->data.primitive.primitive_kind = PRIMITIVE_I32;
            atomic_init(&none_type->ref_count, 1);

            SymbolEntry *none_symbol =
                symbol_entry_create("None", SYMBOL_ENUM_VARIANT, none_type, NULL);

            if (none_symbol) {
                symbol_table_insert_safe(option_type->data.enum_type.variants, "None", none_symbol);

                // Also add qualified name to global scope
                SymbolEntry *qualified_none = symbol_entry_copy(none_symbol);
                if (qualified_none) {
                    free(qualified_none->name);
                    qualified_none->name = strdup("Option.None");
                    symbol_table_insert_safe(analyzer->global_scope, "Option.None", qualified_none);
                }
            }
        }
    }

    // Register Result<T, E> as a builtin generic enum type
    // Create the Result enum type descriptor
    TypeDescriptor *result_type = malloc(sizeof(TypeDescriptor));
    if (!result_type)
        return;

    // Initialize Result type descriptor
    result_type->category = TYPE_ENUM;
    result_type->flags = (TypeFlags){0};
    result_type->flags.is_ffi_compatible = true;
    result_type->size = sizeof(int); // Enum size
    result_type->alignment = _Alignof(int);
    result_type->name = strdup("Result");
    atomic_init(&result_type->ref_count, 1);

    // Create variant symbol table
    result_type->data.enum_type.variants = symbol_table_create(4);
    result_type->data.enum_type.variant_count = 2;

    // Create Result symbol entry as generic
    SymbolEntry *result_symbol = symbol_entry_create("Result", SYMBOL_TYPE, result_type,
                                                     NULL // No AST declaration for builtin
    );

    if (result_symbol) {
        // Mark as generic with 2 type parameters (T and E)
        result_symbol->is_generic = true;
        result_symbol->type_param_count = 2;
        result_symbol->flags.is_predeclared = true;

        // Add to global scope
        symbol_table_insert_safe(analyzer->global_scope, "Result", result_symbol);

        // Add Ok and Err variants
        // Ok variant
        TypeDescriptor *ok_type = malloc(sizeof(TypeDescriptor));
        if (ok_type) {
            ok_type->category = TYPE_PRIMITIVE;
            ok_type->flags = (TypeFlags){0};
            ok_type->flags.is_constant = true;
            ok_type->flags.is_ffi_compatible = true;
            ok_type->size = sizeof(int);
            ok_type->alignment = _Alignof(int);
            ok_type->name = strdup("Result.Ok");
            ok_type->data.primitive.primitive_kind = PRIMITIVE_I32;
            atomic_init(&ok_type->ref_count, 1);

            SymbolEntry *ok_symbol = symbol_entry_create("Ok", SYMBOL_ENUM_VARIANT, ok_type, NULL);

            if (ok_symbol) {
                symbol_table_insert_safe(result_type->data.enum_type.variants, "Ok", ok_symbol);

                // Also add qualified name to global scope
                SymbolEntry *qualified_ok = symbol_entry_copy(ok_symbol);
                if (qualified_ok) {
                    free(qualified_ok->name);
                    qualified_ok->name = strdup("Result.Ok");
                    symbol_table_insert_safe(analyzer->global_scope, "Result.Ok", qualified_ok);
                }
            }
        }

        // Err variant
        TypeDescriptor *err_type = malloc(sizeof(TypeDescriptor));
        if (err_type) {
            err_type->category = TYPE_PRIMITIVE;
            err_type->flags = (TypeFlags){0};
            err_type->flags.is_constant = true;
            err_type->flags.is_ffi_compatible = true;
            err_type->size = sizeof(int);
            err_type->alignment = _Alignof(int);
            err_type->name = strdup("Result.Err");
            err_type->data.primitive.primitive_kind = PRIMITIVE_I32;
            atomic_init(&err_type->ref_count, 1);

            SymbolEntry *err_symbol =
                symbol_entry_create("Err", SYMBOL_ENUM_VARIANT, err_type, NULL);

            if (err_symbol) {
                symbol_table_insert_safe(result_type->data.enum_type.variants, "Err", err_symbol);

                // Also add qualified name to global scope
                SymbolEntry *qualified_err = symbol_entry_copy(err_symbol);
                if (qualified_err) {
                    free(qualified_err->name);
                    qualified_err->name = strdup("Result.Err");
                    symbol_table_insert_safe(analyzer->global_scope, "Result.Err", qualified_err);
                }
            }
        }
    }
}

// =============================================================================
// TYPE INSTANTIATION
// =============================================================================

TypeDescriptor *semantic_create_option_instance(SemanticAnalyzer *analyzer, TypeDescriptor *element_type) {
    if (!analyzer || !element_type)
        return NULL;

    // Create a new Option<T> instance type
    TypeDescriptor *option_instance = malloc(sizeof(TypeDescriptor));
    if (!option_instance)
        return NULL;

    // Initialize as an enum type
    option_instance->category = TYPE_ENUM;
    option_instance->flags = (TypeFlags){0};
    option_instance->flags.is_ffi_compatible = true;
    
    // Size depends on the variant and element type
    // For now, use a simple tagged union representation
    option_instance->size = sizeof(int) + element_type->size;
    option_instance->alignment = element_type->alignment > _Alignof(int) ? 
                                element_type->alignment : _Alignof(int);
    
    // Create name like "Option<i32>"
    char *name = malloc(strlen("Option<>") + strlen(element_type->name) + 1);
    sprintf(name, "Option<%s>", element_type->name);
    option_instance->name = name;
    
    atomic_init(&option_instance->ref_count, 1);

    // Create variant symbol table
    option_instance->data.enum_type.variants = symbol_table_create(4);
    option_instance->data.enum_type.variant_count = 2;

    // TODO: Add Some and None variants with proper types

    return option_instance;
}

TypeDescriptor *semantic_create_result_instance(SemanticAnalyzer *analyzer, 
                                                TypeDescriptor *ok_type, 
                                                TypeDescriptor *err_type) {
    if (!analyzer || !ok_type || !err_type)
        return NULL;

    // Create a new Result<T, E> instance type
    TypeDescriptor *result_instance = malloc(sizeof(TypeDescriptor));
    if (!result_instance)
        return NULL;

    // Initialize as an enum type
    result_instance->category = TYPE_ENUM;
    result_instance->flags = (TypeFlags){0};
    result_instance->flags.is_ffi_compatible = true;
    
    // Size depends on the larger of ok_type and err_type
    size_t max_size = ok_type->size > err_type->size ? ok_type->size : err_type->size;
    result_instance->size = sizeof(int) + max_size;
    
    size_t max_align = ok_type->alignment > err_type->alignment ? 
                       ok_type->alignment : err_type->alignment;
    result_instance->alignment = max_align > _Alignof(int) ? max_align : _Alignof(int);
    
    // Create name like "Result<i32, string>"
    char *name = malloc(strlen("Result<, >") + strlen(ok_type->name) + strlen(err_type->name) + 1);
    sprintf(name, "Result<%s, %s>", ok_type->name, err_type->name);
    result_instance->name = name;
    
    atomic_init(&result_instance->ref_count, 1);

    // Create variant symbol table
    result_instance->data.enum_type.variants = symbol_table_create(4);
    result_instance->data.enum_type.variant_count = 2;

    // TODO: Add Ok and Err variants with proper types

    return result_instance;
}