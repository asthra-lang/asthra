/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Array, Slice, and Index Access
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Analysis of array literals, slice expressions, and index access operations
 */

#include "semantic_arrays.h"
#include "semantic_core.h"
#include "semantic_utilities.h"
#include "semantic_types.h"
#include "semantic_type_helpers.h"
#include "semantic_type_creation.h"
#include "semantic_diagnostics.h"
#include "semantic_builtins.h"
#include "type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>

// =============================================================================
// ARRAY LITERAL ANALYSIS
// =============================================================================

bool analyze_array_literal(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_ARRAY_LITERAL) {
        return false;
    }

    ASTNodeList *elements = expr->data.array_literal.elements;
    if (!elements || elements->count == 0) {
        // Empty array literal - need type context
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, expr->location,
                             "Cannot infer type for empty array literal");
        return false;
    }

    // Analyze all elements and find common type
    TypeDescriptor *common_element_type = NULL;
    bool all_elements_constant = true;
    bool has_side_effects = false;

    for (size_t i = 0; i < elements->count; i++) {
        ASTNode *element = elements->nodes[i];
        if (!semantic_analyze_expression(analyzer, element)) {
            return false;
        }

        all_elements_constant = all_elements_constant && element->flags.is_constant_expr;
        has_side_effects = has_side_effects || element->flags.has_side_effects;

        if (!element->type_info || !element->type_info->type_descriptor) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, element->location,
                                 "Failed to determine type for array element");
            return false;
        }

        if (!common_element_type) {
            common_element_type = (TypeDescriptor*)element->type_info->type_descriptor;
        } else {
            TypeDescriptor *promoted_type = get_promoted_type(analyzer, common_element_type, (TypeDescriptor*)element->type_info->type_descriptor);
            if (!promoted_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, element->location,
                                     "Incompatible types in array literal: %s and %s",
                                     common_element_type->name, ((TypeDescriptor*)element->type_info->type_descriptor)->name);
                return false;
            }
            common_element_type = promoted_type;
        }
    }

    if (!common_element_type) {
         semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                 "Failed to determine common type for array literal");
        return false;
    }

    // Create array type descriptor: [size]ElementType
    // For now, create a simple slice type since we don't have array type creation
    TypeDescriptor *array_type_descriptor = get_builtin_type_descriptor(analyzer, "slice");
    if (!array_type_descriptor) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                             "Failed to create array type descriptor");
        return false;
    }

    expr->type_info = create_type_info_from_descriptor(array_type_descriptor);
    if (!expr->type_info) {
         semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                 "Failed to create type info for array literal");
        return false;
    }

    expr->flags.is_constant_expr = all_elements_constant;
    expr->flags.has_side_effects = has_side_effects;
    expr->flags.is_lvalue = false; // Array literals are rvalues

    return true;
}

// =============================================================================
// SLICE EXPRESSION ANALYSIS
// =============================================================================

