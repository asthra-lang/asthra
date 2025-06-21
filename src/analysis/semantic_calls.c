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
#include "semantic_core.h"
#include "semantic_utilities.h"
#include "semantic_types.h"
#include "semantic_type_helpers.h"
#include "semantic_type_creation.h"
#include "semantic_diagnostics.h"
#include "semantic_builtins.h"
#include "semantic_expression_utils.h"
#include "type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// CALL EXPRESSION ANALYSIS
// =============================================================================

bool analyze_call_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_CALL_EXPR) {
        return false;
    }
    
    ASTNode *function = expr->data.call_expr.function;
    if (!function) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             expr->location, "Invalid function call: missing function");
        return false;
    }
    
    // For simple identifier function calls, verify function exists
    if (function->type == AST_IDENTIFIER) {
        const char *func_name = function->data.identifier.name;
        SymbolEntry *func_symbol = semantic_resolve_identifier(analyzer, func_name);
        
        if (!func_symbol) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                 function->location, "Undefined function: %s", func_name);
            return false;
        }
        
        // Check if symbol is actually a function
        if (func_symbol->kind != SYMBOL_FUNCTION) {
            semantic_report_error_enhanced(analyzer, SEMANTIC_ERROR_NOT_CALLABLE,
                                 function->location,
                                 "cannot call non-function value",
                                 "remove the parentheses or use a function name",
                                 func_symbol->kind == SYMBOL_VARIABLE ? 
                                 "this is a variable, not a function" : 
                                 "this is a type or constant, not a function");
            return false;
        }
        
        func_symbol->flags.is_used = true;
        
        // Validate function arguments against parameter types
        if (!validate_function_arguments(analyzer, expr, func_symbol)) {
            return false;
        }
        
        return true;
    }
    // For enum variant calls (Option.Some(value)), the function is an enum variant
    else if (function->type == AST_ENUM_VARIANT) {
        const char *enum_name = function->data.enum_variant.enum_name;
        const char *variant_name = function->data.enum_variant.variant_name;
        
        if (!enum_name || !variant_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                 function->location, "Invalid enum variant call");
            return false;
        }
        
        // Find the enum symbol
        SymbolEntry *enum_symbol = semantic_resolve_identifier(analyzer, enum_name);
        if (!enum_symbol || enum_symbol->kind != SYMBOL_TYPE || 
            !enum_symbol->type || enum_symbol->type->category != TYPE_ENUM) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                 function->location, "Undefined enum type: %s", enum_name);
            return false;
        }
        
        // Check if variant exists
        SymbolEntry *variant_symbol = symbol_table_lookup_safe(
            enum_symbol->type->data.enum_type.variants, variant_name);
        if (!variant_symbol || variant_symbol->kind != SYMBOL_ENUM_VARIANT) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                 function->location, "Enum '%s' has no variant '%s'", enum_name, variant_name);
            return false;
        }
        
        ASTNodeList *args = expr->data.call_expr.args;
        size_t arg_count = args ? ast_node_list_size(args) : 0;
        
        if (arg_count > 1) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS,
                                 expr->location,
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
                type_descriptor_equals(expected_type->data.generic_instance.base_type, enum_symbol->type)) {
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
                TypeDescriptor *type_args[] = {
                    expected_type->data.result.ok_type,
                    expected_type->data.result.err_type
                };
                TypeDescriptor *generic_instance = type_descriptor_create_generic_instance(
                    enum_symbol->type, type_args, 2);
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
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL,
                                 expr->location,
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
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                 function->location, "Invalid method call");
            return false;
        }
        
        
        // First analyze the object expression
        if (!semantic_analyze_expression(analyzer, object)) {
            return false;
        }
        
        // Get the type of the object
        TypeDescriptor *object_type = semantic_get_expression_type(analyzer, object);
        if (!object_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED,
                                 object->location, "Cannot determine type of object in method call");
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
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS,
                                     expr->location,
                                     "Enum constructors support only single values. Use a tuple for multiple values.");
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
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                     expr->location,
                                     "Invalid enum constructor: missing enum or variant name");
                return false;
            }
            
            // Validate the enum and variant exist
            SymbolEntry *enum_symbol = semantic_resolve_identifier(analyzer, enum_name);
            if (!enum_symbol || enum_symbol->kind != SYMBOL_TYPE || 
                !enum_symbol->type || enum_symbol->type->category != TYPE_ENUM) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                     expr->location,
                                     "Undefined enum type: %s", enum_name);
                return false;
            }
            
            // Check if variant exists in the enum
            SymbolEntry *variant_symbol = symbol_table_lookup_safe(
                enum_symbol->type->data.enum_type.variants, method_name);
            if (!variant_symbol || variant_symbol->kind != SYMBOL_ENUM_VARIANT) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                     expr->location,
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
                    type_descriptor_equals(expected_type->data.generic_instance.base_type, enum_symbol->type)) {
                    // Use the expected type directly (e.g., Result<i32, string>)
                    enum_type_to_use = expected_type;
                } else if (expected_type && expected_type->category == TYPE_RESULT &&
                           enum_symbol->type && enum_symbol->type->name &&
                           strcmp(enum_symbol->type->name, "Result") == 0) {
                    // Handle case where expected type is TYPE_RESULT but enum is the Result enum
                    // Create a generic instance equivalent to the TYPE_RESULT
                    TypeDescriptor *type_args[] = {
                        expected_type->data.result.ok_type,
                        expected_type->data.result.err_type
                    };
                    TypeDescriptor *generic_instance = type_descriptor_create_generic_instance(
                        enum_symbol->type, type_args, 2);
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
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL,
                                     expr->location,
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
                method_symbol = symbol_table_lookup_safe(object_type->data.struct_type.methods, method_name);
            }
            
            if (!method_symbol) {
                type_descriptor_release(object_type);
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                     function->location,
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
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                             function->location, 
                             "Cannot call method '%s' on non-struct type", method_name);
        return false;
    }
    
    // For other function expressions, analyze them recursively
    return semantic_analyze_expression(analyzer, function);
}

