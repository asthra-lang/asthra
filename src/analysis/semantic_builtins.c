/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Builtin Types and Predeclared Identifiers Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Builtin type definitions and predeclared identifier management
 */

#include "semantic_builtins.h"
#include "semantic_symbols.h"
#include "semantic_symbols_entries.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

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
// BUILTIN TYPE DEFINITIONS WITH C17 DESIGNATED INITIALIZERS
// =============================================================================

// Builtin type descriptors with C17 designated initializers
static TypeDescriptor g_builtin_types[] = {
    [PRIMITIVE_VOID] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = true},
        .size = 0,
        .alignment = 1,
        .name = "void",
        .data.primitive = {.primitive_kind = PRIMITIVE_VOID},
        .ref_count = 1000000
    },
    [PRIMITIVE_BOOL] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(bool),
        .alignment = _Alignof(bool),
        .name = "bool",
        .data.primitive = {.primitive_kind = PRIMITIVE_BOOL},
        .ref_count = 1000000
    },
    [PRIMITIVE_I8] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(int8_t),
        .alignment = _Alignof(int8_t),
        .name = "i8",
        .data.primitive = {.primitive_kind = PRIMITIVE_I8},
        .ref_count = 1000000
    },
    [PRIMITIVE_I16] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(int16_t),
        .alignment = _Alignof(int16_t),
        .name = "i16",
        .data.primitive = {.primitive_kind = PRIMITIVE_I16},
        .ref_count = 1000000
    },
    [PRIMITIVE_I32] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(int32_t),
        .alignment = _Alignof(int32_t),
        .name = "i32",
        .data.primitive = {.primitive_kind = PRIMITIVE_I32},
        .ref_count = 1000000
    },
    [PRIMITIVE_I64] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(int64_t),
        .alignment = _Alignof(int64_t),
        .name = "i64",
        .data.primitive = {.primitive_kind = PRIMITIVE_I64},
        .ref_count = 1000000
    },
    [PRIMITIVE_U8] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(uint8_t),
        .alignment = _Alignof(uint8_t),
        .name = "u8",
        .data.primitive = {.primitive_kind = PRIMITIVE_U8},
        .ref_count = 1000000
    },
    [PRIMITIVE_U16] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(uint16_t),
        .alignment = _Alignof(uint16_t),
        .name = "u16",
        .data.primitive = {.primitive_kind = PRIMITIVE_U16},
        .ref_count = 1000000
    },
    [PRIMITIVE_U32] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(uint32_t),
        .alignment = _Alignof(uint32_t),
        .name = "u32",
        .data.primitive = {.primitive_kind = PRIMITIVE_U32},
        .ref_count = 1000000
    },
    [PRIMITIVE_U64] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(uint64_t),
        .alignment = _Alignof(uint64_t),
        .name = "u64",
        .data.primitive = {.primitive_kind = PRIMITIVE_U64},
        .ref_count = 1000000
    },
    [PRIMITIVE_F32] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(float),
        .alignment = _Alignof(float),
        .name = "f32",
        .data.primitive = {.primitive_kind = PRIMITIVE_F32},
        .ref_count = 1000000
    },
    [PRIMITIVE_F64] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(double),
        .alignment = _Alignof(double),
        .name = "f64",
        .data.primitive = {.primitive_kind = PRIMITIVE_F64},
        .ref_count = 1000000
    },
    [PRIMITIVE_CHAR] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(char),
        .alignment = _Alignof(char),
        .name = "char",
        .data.primitive = {.primitive_kind = PRIMITIVE_CHAR},
        .ref_count = 1000000
    },
    [PRIMITIVE_STRING] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = false},
        .size = sizeof(void*),  // String is a pointer type
        .alignment = _Alignof(void*),
        .name = "string",
        .data.primitive = {.primitive_kind = PRIMITIVE_STRING},
        .ref_count = 1000000
    },
    [PRIMITIVE_NEVER] = {
        .category = TYPE_PRIMITIVE,
        .flags = {.is_constant = true},
        .size = 0,  // Never type has no values
        .alignment = 1,
        .name = "Never",
        .data.primitive = {.primitive_kind = PRIMITIVE_NEVER},
        .ref_count = 1000000
    }
};

