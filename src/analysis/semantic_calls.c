/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Call Expressions and Argument Validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of function calls, method calls, and argument validation
 */

#include "semantic_calls.h"
#include "semantic_builtins.h"
#include "semantic_core.h"
#include "semantic_diagnostics.h"
#include "semantic_expression_utils.h"
#include "semantic_type_creation.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CALL EXPRESSION ANALYSIS
// =============================================================================

bool analyze_call_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_CALL_EXPR) {
        return false;
    }

    ASTNode *function = expr->data.call_expr.function;
    if (!function) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, expr->location,
                              "Invalid function call: missing function");
        return false;
    }

    // For simple identifier function calls, verify function exists
    if (function->type == AST_IDENTIFIER) {
        const char *func_name = function->data.identifier.name;
        SymbolEntry *func_symbol = semantic_resolve_identifier(analyzer, func_name);

        if (!func_symbol) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, function->location,
                                  "Undefined function: %s", func_name);
            return false;
        }

        // Check if symbol is actually a function
        if (func_symbol->kind != SYMBOL_FUNCTION) {
            semantic_report_error_enhanced(analyzer, SEMANTIC_ERROR_NOT_CALLABLE,
                                           function->location, "cannot call non-function value",
                                           "remove the parentheses or use a function name",
                                           func_symbol->kind == SYMBOL_VARIABLE
                                               ? "this is a variable, not a function"
                                               : "this is a type or constant, not a function");
            return false;
        }

        func_symbol->flags.is_used = true;

        // Special handling for predeclared functions with generic type checking
        if (func_symbol->flags.is_predeclared && strcmp(func_name, "len") == 0) {
            // len() function: special validation for slice/array arguments
            if (!validate_len_function_call(analyzer, expr)) {
                return false;
            }
        } else if (func_symbol->flags.is_predeclared && strcmp(func_name, "range") == 0) {
            // range() function: special validation for overloaded signatures
            if (!validate_range_function_call(analyzer, expr)) {
                return false;
            }
        } else {
            // Validate function arguments against parameter types
            if (!validate_function_arguments(analyzer, expr, func_symbol)) {
                return false;
            }
        }

        // Set the return type on the call expression
        if (func_symbol->type && func_symbol->type->category == TYPE_FUNCTION) {
            TypeDescriptor *return_type = func_symbol->type->data.function.return_type;
            if (return_type) {
                expr->type_info = type_info_from_descriptor(return_type);
                if (!expr->type_info) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                          "Failed to create type info for function call");
                    return false;
                }
            }
        }

        return true;
    }
    // For enum variant calls (Option.Some(value)), the function is an enum variant
    else if (function->type == AST_ENUM_VARIANT) {
        const char *enum_name = function->data.enum_variant.enum_name;
        const char *variant_name = function->data.enum_variant.variant_name;

        if (!enum_name || !variant_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, function->location,
                                  "Invalid enum variant call");
            return false;
        }

        // Find the enum symbol
        SymbolEntry *enum_symbol = semantic_resolve_identifier(analyzer, enum_name);
        if (!enum_symbol || enum_symbol->kind != SYMBOL_TYPE || !enum_symbol->type ||
            enum_symbol->type->category != TYPE_ENUM) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, function->location,
                                  "Undefined enum type: %s", enum_name);
            return false;
        }

        // Check if variant exists
        SymbolEntry *variant_symbol =
            symbol_table_lookup_safe(enum_symbol->type->data.enum_type.variants, variant_name);
        if (!variant_symbol || variant_symbol->kind != SYMBOL_ENUM_VARIANT) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, function->location,
                                  "Enum '%s' has no variant '%s'", enum_name, variant_name);
            return false;
        }

        ASTNodeList *args = expr->data.call_expr.args;
        size_t arg_count = args ? ast_node_list_size(args) : 0;

        if (arg_count > 1) {
            semantic_report_error(
                analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, expr->location,
                "Enum constructors support only single values. Use a tuple for multiple values.");
            return false;
        }

        // Analyze the argument if present
        if (arg_count == 1) {
            ASTNode *variant_value = ast_node_list_get(args, 0);
            if (!semantic_analyze_expression(analyzer, variant_value)) {
                return false;
            }
        }

        // For generic enums, try to infer type parameters from arguments
        TypeDescriptor *enum_type_to_use = enum_symbol->type;

        // Check if this is a generic enum and try to infer type parameters
        if (enum_symbol->is_generic) {
            // Try to get expected type from context (e.g., variable declaration)
            TypeDescriptor *expected_type = analyzer->expected_type;

            // Expected type context is available for type inference

            if (expected_type && expected_type->category == TYPE_GENERIC_INSTANCE &&
                type_descriptor_equals(expected_type->data.generic_instance.base_type,
                                       enum_symbol->type)) {
                // Use the expected type directly (e.g., Result<i32, string>)
                // Use the expected type directly (e.g., Result<i32, string>)
                enum_type_to_use = expected_type;
            } else if (expected_type && expected_type->category == TYPE_RESULT &&
                       enum_symbol->type && enum_symbol->type->name &&
                       strcmp(enum_symbol->type->name, "Result") == 0) {
                // Handle case where expected type is TYPE_RESULT but enum is the Result enum
                // Create a generic instance equivalent to the TYPE_RESULT
                // Handle case where expected type is TYPE_RESULT but enum is the Result enum
                // Create a generic instance equivalent to the TYPE_RESULT
                TypeDescriptor *type_args[] = {expected_type->data.result.ok_type,
                                               expected_type->data.result.err_type};
                TypeDescriptor *generic_instance =
                    type_descriptor_create_generic_instance(enum_symbol->type, type_args, 2);
                if (generic_instance) {
                    enum_type_to_use = generic_instance;
                }
            } else if (arg_count > 0) {
                // Fallback: infer what we can from the argument
                ASTNode *variant_value = ast_node_list_get(args, 0);
                TypeDescriptor *arg_type = semantic_get_expression_type(analyzer, variant_value);

                if (arg_type) {
                    // For now, create a single-parameter generic instance from the argument
                    // This works for Option<T> and provides partial inference for Result<T, E>
                    TypeDescriptor *type_args[] = {arg_type};
                    TypeDescriptor *generic_instance =
                        type_descriptor_create_generic_instance(enum_symbol->type, type_args, 1);

                    if (generic_instance) {
                        enum_type_to_use = generic_instance;
                    }

                    type_descriptor_release(arg_type);
                }
            }
        }

        // Set the expression type to the (possibly specialized) enum type
        expr->type_info = type_info_from_descriptor(enum_type_to_use);
        if (!expr->type_info) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                  "Failed to create type info for enum variant call");
            return false;
        }

        return true;
    }
    // For method calls (p.method()), the function is a field access
    else if (function->type == AST_FIELD_ACCESS) {
        ASTNode *object = function->data.field_access.object;
        const char *method_name = function->data.field_access.field_name;

        if (!object || !method_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, function->location,
                                  "Invalid method call");
            return false;
        }

        // First analyze the object expression
        if (!semantic_analyze_expression(analyzer, object)) {
            return false;
        }

        // Get the type of the object
        TypeDescriptor *object_type = semantic_get_expression_type(analyzer, object);
        if (!object_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, object->location,
                                  "Cannot determine type of object in method call");
            return false;
        }

        // For enum types, this is actually an enum constructor call, not a method call
        if (object_type->category == TYPE_ENUM) {
            // Handle enum constructor call: EnumName.Variant(value)
            // DON'T transform the AST node - keep it as AST_CALL_EXPR to avoid memory corruption

            ASTNodeList *args = expr->data.call_expr.args;
            size_t arg_count = args ? ast_node_list_size(args) : 0;

            if (arg_count > 1) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, expr->location,
                                      "Enum constructors support only single values. Use a tuple "
                                      "for multiple values.");
                return false;
            }

            // Get the enum name from the object identifier
            const char *enum_name = NULL;
            if (object->type == AST_IDENTIFIER && object->data.identifier.name) {
                enum_name = object->data.identifier.name;
            } else if (object_type->name) {
                enum_name = object_type->name;
            }

            if (!enum_name || !method_name) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, expr->location,
                                      "Invalid enum constructor: missing enum or variant name");
                return false;
            }

            // Validate the enum and variant exist
            SymbolEntry *enum_symbol = semantic_resolve_identifier(analyzer, enum_name);
            if (!enum_symbol || enum_symbol->kind != SYMBOL_TYPE || !enum_symbol->type ||
                enum_symbol->type->category != TYPE_ENUM) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, expr->location,
                                      "Undefined enum type: %s", enum_name);
                return false;
            }

            // Check if variant exists in the enum
            SymbolEntry *variant_symbol =
                symbol_table_lookup_safe(enum_symbol->type->data.enum_type.variants, method_name);
            if (!variant_symbol || variant_symbol->kind != SYMBOL_ENUM_VARIANT) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, expr->location,
                                      "Enum '%s' has no variant '%s'", enum_name, method_name);
                return false;
            }

            // Analyze the argument if present
            if (arg_count == 1) {
                ASTNode *variant_value = ast_node_list_get(args, 0);
                if (!semantic_analyze_expression(analyzer, variant_value)) {
                    type_descriptor_release(object_type);
                    return false;
                }
            }

            // For generic enums, try to infer type parameters from arguments
            TypeDescriptor *enum_type_to_use = enum_symbol->type;

            // Check if this is a generic enum and try to infer type parameters
            if (enum_symbol->is_generic) {
                // Try to get expected type from context (e.g., variable declaration)
                TypeDescriptor *expected_type = analyzer->expected_type;

                if (expected_type && expected_type->category == TYPE_GENERIC_INSTANCE &&
                    type_descriptor_equals(expected_type->data.generic_instance.base_type,
                                           enum_symbol->type)) {
                    // Use the expected type directly (e.g., Result<i32, string>)
                    enum_type_to_use = expected_type;
                } else if (expected_type && expected_type->category == TYPE_RESULT &&
                           enum_symbol->type && enum_symbol->type->name &&
                           strcmp(enum_symbol->type->name, "Result") == 0) {
                    // Handle case where expected type is TYPE_RESULT but enum is the Result enum
                    // Create a generic instance equivalent to the TYPE_RESULT
                    TypeDescriptor *type_args[] = {expected_type->data.result.ok_type,
                                                   expected_type->data.result.err_type};
                    TypeDescriptor *generic_instance =
                        type_descriptor_create_generic_instance(enum_symbol->type, type_args, 2);
                    if (generic_instance) {
                        enum_type_to_use = generic_instance;
                    }
                } else if (arg_count > 0) {
                    // Fallback: infer what we can from the argument
                    ASTNode *variant_value = ast_node_list_get(args, 0);
                    TypeDescriptor *arg_type =
                        semantic_get_expression_type(analyzer, variant_value);

                    if (arg_type) {
                        // For now, create a single-parameter generic instance from the argument
                        // This works for Option<T> and provides partial inference for Result<T, E>
                        TypeDescriptor *type_args[] = {arg_type};
                        TypeDescriptor *generic_instance = type_descriptor_create_generic_instance(
                            enum_symbol->type, type_args, 1);

                        if (generic_instance) {
                            enum_type_to_use = generic_instance;
                        }

                        type_descriptor_release(arg_type);
                    }
                }
            }

            // Set the expression type to the (possibly specialized) enum type
            expr->type_info = type_info_from_descriptor(enum_type_to_use);
            if (!expr->type_info) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                      "Failed to create type info for enum constructor");
                return false;
            }

            type_descriptor_release(object_type);
            return true;
        }

        // For struct types, look up the method in the impl block
        if (object_type->category == TYPE_STRUCT) {
            // Look up method in the struct's method table
            SymbolEntry *method_symbol = NULL;
            if (object_type->data.struct_type.methods) {
                method_symbol =
                    symbol_table_lookup_safe(object_type->data.struct_type.methods, method_name);
            }

            if (!method_symbol) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, function->location,
                                      "Method '%s' not found for struct type", method_name);
                return false;
            }

            // Mark method as used
            method_symbol->flags.is_used = true;

            // Validate method arguments
            if (!validate_method_arguments(analyzer, expr, method_symbol)) {
                type_descriptor_release(object_type);
                return false;
            }

            type_descriptor_release(object_type);
            return true;
        }

        type_descriptor_release(object_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, function->location,
                              "Cannot call method '%s' on non-struct type", method_name);
        return false;
    }

    // For other function expressions, analyze them recursively
    return semantic_analyze_expression(analyzer, function);
}

