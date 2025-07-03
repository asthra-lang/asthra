/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Security Annotation Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Security annotation validation (constant-time, volatile memory)
 */

#include "semantic_security.h"
#include "../parser/ast_operations.h"
#include "semantic_core.h"
#include <stdlib.h>

// =============================================================================
// SECURITY ANNOTATION VALIDATION (STUB IMPLEMENTATIONS)
// =============================================================================

bool semantic_check_security_annotations(SemanticAnalyzer *analyzer, ASTNodeList *annotations) {
    if (!analyzer || !annotations)
        return true; // No annotations to check

    bool all_valid = true;
    for (size_t i = 0; i < annotations->count; i++) {
        ASTNode *annotation = annotations->nodes[i];
        if (!annotation || annotation->type != AST_SECURITY_TAG) {
            // Not a security tag, or invalid node, skip or report error
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION, annotation->location,
                                  "Invalid security annotation structure");
            all_valid = false;
            continue;
        }

        SecurityType sec_type = annotation->data.security_tag.security_type;
        switch (sec_type) {
        case SECURITY_CONSTANT_TIME:
            // This annotation marks a function or block as constant-time.
            // Actual validation happens in `semantic_validate_constant_time_*` functions
            // when analyzing the body of the annotated construct.
            break;
        case SECURITY_VOLATILE_MEMORY:
            // This annotation marks a variable or field as volatile.
            // Actual validation of accesses happens in `semantic_validate_volatile_memory_access`.
            break;
        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION, annotation->location,
                                  "Unsupported security annotation type: %d", sec_type);
            all_valid = false;
            break;
        }
    }
    return all_valid;
}

bool semantic_validate_security_annotation_context(SemanticAnalyzer *analyzer, ASTNode *node,
                                                   SecurityType security_type) {
    (void)analyzer;
    (void)node;
    (void)security_type;
    // TODO: Implement security annotation context validation
    return true;
}

// =============================================================================
// CONSTANT-TIME VALIDATION (STUB IMPLEMENTATIONS)
// =============================================================================

bool semantic_validate_constant_time_function(SemanticAnalyzer *analyzer, ASTNode *func_decl) {
    if (!analyzer || !func_decl || func_decl->type != AST_FUNCTION_DECL)
        return false;

    if (!semantic_has_constant_time_annotation(func_decl))
        return true; // Not a constant-time function

    // For a constant-time function, we must ensure:
    // 1. No data-dependent branches (if, match, loops where condition depends on secret data).
    // 2. No data-dependent memory accesses (indexing arrays with secret data).
    // 3. All operations take constant time (e.g., no early exits from loops).
    // 4. No division/modulo by non-constants.
    // 5. Only calls to other constant-time functions.

    // Use the comprehensive validation for the function body
    ASTNode *body = func_decl->data.function_decl.body;
    if (body) {
        return semantic_validate_constant_time_block(analyzer, body);
    }

    return true;
}

bool semantic_validate_constant_time_block(SemanticAnalyzer *analyzer, ASTNode *block) {
    if (!analyzer || !block || block->type != AST_BLOCK)
        return false;

    // Check all statements in the block for constant-time violations
    if (block->data.block.statements) {
        for (size_t i = 0; i < block->data.block.statements->count; i++) {
            ASTNode *stmt = block->data.block.statements->nodes[i];
            if (stmt && !semantic_validate_constant_time_statement(analyzer, stmt)) {
                return false;
            }
        }
    }

    return true;
}

bool semantic_validate_constant_time_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt)
        return false;

    switch (stmt->type) {
    // Control flow statements are not allowed in constant-time code
    case AST_IF_STMT:
        semantic_report_error(analyzer, SEMANTIC_ERROR_SECURITY_VIOLATION, stmt->location,
                              "If statements are not allowed in constant-time code");
        return false;

    case AST_MATCH_STMT:
        semantic_report_error(analyzer, SEMANTIC_ERROR_SECURITY_VIOLATION, stmt->location,
                              "Match statements are not allowed in constant-time code");
        return false;

    case AST_FOR_STMT:
        semantic_report_error(analyzer, SEMANTIC_ERROR_SECURITY_VIOLATION, stmt->location,
                              "For loops are not allowed in constant-time code");
        return false;

        // Note: Asthra doesn't have while loops

    case AST_RETURN_STMT:
        // Early returns based on data are not allowed
        // For now, allow all returns but ideally should check if it's conditional
        if (stmt->data.return_stmt.expression) {
            return semantic_validate_constant_time_expression(analyzer,
                                                              stmt->data.return_stmt.expression);
        }
        return true;

    case AST_EXPR_STMT:
        // Validate the expression
        return semantic_validate_constant_time_expression(analyzer,
                                                          stmt->data.expr_stmt.expression);

    case AST_LET_STMT:
        // Let statements are allowed, but validate the initializer
        if (stmt->data.let_stmt.initializer) {
            return semantic_validate_constant_time_expression(analyzer,
                                                              stmt->data.let_stmt.initializer);
        }
        return true;

    case AST_ASSIGNMENT:
        // Validate both target and value
        if (!semantic_validate_constant_time_expression(analyzer, stmt->data.assignment.target)) {
            return false;
        }
        return semantic_validate_constant_time_expression(analyzer, stmt->data.assignment.value);

    case AST_BLOCK:
        // Nested blocks are allowed
        return semantic_validate_constant_time_block(analyzer, stmt);

    default:
        // Other statement types are generally allowed
        return true;
    }
}

