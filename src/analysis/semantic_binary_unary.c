/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Binary and Unary Expressions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of binary operators, unary operators, and assignment expressions
 */

#include "semantic_binary_unary.h"
#include "semantic_builtins.h"
#include "semantic_core.h"
#include "semantic_diagnostics.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// EXPRESSION TYPE ANALYSIS
// =============================================================================

// Helper to determine the result type of a binary operation
static TypeDescriptor *get_binary_op_result_type(SemanticAnalyzer *analyzer, BinaryOperator op,
                                                 TypeDescriptor *left_type,
                                                 TypeDescriptor *right_type,
                                                 const SourceLocation *location) {
    // Handle null types
    if (!left_type || !right_type)
        return NULL;

    // For simplicity, let's assume basic type promotion for arithmetic and comparison
    // In a full implementation, this would involve a complex type promotion matrix

    // Check for compatible types for arithmetic, comparison, and logical operations
    bool compatible = false;
    if (is_numeric_type(left_type) && is_numeric_type(right_type)) {
        compatible = true;
    } else if (is_boolean_type(left_type) && is_boolean_type(right_type) &&
               (op == BINOP_AND || op == BINOP_OR)) {
        compatible = true;
    } else if (is_string_type(left_type) && is_string_type(right_type) && op == BINOP_ADD) {
        // String concatenation
        compatible = true;
    } else if (is_pointer_type(left_type) && is_numeric_type(right_type) &&
               (op == BINOP_ADD || op == BINOP_SUB)) {
        // Pointer arithmetic
        compatible = true;
    } else if (left_type->category == TYPE_ENUM && right_type->category == TYPE_ENUM &&
               (op == BINOP_EQ || op == BINOP_NE)) {
        compatible = true; // Enum comparison
    }

    if (!compatible) {
        char msg[256];
        snprintf(msg, sizeof(msg), "incompatible types for binary operation '%s'",
                 binary_op_to_string(op));

        semantic_report_type_mismatch_detailed(analyzer, *location, left_type, right_type,
                                               binary_op_to_string(op));
        return NULL;
    }

    // Determine result type
    switch (op) {
    case BINOP_ADD:
    case BINOP_SUB:
    case BINOP_MUL:
    case BINOP_DIV:
    case BINOP_MOD: {
        // Numeric operations: promote to wider type
        if (is_string_type(left_type) && is_string_type(right_type)) {
            return get_builtin_type_descriptor(analyzer, "string");
        }
        return get_promoted_type(analyzer, left_type, right_type);
    }
    case BINOP_EQ:
    case BINOP_NE:
    case BINOP_LT:
    case BINOP_LE:
    case BINOP_GT:
    case BINOP_GE: {
        // Comparison operations: result is boolean
        return get_builtin_type_descriptor(analyzer, "bool");
    }
    case BINOP_AND:
    case BINOP_OR: {
        // Logical operations: result is boolean
        return get_builtin_type_descriptor(analyzer, "bool");
    }
    case BINOP_BITWISE_AND:
    case BINOP_BITWISE_OR:
    case BINOP_BITWISE_XOR:
    case BINOP_LSHIFT:
    case BINOP_RSHIFT: {
        // Bitwise operations: promote to wider integer type
        return get_promoted_integer_type(analyzer, left_type, right_type);
    }
    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNSUPPORTED_OPERATION, *location,
                              "Unsupported binary operator: %s", binary_op_to_string(op));
        return NULL;
    }
}