// =============================================================================
// PREDECLARED IDENTIFIERS
// =============================================================================

// Predeclared identifier definitions
static PredeclaredIdentifier g_predeclared_identifiers[] = {
    // Utility functions
    {
        .name = "log",
        .signature = "fn(message: string) -> void",
        .kind = SYMBOL_FUNCTION,
        .type = NULL,  // Will be created dynamically
        .is_predeclared = true
    },
    {
        .name = "panic",
        .signature = "fn(message: string) -> Never",
        .kind = SYMBOL_FUNCTION,
        .type = NULL,  // Will be created dynamically
        .is_predeclared = true
    },
    
    // Range functions (overloaded)
    {
        .name = "range",
        .signature = "fn(end: i32) -> []i32",
        .kind = SYMBOL_FUNCTION,
        .type = NULL,
        .is_predeclared = true
    },
    {
        .name = "range",
        .signature = "fn(start: i32, end: i32) -> []i32",
        .kind = SYMBOL_FUNCTION, 
        .type = NULL,
        .is_predeclared = true
    },
    
    // Command-line arguments access
    {
        .name = "args",
        .signature = "fn() -> []string",
        .kind = SYMBOL_FUNCTION,
        .type = NULL,  // Will be created dynamically
        .is_predeclared = true
    },
    
    // Infinite iterator function
    {
        .name = "infinite",
        .signature = "fn() -> InfiniteIterator",
        .kind = SYMBOL_FUNCTION,
        .type = NULL,  // Will be created dynamically
        .is_predeclared = true
    }
};

static const size_t g_predeclared_count = sizeof(g_predeclared_identifiers) / sizeof(PredeclaredIdentifier);

// =============================================================================
// BUILTIN TYPE MANAGEMENT
// =============================================================================

bool semantic_is_builtin_type(const TypeDescriptor *type) {
    if (!type) return false;
    
    // Check if the type pointer is within the builtin types array
    const TypeDescriptor *start = &g_builtin_types[0];
    const TypeDescriptor *end = &g_builtin_types[PRIMITIVE_COUNT];
    
    return (type >= start && type < end);
}

void semantic_init_builtin_types(SemanticAnalyzer *analyzer) {
    if (!analyzer) return;
    
    analyzer->builtin_type_count = PRIMITIVE_COUNT;
    analyzer->builtin_types = malloc(sizeof(TypeDescriptor*) * PRIMITIVE_COUNT);
    if (!analyzer->builtin_types) return;
    
    // Point to static builtin type descriptors
    for (size_t i = 0; i < PRIMITIVE_COUNT; i++) {
        analyzer->builtin_types[i] = &g_builtin_types[i];
    }
    
    // Register builtin types in the global symbol table
    for (size_t i = 0; i < PRIMITIVE_COUNT; i++) {
        TypeDescriptor *type = &g_builtin_types[i];
        if (type->name) {
            SymbolEntry *type_symbol = symbol_entry_create(
                type->name,
                SYMBOL_TYPE,
                type,
                NULL  // No AST declaration for builtin types
            );
            
            if (type_symbol) {
                symbol_table_insert_safe(analyzer->global_scope, type->name, type_symbol);
            }
        }
    }
    
    // Register type aliases for compatibility
    // 'int' is an alias for 'i32'
    TypeDescriptor *i32_type = &g_builtin_types[PRIMITIVE_I32];
    SymbolEntry *int_alias = symbol_entry_create(
        "int",
        SYMBOL_TYPE,
        i32_type,
        NULL
    );
    if (int_alias) {
        symbol_table_insert_safe(analyzer->global_scope, "int", int_alias);
    }
    
    // 'float' is an alias for 'f32'
    TypeDescriptor *f32_type = &g_builtin_types[PRIMITIVE_F32];
    SymbolEntry *float_alias = symbol_entry_create(
        "float",
        SYMBOL_TYPE,
        f32_type,
        NULL
    );
    if (float_alias) {
        symbol_table_insert_safe(analyzer->global_scope, "float", float_alias);
    }
    
    // 'usize' is an alias for 'u64' (on 64-bit systems)
    TypeDescriptor *u64_type = &g_builtin_types[PRIMITIVE_U64];
    SymbolEntry *usize_alias = symbol_entry_create(
        "usize",
        SYMBOL_TYPE,
        u64_type,
        NULL
    );
    if (usize_alias) {
        symbol_table_insert_safe(analyzer->global_scope, "usize", usize_alias);
    }
    
    // 'isize' is an alias for 'i64' (on 64-bit systems)
    TypeDescriptor *i64_type = &g_builtin_types[PRIMITIVE_I64];
    SymbolEntry *isize_alias = symbol_entry_create(
        "isize",
        SYMBOL_TYPE,
        i64_type,
        NULL
    );
    if (isize_alias) {
        symbol_table_insert_safe(analyzer->global_scope, "isize", isize_alias);
    }
}

