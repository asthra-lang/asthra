/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Resolution and Analysis
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Complex type node analysis and resolution
 */

#include "semantic_type_resolution.h"
#include "semantic_type_descriptors.h"
#include "semantic_type_creation.h"
#include "semantic_core.h"
#include "const_evaluator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// TYPE RESOLUTION UTILITIES
// =============================================================================

TypeDescriptor *analyze_type_node(SemanticAnalyzer *analyzer, ASTNode *type_node) {
    if (!analyzer || !type_node) {
        return NULL;
    }
    
    switch (type_node->type) {
        case AST_BASE_TYPE: {
            // Handle built-in primitive types
            const char *type_name = type_node->data.base_type.name;
            if (!type_name) return NULL;
            
            TypeDescriptor *builtin_type = semantic_get_builtin_type(analyzer, type_name);
            if (builtin_type) {
                type_descriptor_retain(builtin_type);
                return builtin_type;
            }
            
            // If not found in builtin array, try symbol table (builtin types are registered there)
            SymbolEntry *type_symbol = symbol_table_lookup_safe(analyzer->current_scope, type_name);
            if (type_symbol && type_symbol->kind == SYMBOL_TYPE && type_symbol->type) {
                type_descriptor_retain(type_symbol->type);
                return type_symbol->type;
            }
            
            // Type not found
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_TYPE,
                                 type_node->location,
                                 "Unknown type: %s", type_name);
            return NULL;
        }
        
        case AST_STRUCT_TYPE: {
            // Handle user-defined types - need to resolve actual type via symbol table
            const char *type_name = type_node->data.struct_type.name;
            if (!type_name) return NULL;
            
            
            // Look up the type in the symbol table
            SymbolEntry *type_symbol = symbol_table_lookup_safe(analyzer->current_scope, type_name);
            if (!type_symbol || (type_symbol->kind != SYMBOL_TYPE && type_symbol->kind != SYMBOL_TYPE_PARAMETER)) {
                // Type not found or not a type symbol - also check for type parameters
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_TYPE,
                                     type_node->location,
                                     "Unknown type: %s", type_name);
                return NULL;
            }
            
            // Handle type parameters (they might not have a type descriptor yet)
            if (type_symbol->kind == SYMBOL_TYPE_PARAMETER) {
                // For type parameters during declaration, we might not have a full type descriptor yet
                // Return the symbol's type if available
            }
            
            // ✅ SEMANTIC RESOLUTION: Return the actual type (struct or enum)
            // This correctly resolves AST_STRUCT_TYPE to the real type during semantic analysis
            TypeDescriptor *resolved_type = type_symbol->type;
            if (!resolved_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_TYPE,
                                     type_node->location,
                                     "Type '%s' has no type descriptor", type_name);
                return NULL;
            }
            type_descriptor_retain(resolved_type);
            
            // Phase 3: Handle generic struct types with type arguments
            if (type_node->data.struct_type.type_args) {
                ASTNodeList *type_args = type_node->data.struct_type.type_args;
                size_t arg_count = ast_node_list_size(type_args);
                
                // Validate that this is actually a generic type
                if (!type_symbol->is_generic) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                         type_node->location,
                                         "Type '%s' is not generic but type arguments provided", type_name);
                    type_descriptor_release(resolved_type);
                    return NULL;
                }
                
                // Validate type argument count matches parameter count
                if (arg_count != type_symbol->type_param_count) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                         type_node->location,
                                         "Type argument count mismatch for '%s': expected %zu, got %zu",
                                         type_name, type_symbol->type_param_count, arg_count);
                    type_descriptor_release(resolved_type);
                    return NULL;
                }
                
                // Collect all type arguments
                TypeDescriptor **arg_types = malloc(arg_count * sizeof(TypeDescriptor*));
                if (!arg_types) {
                    type_descriptor_release(resolved_type);
                    return NULL;
                }
                
                // For each type argument, recursively analyze it
                for (size_t i = 0; i < arg_count; i++) {
                    ASTNode *type_arg = ast_node_list_get(type_args, i);
                    if (type_arg) {
                        TypeDescriptor *arg_type = analyze_type_node(analyzer, type_arg);
                        if (!arg_type) {
                            // Clean up previously analyzed arguments
                            for (size_t j = 0; j < i; j++) {
                                if (arg_types[j]) {
                                    type_descriptor_release(arg_types[j]);
                                }
                            }
                            free(arg_types);
                            type_descriptor_release(resolved_type);
                            return NULL;
                        }
                        arg_types[i] = arg_type;
                    } else {
                        arg_types[i] = NULL;
                    }
                }
                
                // Create the generic instance type
                TypeDescriptor *instance_type = type_descriptor_create_generic_instance(
                    resolved_type, arg_types, arg_count);
                
                // Release the base type and arguments (instance retains them)
                type_descriptor_release(resolved_type);
                for (size_t i = 0; i < arg_count; i++) {
                    if (arg_types[i]) {
                        type_descriptor_release(arg_types[i]);
                    }
                }
                free(arg_types);
                
                if (!instance_type) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL,
                                         type_node->location,
                                         "Failed to create generic instance for '%s'", type_name);
                    return NULL;
                }
                
                return instance_type;
            } else if (type_symbol->is_generic) {
                // Generic type used without type arguments - error
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                     type_node->location,
                                     "Generic type '%s' requires type arguments", type_name);
                type_descriptor_release(resolved_type);
                return NULL;
            }
            
            // Update the AST node type to reflect the resolved type
            if (resolved_type->category == TYPE_ENUM) {
                // Convert AST_STRUCT_TYPE to AST_ENUM_TYPE for consistency
                type_node->type = AST_ENUM_TYPE;
                // Note: We keep the data as struct_type since it only has name field
                // which is compatible with enum_type.name
            }
            
            return resolved_type;
        }
        
        case AST_ENUM_TYPE: {
            // Handle enum types with type arguments
            // Note: The parser uses a heuristic where types with type arguments
            // are parsed as AST_ENUM_TYPE, but they could actually be generic structs
            const char *type_name = type_node->data.enum_type.name;
if (!type_name) return NULL;
            
            // Look up the type in the symbol table
            SymbolEntry *type_symbol = symbol_table_lookup_safe(analyzer->current_scope, type_name);
            if (!type_symbol || type_symbol->kind != SYMBOL_TYPE || !type_symbol->type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                     type_node->location,
                                     "Unknown type: %s", type_name);
                return NULL;
            }
            
            // Check if it's actually a struct or enum
            TypeCategory actual_category = type_symbol->type->category;
