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
#include "const_evaluator.h"
#include "semantic_builtins.h"
#include "semantic_core.h"
#include "semantic_diagnostics.h"
#include "semantic_type_creation.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_info.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    // Check if this is a repeated array [value; count]
    // Repeated arrays have exactly 3 elements: marker, value, count
    if (elements->count == 3) {
        ASTNode *first = elements->nodes[0];
        if (first && first->type == AST_IDENTIFIER && first->data.identifier.name &&
            strcmp(first->data.identifier.name, "__repeated_array__") == 0) {
            // This is a repeated array: [value; count]
            ASTNode *value_expr = elements->nodes[1];
            ASTNode *count_expr = elements->nodes[2];

            // Extract element type from expected type if available
            TypeDescriptor *expected_element_type = NULL;
            TypeDescriptor *saved_expected_type = analyzer->expected_type;
            
            if (analyzer->expected_type && analyzer->expected_type->category == TYPE_ARRAY) {
                expected_element_type = analyzer->expected_type->data.array.element_type;
                analyzer->expected_type = expected_element_type;
            }

            // Analyze the value expression with the expected element type context
            if (!semantic_analyze_expression(analyzer, value_expr)) {
                analyzer->expected_type = saved_expected_type;
                return false;
            }
            
            // Restore the expected type
            analyzer->expected_type = saved_expected_type;

            // Analyze the count expression (should be a constant integer)
            if (!semantic_analyze_expression(analyzer, count_expr)) {
                return false;
            }

            // Get the element type - prefer expected type if available
            TypeDescriptor *element_type = NULL;
            
            // If we extracted an expected element type earlier, use it
            if (expected_element_type) {
                element_type = expected_element_type;
                // Increase ref count since we'll be using it
                type_descriptor_retain(element_type);
            } else {
                // Otherwise, use the type from the value expression
                element_type = semantic_get_expression_type(analyzer, value_expr);
                if (!element_type) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED,
                                          value_expr->location,
                                          "Failed to determine type for repeated array value");
                    return false;
                }
            }

            // Verify count is a constant expression
            if (!count_expr->flags.is_constant_expr) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                      count_expr->location,
                                      "Array size must be a compile-time constant");
                type_descriptor_release(element_type);
                return false;
            }

            // Evaluate the constant expression to get the size
            ConstValue *count_value = NULL;

            // Handle different types of constant expressions
            if (count_expr->type == AST_INTEGER_LITERAL) {
                // Direct integer literal
                count_value = const_value_create_integer(count_expr->data.integer_literal.value);
            } else if (count_expr->type == AST_IDENTIFIER) {
                // Const identifier - look up in symbol table
                SymbolEntry *symbol =
                    semantic_resolve_identifier(analyzer, count_expr->data.identifier.name);
                if (symbol && symbol->kind == SYMBOL_CONST && symbol->const_value) {
                    if (symbol->const_value->type == CONST_VALUE_INTEGER) {
                        count_value =
                            const_value_create_integer(symbol->const_value->data.integer_value);
                    }
                }
            } else {
                // Try the general const expression evaluator
                count_value = evaluate_literal_as_const(analyzer, count_expr);
            }
            if (!count_value) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                      count_expr->location, "Failed to evaluate array size");
                type_descriptor_release(element_type);
                return false;
            }

            // Ensure it's an integer
            if (count_value->type != CONST_VALUE_INTEGER) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, count_expr->location,
                                      "Array size must be an integer");
                const_value_destroy(count_value);
                type_descriptor_release(element_type);
                return false;
            }

            // Get the size value
            int64_t array_size = count_value->data.integer_value;
            const_value_destroy(count_value);

            // Validate size is positive
            if (array_size <= 0) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                      count_expr->location, "Array size must be positive, got %lld",
                                      (long long)array_size);
                type_descriptor_release(element_type);
                return false;
            }

            // Create fixed-size array type
            TypeDescriptor *array_type =
                type_descriptor_create_array(element_type, (size_t)array_size);
            if (!array_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                      "Failed to create array type");
                type_descriptor_release(element_type);
                return false;
            }

            // Set the type info for the expression
            semantic_set_expression_type(analyzer, expr, array_type);

            // Release references (we always own element_type now)
            type_descriptor_release(element_type);
            type_descriptor_release(array_type);

            expr->flags.is_constant_expr =
                value_expr->flags.is_constant_expr && count_expr->flags.is_constant_expr;
            expr->flags.has_side_effects =
                value_expr->flags.has_side_effects || count_expr->flags.has_side_effects;
            expr->flags.is_lvalue = false; // Array literals are rvalues

            return true;
        }
    }

    // Regular array literal - analyze all elements and find common type
    TypeDescriptor *common_element_type = NULL;
    bool all_elements_constant = true;
    bool has_side_effects = false;
    
    // Extract expected element type if available
    TypeDescriptor *expected_element_type = NULL;
    TypeDescriptor *saved_expected_type = analyzer->expected_type;
    
    if (analyzer->expected_type && analyzer->expected_type->category == TYPE_ARRAY) {
        expected_element_type = analyzer->expected_type->data.array.element_type;
        analyzer->expected_type = expected_element_type;
    }

    for (size_t i = 0; i < elements->count; i++) {
        ASTNode *element = elements->nodes[i];
        if (!semantic_analyze_expression(analyzer, element)) {
            analyzer->expected_type = saved_expected_type;
            if (common_element_type) {
                type_descriptor_release(common_element_type);
            }
            return false;
        }

        all_elements_constant = all_elements_constant && element->flags.is_constant_expr;
        has_side_effects = has_side_effects || element->flags.has_side_effects;

        TypeDescriptor *element_type = semantic_get_expression_type(analyzer, element);
        if (!element_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, element->location,
                                  "Failed to determine type for array element");
            if (common_element_type) {
                type_descriptor_release(common_element_type);
            }
            return false;
        }

        if (!common_element_type) {
            common_element_type = element_type;
            type_descriptor_retain(common_element_type);
        } else {
            // Check if types are compatible
            if (!semantic_check_type_compatibility(analyzer, element_type, common_element_type)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, element->location,
                                      "Incompatible types in array literal: %s and %s",
                                      common_element_type->name ? common_element_type->name
                                                                : "unknown",
                                      element_type->name ? element_type->name : "unknown");
                type_descriptor_release(element_type);
                type_descriptor_release(common_element_type);
                return false;
            }
        }
        type_descriptor_release(element_type);
    }
    
    // Restore the expected type
    analyzer->expected_type = saved_expected_type;

    // If we have an expected element type and no conflicts, use it
    if (expected_element_type && common_element_type) {
        if (semantic_check_type_compatibility(analyzer, common_element_type, expected_element_type)) {
            type_descriptor_release(common_element_type);
            common_element_type = expected_element_type;
            type_descriptor_retain(common_element_type);
        }
    }

    if (!common_element_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                              "Failed to determine common type for array literal");
        return false;
    }

    // Create fixed-size array type [size]ElementType
    TypeDescriptor *array_type = type_descriptor_create_array(common_element_type, elements->count);
    if (!array_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                              "Failed to create array type descriptor");
        type_descriptor_release(common_element_type);
        return false;
    }

    // Set the type info for the expression
    semantic_set_expression_type(analyzer, expr, array_type);

    // Release references
    type_descriptor_release(common_element_type);
    type_descriptor_release(array_type);

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
    TypeDescriptor *array_type =
        array->type_info ? (TypeDescriptor *)array->type_info->type_descriptor : NULL;
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

        TypeDescriptor *start_type =
            start->type_info ? (TypeDescriptor *)start->type_info->type_descriptor : NULL;
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

        TypeDescriptor *end_type =
            end->type_info ? (TypeDescriptor *)end->type_info->type_descriptor : NULL;
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

    ASTNode *base = expr->data.index_access.array; // Corrected field name
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

    TypeDescriptor *base_type =
        base->type_info ? (TypeDescriptor *)base->type_info->type_descriptor : NULL;
    if (!base_type || (base_type->category != TYPE_SLICE && base_type->category != TYPE_POINTER &&
                       base_type->category != TYPE_ARRAY)) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, base->location,
                              "Cannot index non-array/non-slice/non-pointer type: %s",
                              base_type ? base_type->name : "unknown");
        return false;
    }

    TypeDescriptor *index_type =
        index->type_info ? (TypeDescriptor *)index->type_info->type_descriptor : NULL;
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
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, expr->location,
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