TypeDescriptor *semantic_get_builtin_type(SemanticAnalyzer *analyzer, const char *name) {
    if (!analyzer || !name) return NULL;
    
    for (size_t i = 0; i < analyzer->builtin_type_count; i++) {
        TypeDescriptor *type = analyzer->builtin_types[i];
        if (type && type->name && strcmp(type->name, name) == 0) {
            return type;
        }
    }
    
    // Check type aliases
    if (strcmp(name, "int") == 0) {
        return analyzer->builtin_types[PRIMITIVE_I32];
    } else if (strcmp(name, "float") == 0) {
        return analyzer->builtin_types[PRIMITIVE_F32];
    } else if (strcmp(name, "usize") == 0) {
        return analyzer->builtin_types[PRIMITIVE_U64];
    } else if (strcmp(name, "isize") == 0) {
        return analyzer->builtin_types[PRIMITIVE_I64];
    }
    
    return NULL;
}

// Simple helper to get builtin type without analyzer
TypeDescriptor *get_builtin_type(const char *name) {
    if (!name) return NULL;
    
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

// =============================================================================
// PREDECLARED IDENTIFIERS
// =============================================================================

TypeDescriptor *create_predeclared_function_type(const char *name, const char *signature) {
    // For now, create a basic function type descriptor
    // This is a simplified implementation - in a full implementation,
    // we would parse the signature string to create proper parameter types
    
    TypeDescriptor *func_type = malloc(sizeof(TypeDescriptor));
    if (!func_type) return NULL;
    
    // Initialize function type descriptor
    func_type->category = TYPE_FUNCTION;
    func_type->flags = (TypeFlags){0};
    func_type->size = sizeof(void*);  // Function pointer size
    func_type->alignment = _Alignof(void*);
    func_type->name = NULL;  // Function types don't have names
    atomic_init(&func_type->ref_count, 1);
    
    // Create function data - simplified for log and range
    func_type->data.function.is_extern = false;
    func_type->data.function.extern_name = NULL;
    func_type->data.function.ffi_annotations = NULL;
    func_type->data.function.ffi_annotation_count = 0;
    func_type->data.function.requires_ffi_marshaling = false;
    func_type->data.function.param_count = 0;
    func_type->data.function.param_types = NULL;
    func_type->data.function.return_type = NULL;
    
    if (strcmp(name, "log") == 0) {
        // log(message: string) -> void
        func_type->data.function.param_count = 1;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor*));
        func_type->data.function.param_types[0] = &g_builtin_types[PRIMITIVE_STRING];
        func_type->data.function.return_type = &g_builtin_types[PRIMITIVE_VOID];
    } else if (strcmp(name, "panic") == 0) {
        // panic(message: string) -> Never
        func_type->data.function.param_count = 1;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor*));
        func_type->data.function.param_types[0] = &g_builtin_types[PRIMITIVE_STRING];
        func_type->data.function.return_type = &g_builtin_types[PRIMITIVE_NEVER];
    } else if (strcmp(name, "range") == 0 && strstr(signature, "fn(end: i32)")) {
        // range(end: i32) -> []i32
        func_type->data.function.param_count = 1;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor*));
        func_type->data.function.param_types[0] = &g_builtin_types[PRIMITIVE_I32];
        
        // Create slice type for []i32
        TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
        slice_type->category = TYPE_SLICE;
        slice_type->flags = (TypeFlags){0};
        slice_type->size = sizeof(void*) + sizeof(size_t);  // ptr + len
        slice_type->alignment = _Alignof(void*);
        slice_type->name = strdup("[]i32");
        atomic_init(&slice_type->ref_count, 1);
        slice_type->data.slice.element_type = &g_builtin_types[PRIMITIVE_I32];
        
        func_type->data.function.return_type = slice_type;
    } else if (strcmp(name, "range") == 0 && strstr(signature, "fn(start: i32, end: i32)")) {
        // range(start: i32, end: i32) -> []i32
        func_type->data.function.param_count = 2;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor*) * 2);
        func_type->data.function.param_types[0] = &g_builtin_types[PRIMITIVE_I32];
        func_type->data.function.param_types[1] = &g_builtin_types[PRIMITIVE_I32];
        
        // Create slice type for []i32 (reuse from above if needed)
        TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
        slice_type->category = TYPE_SLICE;
        slice_type->flags = (TypeFlags){0};
        slice_type->size = sizeof(void*) + sizeof(size_t);  // ptr + len
        slice_type->alignment = _Alignof(void*);
        slice_type->name = strdup("[]i32");
        atomic_init(&slice_type->ref_count, 1);
        slice_type->data.slice.element_type = &g_builtin_types[PRIMITIVE_I32];
        
        func_type->data.function.return_type = slice_type;
    } else if (strcmp(name, "args") == 0) {
        // args() -> []string
        func_type->data.function.param_count = 0;
        func_type->data.function.param_types = NULL;
        
        // Create slice type for []string
        TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
        slice_type->category = TYPE_SLICE;
        slice_type->flags = (TypeFlags){0};
        slice_type->size = sizeof(void*) + sizeof(size_t);  // ptr + len
        slice_type->alignment = _Alignof(void*);
        slice_type->name = strdup("[]string");
        atomic_init(&slice_type->ref_count, 1);
        slice_type->data.slice.element_type = &g_builtin_types[PRIMITIVE_STRING];
        
        func_type->data.function.return_type = slice_type;
    } else if (strcmp(name, "infinite") == 0) {
        // infinite() -> InfiniteIterator
        func_type->data.function.param_count = 0;
        func_type->data.function.param_types = NULL;
        
        // Create InfiniteIterator type - a special slice type that represents an infinite iterator
        TypeDescriptor *iterator_type = malloc(sizeof(TypeDescriptor));
        iterator_type->category = TYPE_SLICE;  // Use slice type as base for iterators
        iterator_type->flags = (TypeFlags){0};
        iterator_type->size = sizeof(void*) + sizeof(size_t);  // ptr + len (though len is meaningless for infinite)
        iterator_type->alignment = _Alignof(void*);
        iterator_type->name = strdup("InfiniteIterator");
        atomic_init(&iterator_type->ref_count, 1);
        
        // The element type is void for now - the actual type will be determined in usage context
        iterator_type->data.slice.element_type = &g_builtin_types[PRIMITIVE_VOID];
        
        func_type->data.function.return_type = iterator_type;
    } else {
        // Default case - should not happen
        fprintf(stderr, "WARNING: Unknown predeclared function signature: %s\n", signature);
    }
    
    return func_type;
}

