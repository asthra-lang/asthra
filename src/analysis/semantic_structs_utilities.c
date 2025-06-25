/*
 * Semantic Structs Utilities Module
 * Helper functions and utilities for struct semantic analysis
 *
 * Part of semantic_structs.c split (578 lines -> 5 focused modules)
 * Provides utility functions for type checking and validation
 */

#include "semantic_structs_common.h"

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Check if a name conflicts with built-in types
 */
bool is_builtin_type_name(const char *name) {
    if (!name)
        return false;

    static const char *builtin_types[] = {"int",   "float", "bool", "string", "void", "usize",
                                          "isize", "u8",    "i8",   "u16",    "i16",  "u32",
                                          "i32",   "u64",   "i64",  "f32",    "f64",  NULL};

    for (size_t i = 0; builtin_types[i]; i++) {
        if (strcmp(name, builtin_types[i]) == 0) {
            return true;
        }
    }

    return false;
}
