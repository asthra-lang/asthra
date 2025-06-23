/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Helper Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Helper functions for type checking and manipulation
 */

#ifndef ASTHRA_SEMANTIC_TYPE_HELPERS_H
#define ASTHRA_SEMANTIC_TYPE_HELPERS_H

#include "semantic_types_defs.h"
#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE CHECKING HELPERS
// =============================================================================

/**
 * Check if a type is numeric (integer or floating point)
 */
static inline bool is_numeric_type(TypeDescriptor *type) {
    if (!type) return false;
    if (type->category == TYPE_PRIMITIVE) {
        PrimitiveKind kind = (PrimitiveKind)type->data.primitive.primitive_kind;
        return (kind >= PRIMITIVE_I8 && kind <= PRIMITIVE_USIZE) ||
               (kind == PRIMITIVE_F32 || kind == PRIMITIVE_F64);
    }
    return type->category == TYPE_INTEGER || 
           type->category == TYPE_FLOAT;
}

/**
 * Check if a type is an integer type
 */
static inline bool is_integer_type(TypeDescriptor *type) {
    if (!type) return false;
    if (type->category == TYPE_PRIMITIVE) {
        PrimitiveKind kind = (PrimitiveKind)type->data.primitive.primitive_kind;
        return (kind >= PRIMITIVE_I8 && kind <= PRIMITIVE_USIZE);
    }
    return type->category == TYPE_INTEGER;
}

/**
 * Check if a type is a boolean type
 */
static inline bool is_boolean_type(TypeDescriptor *type) {
    if (!type) return false;
    if (type->category == TYPE_PRIMITIVE) {
        PrimitiveKind kind = (PrimitiveKind)type->data.primitive.primitive_kind;
        return kind == PRIMITIVE_BOOL;
    }
    return type->category == TYPE_BOOL;
}

/**
 * Check if a type is a pointer type
 */
static inline bool is_pointer_type(TypeDescriptor *type) {
    if (!type) return false;
    return type->category == TYPE_POINTER;
}

/**
 * Check if a type is a string type
 */
static inline bool is_string_type(TypeDescriptor *type) {
    if (!type) return false;
    // In Asthra, string is a builtin type
    return type->category == TYPE_BUILTIN && 
           strcmp(type->name, "string") == 0;
}

/**
 * Check if a type is an array type
 */
static inline bool is_array_type(TypeDescriptor *type) {
    if (!type) return false;
    return type->category == TYPE_ARRAY;
}

/**
 * Check if a type is a slice type
 */
static inline bool is_slice_type(TypeDescriptor *type) {
    if (!type) return false;
    return type->category == TYPE_SLICE;
}

/**
 * Check if a type is indexable (array, slice, or pointer)
 */
static inline bool is_indexable_type(TypeDescriptor *type) {
    if (!type) return false;
    return type->category == TYPE_ARRAY || 
           type->category == TYPE_SLICE || 
           type->category == TYPE_POINTER;
}

// =============================================================================
// TYPE PROMOTION AND CONVERSION
// =============================================================================

/**
 * Get promoted type for arithmetic operations
 */
TypeDescriptor* get_promoted_type(SemanticAnalyzer *analyzer, 
                                 TypeDescriptor *left_type, 
                                 TypeDescriptor *right_type);

/**
 * Get promoted integer type for bitwise operations
 */
TypeDescriptor* get_promoted_integer_type(SemanticAnalyzer *analyzer,
                                        TypeDescriptor *left_type,
                                        TypeDescriptor *right_type);

/**
 * Get builtin type descriptor by name
 */
TypeDescriptor* get_builtin_type_descriptor(SemanticAnalyzer *analyzer, const char *name);

/**
 * Create type info from type descriptor
 */
TypeInfo* create_type_info_from_descriptor(TypeDescriptor *desc);

// =============================================================================
// OPERATOR STRING CONVERSION
// =============================================================================

/**
 * Convert binary operator to string
 */
static inline const char* binary_op_to_string(BinaryOperator op) {
    switch (op) {
        case BINOP_ADD: return "+";
        case BINOP_SUB: return "-";
        case BINOP_MUL: return "*";
        case BINOP_DIV: return "/";
        case BINOP_MOD: return "%";
        case BINOP_EQ: return "==";
        case BINOP_NE: return "!=";
        case BINOP_LT: return "<";
        case BINOP_LE: return "<=";
        case BINOP_GT: return ">";
        case BINOP_GE: return ">=";
        case BINOP_AND: return "&&";
        case BINOP_OR: return "||";
        case BINOP_BITWISE_AND: return "&";
        case BINOP_BITWISE_OR: return "|";
        case BINOP_BITWISE_XOR: return "^";
        case BINOP_LSHIFT: return "<<";
        case BINOP_RSHIFT: return ">>";
        default: return "unknown";
    }
}

/**
 * Convert unary operator to string
 */
static inline const char* unary_op_to_string(UnaryOperator op) {
    switch (op) {
        case UNOP_MINUS: return "-";
        case UNOP_NOT: return "!";
        case UNOP_BITWISE_NOT: return "~";
        case UNOP_ADDRESS_OF: return "&";
        case UNOP_DEREF: return "*";
        case UNOP_SIZEOF: return "sizeof";
        default: return "unknown";
    }
}

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_TYPE_HELPERS_H