if (actual_category != TYPE_ENUM && actual_category != TYPE_STRUCT) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                     type_node->location,
                                     "Type '%s' cannot have type arguments", type_name);
                return NULL;
            }
            
            // For now, return the base type (could be enum or struct)
            // TODO: In the future, handle generic type instantiation for type arguments
            TypeDescriptor *base_type = type_symbol->type;
            type_descriptor_retain(base_type);
            
            // Handle type arguments if present
            if (type_node->data.enum_type.type_args) {
                ASTNodeList *type_args = type_node->data.enum_type.type_args;
                size_t arg_count = ast_node_list_size(type_args);
                
                // Validate that this is actually a generic type
                if (!type_symbol->is_generic) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                         type_node->location,
                                         "Type '%s' is not generic but type arguments provided", type_name);
                    type_descriptor_release(base_type);
                    return NULL;
                }
                
                // Validate type argument count matches parameter count
                if (arg_count != type_symbol->type_param_count) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                         type_node->location,
                                         "Type argument count mismatch for type '%s': expected %zu, got %zu",
                                         type_name, type_symbol->type_param_count, arg_count);
                    type_descriptor_release(base_type);
                    return NULL;
                }
                
                // Collect all type arguments
                TypeDescriptor **arg_types = malloc(arg_count * sizeof(TypeDescriptor*));
                if (!arg_types) {
                    type_descriptor_release(base_type);
                    return NULL;
                }
                
                // For each type argument, recursively analyze it
                for (size_t i = 0; i < arg_count; i++) {
                    ASTNode *type_arg = ast_node_list_get(type_args, i);
                    if (type_arg) {
TypeDescriptor *arg_type = analyze_type_node(analyzer, type_arg);
                        if (!arg_type) {
                            // Clean up previously analyzed arguments
                            for (size_t j = 0; j < i; j++) {
                                if (arg_types[j]) {
                                    type_descriptor_release(arg_types[j]);
                                }
                            }
                            free(arg_types);
                            type_descriptor_release(base_type);
                            return NULL;
                        }
                        arg_types[i] = arg_type;
                    } else {
                        arg_types[i] = NULL;
                    }
                }
                
                // Create the generic instance type
                TypeDescriptor *instance_type = type_descriptor_create_generic_instance(
                    base_type, arg_types, arg_count);
                
                // Release the base type and arguments (instance retains them)
                type_descriptor_release(base_type);
                for (size_t i = 0; i < arg_count; i++) {
                    if (arg_types[i]) {
                        type_descriptor_release(arg_types[i]);
                    }
                }
                free(arg_types);
                
                if (!instance_type) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL,
                                         type_node->location,
                                         "Failed to create generic instance for type '%s'", type_name);
                    return NULL;
                }
                
                return instance_type;
            } else if (type_symbol->is_generic) {
                // Generic type used without type arguments - error
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                     type_node->location,
                                     "Generic type '%s' requires type arguments", type_name);
                type_descriptor_release(base_type);
                return NULL;
            }
            
            return base_type;
        }
        
        case AST_SLICE_TYPE: {
            // Handle slice types: []ElementType
            ASTNode *element_type_node = type_node->data.slice_type.element_type;
            if (!element_type_node) return NULL;
            
            TypeDescriptor *element_type = analyze_type_node(analyzer, element_type_node);
            if (!element_type) return NULL;
            
            TypeDescriptor *slice_type = type_descriptor_create_slice(element_type);
            type_descriptor_release(element_type); // create_slice retains it
            
            return slice_type;
        }
        
        case AST_ARRAY_TYPE: {
            // Handle array types: [size]ElementType
            ASTNode *element_type_node = type_node->data.array_type.element_type;
            ASTNode *size_node = type_node->data.array_type.size;
            if (!element_type_node || !size_node) return NULL;
            
            TypeDescriptor *element_type = analyze_type_node(analyzer, element_type_node);
            if (!element_type) return NULL;
            
            // Analyze the size expression - must be a compile-time constant
            if (!semantic_analyze_expression(analyzer, size_node)) {
                type_descriptor_release(element_type);
                return NULL;
            }
            
            // Verify size is a constant expression
            if (!size_node->flags.is_constant_expr) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                     size_node->location,
                                     "Array size must be a compile-time constant");
                type_descriptor_release(element_type);
                return NULL;
            }
            
            // Evaluate the constant expression to get the size
            ConstValue *size_value = evaluate_const_expression(analyzer, size_node);
            if (!size_value) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                     size_node->location,
                                     "Failed to evaluate array size");
                type_descriptor_release(element_type);
                return NULL;
            }
            
            // Ensure it's an integer
            if (size_value->type != CONST_VALUE_INTEGER) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                                     size_node->location,
                                     "Array size must be an integer");
                const_value_destroy(size_value);
                type_descriptor_release(element_type);
                return NULL;
            }
            
            // Get the size value
            int64_t array_size = size_value->data.integer_value;
            const_value_destroy(size_value);
            
            // Validate size is positive
            if (array_size <= 0) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                     size_node->location,
                                     "Array size must be positive, got %lld", (long long)array_size);
                type_descriptor_release(element_type);
                return NULL;
            }
            
            // Create fixed-size array type
            TypeDescriptor *array_type = type_descriptor_create_array(element_type, (size_t)array_size);
            type_descriptor_release(element_type); // create_array retains it
            
            if (!array_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL,
                                     type_node->location,
                                     "Failed to create array type");
                return NULL;
            }
            
            return array_type;
        }
        
        case AST_PTR_TYPE: {
            // Handle pointer types: *mut Type / *const Type
            ASTNode *pointee_type_node = type_node->data.ptr_type.pointee_type;
            if (!pointee_type_node) return NULL;
            
            TypeDescriptor *pointee_type = analyze_type_node(analyzer, pointee_type_node);
            if (!pointee_type) return NULL;
            
            TypeDescriptor *ptr_type = type_descriptor_create_pointer(pointee_type);
            if (ptr_type) {
                ptr_type->flags.is_mutable = type_node->data.ptr_type.is_mutable;
            }
            type_descriptor_release(pointee_type); // create_pointer retains it
            
            return ptr_type;
        }
        
        case AST_RESULT_TYPE: {
            // Handle Result<T, E> types
            ASTNode *ok_type_node = type_node->data.result_type.ok_type;
            ASTNode *err_type_node = type_node->data.result_type.err_type;
            
            if (!ok_type_node || !err_type_node) return NULL;
            
            TypeDescriptor *ok_type = analyze_type_node(analyzer, ok_type_node);
            if (!ok_type) return NULL;
            
            TypeDescriptor *err_type = analyze_type_node(analyzer, err_type_node);
            if (!err_type) {
                type_descriptor_release(ok_type);
                return NULL;
            }
            
            TypeDescriptor *result_type = type_descriptor_create_result(ok_type, err_type);
            type_descriptor_release(ok_type);   // create_result retains it
            type_descriptor_release(err_type);  // create_result retains it
            
            return result_type;
        }
        
        case AST_TUPLE_TYPE: {
            // Handle tuple types: (T1, T2, ...)
            ASTNodeList *element_types = type_node->data.tuple_type.element_types;
            if (!element_types) return NULL;
            
            size_t element_count = ast_node_list_size(element_types);
            if (element_count < 2) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                     type_node->location,
                                     "Tuple types must have at least 2 elements");
                return NULL;
            }
            
            // Analyze all element types
            TypeDescriptor **types = malloc(element_count * sizeof(TypeDescriptor*));
            if (!types) return NULL;
            
            for (size_t i = 0; i < element_count; i++) {
                ASTNode *elem_type_node = ast_node_list_get(element_types, i);
                if (!elem_type_node) {
                    // Clean up previously analyzed types
                    for (size_t j = 0; j < i; j++) {
                        type_descriptor_release(types[j]);
                    }
                    free(types);
                    return NULL;
                }
                
                TypeDescriptor *elem_type = analyze_type_node(analyzer, elem_type_node);
                if (!elem_type) {
                    // Clean up previously analyzed types
                    for (size_t j = 0; j < i; j++) {
                        type_descriptor_release(types[j]);
                    }
                    free(types);
                    return NULL;
                }
                
                types[i] = elem_type;
            }
            
            // Create tuple type descriptor
            TypeDescriptor *tuple_type = type_descriptor_create_tuple(types, element_count);
            
            // Release element types (create_tuple retains them)
            for (size_t i = 0; i < element_count; i++) {
                type_descriptor_release(types[i]);
            }
            free(types);
            
            return tuple_type;
        }
        
        case AST_IDENTIFIER: {
            // Handle type parameters and type aliases
            const char *type_name = type_node->data.identifier.name;
            if (!type_name) return NULL;
            
            
            // First check if it's a builtin type
            TypeDescriptor *builtin_type = semantic_get_builtin_type(analyzer, type_name);
            if (builtin_type) {
                type_descriptor_retain(builtin_type);
                return builtin_type;
            }
            
            // Look up the identifier in the symbol table
            SymbolEntry *symbol = symbol_table_lookup_safe(analyzer->current_scope, type_name);
            if (!symbol) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_TYPE,
                                     type_node->location,
                                     "Unknown type: %s", type_name);
                return NULL;
            }
            
            // Check if it's a type symbol or type parameter
            if (symbol->kind == SYMBOL_TYPE || symbol->kind == SYMBOL_TYPE_PARAMETER) {
                if (!symbol->type) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                         type_node->location,
                                         "Type '%s' has no associated type descriptor", type_name);
                    return NULL;
                }
                type_descriptor_retain(symbol->type);
                return symbol->type;
            } else {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                     type_node->location,
                                     "'%s' is not a type", type_name);
                return NULL;
            }
        }
        
        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                 type_node->location,
                                 "Unsupported type node: %d", type_node->type);
            return NULL;
    }
}