void semantic_init_predeclared_identifiers(SemanticAnalyzer *analyzer) {
    if (!analyzer) return;
    
    analyzer->predeclared_count = g_predeclared_count;
    analyzer->predeclared_identifiers = malloc(sizeof(PredeclaredIdentifier) * g_predeclared_count);
    if (!analyzer->predeclared_identifiers) return;
    
    // Copy predeclared identifiers and create their type descriptors
    for (size_t i = 0; i < g_predeclared_count; i++) {
        analyzer->predeclared_identifiers[i] = g_predeclared_identifiers[i];
        
        // Create type descriptor from name and signature string
        analyzer->predeclared_identifiers[i].type = 
            create_predeclared_function_type(g_predeclared_identifiers[i].name,
                                           g_predeclared_identifiers[i].signature);
    }
    
    // Add predeclared identifiers to global scope with special flag
    for (size_t i = 0; i < analyzer->predeclared_count; i++) {
        PredeclaredIdentifier *pred = &analyzer->predeclared_identifiers[i];
        
        SymbolEntry *entry = symbol_entry_create(
            pred->name,
            pred->kind, 
            pred->type,
            NULL  // No AST declaration node for predeclared
        );
        
        if (entry) {
            entry->flags.is_predeclared = true;  // Mark as predeclared
            symbol_table_insert_safe(analyzer->global_scope, pred->name, entry);
        }
    }
}