bool semantic_validate_constant_time_call(SemanticAnalyzer *analyzer, ASTNode *call_expr) {
    if (!analyzer || !call_expr)
        return false;

    // For function calls in constant-time code:
    // 1. The called function must also be constant-time
    // 2. Arguments must be constant-time expressions

    // First, validate all arguments
    if (call_expr->type == AST_CALL_EXPR) {
        // Validate function expression
        if (!semantic_validate_constant_time_expression(analyzer,
                                                        call_expr->data.call_expr.function)) {
            return false;
        }

        // Validate arguments
        if (call_expr->data.call_expr.args) {
            for (size_t i = 0; i < call_expr->data.call_expr.args->count; i++) {
                ASTNode *arg = call_expr->data.call_expr.args->nodes[i];
                if (arg && !semantic_validate_constant_time_expression(analyzer, arg)) {
                    return false;
                }
            }
        }

        // TODO: Check if the called function is marked as constant-time
        // This requires resolving the function and checking its annotations
        // For now, we'll just validate the arguments
        return true;
    }

    // Similar checks for method calls and associated function calls
    else if (call_expr->type == AST_ASSOCIATED_FUNC_CALL) {
        // Validate arguments for associated function calls
        if (call_expr->data.associated_func_call.args) {
            for (size_t i = 0; i < call_expr->data.associated_func_call.args->count; i++) {
                ASTNode *arg = call_expr->data.associated_func_call.args->nodes[i];
                if (arg && !semantic_validate_constant_time_expression(analyzer, arg)) {
                    return false;
                }
            }
        }
        return true;
    }

    return true;
}

