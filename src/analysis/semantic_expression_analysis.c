/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Expression Analysis Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core semantic analysis functions for expressions
 */

#include "semantic_expression_analysis.h"
#include "../parser/ast.h"
#include "semantic_annotations.h"
#include "semantic_arrays.h"
#include "semantic_binary_unary.h"
#include "semantic_calls.h"
#include "semantic_concurrency.h"
#include "semantic_const_declarations.h"
#include "semantic_control_flow.h"
#include "semantic_core.h"
#include "semantic_declarations.h"
#include "semantic_errors.h"
#include "semantic_expression_utils.h"
#include "semantic_expressions.h"
#include "semantic_ffi.h"
#include "semantic_field_access.h"
#include "semantic_literals.h"
#include "semantic_loops.h"
#include "semantic_scopes.h"
#include "semantic_security.h"
#include "semantic_statements.h"
#include "semantic_statistics.h"
#include "semantic_structs.h"
#include "semantic_symbols.h"
#include "semantic_type_creation.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_checking.h"
#include "type_info.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Atomic operation wrapper for fetch_add only (used in this file)
#if ASTHRA_HAS_ATOMICS
#define ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
#else
#define ATOMIC_FETCH_ADD(ptr, val) __sync_fetch_and_add(ptr, val)
#endif

