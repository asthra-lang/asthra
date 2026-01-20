/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Const Declaration Analysis
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 2: Semantic Analysis Implementation
 * Analysis of const declarations and compile-time evaluation
 */

#include "semantic_const_declarations.h"
#include "const_evaluator.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CONST DECLARATION ANALYSIS
// =============================================================================

/**
 * Analyze a const declaration and add it to the symbol table
 */
bool analyze_const_declaration(SemanticAnalyzer *analyzer, ASTNode *const_decl) {
    if (!analyzer || !const_decl || const_decl->type != AST_CONST_DECL) {
        return false;
    }

    const char *const_name = const_decl->data.const_decl.name;
    ASTNode *type_node = const_decl->data.const_decl.type;
    ASTNode *value_node = const_decl->data.const_decl.value;
    VisibilityType visibility = const_decl->data.const_decl.visibility;

    if (!const_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, const_decl->location,
                              "Const declaration missing name");
        return false;
    }

    if (!type_node) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, const_decl->location,
                              "Const declaration missing required type annotation");
        return false;
    }

    if (!value_node) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, const_decl->location,
                              "Const declaration missing required value expression");
        return false;
    }

    // Check if const name is already declared in current scope
    SymbolEntry *existing = symbol_table_lookup_local(analyzer->current_scope, const_name);
    if (existing) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, const_decl->location,
                              "Const '%s' is already declared", const_name);
        return false;
    }

    // Analyze the type annotation
    TypeDescriptor *const_type = analyze_type_node(analyzer, type_node);
    if (!const_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, type_node->location,
                              "Invalid type annotation in const declaration for '%s'", const_name);
        return false;
    }

    // Check for dependency cycles before evaluation
    if (!check_const_dependency_cycle(analyzer, const_name, value_node)) {
        type_descriptor_release(const_type);
        return false;
    }

    // Evaluate the const expression
    ConstValue *const_value = evaluate_const_expression(analyzer, value_node);
    if (!const_value) {
        type_descriptor_release(const_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, value_node->location,
                              "Cannot evaluate const expression for '%s'", const_name);
        return false;
    }

    // Validate type compatibility between declared type and evaluated value
    if (!validate_const_type_compatibility(analyzer, const_type, const_value,
                                           const_decl->location)) {
        const_value_destroy(const_value);
        type_descriptor_release(const_type);
        return false;
    }

    // Create symbol entry for the const
    SymbolEntry *const_symbol =
        symbol_entry_create(const_name, SYMBOL_CONST, const_type, const_decl);
    if (!const_symbol) {
        const_value_destroy(const_value);
        type_descriptor_release(const_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, const_decl->location,
                              "Failed to create symbol entry for const '%s'", const_name);
        return false;
    }

    // Set const-specific properties
    const_symbol->const_value = const_value;
    const_symbol->visibility = visibility;
    const_symbol->flags.is_initialized = true;
    const_symbol->flags.is_mutable = false; // Constants are immutable

    // Register the const in the symbol table
    if (!symbol_table_insert_safe(analyzer->current_scope, const_name, const_symbol)) {
        const_value_destroy(const_value);
        symbol_entry_destroy(const_symbol);
        type_descriptor_release(const_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, const_decl->location,
                              "Failed to register const '%s' in symbol table", const_name);
        return false;
    }

    return true;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get a human-readable name for a const value type
 */
static const char *get_const_value_type_name(ConstValueType type) {
    switch (type) {
    case CONST_VALUE_INTEGER:
        return "integer";
    case CONST_VALUE_FLOAT:
        return "float";
    case CONST_VALUE_STRING:
        return "string";
    case CONST_VALUE_BOOLEAN:
        return "boolean";
    default:
        return "unknown";
    }
}

/**
 * Validate integer value fits in declared integer type
 */
