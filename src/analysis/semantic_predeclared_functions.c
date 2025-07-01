/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Predeclared Functions Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Predeclared function definitions and management
 */

#include "semantic_predeclared_functions.h"
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
// PREDECLARED FUNCTION DEFINITIONS
// =============================================================================

// Predeclared identifier definitions
static PredeclaredIdentifier g_predeclared_identifiers[] = {
    // Utility functions
    {.name = "log",
     .signature = "fn(message: string) -> void",
     .kind = SYMBOL_FUNCTION,
     .type = NULL, // Will be created dynamically
     .is_predeclared = true},
    {.name = "panic",
     .signature = "fn(message: string) -> Never",
     .kind = SYMBOL_FUNCTION,
     .type = NULL, // Will be created dynamically
     .is_predeclared = true},
    {.name = "exit",
     .signature = "fn(status: i32) -> Never",
     .kind = SYMBOL_FUNCTION,
     .type = NULL, // Will be created dynamically
     .is_predeclared = true},

    // Range functions (overloaded)
    {.name = "range",
     .signature = "fn(end: i32) -> []i32",
     .kind = SYMBOL_FUNCTION,
     .type = NULL,
     .is_predeclared = true},
    {.name = "range",
     .signature = "fn(start: i32, end: i32) -> []i32",
     .kind = SYMBOL_FUNCTION,
     .type = NULL,
     .is_predeclared = true},

    // Command-line arguments access
    {.name = "args",
     .signature = "fn() -> []string",
     .kind = SYMBOL_FUNCTION,
     .type = NULL, // Will be created dynamically
     .is_predeclared = true},

    // Infinite iterator function
    {.name = "infinite",
     .signature = "fn() -> InfiniteIterator",
     .kind = SYMBOL_FUNCTION,
     .type = NULL, // Will be created dynamically
     .is_predeclared = true},

    // Length function (overloaded for slices and arrays)
    {.name = "len",
     .signature = "fn(slice: []T) -> usize",
     .kind = SYMBOL_FUNCTION,
     .type = NULL, // Will be created dynamically
     .is_predeclared = true}};

static const size_t g_predeclared_count =
    sizeof(g_predeclared_identifiers) / sizeof(PredeclaredIdentifier);

// =============================================================================
// FUNCTION TYPE CREATION
// =============================================================================