// =============================================================================
// FUNCTION ARGUMENT VALIDATION
// =============================================================================

bool validate_function_arguments(SemanticAnalyzer *analyzer, ASTNode *call_expr,
                                 SymbolEntry *func_symbol) {
    if (!analyzer || !call_expr || !func_symbol || !func_symbol->type) {
        return false;
    }

    ASTNodeList *args = NULL;

    // Handle both AST_CALL_EXPR and AST_ASSOCIATED_FUNC_CALL
    if (call_expr->type == AST_CALL_EXPR) {
        args = call_expr->data.call_expr.args;
    } else if (call_expr->type == AST_ASSOCIATED_FUNC_CALL) {
        args = call_expr->data.associated_func_call.args;
    } else {
        return false;
    }

    TypeDescriptor *func_type = func_symbol->type;

    // Check if function type has parameter information
    if (func_type->category != TYPE_FUNCTION) {
        // For non-function types, skip argument validation
        return true;
    }

    // Get expected parameter count
    size_t expected_param_count = func_type->data.function.param_count;
    size_t actual_arg_count = args ? ast_node_list_size(args) : 0;

    // Check argument count
    if (actual_arg_count != expected_param_count) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, call_expr->location,
                              "Function '%s' expects %zu arguments, got %zu",
                              func_symbol->name ? func_symbol->name : "unknown",
                              expected_param_count, actual_arg_count);
        return false;
    }

    // Check argument types against parameter types
    if (args && func_type->data.function.param_types) {
        for (size_t i = 0; i < actual_arg_count; i++) {
            ASTNode *arg = ast_node_list_get(args, i);
            TypeDescriptor *expected_param_type = func_type->data.function.param_types[i];

            if (!arg || !expected_param_type) {
                continue;
            }

            // Set expected type context for the argument
            TypeDescriptor *old_expected_type = analyzer->expected_type;
            analyzer->expected_type = expected_param_type;

            // Analyze the argument expression with type context
            bool analyze_result = semantic_analyze_expression(analyzer, arg);

            // Restore previous expected type
            analyzer->expected_type = old_expected_type;

            if (!analyze_result) {
                return false;
            }

            // Get the argument's type
            TypeDescriptor *arg_type = semantic_get_expression_type(analyzer, arg);
            if (!arg_type) {
                continue;
            }

            // Check type compatibility
            if (!semantic_check_type_compatibility(analyzer, arg_type, expected_param_type)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, arg->location,
                                      "Argument %zu type mismatch: expected %s, got %s", i + 1,
                                      expected_param_type->name ? expected_param_type->name
                                                                : "unknown",
                                      arg_type->name ? arg_type->name : "unknown");
                type_descriptor_release(arg_type);
                return false;
            }

            type_descriptor_release(arg_type);
        }
    }

    return true;
}