// =============================================================================
// FUNCTION ARGUMENT VALIDATION
// =============================================================================

bool validate_function_arguments(SemanticAnalyzer *analyzer, ASTNode *call_expr, SymbolEntry *func_symbol) {
    if (!analyzer || !call_expr || !func_symbol || !func_symbol->type) {
        return false;
    }
    
    ASTNodeList *args = call_expr->data.call_expr.args;
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
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS,
                             call_expr->location,
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
            
            // Analyze the argument expression
            if (!semantic_analyze_expression(analyzer, arg)) {
                return false;
            }
            
            // Get the argument's type
            TypeDescriptor *arg_type = semantic_get_expression_type(analyzer, arg);
            if (!arg_type) {
                continue;
            }
            
            // Check type compatibility
            if (!semantic_check_type_compatibility(analyzer, arg_type, expected_param_type)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS,
                                     arg->location,
                                     "Argument %zu type mismatch: expected %s, got %s",
                                     i + 1,
                                     expected_param_type->name ? expected_param_type->name : "unknown",
                                     arg_type->name ? arg_type->name : "unknown");
                type_descriptor_release(arg_type);
                return false;
            }
            
            type_descriptor_release(arg_type);
        }
    }
    
    return true;
}

bool validate_method_arguments(SemanticAnalyzer *analyzer, ASTNode *call_expr, SymbolEntry *method_symbol) {
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
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS,
                             call_expr->location,
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
            TypeDescriptor *expected_param_type = method_type->data.function.param_types[i + param_offset];
            
            if (!arg || !expected_param_type) {
                continue;
            }
            
            // Analyze the argument expression
            if (!semantic_analyze_expression(analyzer, arg)) {
                return false;
            }
            
            // Get the argument's type
            TypeDescriptor *arg_type = semantic_get_expression_type(analyzer, arg);
            if (!arg_type) {
                continue;
            }
            
            // Check type compatibility
            if (!semantic_check_type_compatibility(analyzer, arg_type, expected_param_type)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ARGUMENTS,
                                     arg->location,
                                     "Argument %zu type mismatch: expected %s, got %s",
                                     i + 1,
                                     expected_param_type->name ? expected_param_type->name : "unknown",
                                     arg_type->name ? arg_type->name : "unknown");
                type_descriptor_release(arg_type);
                return false;
            }
            
            type_descriptor_release(arg_type);
        }
    }
    
    return true;
}