TypeDescriptor *create_predeclared_function_type(const char *name, const char *signature) {
    // For now, create a basic function type descriptor
    // This is a simplified implementation - in a full implementation,
    // we would parse the signature string to create proper parameter types

    TypeDescriptor *func_type = malloc(sizeof(TypeDescriptor));
    if (!func_type)
        return NULL;

    // Get primitive types array
    TypeDescriptor *primitive_types = semantic_get_primitive_types_array();

    // Initialize function type descriptor
    func_type->category = TYPE_FUNCTION;
    func_type->flags = (TypeFlags){0};
    func_type->size = sizeof(void *); // Function pointer size
    func_type->alignment = _Alignof(void *);
    func_type->name = NULL; // Function types don't have names
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
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor *));
        func_type->data.function.param_types[0] = &primitive_types[PRIMITIVE_STRING];
        func_type->data.function.return_type = &primitive_types[PRIMITIVE_VOID];
    } else if (strcmp(name, "panic") == 0) {
        // panic(message: string) -> Never
        func_type->data.function.param_count = 1;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor *));
        func_type->data.function.param_types[0] = &primitive_types[PRIMITIVE_STRING];
        func_type->data.function.return_type = &primitive_types[PRIMITIVE_NEVER];
    } else if (strcmp(name, "exit") == 0) {
        // exit(status: i32) -> Never
        func_type->data.function.param_count = 1;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor *));
        func_type->data.function.param_types[0] = &primitive_types[PRIMITIVE_I32];
        func_type->data.function.return_type = &primitive_types[PRIMITIVE_NEVER];
    } else if (strcmp(name, "range") == 0 && strstr(signature, "fn(end: i32)")) {
        // range(end: i32) -> []i32
        func_type->data.function.param_count = 1;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor *));
        func_type->data.function.param_types[0] = &primitive_types[PRIMITIVE_I32];

        // Create slice type for []i32
        TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
        slice_type->category = TYPE_SLICE;
        slice_type->flags = (TypeFlags){0};
        slice_type->size = sizeof(void *) + sizeof(size_t); // ptr + len
        slice_type->alignment = _Alignof(void *);
        slice_type->name = strdup("[]i32");
        atomic_init(&slice_type->ref_count, 1);
        slice_type->data.slice.element_type = &primitive_types[PRIMITIVE_I32];

        func_type->data.function.return_type = slice_type;
    } else if (strcmp(name, "range") == 0 && strstr(signature, "fn(start: i32, end: i32)")) {
        // range(start: i32, end: i32) -> []i32
        func_type->data.function.param_count = 2;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor *) * 2);
        func_type->data.function.param_types[0] = &primitive_types[PRIMITIVE_I32];
        func_type->data.function.param_types[1] = &primitive_types[PRIMITIVE_I32];

        // Create slice type for []i32 (reuse from above if needed)
        TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
        slice_type->category = TYPE_SLICE;
        slice_type->flags = (TypeFlags){0};
        slice_type->size = sizeof(void *) + sizeof(size_t); // ptr + len
        slice_type->alignment = _Alignof(void *);
        slice_type->name = strdup("[]i32");
        atomic_init(&slice_type->ref_count, 1);
        slice_type->data.slice.element_type = &primitive_types[PRIMITIVE_I32];

        func_type->data.function.return_type = slice_type;
    } else if (strcmp(name, "args") == 0) {
        // args() -> []string
        func_type->data.function.param_count = 0;
        func_type->data.function.param_types = NULL;

        // Create slice type for []string
        TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
        slice_type->category = TYPE_SLICE;
        slice_type->flags = (TypeFlags){0};
        slice_type->size = sizeof(void *) + sizeof(size_t); // ptr + len
        slice_type->alignment = _Alignof(void *);
        slice_type->name = strdup("[]string");
        atomic_init(&slice_type->ref_count, 1);
        slice_type->data.slice.element_type = &primitive_types[PRIMITIVE_STRING];

        func_type->data.function.return_type = slice_type;
    } else if (strcmp(name, "infinite") == 0) {
        // infinite() -> InfiniteIterator
        func_type->data.function.param_count = 0;
        func_type->data.function.param_types = NULL;

        // Create InfiniteIterator type - a special slice type that represents an infinite iterator
        TypeDescriptor *iterator_type = malloc(sizeof(TypeDescriptor));
        iterator_type->category = TYPE_SLICE; // Use slice type as base for iterators
        iterator_type->flags = (TypeFlags){0};
        iterator_type->size =
            sizeof(void *) + sizeof(size_t); // ptr + len (though len is meaningless for infinite)
        iterator_type->alignment = _Alignof(void *);
        iterator_type->name = strdup("InfiniteIterator");
        atomic_init(&iterator_type->ref_count, 1);

        // The element type is void for now - the actual type will be determined in usage context
        iterator_type->data.slice.element_type = &primitive_types[PRIMITIVE_VOID];

        func_type->data.function.return_type = iterator_type;
    } else if (strcmp(name, "len") == 0) {
        // len(slice: []T) -> usize
        func_type->data.function.param_count = 1;
        func_type->data.function.param_types = malloc(sizeof(TypeDescriptor *));
        
        // Create a generic slice type for []T - using void as placeholder
        TypeDescriptor *slice_type = malloc(sizeof(TypeDescriptor));
        slice_type->category = TYPE_SLICE;
        slice_type->flags = (TypeFlags){0};
        slice_type->size = sizeof(void *) + sizeof(size_t);
        slice_type->alignment = _Alignof(void *);
        slice_type->name = strdup("[]T");
        atomic_init(&slice_type->ref_count, 1);
        slice_type->data.slice.element_type = &primitive_types[PRIMITIVE_VOID]; // Generic placeholder
        
        func_type->data.function.param_types[0] = slice_type;
        func_type->data.function.return_type = &primitive_types[PRIMITIVE_USIZE];
    } else {
        // Default case - should not happen
        fprintf(stderr, "WARNING: Unknown predeclared function signature: %s\n", signature);
    }

    return func_type;
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

void semantic_init_predeclared_functions(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    analyzer->predeclared_count = g_predeclared_count;
    analyzer->predeclared_identifiers = malloc(sizeof(PredeclaredIdentifier) * g_predeclared_count);
    if (!analyzer->predeclared_identifiers)
        return;

    // Copy predeclared identifiers and create their type descriptors
    for (size_t i = 0; i < g_predeclared_count; i++) {
        analyzer->predeclared_identifiers[i] = g_predeclared_identifiers[i];

        // Create type descriptor from name and signature string
        analyzer->predeclared_identifiers[i].type = create_predeclared_function_type(
            g_predeclared_identifiers[i].name, g_predeclared_identifiers[i].signature);
    }

    // Add predeclared identifiers to global scope with special flag
    for (size_t i = 0; i < analyzer->predeclared_count; i++) {
        PredeclaredIdentifier *pred = &analyzer->predeclared_identifiers[i];

        SymbolEntry *entry = symbol_entry_create(pred->name, pred->kind, pred->type,
                                                 NULL // No AST declaration node for predeclared
        );

        if (entry) {
            entry->flags.is_predeclared = true; // Mark as predeclared
            symbol_table_insert_safe(analyzer->global_scope, pred->name, entry);
        }
    }
}

size_t semantic_get_predeclared_function_count(void) {
    return g_predeclared_count;
}