bool analyze_binary_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_BINARY_EXPR) {
        return false;
    }

    BinaryOperator op = expr->data.binary_expr.operator;
    ASTNode *left = expr->data.binary_expr.left;
    ASTNode *right = expr->data.binary_expr.right;

    if (!left || !right) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                              "Binary expression missing operand(s)");
        return false;
    }

    // Recursively analyze left operand first
    if (!semantic_analyze_expression(analyzer, left)) {
        return false;
    }

    // Get left operand type to use as context for right operand
    TypeDescriptor *left_type_hint = NULL;
    if (left->type_info && left->type_info->type_descriptor) {
        left_type_hint = (TypeDescriptor *)left->type_info->type_descriptor;
    }

    // For comparison and arithmetic operations, set expected type for right operand
    TypeDescriptor *old_expected_type = analyzer->expected_type;
    if (left_type_hint && (op == BINOP_EQ || op == BINOP_NE || op == BINOP_LT || op == BINOP_GT ||
                           op == BINOP_LE || op == BINOP_GE || op == BINOP_ADD || op == BINOP_SUB ||
                           op == BINOP_MUL || op == BINOP_DIV || op == BINOP_MOD)) {
        analyzer->expected_type = left_type_hint;
    }

    // Analyze right operand with expected type context
    bool right_result = semantic_analyze_expression(analyzer, right);

    // Restore previous expected type
    analyzer->expected_type = old_expected_type;

    if (!right_result) {
        return false;
    }

    // Get operand types
    TypeDescriptor *left_type =
        left->type_info ? (TypeDescriptor *)left->type_info->type_descriptor : NULL;
    TypeDescriptor *right_type =
        right->type_info ? (TypeDescriptor *)right->type_info->type_descriptor : NULL;

    if (!left_type || !right_type) {
        // More specific error reporting
        if (!left->type_info) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, expr->location,
                                  "Left operand has no type information attached");
        } else if (!left->type_info->type_descriptor) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, expr->location,
                                  "Left operand type info has no type descriptor");
        } else if (!right->type_info) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, expr->location,
                                  "Right operand has no type information attached");
        } else if (!right->type_info->type_descriptor) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, expr->location,
                                  "Right operand type info has no type descriptor");
        } else {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, expr->location,
                                  "Could not infer types for binary expression operands");
        }
        return false;
    }

    // DEBUG: Log binary operation type checking
    // printf("DEBUG: Binary operation %d with left='%s', right='%s'\n", op,
    //        left_type->name ? left_type->name : "NULL",
    //        right_type->name ? right_type->name : "NULL");

    // Determine result type and check compatibility
    TypeDescriptor *result_type_descriptor =
        get_binary_op_result_type(analyzer, op, left_type, right_type, &expr->location);

    // printf("DEBUG: get_binary_op_result_type returned: %s\n",
    //        result_type_descriptor
    //            ? (result_type_descriptor->name ? result_type_descriptor->name : "NULL")
    //            : "NULL");

    if (!result_type_descriptor) {
        return false; // Error already reported by get_binary_op_result_type
    }

    // Handle potential overflow for constant integer expressions (simplified)
    if (left->flags.is_constant_expr && right->flags.is_constant_expr &&
        is_integer_type(result_type_descriptor)) {
        // TODO: Implement actual constant evaluation and overflow detection
        // For now, just a placeholder check
        if (op == BINOP_DIV) {
            bool is_zero = false;
            if (right->type == AST_INTEGER_LITERAL && right->data.integer_literal.value == 0) {
                is_zero = true;
            } else if (right->type == AST_FLOAT_LITERAL && right->data.float_literal.value == 0.0) {
                is_zero = true;
            }
            if (is_zero) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_DIVISION_BY_ZERO, expr->location,
                                      "Division by zero in constant expression");
                return false;
            }
        }
    }

    // Set the expression's type info
    expr->type_info = create_type_info_from_descriptor(result_type_descriptor);
    if (!expr->type_info) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                              "Failed to create type info for binary expression result");
        return false;
    }

    // Determine if the result is a constant expression
    expr->flags.is_constant_expr = left->flags.is_constant_expr && right->flags.is_constant_expr;
    expr->flags.has_side_effects = left->flags.has_side_effects || right->flags.has_side_effects;
    expr->flags.is_lvalue = false; // Binary expressions are generally rvalues

    return true;
}

