/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Helper Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Type promotion, node analysis dispatch, and generic helper functions
 */

#include "semantic_helpers.h"
#include "type_checking.h"
#include "semantic_utilities.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_errors.h"
#include "semantic_macros.h"
#include "type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// C17 feature detection and compatibility
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
    #define ASTHRA_HAS_C17 1
#else
    #define ASTHRA_HAS_C17 0
#endif

#if ASTHRA_HAS_C17 && !defined(__STDC_NO_ATOMICS__)
    #define ASTHRA_HAS_ATOMICS 1
#else
    #define ASTHRA_HAS_ATOMICS 0
#endif

// Atomic operation wrappers with fallbacks
#if ASTHRA_HAS_ATOMICS
    #define ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
#else
    #define ATOMIC_FETCH_ADD(ptr, val) __sync_fetch_and_add(ptr, val)
#endif

// =============================================================================
// TYPE PROMOTION AND UTILITY FUNCTIONS
// =============================================================================

/**
 * Promote arithmetic types according to type promotion rules
 * Returns the wider type that both operands should be promoted to
 */
TypeDescriptor *semantic_promote_arithmetic_types(SemanticAnalyzer *analyzer, 
                                                 TypeDescriptor *left_type, 
                                                 TypeDescriptor *right_type) {
    if (!analyzer || !left_type || !right_type) {
        return NULL;
    }
    
    // If both types are the same, no promotion needed
    if (semantic_types_equal(left_type, right_type)) {
        type_descriptor_retain(left_type);
        return left_type;
    }
    
    // For primitive types, apply standard promotion rules
    if (left_type->category == TYPE_PRIMITIVE && right_type->category == TYPE_PRIMITIVE) {
        PrimitiveKind left_prim = (PrimitiveKind)left_type->data.primitive.primitive_kind;
        PrimitiveKind right_prim = (PrimitiveKind)right_type->data.primitive.primitive_kind;
        
        // Float promotions (f32 -> f64)
        if ((left_prim == PRIMITIVE_F32 && right_prim == PRIMITIVE_F64) ||
            (left_prim == PRIMITIVE_F64 && right_prim == PRIMITIVE_F32)) {
            TypeDescriptor *f64_type = semantic_get_builtin_type(analyzer, "f64");
            if (f64_type) {
                type_descriptor_retain(f64_type);
            }
            return f64_type;
        }
        
        // Integer promotions (i8 -> i16 -> i32 -> i64)
        static const struct {
            PrimitiveKind kind;
            const char *name;
            int rank;
        } integer_ranks[] = {
            {PRIMITIVE_I8, "i8", 1},
            {PRIMITIVE_I16, "i16", 2}, 
            {PRIMITIVE_I32, "i32", 3},
            {PRIMITIVE_I64, "i64", 4},
            {PRIMITIVE_U8, "u8", 1},
            {PRIMITIVE_U16, "u16", 2},
            {PRIMITIVE_U32, "u32", 3}, 
            {PRIMITIVE_U64, "u64", 4}
        };
        
        int left_rank = 0, right_rank = 0;
        const char *result_type_name = NULL;
        
        // Find ranks for both types
        for (size_t i = 0; i < sizeof(integer_ranks)/sizeof(integer_ranks[0]); i++) {
            if (integer_ranks[i].kind == left_prim) {
                left_rank = integer_ranks[i].rank;
            }
            if (integer_ranks[i].kind == right_prim) {
                right_rank = integer_ranks[i].rank;
            }
        }
        
        // Promote to the higher rank type
        if (left_rank > 0 && right_rank > 0) {
            int target_rank = (left_rank > right_rank) ? left_rank : right_rank;
            
            // Find the type name for the target rank
            for (size_t i = 0; i < sizeof(integer_ranks)/sizeof(integer_ranks[0]); i++) {
                if (integer_ranks[i].rank == target_rank) {
                    result_type_name = integer_ranks[i].name;
                    break;
                }
            }
            
            if (result_type_name) {
                TypeDescriptor *result_type = semantic_get_builtin_type(analyzer, result_type_name);
                if (result_type) {
                    type_descriptor_retain(result_type);
                }
                return result_type;
            }
        }
    }
    
    // Default: return the left type if no specific promotion rule applies
    type_descriptor_retain(left_type);
    return left_type;
}

