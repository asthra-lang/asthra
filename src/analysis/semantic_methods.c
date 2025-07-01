/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Method and Impl Block Analysis
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of impl blocks, methods, and visibility checking
 */

#include "semantic_methods.h"
#include "semantic_annotations.h" // Phase 3: For method annotation analysis
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "type_info_lifecycle.h"
#include "type_info_integration.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// IMPL BLOCK ANALYSIS
// =============================================================================

bool analyze_impl_block(SemanticAnalyzer *analyzer, ASTNode *impl_block) {
    if (!analyzer || !impl_block || impl_block->type != AST_IMPL_BLOCK) {
        return false;
    }

    const char *struct_name = impl_block->data.impl_block.struct_name;
    ASTNodeList *methods = impl_block->data.impl_block.methods;

    if (!struct_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, impl_block->location,
                              "Impl block missing struct name");
        return false;
    }

    // Verify the struct exists
    SymbolEntry *struct_symbol = symbol_table_lookup_safe(analyzer->current_scope, struct_name);
    if (!struct_symbol || struct_symbol->kind != SYMBOL_TYPE) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, impl_block->location,
                              "Impl block for unknown struct '%s'", struct_name);
        return false;
    }

    // Get the struct type descriptor
    TypeDescriptor *struct_type = struct_symbol->type;
    if (!struct_type || struct_type->category != TYPE_STRUCT) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, impl_block->location,
                              "Symbol '%s' is not a struct type", struct_name);
        return false;
    }

    // Create a method table if it doesn't exist
    if (!struct_type->data.struct_type.methods) {
        struct_type->data.struct_type.methods = symbol_table_create(8);
        if (!struct_type->data.struct_type.methods) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, impl_block->location,
                                  "Failed to create method table for struct '%s'", struct_name);
            return false;
        }
    }

    // Enter a new scope for the impl block (methods can reference Self)
    semantic_enter_scope(analyzer);

    // Add 'Self' type alias in this scope
    SymbolEntry *self_symbol = symbol_entry_create("Self", SYMBOL_TYPE, struct_type, NULL);
    if (self_symbol) {
        type_descriptor_retain(struct_type); // Retain reference for Self alias
        symbol_table_insert_safe(analyzer->current_scope, "Self", self_symbol);
    }

    // Analyze each method
    bool success = true;
    if (methods) {
        size_t method_count = ast_node_list_size(methods);
        for (size_t i = 0; i < method_count; i++) {
            ASTNode *method = ast_node_list_get(methods, i);
            if (method && method->type == AST_METHOD_DECL) {
                // Phase 3: Analyze method annotations before analyzing the method
                if (!analyze_declaration_annotations(analyzer, method)) {
                    success = false;
                    continue; // Continue analyzing other methods
                }

                if (!analyze_method_declaration(analyzer, method, struct_name)) {
                    success = false;
                    // Continue analyzing other methods
                }
            }
        }
    }

    // Exit the impl block scope
    semantic_exit_scope(analyzer);

    return success;
}

// =============================================================================
// METHOD DECLARATION ANALYSIS
// =============================================================================

