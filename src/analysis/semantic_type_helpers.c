/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Helper Functions Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_errors.h"
#include "semantic_utilities.h"
#include "semantic_symbols.h"
#include "type_info.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TYPE PROMOTION AND CONVERSION
// =============================================================================

TypeDescriptor* get_promoted_type(SemanticAnalyzer *analyzer, 
                                 TypeDescriptor *left_type, 
                                 TypeDescriptor *right_type) {
    if (!analyzer || !left_type || !right_type) return NULL;
    
    // If same type, return it
    if (semantic_types_equal(left_type, right_type)) {
        return left_type;
    }
    
    // Handle string concatenation
    if (is_string_type(left_type) && is_string_type(right_type)) {
        return get_builtin_type_descriptor(analyzer, "string");
    }
    
    // Numeric type promotion
    if (is_numeric_type(left_type) && is_numeric_type(right_type)) {
        // Float takes precedence over integer
        if (left_type->category == TYPE_FLOAT || right_type->category == TYPE_FLOAT) {
            // Promote to the larger float type
            if (left_type->category == TYPE_FLOAT && right_type->category == TYPE_FLOAT) {
                // f64 > f32
                if (strcmp(left_type->name, "f64") == 0 || strcmp(right_type->name, "f64") == 0) {
                    return get_builtin_type_descriptor(analyzer, "f64");
                }
                return get_builtin_type_descriptor(analyzer, "f32");
            }
            // One float, one integer - promote to float
            return (left_type->category == TYPE_FLOAT) ? left_type : right_type;
        }
        
        // Both integers - promote to larger size
        // i64 > i32 > i16 > i8
        // u64 > u32 > u16 > u8
        
        // If one is signed and one unsigned of same size, promote to signed
        bool left_signed = (left_type->name[0] == 'i');
        bool right_signed = (right_type->name[0] == 'i');
        
        // Extract size from type name (e.g., "i32" -> 32)
        int left_size = atoi(left_type->name + 1);
        int right_size = atoi(right_type->name + 1);
        
        if (left_size > right_size) return left_type;
        if (right_size > left_size) return right_type;
        
        // Same size - prefer signed
        if (left_signed) return left_type;
        if (right_signed) return right_type;
        
        return left_type; // Both same size and signedness
    }
    
    return NULL;
}

TypeDescriptor* get_promoted_integer_type(SemanticAnalyzer *analyzer,
                                        TypeDescriptor *left_type,
                                        TypeDescriptor *right_type) {
    if (!analyzer || !left_type || !right_type) return NULL;
    
    // Both must be integers
    if (!is_integer_type(left_type) || !is_integer_type(right_type)) {
        return NULL;
    }
    
    // Use same promotion logic as general numeric promotion
    return get_promoted_type(analyzer, left_type, right_type);
}

TypeDescriptor* get_builtin_type_descriptor(SemanticAnalyzer *analyzer, const char *name) {
    if (!analyzer || !name) return NULL;
    
    // Look up in the builtin types
    TypeDescriptor *type = get_builtin_type(name);
    if (type) return type;
    
    // If not found in builtins, try to find in symbol table
    SymbolEntry *symbol = symbol_table_lookup_safe(analyzer->current_scope, name);
    if (symbol && symbol->kind == SYMBOL_TYPE) {
        return symbol->type;
    }
    
    return NULL;
}

TypeInfo* create_type_info_from_descriptor(TypeDescriptor *desc) {
    if (!desc) return NULL;
    
    // Use the proper type_info creation function
    TypeInfo *info = type_info_from_descriptor(desc);
    return info;
}