static bool validate_integer_range(SemanticAnalyzer *analyzer, PrimitiveKind int_type,
                                   int64_t value, SourceLocation location) {
    if (!analyzer)
        return false;

    switch (int_type) {
    case PRIMITIVE_I8:
        if (value < -128 || value > 127) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld out of range for i8 (-128 to 127)", (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_U8:
        if (value < 0 || value > 255) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld out of range for u8 (0 to 255)", (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_I16:
        if (value < -32768 || value > 32767) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld out of range for i16 (-32768 to 32767)",
                                  (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_U16:
        if (value < 0 || value > 65535) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld out of range for u16 (0 to 65535)", (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_I32:
        if (value < -2147483648LL || value > 2147483647LL) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld out of range for i32", (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_U32:
        if (value < 0 || (uint64_t)value > 4294967295ULL) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld out of range for u32", (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_I64:
        // int64_t can hold any valid i64 value
        break;

    case PRIMITIVE_U64:
        if (value < 0) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld cannot be negative for unsigned type",
                                  (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_USIZE:
        // usize is platform-dependent, typically 64-bit on 64-bit systems
        if (value < 0) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld cannot be negative for usize type", (long long)value);
            return false;
        }
        break;

    case PRIMITIVE_ISIZE:
        // isize is platform-dependent, typically 64-bit on 64-bit systems
        // Can hold any int64_t value
        break;

    case PRIMITIVE_CHAR:
        // char is typically 0-255 (unsigned on most platforms)
        if (value < 0 || value > 255) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                  "Value %lld out of range for char (0 to 255)", (long long)value);
            return false;
        }
        break;

    default:
        return false;
    }

    return true;
}

// =============================================================================
// TYPE COMPATIBILITY VALIDATION
// =============================================================================

/**
 * Validate that the evaluated const value is compatible with the declared type
 */
bool validate_const_type_compatibility(SemanticAnalyzer *analyzer, TypeDescriptor *declared_type,
                                       ConstValue *const_value, SourceLocation location) {
    if (!analyzer || !declared_type || !const_value)
        return false;

    // Check primitive type compatibility
    if (declared_type->category == TYPE_PRIMITIVE) {
        switch (declared_type->data.primitive.primitive_kind) {
        case PRIMITIVE_I8:
        case PRIMITIVE_I16:
        case PRIMITIVE_I32:
        case PRIMITIVE_I64:
        case PRIMITIVE_U8:
        case PRIMITIVE_U16:
        case PRIMITIVE_U32:
        case PRIMITIVE_U64:
        case PRIMITIVE_USIZE:
        case PRIMITIVE_ISIZE:
        case PRIMITIVE_CHAR:
            if (const_value->type != CONST_VALUE_INTEGER) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                      "Integer type expected, but got %s",
                                      get_const_value_type_name(const_value->type));
                return false;
            }

            // Check value range for specific integer types
            return validate_integer_range(
                analyzer, (PrimitiveKind)declared_type->data.primitive.primitive_kind,
                const_value->data.integer_value, location);

        case PRIMITIVE_F32:
        case PRIMITIVE_F64:
            if (const_value->type != CONST_VALUE_FLOAT &&
                const_value->type != CONST_VALUE_INTEGER) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                      "Numeric type expected, but got %s",
                                      get_const_value_type_name(const_value->type));
                return false;
            }
            return true;

        case PRIMITIVE_BOOL:
            if (const_value->type != CONST_VALUE_BOOLEAN) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                      "Boolean type expected, but got %s",
                                      get_const_value_type_name(const_value->type));
                return false;
            }
            return true;

        case PRIMITIVE_STRING:
            if (const_value->type != CONST_VALUE_STRING) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                                      "String type expected, but got %s",
                                      get_const_value_type_name(const_value->type));
                return false;
            }
            return true;

        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, location,
                                  "Unsupported type for const declaration");
            return false;
        }
    }

    // For now, only primitive types are supported in const declarations
    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, location,
                          "Only primitive types are supported in const declarations");
    return false;
}