bool validate_method_arguments(SemanticAnalyzer *analyzer, ASTNode *call_expr,
                               SymbolEntry *method_symbol) {
    if (!analyzer || !call_expr || !method_symbol || !method_symbol->type) {
        return false;
    }

    ASTNodeList *args = call_expr->data.call_expr.args;
    TypeDescriptor *method_type = method_symbol->type;

    // Check if method type has parameter information
    if (method_type->category != TYPE_FUNCTION) {
        // For non-function types, skip argument validation
        return true;
    }

    // Get expected parameter count
    size_t expected_param_count = method_type->data.function.param_count;
    size_t actual_arg_count = args ? ast_node_list_size(args) : 0;

    // Adjust expected count for instance methods (subtract self parameter)
    if (method_symbol->is_instance_method && expected_param_count > 0) {
        expected_param_count--;
    }

    // Check argument count
    if (actual_arg_count != expected_param_count) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, call_expr->location,
                              "Method '%s' expects %zu arguments, got %zu",
                              method_symbol->name ? method_symbol->name : "unknown",
                              expected_param_count, actual_arg_count);
        return false;
    }

    // Check argument types against parameter types (skip self parameter)
    if (args && method_type->data.function.param_types) {
        size_t param_offset = method_symbol->is_instance_method ? 1 : 0;

        for (size_t i = 0; i < actual_arg_count; i++) {
            ASTNode *arg = ast_node_list_get(args, i);
            TypeDescriptor *expected_param_type =
                method_type->data.function.param_types[i + param_offset];

            if (!arg || !expected_param_type) {
                continue;
            }

            // Set expected type context for the argument
            TypeDescriptor *old_expected_type = analyzer->expected_type;
            analyzer->expected_type = expected_param_type;

            // Analyze the argument expression with type context
            bool analyze_result = semantic_analyze_expression(analyzer, arg);

            // Restore previous expected type
            analyzer->expected_type = old_expected_type;

            if (!analyze_result) {
                return false;
            }

            // Get the argument's type
            TypeDescriptor *arg_type = semantic_get_expression_type(analyzer, arg);
            if (!arg_type) {
                continue;
            }

            // Check type compatibility
            if (!semantic_check_type_compatibility(analyzer, arg_type, expected_param_type)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, arg->location,
                                      "Argument %zu type mismatch: expected %s, got %s", i + 1,
                                      expected_param_type->name ? expected_param_type->name
                                                                : "unknown",
                                      arg_type->name ? arg_type->name : "unknown");
                type_descriptor_release(arg_type);
                return false;
            }

            type_descriptor_release(arg_type);
        }
    }

    return true;
}