bool analyze_unary_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_UNARY_EXPR) {
        return false;
    }

    UnaryOperator op = expr->data.unary_expr.operator;
    ASTNode *operand = expr->data.unary_expr.operand;

    if (!operand) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                              "Unary expression missing operand");
        return false;
    }

    // Check for unsafe operations that require unsafe blocks
    if (op == UNOP_DEREF) {
        // Pointer dereference requires unsafe block
        if (!analyzer->in_unsafe_context) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, expr->location,
                                  "Pointer dereference requires unsafe block");
            return false;
        }
    }

    // Handle sizeof specially - its operand is a type, not an expression
    TypeDescriptor *operand_type = NULL;
    if (op == UNOP_SIZEOF) {
        // For sizeof, the operand is a type node, not an expression
        operand_type = analyze_type_node(analyzer, operand);
        if (!operand_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, operand->location,
                                  "Cannot determine type for sizeof operand");
            return false;
        }
    } else {
        // For other unary operators, analyze the operand as an expression
        if (!semantic_analyze_expression(analyzer, operand)) {
            return false;
        }

        // Get the operand type
        operand_type = semantic_get_expression_type(analyzer, operand);
        if (!operand_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, operand->location,
                                  "Cannot determine type of operand in unary expression");
            return false;
        }
    }

    // Determine result type based on operator and operand type
    TypeDescriptor *result_type = NULL;
    TypeInfo *type_info = NULL;

    switch (op) {
    case UNOP_MINUS:
        // Arithmetic unary operators - operand must be numeric
        if (!is_numeric_type(operand_type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                                  "Unary minus operator requires numeric operand");
            type_descriptor_release(operand_type);
            return false;
        }
        result_type = operand_type; // Result type is same as operand
        type_descriptor_retain(result_type);
        break;

    case UNOP_NOT:
        // Logical NOT - operand must be boolean
        if (!is_boolean_type(operand_type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                                  "Logical NOT operator requires boolean operand");
            type_descriptor_release(operand_type);
            return false;
        }
        result_type = operand_type; // Result type is boolean
        type_descriptor_retain(result_type);
        break;

    case UNOP_BITWISE_NOT:
        // Bitwise NOT - operand must be integral
        if (!is_integer_type(operand_type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                                  "Bitwise NOT operator requires integral operand");
            type_descriptor_release(operand_type);
            return false;
        }
        result_type = operand_type; // Result type is same as operand
        type_descriptor_retain(result_type);
        break;

    case UNOP_DEREF:
        // Pointer dereference - operand must be pointer
        if (!is_pointer_type(operand_type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                                  "Dereference operator requires pointer operand");
            type_descriptor_release(operand_type);
            return false;
        }
        result_type = operand_type->data.pointer.pointee_type;
        type_descriptor_retain(result_type);
        break;

    case UNOP_ADDRESS_OF:
        // Address-of operator - create pointer type
        result_type = type_descriptor_create_pointer(operand_type);
        break;

    case UNOP_SIZEOF:
        // sizeof operator - result is always usize
        result_type = semantic_get_builtin_type(analyzer, "usize");
        if (result_type) {
            type_descriptor_retain(result_type);
        }
        break;

    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNSUPPORTED_OPERATION, expr->location,
                              "Unsupported unary operator");
        type_descriptor_release(operand_type);
        return false;
    }

    // Attach type information to the expression
    if (result_type) {
        // Create TypeInfo from TypeDescriptor using the helper function
        type_info = create_type_info_from_descriptor(result_type);
        if (type_info) {
            expr->type_info = type_info;
            // NOTE: Do NOT release result_type here! The TypeInfo now owns this reference.
        } else {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                  "Failed to create type info for unary expression");
            // Only release on failure
            type_descriptor_release(result_type);
        }
    }

    type_descriptor_release(operand_type);
    return true;
}

bool analyze_assignment_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    (void)analyzer;
    (void)expr;
    // TODO: Implement assignment expression analysis
    return true;
}