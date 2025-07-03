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
#include <stdio.h>
#include <string.h>

// =============================================================================
// BUILTIN TYPE MANAGEMENT
// =============================================================================

bool semantic_is_builtin_type(const TypeDescriptor *type) {
    // Delegate to primitive type check
    return semantic_is_primitive_type(type);
}

void semantic_init_builtin_types(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    // Initialize primitive types
    semantic_init_primitive_types(analyzer);
}

TypeDescriptor *semantic_get_builtin_type(SemanticAnalyzer *analyzer, const char *name) {
    if (!analyzer || !name)
        return NULL;

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
    }
    // Note: usize and isize are not aliases - they have their own primitive types
    // They should be found in the main loop above

    // Fallback for usize/isize if not found in main loop
    if (strcmp(name, "usize") == 0) {
        return get_builtin_type("usize");
    } else if (strcmp(name, "isize") == 0) {
        return get_builtin_type("isize");
    }

    return NULL;
}

// =============================================================================
// PREDECLARED IDENTIFIERS
// =============================================================================

void semantic_init_predeclared_identifiers(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    // Initialize predeclared functions
    semantic_init_predeclared_functions(analyzer);

    // Initialize builtin generic types
    semantic_init_builtin_generic_types(analyzer);
}