// =============================================================================
// BUILTIN GENERIC TYPES
// =============================================================================

void semantic_init_builtin_generic_types(SemanticAnalyzer *analyzer) {
    if (!analyzer) return;
    
    // Register Option<T> as a builtin generic enum type
    // Create the Option enum type descriptor
    TypeDescriptor *option_type = malloc(sizeof(TypeDescriptor));
    if (!option_type) return;
    
    // Initialize Option type descriptor
    option_type->category = TYPE_ENUM;
    option_type->flags = (TypeFlags){0};
    option_type->flags.is_ffi_compatible = true;
    option_type->size = sizeof(int);  // Enum size
    option_type->alignment = _Alignof(int);
    option_type->name = strdup("Option");
    atomic_init(&option_type->ref_count, 1);
    
    // Create variant symbol table
    option_type->data.enum_type.variants = symbol_table_create(4);
    option_type->data.enum_type.variant_count = 2;
    
    // Create Option symbol entry as generic
    SymbolEntry *option_symbol = symbol_entry_create(
        "Option",
        SYMBOL_TYPE,
        option_type,
        NULL  // No AST declaration for builtin
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
            
            SymbolEntry *some_symbol = symbol_entry_create(
                "Some",
                SYMBOL_ENUM_VARIANT,
                some_type,
                NULL
            );
            
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
            
            SymbolEntry *none_symbol = symbol_entry_create(
                "None",
                SYMBOL_ENUM_VARIANT,
                none_type,
                NULL
            );
            
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
    if (!result_type) return;
    
    // Initialize Result type descriptor
    result_type->category = TYPE_ENUM;
    result_type->flags = (TypeFlags){0};
    result_type->flags.is_ffi_compatible = true;
    result_type->size = sizeof(int);  // Enum size
    result_type->alignment = _Alignof(int);
    result_type->name = strdup("Result");
    atomic_init(&result_type->ref_count, 1);
    
    // Create variant symbol table
    result_type->data.enum_type.variants = symbol_table_create(4);
    result_type->data.enum_type.variant_count = 2;
    
    // Create Result symbol entry as generic
    SymbolEntry *result_symbol = symbol_entry_create(
        "Result",
        SYMBOL_TYPE,
        result_type,
        NULL  // No AST declaration for builtin
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
            
            SymbolEntry *ok_symbol = symbol_entry_create(
                "Ok",
                SYMBOL_ENUM_VARIANT,
                ok_type,
                NULL
            );
            
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
            
            SymbolEntry *err_symbol = symbol_entry_create(
                "Err",
                SYMBOL_ENUM_VARIANT,
                err_type,
                NULL
            );
            
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