// =============================================================================
// PREDECLARED FUNCTION VALIDATION
// =============================================================================

// Validate len() function call with special slice/array type checking
bool validate_len_function_call(SemanticAnalyzer *analyzer, ASTNode *call_expr) {
    if (!analyzer || !call_expr || call_expr->type != AST_CALL_EXPR) {
        return false;
    }

    ASTNodeList *args = call_expr->data.call_expr.args;
    size_t arg_count = args ? ast_node_list_size(args) : 0;

    // len() requires exactly one argument
    if (arg_count != 1) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, call_expr->location,
                              "len() requires exactly one argument, got %zu", arg_count);
        return false;
    }

    ASTNode *arg = ast_node_list_get(args, 0);
    if (!arg) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, call_expr->location,
                              "Failed to get len() argument");
        return false;
    }

    // Analyze the argument expression
    if (!semantic_analyze_expression(analyzer, arg)) {
        return false;
    }

    // Get the argument's type
    TypeDescriptor *arg_type = semantic_get_expression_type(analyzer, arg);
    if (!arg_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, arg->location,
                              "Cannot determine type of len() argument");
        return false;
    }

    // Validate that the argument is a slice or array
    bool is_valid_type = false;
    if (arg_type->category == TYPE_SLICE) {
        is_valid_type = true;
    } else if (arg_type->category == TYPE_ARRAY) {
        is_valid_type = true;
    }

    if (!is_valid_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, arg->location,
                              "len() can only be called on slices or arrays, got %s",
                              arg_type->name ? arg_type->name : "unknown");
        type_descriptor_release(arg_type);
        return false;
    }

    // Set the return type to usize
    TypeDescriptor *usize_type = &semantic_get_primitive_types_array()[PRIMITIVE_USIZE];
    call_expr->type_info = type_info_from_descriptor(usize_type);
    if (!call_expr->type_info) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, call_expr->location,
                              "Failed to create type info for len() return type");
        type_descriptor_release(arg_type);
        return false;
    }

    type_descriptor_release(arg_type);
    return true;
}