bool semantic_validate_constant_time_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr)
        return false;

    switch (expr->type) {
    // Literals are always constant-time
    case AST_INTEGER_LITERAL:
    case AST_FLOAT_LITERAL:
    case AST_STRING_LITERAL:
    case AST_BOOL_LITERAL:
    case AST_CHAR_LITERAL:
    case AST_UNIT_LITERAL:
        return true;

    // Identifiers are allowed
    case AST_IDENTIFIER:
        return true;

    // Binary operations
    case AST_BINARY_EXPR: {
        // First check operands
        if (!semantic_validate_constant_time_expression(analyzer, expr->data.binary_expr.left) ||
            !semantic_validate_constant_time_expression(analyzer, expr->data.binary_expr.right)) {
            return false;
        }

        // Division and modulo by non-constants are not constant-time
        if (expr->data.binary_expr.operator== BINOP_DIV ||
            expr->data.binary_expr.operator== BINOP_MOD) {
            // Check if right operand is a constant
            ASTNode *right = expr->data.binary_expr.right;
            if (right->type != AST_INTEGER_LITERAL && right->type != AST_FLOAT_LITERAL) {
                semantic_report_error(
                    analyzer, SEMANTIC_ERROR_SECURITY_VIOLATION, expr->location,
                    "Division/modulo by non-constant value is not allowed in constant-time code");
                return false;
            }
        }
        return true;
    }

    // Unary operations
    case AST_UNARY_EXPR:
        return semantic_validate_constant_time_expression(analyzer, expr->data.unary_expr.operand);

    // Array/slice indexing with data-dependent indices is not allowed
    case AST_INDEX_ACCESS: {
        // Validate array expression
        if (!semantic_validate_constant_time_expression(analyzer, expr->data.index_access.array)) {
            return false;
        }

        // Index must be a compile-time constant
        ASTNode *index = expr->data.index_access.index;
        if (index->type != AST_INTEGER_LITERAL) {
            semantic_report_error(
                analyzer, SEMANTIC_ERROR_SECURITY_VIOLATION, expr->location,
                "Array indexing with non-constant index is not allowed in constant-time code");
            return false;
        }
        return true;
    }

    // Field access is allowed
    case AST_FIELD_ACCESS:
        return semantic_validate_constant_time_expression(analyzer, expr->data.field_access.object);

    // Function calls
    case AST_CALL_EXPR:
    case AST_ASSOCIATED_FUNC_CALL:
        return semantic_validate_constant_time_call(analyzer, expr);

    // Cast expressions
    case AST_CAST_EXPR:
        return semantic_validate_constant_time_expression(analyzer,
                                                          expr->data.cast_expr.expression);

    // Tuple and array literals
    case AST_TUPLE_LITERAL:
        if (expr->data.tuple_literal.elements) {
            for (size_t i = 0; i < expr->data.tuple_literal.elements->count; i++) {
                ASTNode *elem = expr->data.tuple_literal.elements->nodes[i];
                if (elem && !semantic_validate_constant_time_expression(analyzer, elem)) {
                    return false;
                }
            }
        }
        return true;

    case AST_ARRAY_LITERAL:
        if (expr->data.array_literal.elements) {
            for (size_t i = 0; i < expr->data.array_literal.elements->count; i++) {
                ASTNode *elem = expr->data.array_literal.elements->nodes[i];
                if (elem && !semantic_validate_constant_time_expression(analyzer, elem)) {
                    return false;
                }
            }
        }
        return true;

    // Struct literals
    case AST_STRUCT_LITERAL:
        if (expr->data.struct_literal.field_inits) {
            for (size_t i = 0; i < expr->data.struct_literal.field_inits->count; i++) {
                ASTNode *field_init = expr->data.struct_literal.field_inits->nodes[i];
                if (field_init && field_init->type == AST_ASSIGNMENT) {
                    // Field initialization is stored as assignment: field_name = value
                    ASTNode *field_value = field_init->data.assignment.value;
                    if (field_value &&
                        !semantic_validate_constant_time_expression(analyzer, field_value)) {
                        return false;
                    }
                }
            }
        }
        return true;

    default:
        // Conservative: disallow unknown expression types
        semantic_report_error(analyzer, SEMANTIC_ERROR_SECURITY_VIOLATION, expr->location,
                              "Expression type %s is not allowed in constant-time code",
                              ast_node_type_name(expr->type));
        return false;
    }
}

// =============================================================================
// VOLATILE MEMORY VALIDATION
// =============================================================================

/**
 * Validate access to volatile-annotated memory locations.
 * This is a simplified implementation, focusing on explicit volatile keywords.
 * A full implementation would involve data flow analysis to track volatility.
 */
bool semantic_validate_volatile_memory_access(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!analyzer || !node)
        return false;

    // For now, assume any access to a variable/field/dereference that is (or points to) volatile
    // should be validated. This is a simplified check.
    switch (node->type) {
    case AST_IDENTIFIER: {
        SymbolEntry *symbol = semantic_resolve_identifier(analyzer, node->data.identifier.name);
        if (symbol && (symbol->kind == SYMBOL_VARIABLE || symbol->kind == SYMBOL_PARAMETER)) {
            if (symbol->type && symbol->type->flags.is_volatile) {
                // Accessing a volatile variable/parameter
                // For now, no specific error unless it's a non-atomic access to volatile.
                // More advanced checks would go here.
                return true;
            }
        }
        break;
    }
    case AST_FIELD_ACCESS: {
        // Check if the base object or the field itself is volatile
        // For now, a simplified check: if the base is volatile, the access is volatile.
        if (semantic_validate_volatile_memory_access(analyzer, node->data.field_access.object)) {
            return true;
        }
        // TODO: Also check if the *field declaration itself* has a volatile annotation.
        break;
    }
    case AST_INDEX_ACCESS: {
        // Check if the base array/slice/pointer is volatile
        if (semantic_validate_volatile_memory_access(analyzer, node->data.index_access.array)) {
            return true;
        }
        break;
    }
    case AST_UNARY_EXPR: {
        // Check if this is a dereference operation and if the pointer being dereferenced points to
        // volatile data
        if (node->data.unary_expr.operator== UNOP_DEREF) {
            if (semantic_validate_volatile_memory_access(analyzer, node->data.unary_expr.operand)) {
                return true;
            }
            // TODO: Check if the pointer type itself indicates volatile pointee (e.g., `*volatile
            // i32`)
        }
        break;
    }
    default:
        // Other expression types are not direct memory accesses for this check
        break;
    }
    return true; // Assume valid if no specific volatile issue found
}

