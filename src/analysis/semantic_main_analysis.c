/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Main Analysis Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core semantic analysis functions for programs, declarations, statements, and expressions
 */

#include "../parser/ast.h"
#include "semantic_annotations.h"
#include "semantic_arrays.h"
#include "semantic_binary_unary.h"
#include "semantic_calls.h"
#include "semantic_const_declarations.h"
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

bool semantic_analyze_program(SemanticAnalyzer *analyzer, ASTNode *program) {
    if (!analyzer || !program || program->type != AST_PROGRAM) {
        return false;
    }

    // Analyze imports first
    if (program->data.program.imports) {
        ASTNodeList *imports = program->data.program.imports;
        size_t import_count = ast_node_list_size(imports);

        for (size_t i = 0; i < import_count; i++) {
            ASTNode *import_decl = ast_node_list_get(imports, i);
            if (!semantic_analyze_declaration(analyzer, import_decl)) {
                return false;
            }
        }
    }

    // Then analyze declarations
    if (program->data.program.declarations) {
        ASTNodeList *declarations = program->data.program.declarations;
        size_t decl_count = ast_node_list_size(declarations);

        for (size_t i = 0; i < decl_count; i++) {
            ASTNode *decl = ast_node_list_get(declarations, i);
            if (!semantic_analyze_declaration(analyzer, decl)) {
                return false;
            }
        }
    }

    // Return false if any errors were found during analysis
    return analyzer->error_count == 0;
}

bool semantic_analyze_declaration(SemanticAnalyzer *analyzer, ASTNode *decl) {
    if (!analyzer || !decl) {
        return false;
    }

    ATOMIC_FETCH_ADD(&analyzer->stats.nodes_analyzed, 1);

    switch (decl->type) {
    case AST_IMPORT_DECL:
        return analyze_import_declaration(analyzer, decl);

    case AST_VISIBILITY_MODIFIER:
        return analyze_visibility_modifier(analyzer, decl);

    case AST_FUNCTION_DECL:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_function_declaration(analyzer, decl);

    case AST_STRUCT_DECL:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_struct_declaration(analyzer, decl);

    case AST_ENUM_DECL:
        // Phase 4: Integrate enum declaration analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_enum_declaration(analyzer, decl);

    case AST_EXTERN_DECL:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_extern_declaration(analyzer, decl);

    case AST_IMPL_BLOCK:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_impl_block(analyzer, decl);

    case AST_CONST_DECL:
        // Phase 2: Integrate const declaration analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_const_declaration(analyzer, decl);

    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, decl->location,
                              "Unsupported declaration type: %d", decl->type);
        return false;
    }
}

bool semantic_analyze_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt)
        return false;

    ATOMIC_FETCH_ADD(&analyzer->stats.nodes_analyzed, 1);

    switch (stmt->type) {
    case AST_LET_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_let_statement(analyzer, stmt);

    case AST_BLOCK:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_block_statement(analyzer, stmt);

    case AST_EXPR_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_expression_statement(analyzer, stmt);

    case AST_RETURN_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_return_statement(analyzer, stmt);

    case AST_IF_STMT: {
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        bool result = analyze_if_statement(analyzer, stmt);
        if (result) {
            // Store type information for if-else expressions
            TypeDescriptor *type = semantic_get_expression_type(analyzer, stmt);
            if (type) {
                semantic_set_expression_type(analyzer, stmt, type);
                type_descriptor_release(type);
            }
        }
        return result;
    }

    case AST_IF_LET_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_if_let_statement(analyzer, stmt);

    case AST_MATCH_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_match_statement(analyzer, stmt);

    case AST_SPAWN_STMT:
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, stmt)) {
            return false;
        }
        return analyze_spawn_statement(analyzer, stmt);

    case AST_SPAWN_WITH_HANDLE_STMT:
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, stmt)) {
            return false;
        }
        return analyze_spawn_with_handle_statement(analyzer, stmt);

    case AST_AWAIT_EXPR:
        // Note: await is an expression, but might appear as statement too
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, stmt)) {
            return false;
        }
        return analyze_await_statement(analyzer, stmt);

    case AST_UNSAFE_BLOCK:
        // Analyze unsafe blocks with safety validation
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_unsafe_block_statement(analyzer, stmt);

    case AST_ASSIGNMENT:
        // Analyze assignment statements with mutability validation
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_assignment_validation(analyzer, stmt);

    case AST_FOR_STMT:
        // Analyze for loop statements
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_for_statement(analyzer, stmt);

    case AST_BREAK_STMT:
        // Analyze break statements
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_break_statement(analyzer, stmt);

    case AST_CONTINUE_STMT:
        // Analyze continue statements
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_continue_statement(analyzer, stmt);

    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, stmt->location,
                              "Unsupported statement type: %d", stmt->type);
        return false;
    }
}

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
        result = true;
        if (expr->data.associated_func_call.args) {
            for (size_t i = 0; i < ast_node_list_size(expr->data.associated_func_call.args); i++) {
                ASTNode *arg = ast_node_list_get(expr->data.associated_func_call.args, i);
                if (!semantic_analyze_expression(analyzer, arg)) {
                    result = false;
                    break;
                }
            }
        }

        // TODO: Check that the struct and function exist and are accessible
        break;

    case AST_AWAIT_EXPR:
        // Tier 1 concurrency: Basic await is deterministic and doesn't require annotation
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, expr)) {
            result = false;
        } else {
            // TODO: Validate the awaited handle is valid
            result = true;
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

    default:
        // For other expression types, just return true for now
        // TODO: Implement analysis for other expression types
        result = true;
        break;
    }

    return result;
}