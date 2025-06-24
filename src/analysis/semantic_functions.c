/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Function Declaration Analysis
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Analysis of function declarations
 */

#include "semantic_functions.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// FUNCTION DECLARATION ANALYSIS
// =============================================================================

bool analyze_function_declaration(SemanticAnalyzer *analyzer, ASTNode *func_decl) {
    if (!analyzer || !func_decl || func_decl->type != AST_FUNCTION_DECL) {
        return false;
    }
    
    const char *func_name = func_decl->data.function_decl.name;
    ASTNodeList *params = func_decl->data.function_decl.params;
    ASTNode *return_type_node = func_decl->data.function_decl.return_type;
    ASTNode *body = func_decl->data.function_decl.body;
    
    if (!func_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             func_decl->location, "Function declaration missing name");
        return false;
    }
    
    // Check for duplicate function declaration in current scope
    SymbolEntry *existing = symbol_table_lookup_safe(analyzer->current_scope, func_name);
    if (existing) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                             func_decl->location, 
                             "Duplicate function declaration: %s", func_name);
        return false;
    }
    
    // Analyze return type
    TypeDescriptor *return_type = NULL;
    if (return_type_node) {
        return_type = analyze_type_node(analyzer, return_type_node);
        if (!return_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                 return_type_node->location, 
                                 "Invalid return type for function '%s'", func_name);
            return false;
        }
    } else {
        // Default to void if no return type specified
        return_type = semantic_get_builtin_type(analyzer, "void");
        if (return_type) type_descriptor_retain(return_type);
    }
    
    // Create function type descriptor
    TypeDescriptor *func_type = type_descriptor_create_function();
    
    if (!func_type) {
        if (return_type) type_descriptor_release(return_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE,
                             func_decl->location, 
                             "Failed to create function type for '%s'", func_name);
        return false;
    }
    
    // Set function name and return type
    func_type->name = strdup(func_name);
    func_type->data.function.return_type = return_type;
    
    // Analyze parameters
    size_t param_count = params ? ast_node_list_size(params) : 0;
    func_type->data.function.param_count = param_count;
    
    if (param_count > 0) {
        func_type->data.function.param_types = calloc(param_count, sizeof(TypeDescriptor*));
        if (!func_type->data.function.param_types) {
            type_descriptor_release(func_type);
            semantic_report_error(analyzer, SEMANTIC_ERROR_NONE,
                                 func_decl->location, 
                                 "Memory allocation failed for function parameters");
            return false;
        }
        
        for (size_t i = 0; i < param_count; i++) {
            ASTNode *param = ast_node_list_get(params, i);
            if (!param || param->type != AST_PARAM_DECL) {
                continue;
            }
            
            ASTNode *param_type_node = param->data.param_decl.type;
            if (!param_type_node) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     param->location, 
                                     "Parameter missing type annotation in function '%s'", func_name);
                type_descriptor_release(func_type);
                return false;
            }
            
            TypeDescriptor *param_type = analyze_type_node(analyzer, param_type_node);
            if (!param_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                     param_type_node->location, 
                                     "Invalid parameter type in function '%s'", func_name);
                type_descriptor_release(func_type);
                return false;
            }
            
            func_type->data.function.param_types[i] = param_type;
        }
    }
    
    // Create function symbol and add to symbol table
    SymbolEntry *func_symbol = NULL;
    if (!semantic_declare_symbol(analyzer, func_name, SYMBOL_FUNCTION, func_type, func_decl)) {
        type_descriptor_release(func_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             func_decl->location, 
                             "Failed to declare function: %s", func_name);
        return false;
    }
    
    // Get the function symbol that was just created
    func_symbol = semantic_resolve_identifier(analyzer, func_name);
    if (!func_symbol) {
        type_descriptor_release(func_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             func_decl->location, 
                             "Failed to resolve function symbol: %s", func_name);
        return false;
    }
    
    // Set the current function context
    SymbolEntry *previous_function = analyzer->current_function;
    analyzer->current_function = func_symbol;
    
    // Enter function scope for parameter and body analysis
    semantic_enter_scope(analyzer);
    
    bool success = true;
    
    // Add function parameters to the function scope
    if (params && param_count > 0) {
        for (size_t i = 0; i < param_count; i++) {
            ASTNode *param = ast_node_list_get(params, i);
            if (!param || param->type != AST_PARAM_DECL) {
                continue;
            }
            
            const char *param_name = param->data.param_decl.name;
            if (!param_name) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     param->location, 
                                     "Parameter missing name in function '%s'", func_name);
                success = false;
                continue;
            }
            
            // Get the parameter type (already validated above)
            TypeDescriptor *param_type = func_type->data.function.param_types[i];
            if (param_type) {
                type_descriptor_retain(param_type);
                if (!semantic_declare_symbol(analyzer, param_name, SYMBOL_VARIABLE, param_type, param)) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                                         param->location, 
                                         "Duplicate parameter name '%s' in function '%s'", param_name, func_name);
                    type_descriptor_release(param_type);
                    success = false;
                }
            }
        }
    }
    
    // Analyze the function body
    if (body && success) {
if (!semantic_analyze_statement(analyzer, body)) {
            success = false;
        }
    }
    
    // Exit function scope
    semantic_exit_scope(analyzer);
    
    // Restore previous function context
    analyzer->current_function = previous_function;
    
    // Set TypeInfo on the AST node for code generation
    if (success && func_type) {
        TypeInfo *type_info = type_info_from_descriptor(func_type);
        if (type_info) {
            ast_node_set_type_info(func_decl, type_info);
            type_info_release(type_info); // ast_node_set_type_info retains it
        }
    }
    
    return success;
}

// =============================================================================
// EXTERN DECLARATION ANALYSIS
// =============================================================================

bool analyze_extern_declaration(SemanticAnalyzer *analyzer, ASTNode *extern_decl) {
    (void)analyzer;
    (void)extern_decl;
    // TODO: Implement extern declaration analysis
    return true;
} 