bool semantic_analyze_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return false;
    }

    // DEBUG: printf("DEBUG: semantic_analyze_expression - expr type: %d\n", expr->type);

    ATOMIC_FETCH_ADD(&analyzer->stats.nodes_analyzed, 1);

    bool result = false;
    switch (expr->type) {
    case AST_FIELD_ACCESS:
        result = analyze_field_access(analyzer, expr);
        break;

    case AST_IDENTIFIER:
        result = analyze_identifier_expression(analyzer, expr);
        break;

    case AST_CONST_EXPR:
        result = analyze_const_expression(analyzer, expr);
        break;

    case AST_INTEGER_LITERAL:
    case AST_FLOAT_LITERAL:
    case AST_STRING_LITERAL:
    case AST_BOOL_LITERAL:
    case AST_UNIT_LITERAL:
        // Analyze literals and attach type information
        result = analyze_literal_expression(analyzer, expr);
        break;

    case AST_CHAR_LITERAL:
        // Phase 4: Character literals require explicit type annotation enforcement
        result = analyze_char_literal(analyzer, expr);
        break;

    case AST_BINARY_EXPR: {
        // Analyze binary expression with proper type checking
        bool binary_result = analyze_binary_expression(analyzer, expr);
        if (binary_result) {
            // Store type information for this binary expression
            TypeDescriptor *type = semantic_get_expression_type(analyzer, expr);
            if (type) {
                semantic_set_expression_type(analyzer, expr, type);
                type_descriptor_release(type);
            }
        }
        result = binary_result;
        break;
    }

    case AST_UNARY_EXPR:
        result = analyze_unary_expression(analyzer, expr);
        break;

    case AST_CALL_EXPR: {
        // Phase 3: Analyze expression annotations
        if (!analyze_expression_annotations(analyzer, expr)) {
            result = false;
            break;
        }

        // Validate function exists and is callable
        if (!analyze_call_expression(analyzer, expr)) {
            result = false;
            break;
        }

        // Analyze each argument
        result = true;
        if (expr->data.call_expr.args) {
            for (size_t i = 0; i < ast_node_list_size(expr->data.call_expr.args); i++) {
                ASTNode *arg = ast_node_list_get(expr->data.call_expr.args, i);
                if (!semantic_analyze_expression(analyzer, arg)) {
                    result = false;
                    break;
                }
            }
        }

        // Store type information for this call expression
        if (result) {
            TypeDescriptor *type = semantic_get_expression_type(analyzer, expr);
            if (type) {
                semantic_set_expression_type(analyzer, expr, type);
                type_descriptor_release(type);
            }
        }
        break;
    }

    case AST_ASSIGNMENT:
        // Analyze assignment expressions with immutable-by-default validation
        result = analyze_assignment_validation(analyzer, expr);
        break;

    case AST_ASSOCIATED_FUNC_CALL:
        // Analyze associated function calls like Struct::function()
        result = analyze_associated_function_call(analyzer, expr);
        break;

    case AST_AWAIT_EXPR:
        // Tier 1 concurrency: Basic await is deterministic and doesn't require annotation
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, expr)) {
            result = false;
        } else {
            result = analyze_await_expression(analyzer, expr);
        }
        break;

    case AST_ENUM_VARIANT:
        // Phase 4: Analyze enum variant expressions
        result = analyze_enum_variant(analyzer, expr);
        break;

    case AST_STRUCT_LITERAL:
        // Analyze struct literal expressions
        result = analyze_struct_literal_expression(analyzer, expr);
        break;

    case AST_ARRAY_LITERAL: {
        // Analyze array literal expressions with type checking
        result = true;
        if (expr->data.array_literal.elements) {
            size_t element_count = ast_node_list_size(expr->data.array_literal.elements);

            // Check for repeated array syntax [value; count]
            if (element_count >= 3) {
                ASTNode *first = ast_node_list_get(expr->data.array_literal.elements, 0);
                if (first && first->type == AST_IDENTIFIER && first->data.identifier.name &&
                    strcmp(first->data.identifier.name, "__repeated_array__") == 0) {
                    // This is a repeated array literal
                    if (element_count != 3) {
                        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                              expr->location, "Invalid repeated array syntax");
                        result = false;
                        break;
                    }

                    // Analyze value expression (element 1)
                    ASTNode *value_expr = ast_node_list_get(expr->data.array_literal.elements, 1);
                    if (!semantic_analyze_expression(analyzer, value_expr)) {
                        result = false;
                        break;
                    }

                    // Analyze count expression (element 2)
                    ASTNode *count_expr = ast_node_list_get(expr->data.array_literal.elements, 2);
                    if (!semantic_analyze_expression(analyzer, count_expr)) {
                        result = false;
                        break;
                    }

                    // Verify count is constant integer
                    if (!count_expr->flags.is_constant_expr) {
                        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                              count_expr->location,
                                              "Array size must be a compile-time constant");
                        result = false;
                        break;
                    }

                    // Evaluate the constant expression to get the size
                    ConstValue *count_value = NULL;

                    // Handle different types of constant expressions
                    if (count_expr->type == AST_INTEGER_LITERAL) {
                        // Direct integer literal
                        count_value =
                            const_value_create_integer(count_expr->data.integer_literal.value);
                    } else if (count_expr->type == AST_IDENTIFIER) {
                        // Const identifier - look up in symbol table
                        SymbolEntry *symbol =
                            semantic_resolve_identifier(analyzer, count_expr->data.identifier.name);
                        if (symbol && symbol->kind == SYMBOL_CONST && symbol->const_value) {
                            if (symbol->const_value->type == CONST_VALUE_INTEGER) {
                                count_value = const_value_create_integer(
                                    symbol->const_value->data.integer_value);
                            }
                        }
                    } else {
                        // Try the general const expression evaluator
                        count_value = evaluate_literal_as_const(analyzer, count_expr);
                    }
                    if (!count_value) {
                        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                              count_expr->location,
                                              "Failed to evaluate array size");
                        result = false;
                        break;
                    }

                    // Ensure it's an integer
                    if (count_value->type != CONST_VALUE_INTEGER) {
                        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                                              count_expr->location,
                                              "Array size must be an integer");
                        const_value_destroy(count_value);
                        result = false;
                        break;
                    }

                    // Get the size value
                    int64_t array_size = count_value->data.integer_value;
                    const_value_destroy(count_value);

                    // Validate size is positive
                    if (array_size <= 0) {
                        semantic_report_error(
                            analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, count_expr->location,
                            "Array size must be positive, got %lld", (long long)array_size);
                        result = false;
                        break;
                    }

                    // Get the element type from the value expression
                    TypeDescriptor *element_type =
                        semantic_get_expression_type(analyzer, value_expr);
                    if (!element_type) {
                        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED,
                                              value_expr->location,
                                              "Failed to determine element type");
                        result = false;
                        break;
                    }

                    // Create fixed-size array type
                    TypeDescriptor *array_type =
                        type_descriptor_create_array(element_type, (size_t)array_size);
                    if (!array_type) {
                        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                              "Failed to create array type");
                        type_descriptor_release(element_type);
                        result = false;
                        break;
                    }

                    // Set the type info for the expression
                    semantic_set_expression_type(analyzer, expr, array_type);

                    // Set flags based on value expression
                    expr->flags.is_constant_expr = value_expr->flags.is_constant_expr;
                    expr->flags.has_side_effects = value_expr->flags.has_side_effects;
                    expr->flags.is_lvalue = false; // Array literals are rvalues

                    // Release references
                    type_descriptor_release(element_type);
                    type_descriptor_release(array_type);

                    result = true;
                    break;
                }
            }

            // Regular array literal
            TypeDescriptor *expected_element_type = NULL;

            for (size_t i = 0; i < element_count; i++) {
                ASTNode *element = ast_node_list_get(expr->data.array_literal.elements, i);
                if (!semantic_analyze_expression(analyzer, element)) {
                    if (expected_element_type)
                        type_descriptor_release(expected_element_type);
                    result = false;
                    break;
                }

                // Check element type consistency
                TypeDescriptor *element_type = semantic_get_expression_type(analyzer, element);
                if (element_type) {
                    if (expected_element_type == NULL) {
                        // First element sets the expected type
                        expected_element_type = element_type;
                        type_descriptor_retain(expected_element_type);
                    } else {
                        // Subsequent elements must be compatible
                        if (!semantic_check_type_compatibility(analyzer, element_type,
                                                               expected_element_type)) {
                            semantic_report_error(
                                analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, element->location,
                                "Array element type mismatch: expected %s, got %s",
                                expected_element_type->name ? expected_element_type->name
                                                            : "unknown",
                                element_type->name ? element_type->name : "unknown");
                            type_descriptor_release(element_type);
                            type_descriptor_release(expected_element_type);
                            result = false;
                            break;
                        }
                    }
                    type_descriptor_release(element_type);
                }
            }

            if (expected_element_type) {
                type_descriptor_release(expected_element_type);
            }
        }
        break;
    }

    case AST_INDEX_ACCESS:
        // Analyze array/slice index access
        result = analyze_index_access(analyzer, expr);
        break;

    case AST_SLICE_EXPR:
        // Analyze slice expression (array[start:end])
        result = analyze_slice_expression(analyzer, expr);
        break;

    case AST_TUPLE_LITERAL: {
        // Analyze tuple literal expressions
        result = true;
        if (expr->data.tuple_literal.elements) {
            size_t element_count = ast_node_list_size(expr->data.tuple_literal.elements);

            // Validate minimum 2 elements
            if (element_count < 2) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                                      "Tuple literals must have at least 2 elements");
                result = false;
                break;
            }

            // Analyze each element
            for (size_t i = 0; i < element_count; i++) {
                ASTNode *element = ast_node_list_get(expr->data.tuple_literal.elements, i);
                if (!semantic_analyze_expression(analyzer, element)) {
                    result = false;
                    break;
                }
            }

            // Create and store tuple type
            if (result) {
                TypeDescriptor **element_types = malloc(element_count * sizeof(TypeDescriptor *));
                if (element_types) {
                    bool all_types_valid = true;
                    for (size_t i = 0; i < element_count; i++) {
                        ASTNode *element = ast_node_list_get(expr->data.tuple_literal.elements, i);
                        element_types[i] = semantic_get_expression_type(analyzer, element);
                        if (!element_types[i]) {
                            all_types_valid = false;
                            // Clean up previously allocated types
                            for (size_t j = 0; j < i; j++) {
                                if (element_types[j]) {
                                    type_descriptor_release(element_types[j]);
                                }
                            }
                            break;
                        }
                    }

                    if (all_types_valid) {
                        TypeDescriptor *tuple_type =
                            type_descriptor_create_tuple(element_types, element_count);
                        if (tuple_type) {
                            semantic_set_expression_type(analyzer, expr, tuple_type);
                            type_descriptor_release(tuple_type);
                        }
                    }

                    // Release element types
                    for (size_t i = 0; i < element_count; i++) {
                        if (element_types[i]) {
                            type_descriptor_release(element_types[i]);
                        }
                    }
                    free(element_types);
                }
            }
        }
        break;
    }

    case AST_POSTFIX_EXPR:
        // Analyze postfix expressions (should be handled by the individual postfix operations)
        // This type might be deprecated in favor of specific operations like FIELD_ACCESS,
        // INDEX_ACCESS
        result = semantic_analyze_expression(analyzer, expr->data.postfix_expr.base);
        if (result && expr->data.postfix_expr.base) {
            // Propagate type from base expression
            TypeDescriptor *base_type =
                semantic_get_expression_type(analyzer, expr->data.postfix_expr.base);
            if (base_type) {
                semantic_set_expression_type(analyzer, expr, base_type);
                type_descriptor_release(base_type);
            }
        }
        break;

    case AST_SLICE_LENGTH_ACCESS:
        // Analyze slice length access (.len operation)
        result = true;
        if (expr->data.slice_length_access.slice) {
            result = semantic_analyze_expression(analyzer, expr->data.slice_length_access.slice);
            if (result) {
                // Verify the operand is actually a slice or array
                TypeDescriptor *slice_type =
                    semantic_get_expression_type(analyzer, expr->data.slice_length_access.slice);
                if (slice_type) {
                    if (slice_type->category == TYPE_SLICE || slice_type->category == TYPE_ARRAY) {
                        // .len always returns usize
                        TypeDescriptor *usize_type =
                            type_descriptor_create_primitive(PRIMITIVE_USIZE);
                        if (usize_type) {
                            semantic_set_expression_type(analyzer, expr, usize_type);
                            type_descriptor_release(usize_type);
                        }
                    } else {
                        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                                              expr->location,
                                              "Cannot access length of non-slice/array type");
                        result = false;
                    }
                    type_descriptor_release(slice_type);
                }
            }
        } else {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                                  "Slice length access missing operand");
            result = false;
        }
        break;

    case AST_CAST_EXPR:
        // Analyze cast expressions (expr as Type)
        result = true;
        if (expr->data.cast_expr.expression && expr->data.cast_expr.target_type) {
            // Analyze the source expression
            if (!semantic_analyze_expression(analyzer, expr->data.cast_expr.expression)) {
                result = false;
                break;
            }

            // Resolve the target type
            TypeDescriptor *target_type =
                analyze_type_node(analyzer, expr->data.cast_expr.target_type);
            if (!target_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                      expr->data.cast_expr.target_type->location,
                                      "Invalid target type in cast expression");
                result = false;
                break;
            }

            // Get source type and validate cast is allowed
            TypeDescriptor *source_type =
                semantic_get_expression_type(analyzer, expr->data.cast_expr.expression);
            if (source_type) {
                // Basic cast validation - ensure types are compatible for casting
                if (!semantic_validate_cast_compatibility(analyzer, source_type, target_type,
                                                          expr->location)) {
                    result = false;
                } else {
                    // Set the result type to the target type
                    semantic_set_expression_type(analyzer, expr, target_type);
                }
                type_descriptor_release(source_type);
            } else {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED,
                                      expr->location,
                                      "Cannot determine source type for cast expression");
                result = false;
            }

            type_descriptor_release(target_type);
        } else {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                                  "Cast expression missing source expression or target type");
            result = false;
        }
        break;

    case AST_MATCH_EXPR:
        // Analyze match expressions
        result = analyze_match_expression(analyzer, expr);
        break;

    default:
        // All major expression types should now be handled
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                              "Unsupported expression type %d in semantic analysis", expr->type);
        result = false;
        break;
    }

    return result;
}