// Validate range() function call with overloaded signatures
bool validate_range_function_call(SemanticAnalyzer *analyzer, ASTNode *call_expr) {
    if (!analyzer || !call_expr || call_expr->type != AST_CALL_EXPR) {
        return false;
    }

    ASTNodeList *args = call_expr->data.call_expr.args;
    size_t arg_count = args ? ast_node_list_size(args) : 0;

    // range() accepts 1 or 2 arguments
    if (arg_count != 1 && arg_count != 2) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, call_expr->location,
                              "range() requires 1 or 2 arguments, got %zu", arg_count);
        return false;
    }

    // Analyze all arguments
    for (size_t i = 0; i < arg_count; i++) {
        ASTNode *arg = ast_node_list_get(args, i);
        if (!arg) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, call_expr->location,
                                  "Failed to get range() argument %zu", i + 1);
            return false;
        }

        // Analyze the argument expression
        if (!semantic_analyze_expression(analyzer, arg)) {
            return false;
        }

        // Get the argument's type
        TypeDescriptor *arg_type = semantic_get_expression_type(analyzer, arg);
        if (!arg_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, arg->location,
                                  "Cannot determine type of range() argument %zu", i + 1);
            return false;
        }

        // Validate that the argument is an integer type
        bool is_valid_type = is_integer_type(arg_type);

        if (!is_valid_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS, arg->location,
                                  "range() argument %zu must be an integer type, got %s", i + 1,
                                  arg_type->name ? arg_type->name : "unknown");
            type_descriptor_release(arg_type);
            return false;
        }

        type_descriptor_release(arg_type);
    }

    return true;
}

