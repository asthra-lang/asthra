/**
 * Asthra Programming Language Compiler
 * Code generation types - type conversion utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <string.h>

// Helper function to convert AST type nodes to C type strings
const char *get_c_type_string(ASTNode *type_node) {
    if (!type_node)
        return "void";

    switch (type_node->type) {
    case AST_BASE_TYPE:
        if (type_node->data.base_type.name) {
            const char *name = type_node->data.base_type.name;
            if (strcmp(name, "bool") == 0)
                return "int";
            if (strcmp(name, "i32") == 0 || strcmp(name, "int") == 0)
                return "int";
            if (strcmp(name, "i64") == 0)
                return "long long";
            if (strcmp(name, "f32") == 0 || strcmp(name, "float") == 0)
                return "float";
            if (strcmp(name, "f64") == 0)
                return "double";
            if (strcmp(name, "string") == 0)
                return "char*";
            if (strcmp(name, "void") == 0)
                return "void";
            if (strcmp(name, "usize") == 0)
                return "size_t";
        }
        break;
    case AST_SLICE_TYPE:
        return "AsthraSliceHeader";
    case AST_PTR_TYPE:
        // Simplified pointer type handling
        return "void*";
    default:
        break;
    }

    return "void";
}