bool analyze_slice_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_SLICE_EXPR) {
        return false;
    }

    ASTNode *array = expr->data.slice_expr.array;
    ASTNode *start = expr->data.slice_expr.start;
    ASTNode *end = expr->data.slice_expr.end;

    if (!array) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                             "Slice expression missing array expression");
        return false;
    }

    // Analyze the array expression
    if (!semantic_analyze_expression(analyzer, array)) {
        return false;
    }

    // Verify the array is sliceable (slice or array type)
    TypeDescriptor *array_type = array->type_info ? (TypeDescriptor*)array->type_info->type_descriptor : NULL;
    if (!array_type || (array_type->category != TYPE_SLICE && array_type->category != TYPE_ARRAY)) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, array->location,
                             "Cannot slice non-array/non-slice type: %s", 
                             array_type ? array_type->name : "unknown");
        return false;
    }

    // Analyze optional start expression
    if (start) {
        if (!semantic_analyze_expression(analyzer, start)) {
            return false;
        }
        
        TypeDescriptor *start_type = start->type_info ? (TypeDescriptor*)start->type_info->type_descriptor : NULL;
        if (!start_type || !is_integer_type(start_type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, start->location,
                                 "Slice start index must be an integer type");
            return false;
        }
    }

    // Analyze optional end expression
    if (end) {
        if (!semantic_analyze_expression(analyzer, end)) {
            return false;
        }
        
        TypeDescriptor *end_type = end->type_info ? (TypeDescriptor*)end->type_info->type_descriptor : NULL;
        if (!end_type || !is_integer_type(end_type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, end->location,
                                 "Slice end index must be an integer type");
            return false;
        }
    }

    // The result of a slice operation is a slice type with the same element type
    TypeDescriptor *element_type = NULL;
    if (array_type->category == TYPE_SLICE) {
        element_type = array_type->data.slice.element_type;
    } else if (array_type->category == TYPE_ARRAY) {
        element_type = array_type->data.array.element_type;
    }
    
    if (!element_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                             "Failed to determine element type for slice");
        return false;
    }
    
    // Create a slice type with the proper element type
    TypeDescriptor *result_type_descriptor = type_descriptor_create_slice(element_type);
    if (!result_type_descriptor) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                             "Failed to create slice type descriptor");
        return false;
    }

    // WORKAROUND: For now, don't create TypeInfo for slice expressions to avoid hangs
    // TODO: Fix the underlying issue in create_type_info_from_descriptor
    expr->type_info = NULL;
    
    type_descriptor_release(result_type_descriptor);

    expr->flags.is_constant_expr = false; // Slices are not constant expressions
    expr->flags.has_side_effects = array->flags.has_side_effects || 
                                  (start && start->flags.has_side_effects) ||
                                  (end && end->flags.has_side_effects);
    expr->flags.is_lvalue = false; // Slice expressions are rvalues

    return true;
}

// =============================================================================
// INDEX ACCESS ANALYSIS
// =============================================================================

bool analyze_index_access(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_INDEX_ACCESS) {
        return false;
    }

    ASTNode *base = expr->data.index_access.array;  // Corrected field name
    ASTNode *index = expr->data.index_access.index;

    if (!base || !index) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                             "Index access missing base or index");
        return false;
    }

    if (!semantic_analyze_expression(analyzer, base) ||
        !semantic_analyze_expression(analyzer, index)) {
        return false;
    }

    TypeDescriptor *base_type = base->type_info ? (TypeDescriptor*)base->type_info->type_descriptor : NULL;
    if (!base_type || (base_type->category != TYPE_SLICE && base_type->category != TYPE_POINTER && base_type->category != TYPE_ARRAY)) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, base->location,
                             "Cannot index non-array/non-slice/non-pointer type: %s", 
                             base_type ? base_type->name : "unknown");
        return false;
    }

    TypeDescriptor *index_type = index->type_info ? (TypeDescriptor*)index->type_info->type_descriptor : NULL;
    if (!index_type || !is_integer_type(index_type)) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, index->location,
                             "Index must be an integer type");
        return false;
    }

    // Determine the result type (element type of the array/slice/pointer)
    TypeDescriptor *result_type_descriptor = NULL;
    if (base_type->category == TYPE_SLICE) {
        result_type_descriptor = base_type->data.slice.element_type;
    } else if (base_type->category == TYPE_ARRAY) {
        result_type_descriptor = base_type->data.array.element_type;
    } else if (base_type->category == TYPE_POINTER) {
        result_type_descriptor = base_type->data.pointer.pointee_type;
        // Pointer indexing (dereference + offset) requires unsafe context
        if (!analyzer->in_unsafe_context) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                 expr->location, 
                                 "Pointer indexing requires unsafe block");
            return false;
        }
    }

    if (!result_type_descriptor) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                             "Failed to determine result type for index access");
        return false;
    }

    expr->type_info = create_type_info_from_descriptor(result_type_descriptor);
    if (!expr->type_info) {
         semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                 "Failed to create type info for index access");
        return false;
    }

    expr->flags.is_constant_expr = base->flags.is_constant_expr && index->flags.is_constant_expr;
    expr->flags.has_side_effects = base->flags.has_side_effects || index->flags.has_side_effects;
    expr->flags.is_lvalue = true; // Index access yields an lvalue

    return true;
}