/**
 * Check if a type is a boolean type
 */
bool semantic_is_bool_type(TypeDescriptor *type) {
    if (!type || type->category != TYPE_PRIMITIVE) {
        return false;
    }
    return type->data.primitive.primitive_kind == PRIMITIVE_BOOL;
}

/**
 * Set the type of an expression (store it in the expression node)
 */
void semantic_set_expression_type(SemanticAnalyzer *analyzer, ASTNode *expr, TypeDescriptor *type) {
    (void)analyzer; // Unused for now
    if (!expr || !type) {
        return;
    }
    
    // Create a proper TypeInfo structure and store the type
    if (!expr->type_info) {
        expr->type_info = malloc(sizeof(TypeInfo));
        if (!expr->type_info) {
            return; // Memory allocation failed
        }
        memset(expr->type_info, 0, sizeof(TypeInfo));
    }
    
    // Store the type descriptor with proper reference counting
    if (expr->type_info->type_descriptor) {
        type_descriptor_release((TypeDescriptor*)expr->type_info->type_descriptor);
    }
    expr->type_info->type_descriptor = type;
    type_descriptor_retain(type);
}

/**
 * Check if two types are exactly equal
 */
bool semantic_types_equal(TypeDescriptor *type1, TypeDescriptor *type2) {
    if (!type1 || !type2) {
        return false;
    }
    
    if (type1->category != type2->category) {
        return false;
    }
    
    switch (type1->category) {
        case TYPE_PRIMITIVE:
            return type1->data.primitive.primitive_kind == type2->data.primitive.primitive_kind;
            
        case TYPE_POINTER:
            return semantic_types_equal(type1->data.pointer.pointee_type, 
                                      type2->data.pointer.pointee_type);
            
        case TYPE_SLICE:
            return semantic_types_equal(type1->data.slice.element_type,
                                      type2->data.slice.element_type);
            
        case TYPE_STRUCT:
        case TYPE_ENUM:
            // For named types, compare by name
            if (!type1->name || !type2->name) {
                return false;
            }
            return strcmp(type1->name, type2->name) == 0;
            
        default:
            return false;
    }
}

// =============================================================================
// C17 GENERIC IMPLEMENTATION FUNCTIONS
// =============================================================================

bool semantic_analyze_node_impl(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!analyzer || !node) return false;
    
    ATOMIC_FETCH_ADD(&analyzer->stats.nodes_analyzed, 1);
    
    switch (node->type) {
        case AST_PROGRAM:
            return semantic_analyze_program(analyzer, node);
        
        case AST_FUNCTION_DECL:
        case AST_STRUCT_DECL:
        case AST_EXTERN_DECL:
            return semantic_analyze_declaration(analyzer, node);
        
        case AST_BLOCK:
        case AST_EXPR_STMT:
        case AST_LET_STMT:
        case AST_RETURN_STMT:
        case AST_IF_STMT:
        case AST_IF_LET_STMT:
        case AST_MATCH_STMT:
            return semantic_analyze_statement(analyzer, node);
        
        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR:
        case AST_CALL_EXPR:
        case AST_IDENTIFIER:
        case AST_INTEGER_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_STRING_LITERAL:
        case AST_BOOL_LITERAL:
        case AST_UNIT_LITERAL:
            return semantic_analyze_expression(analyzer, node);
        
        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, 
                                 node->location, 
                                 "Unsupported AST node type: %d", node->type);
            return false;
    }
}

bool semantic_analyze_node_const_impl(SemanticAnalyzer *analyzer, const ASTNode *node) {
    // For const nodes, we can only perform read-only analysis
    // Note: casting away const here is safe because semantic_analyze_node_impl
    // only reads the node structure for analysis purposes
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
    return semantic_analyze_node_impl(analyzer, (ASTNode*)node);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

// =============================================================================
// FUNCTION CONTEXT UTILITIES
// =============================================================================

SymbolEntry *semantic_get_current_function(SemanticAnalyzer *analyzer) {
    if (!analyzer) {
        return NULL;
    }
    
    // Return the currently tracked function
    return analyzer->current_function;
}