// =============================================================================
// ASSOCIATED FUNCTION CALL ANALYSIS
// =============================================================================

bool analyze_associated_function_call(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_ASSOCIATED_FUNC_CALL) {
        return false;
    }

    const char *struct_name = expr->data.associated_func_call.struct_name;
    const char *func_name = expr->data.associated_func_call.function_name;
    ASTNodeList *type_args = expr->data.associated_func_call.type_args;
    ASTNodeList *args = expr->data.associated_func_call.args;

    if (!struct_name || !func_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, expr->location,
                              "Invalid associated function call");
        return false;
    }

    // Look up the struct type
    SymbolEntry *struct_symbol = semantic_resolve_identifier(analyzer, struct_name);
    if (!struct_symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, expr->location,
                              "Undefined struct: %s", struct_name);
        return false;
    }

    if (struct_symbol->kind != SYMBOL_TYPE) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                              "%s is not a type", struct_name);
        return false;
    }

    TypeDescriptor *type = struct_symbol->type;
    if (!type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                              "%s has no type descriptor", struct_name);
        return false;
    }

    // Handle generic type arguments if provided
    TypeDescriptor *type_to_use = type;
    if (type_args && ast_node_list_size(type_args) > 0) {
        // This is a generic type instantiation (e.g., Vec<i32>::new())
        size_t type_arg_count = ast_node_list_size(type_args);

        // Resolve type arguments to TypeDescriptors
        TypeDescriptor **resolved_type_args = calloc(type_arg_count, sizeof(TypeDescriptor *));
        if (!resolved_type_args) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                  "Failed to allocate memory for type arguments");
            return false;
        }

        bool success = true;
        for (size_t i = 0; i < type_arg_count; i++) {
            ASTNode *type_arg_node = ast_node_list_get(type_args, i);
            if (!type_arg_node) {
                success = false;
                break;
            }

            // Analyze the type argument
            TypeDescriptor *type_arg = analyze_type_node(analyzer, type_arg_node);
            if (!type_arg) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED,
                                      type_arg_node->location,
                                      "Failed to resolve type argument %zu", i + 1);
                success = false;
                break;
            }

            resolved_type_args[i] = type_arg;
        }

        if (success) {
            // Create a generic instance with the resolved type arguments
            TypeDescriptor *generic_instance =
                type_descriptor_create_generic_instance(type, resolved_type_args, type_arg_count);

            if (generic_instance) {
                type_to_use = generic_instance;
            } else {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                                      "Failed to instantiate generic type %s", struct_name);
                success = false;
            }
        }

        // Clean up resolved type arguments (they're retained by generic instance if successful)
        for (size_t i = 0; i < type_arg_count; i++) {
            if (resolved_type_args[i]) {
                type_descriptor_release(resolved_type_args[i]);
            }
        }
        free(resolved_type_args);

        if (!success) {
            return false;
        }
    }

    SymbolEntry *method_symbol = NULL;

    // Use the potentially instantiated generic type
    if (type_to_use->category == TYPE_STRUCT) {
        // Look up the method in the struct's method table
        if (!type_to_use->data.struct_type.methods) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, expr->location,
                                  "Struct %s has no methods", struct_name);
            if (type_to_use != type) {
                type_descriptor_release(type_to_use);
            }
            return false;
        }

        method_symbol = symbol_table_lookup_safe(type_to_use->data.struct_type.methods, func_name);
        if (!method_symbol) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, expr->location,
                                  "Undefined method %s::%s", struct_name, func_name);
            if (type_to_use != type) {
                type_descriptor_release(type_to_use);
            }
            return false;
        }
    } else if (type_to_use->category == TYPE_ENUM) {
        // Error: :: is not allowed for enum variants
        semantic_report_error(
            analyzer, SEMANTIC_ERROR_INVALID_OPERATION, expr->location,
            "Use '.' instead of '::' for enum variants (e.g., %s.%s instead of %s::%s)",
            struct_name, func_name, struct_name, func_name);
        if (type_to_use != type) {
            type_descriptor_release(type_to_use);
        }
        return false;
    } else if (type_to_use->category == TYPE_GENERIC_INSTANCE) {
        // For generic instances, look up methods on the base type
        TypeDescriptor *base_type = type_to_use->data.generic_instance.base_type;
        if (base_type->category == TYPE_STRUCT) {
            if (!base_type->data.struct_type.methods) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, expr->location,
                                      "Struct %s has no methods", struct_name);
                if (type_to_use != type) {
                    type_descriptor_release(type_to_use);
                }
                return false;
            }
            method_symbol =
                symbol_table_lookup_safe(base_type->data.struct_type.methods, func_name);
            if (!method_symbol) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, expr->location,
                                      "Undefined method %s::%s", struct_name, func_name);
                if (type_to_use != type) {
                    type_descriptor_release(type_to_use);
                }
                return false;
            }
        } else if (base_type->category == TYPE_ENUM) {
            // Error: :: is not allowed for enum variants (even for generic enums)
            semantic_report_error(
                analyzer, SEMANTIC_ERROR_INVALID_OPERATION, expr->location,
                "Use '.' instead of '::' for enum variants (e.g., %s.%s instead of %s::%s)",
                struct_name, func_name, struct_name, func_name);
            if (type_to_use != type) {
                type_descriptor_release(type_to_use);
            }
            return false;
        }
    } else {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, expr->location,
                              "%s is not a struct or enum type", struct_name);
        if (type_to_use != type) {
            type_descriptor_release(type_to_use);
        }
        return false;
    }

    method_symbol->flags.is_used = true;

    // Analyze arguments first
    if (args) {
        for (size_t i = 0; i < ast_node_list_size(args); i++) {
            ASTNode *arg = ast_node_list_get(args, i);
            if (!semantic_analyze_expression(analyzer, arg)) {
                return false;
            }
        }
    }

    // For associated functions (not instance methods), validate arguments
    if (!method_symbol->is_instance_method) {
        if (!validate_function_arguments(analyzer, expr, method_symbol)) {
            return false;
        }
    }

    // Set the return type on the call expression
    if (method_symbol->type && method_symbol->type->category == TYPE_FUNCTION) {
        TypeDescriptor *return_type = method_symbol->type->data.function.return_type;
        if (return_type) {
            expr->type_info = type_info_from_descriptor(return_type);
            if (!expr->type_info) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                      "Failed to create type info for associated function call");
                if (type_to_use != type) {
                    type_descriptor_release(type_to_use);
                }
                return false;
            }
            // DEBUG: Print type info creation
            // fprintf(stderr, "DEBUG: Set type_info for associated function call %s::%s,
            // type=%s\n",
            //         struct_name, func_name, return_type->name ? return_type->name : "unknown");
        }
    }

    // Clean up the generic instance if we created one
    if (type_to_use != type) {
        type_descriptor_release(type_to_use);
    }

    return true;
}