bool semantic_validate_volatile_memory_argument(SemanticAnalyzer *analyzer, ASTNode *arg) {
    (void)analyzer;
    (void)arg;
    // TODO: Implement volatile memory argument validation
    // Ensure arguments passed to functions expecting volatile pointers/data are compatible.
    return true;
}

bool semantic_check_volatile_assignment(SemanticAnalyzer *analyzer, ASTNode *assignment) {
    (void)analyzer;
    (void)assignment;
    // TODO: Implement volatile assignment checking
    // Ensure assignments to volatile locations are handled correctly (e.g., no reordering by
    // optimizer).
    return true;
}

bool semantic_check_volatile_field_access(SemanticAnalyzer *analyzer, ASTNode *field_access) {
    (void)analyzer;
    (void)field_access;
    // TODO: Implement volatile field access checking
    // Ensure access to volatile fields within structs is handled correctly.
    return true;
}

bool semantic_check_volatile_dereference(SemanticAnalyzer *analyzer, ASTNode *deref) {
    (void)analyzer;
    (void)deref;
    // TODO: Implement volatile dereference checking
    // Ensure dereferencing of volatile pointers is handled correctly.
    return true;
}

// =============================================================================
// ANNOTATION HELPERS
// =============================================================================

/**
 * Helper to check if a declaration has a volatile annotation.
 */
bool semantic_has_volatile_annotation(ASTNode *declaration) {
    if (!declaration)
        return false;

    ASTNodeList *annotations = NULL;
    // Get annotations list based on node type
    switch (declaration->type) {
    case AST_FUNCTION_DECL:
        annotations = declaration->data.function_decl.annotations;
        break;
    case AST_STRUCT_DECL:
        annotations = declaration->data.struct_decl.annotations;
        break;
    case AST_ENUM_DECL:
        annotations = declaration->data.enum_decl.annotations;
        break;
    case AST_EXTERN_DECL:
        annotations = declaration->data.extern_decl.annotations;
        break;
    case AST_PARAM_DECL:
        annotations = declaration->data.param_decl.annotations;
        break;
    case AST_METHOD_DECL:
        annotations = declaration->data.method_decl.annotations;
        break;
    case AST_IMPL_BLOCK:
        annotations = declaration->data.impl_block.annotations;
        break;
    case AST_CONST_DECL:
        annotations = declaration->data.const_decl.annotations;
        break;
    // For AST_VAR_DECL (let statement), annotations are on the variable node itself
    case AST_LET_STMT:
        // Assuming a simple way to get variable annotations, or handled by
        // semantic_type_is_volatile on type. For now, if the variable's type is marked volatile,
        // it's considered volatile.
        return false; // Not handled directly via node annotations here
    default:
        // No annotations for this node type or not applicable
        return false;
    }

    if (!annotations)
        return false;

    for (size_t i = 0; i < annotations->count; i++) {
        ASTNode *annotation = annotations->nodes[i];
        if (annotation && annotation->type == AST_SECURITY_TAG &&
            annotation->data.security_tag.security_type == SECURITY_VOLATILE_MEMORY) {
            return true;
        }
    }
    return false;
}

/**
 * Helper to check if a declaration has a constant_time annotation.
 */
bool semantic_has_constant_time_annotation(ASTNode *declaration) {
    if (!declaration)
        return false;

    ASTNodeList *annotations = NULL;
    // Get annotations list based on node type
    switch (declaration->type) {
    case AST_FUNCTION_DECL:
        annotations = declaration->data.function_decl.annotations;
        break;
    case AST_STRUCT_DECL:
        annotations = declaration->data.struct_decl.annotations;
        break;
    case AST_ENUM_DECL:
        annotations = declaration->data.enum_decl.annotations;
        break;
    case AST_EXTERN_DECL:
        annotations = declaration->data.extern_decl.annotations;
        break;
    case AST_PARAM_DECL:
        annotations = declaration->data.param_decl.annotations;
        break;
    case AST_METHOD_DECL:
        annotations = declaration->data.method_decl.annotations;
        break;
    case AST_IMPL_BLOCK:
        annotations = declaration->data.impl_block.annotations;
        break;
    case AST_CONST_DECL:
        annotations = declaration->data.const_decl.annotations;
        break;
    default:
        // No annotations for this node type or not applicable
        return false;
    }

    if (!annotations)
        return false;

    for (size_t i = 0; i < annotations->count; i++) {
        ASTNode *annotation = annotations->nodes[i];
        if (annotation && annotation->type == AST_SECURITY_TAG &&
            annotation->data.security_tag.security_type == SECURITY_CONSTANT_TIME) {
            return true;
        }
    }
    return false;
}