bool analyze_method_declaration(SemanticAnalyzer *analyzer, ASTNode *method_decl,
                                const char *struct_name) {
    if (!analyzer || !method_decl || method_decl->type != AST_METHOD_DECL || !struct_name) {
        return false;
    }

    const char *method_name = method_decl->data.method_decl.name;
    ASTNodeList *params = method_decl->data.method_decl.params;
    // TODO: return_type will be used when return type validation is implemented
    ASTNode *body = method_decl->data.method_decl.body;
    bool is_instance_method = method_decl->data.method_decl.is_instance_method;
    VisibilityType visibility = method_decl->data.method_decl.visibility;

    if (!method_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, method_decl->location,
                              "Method declaration missing name");
        return false;
    }

    // Get the struct symbol and type
    SymbolEntry *struct_symbol = symbol_table_lookup_safe(analyzer->current_scope, struct_name);
    if (!struct_symbol || !struct_symbol->type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, method_decl->location,
                              "Unknown struct '%s' in method declaration", struct_name);
        return false;
    }

    TypeDescriptor *struct_type = struct_symbol->type;

    // Check for duplicate method names
    if (struct_type->data.struct_type.methods) {
        SymbolEntry *existing =
            symbol_table_lookup_local(struct_type->data.struct_type.methods, method_name);
        if (existing) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, method_decl->location,
                                  "Method '%s' is already defined for struct '%s'", method_name,
                                  struct_name);
            return false;
        }
    }

    // Validate self parameter if present
    if (is_instance_method) {
        if (!params || ast_node_list_size(params) == 0) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, method_decl->location,
                                  "Instance method '%s' must have self parameter", method_name);
            return false;
        }

        ASTNode *first_param = ast_node_list_get(params, 0);
        if (!first_param || first_param->type != AST_PARAM_DECL) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, method_decl->location,
                                  "Invalid first parameter in instance method '%s'", method_name);
            return false;
        }

        const char *param_name = first_param->data.param_decl.name;
        if (!param_name || strcmp(param_name, "self") != 0) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, method_decl->location,
                                  "First parameter of instance method '%s' must be 'self'",
                                  method_name);
            return false;
        }
    }

    // Create method type descriptor
    TypeDescriptor *method_type = malloc(sizeof(TypeDescriptor));
    if (!method_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, method_decl->location,
                              "Memory allocation failed for method type");
        return false;
    }

    method_type->category = TYPE_FUNCTION;
    method_type->name = strdup(method_name);
    method_type->flags.is_mutable = false;
    method_type->flags.is_owned = false;
    method_type->flags.is_borrowed = false;
    method_type->flags.is_constant = false;
    method_type->flags.is_volatile = false;
    method_type->flags.is_atomic = false;
    method_type->flags.is_ffi_compatible = true;
    method_type->flags.reserved = 0;
    method_type->size = sizeof(void *); // Function pointer size
    method_type->alignment = _Alignof(void *);
    atomic_init(&method_type->ref_count, 1);

    // Set up function type data
    method_type->data.function.param_count = params ? ast_node_list_size(params) : 0;
    method_type->data.function.return_type = NULL; // TODO: resolve return type
    method_type->data.function.param_types = NULL; // TODO: resolve parameter types

    // Create method symbol with visibility information
    SymbolEntry *method_symbol =
        symbol_entry_create(method_name, SYMBOL_METHOD, method_type, method_decl);
    if (!method_symbol) {
        free((char *)method_type->name);
        free(method_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, method_decl->location,
                              "Failed to create symbol entry for method '%s'", method_name);
        return false;
    }

    // Store visibility information (extend SymbolEntry if needed)
    method_symbol->visibility = visibility;
    method_symbol->is_instance_method = is_instance_method;

    // Add method to struct's method table
    if (!symbol_table_insert_safe(struct_type->data.struct_type.methods, method_name,
                                  method_symbol)) {
        symbol_entry_destroy(method_symbol);
        free((char *)method_type->name);
        free(method_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, method_decl->location,
                              "Failed to register method '%s' for struct '%s'", method_name,
                              struct_name);
        return false;
    }

    // Enter new scope for method body analysis
    semantic_enter_scope(analyzer);

    // Add parameters to scope
    if (params) {
        size_t param_count = ast_node_list_size(params);
        for (size_t i = 0; i < param_count; i++) {
            ASTNode *param = ast_node_list_get(params, i);
            if (param && param->type == AST_PARAM_DECL) {
                const char *param_name = param->data.param_decl.name;
                if (param_name) {
                    // Resolve parameter type using semantic resolution
                    TypeDescriptor *param_type = NULL;
                    if (i == 0 && is_instance_method && strcmp(param_name, "self") == 0) {
                        // For self parameter, use the struct type
                        param_type = struct_type;
                        type_descriptor_retain(param_type);
                    } else {
                        // Use analyze_type_node for proper semantic resolution
                        ASTNode *param_type_node = param->data.param_decl.type;
                        if (param_type_node) {
                            param_type = analyze_type_node(analyzer, param_type_node);
                            if (!param_type) {
                                semantic_report_error(
                                    analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                    param_type_node->location,
                                    "Invalid type for parameter '%s' in method '%s'", param_name,
                                    method_name);
                                continue;
                            }
                        } else {
                            semantic_report_error(
                                analyzer, SEMANTIC_ERROR_INVALID_OPERATION, param->location,
                                "Parameter '%s' in method '%s' missing type annotation", param_name,
                                method_name);
                            continue;
                        }
                    }

                    if (param_type) {
                        // Set TypeInfo on the parameter node for code generation
                        TypeInfo *param_type_info = type_info_from_descriptor(param_type);
                        if (param_type_info) {
                            ast_node_set_type_info(param, param_type_info);
                            type_info_release(param_type_info); // ast_node_set_type_info retains it
                        }
                        
                        SymbolEntry *param_symbol =
                            symbol_entry_create(param_name, SYMBOL_VARIABLE, param_type, param);
                        if (param_symbol) {
                            symbol_table_insert_safe(analyzer->current_scope, param_name,
                                                     param_symbol);
                        }
                    }
                }
            }
        }
    }

    // Analyze method body
    bool body_success = true;
    if (body) {
        body_success = semantic_analyze_statement(analyzer, body);
    }

    // TODO: Verify return type matches

    // Exit method scope
    semantic_exit_scope(analyzer);

    return body_success;
}

// =============================================================================
// VISIBILITY CHECKING
// =============================================================================

bool check_method_visibility(SemanticAnalyzer *analyzer, const char *struct_name,
                             const char *method_name, SourceLocation location) {
    if (!analyzer || !struct_name || !method_name) {
        return false;
    }

    // Get the struct symbol
    SymbolEntry *struct_symbol = symbol_table_lookup_safe(analyzer->current_scope, struct_name);
    if (!struct_symbol || !struct_symbol->type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, location,
                              "Unknown struct '%s' in method access", struct_name);
        return false;
    }

    TypeDescriptor *struct_type = struct_symbol->type;
    if (struct_type->category != TYPE_STRUCT || !struct_type->data.struct_type.methods) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, location,
                              "Symbol '%s' has no methods", struct_name);
        return false;
    }

    // Look up the method
    SymbolEntry *method_symbol =
        symbol_table_lookup_local(struct_type->data.struct_type.methods, method_name);
    if (!method_symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, location,
                              "Struct '%s' has no method '%s'", struct_name, method_name);
        return false;
    }

    // Check visibility
    if (method_symbol->visibility == VISIBILITY_PRIVATE) {
        // TODO: Check if we're in the same package/module
        // For now, we'll allow private access (should be restricted in real implementation)
    }

    return true;
}
