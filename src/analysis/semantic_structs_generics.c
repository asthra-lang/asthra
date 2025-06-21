/*
 * Generic Struct Type Parameter Module
 * Generic struct validation and type parameter management
 * 
 * Part of semantic_structs.c split (578 lines -> 5 focused modules)
 * Handles generic struct type parameter validation, registration, and field type checking
 */

#include "semantic_structs_common.h"

// ============================================================================
// GENERIC STRUCT TYPE PARAMETER VALIDATION
// ============================================================================

/**
 * Validate type parameters for generic structs
 * Similar to validate_enum_type_parameters but for structs
 */
bool validate_struct_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params, SourceLocation location) {
    if (!analyzer || !type_params) {
        return true; // No type parameters to validate
    }
    
    size_t param_count = ast_node_list_size(type_params);
    if (param_count == 0) {
        return true;
    }
    
    // Check for duplicate type parameter names
    for (size_t i = 0; i < param_count; i++) {
        ASTNode *param_i = ast_node_list_get(type_params, i);
        if (!param_i || param_i->type != AST_IDENTIFIER) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                 param_i ? param_i->location : location,
                                 "Type parameter must be an identifier");
            return false;
        }
        
        const char *name_i = param_i->data.identifier.name;
        if (!name_i) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                 param_i->location,
                                 "Type parameter missing name");
            return false;
        }
        
        // Check for duplicates
        for (size_t j = i + 1; j < param_count; j++) {
            ASTNode *param_j = ast_node_list_get(type_params, j);
            if (!param_j || param_j->type != AST_IDENTIFIER) {
                continue;
            }
            
            const char *name_j = param_j->data.identifier.name;
            if (name_j && strcmp(name_i, name_j) == 0) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                                     param_j->location,
                                     "Duplicate type parameter '%s'", name_i);
                return false;
            }
        }
        
        // Check that type parameter name doesn't conflict with built-in types
        if (is_builtin_type_name(name_i)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                 param_i->location,
                                 "Type parameter '%s' conflicts with built-in type", name_i);
            return false;
        }
    }
    
    return true;
}

/**
 * Register type parameters in the current scope for field validation
 */
bool register_struct_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params, SourceLocation location) {
    if (!analyzer || !type_params) {
        return true;
    }
    
    size_t param_count = ast_node_list_size(type_params);
    for (size_t i = 0; i < param_count; i++) {
        ASTNode *param = ast_node_list_get(type_params, i);
        if (!param || param->type != AST_IDENTIFIER) {
            continue;
        }
        
        const char *param_name = param->data.identifier.name;
        if (!param_name) {
            continue;
        }
        
        // Create a type descriptor for the type parameter
        TypeDescriptor *param_type = malloc(sizeof(TypeDescriptor));
        if (!param_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION,
                                 location,
                                 "Failed to allocate type parameter descriptor");
            return false;
        }
        
        param_type->category = TYPE_UNKNOWN; // Type parameters are resolved during instantiation
        param_type->name = strdup(param_name);
        param_type->flags.is_mutable = false;
        param_type->flags.is_owned = false;
        param_type->flags.is_borrowed = false;
        param_type->flags.is_constant = false;
        param_type->flags.is_volatile = false;
        param_type->flags.is_atomic = false;
        param_type->flags.is_ffi_compatible = true;
        param_type->flags.reserved = 0;
        param_type->size = 0;
        param_type->alignment = 0;
        atomic_init(&param_type->ref_count, 1);
        
        // Create symbol entry for the type parameter
        SymbolEntry *param_symbol = symbol_entry_create(param_name, SYMBOL_TYPE_PARAMETER, param_type, param);
        if (!param_symbol) {
            free((char*)param_type->name);
            free(param_type);
            semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION,
                                 location,
                                 "Failed to create type parameter symbol");
            return false;
        }
        
        // Add to current scope
        if (!symbol_table_insert_safe(analyzer->current_scope, param_name, param_symbol)) {
            symbol_entry_destroy(param_symbol);
            semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                                 param->location,
                                 "Failed to register type parameter '%s'", param_name);
            return false;
        }
    }
    
    return true;
}

/**
 * Validate that field types in generic structs properly reference type parameters
 */
bool validate_field_type_parameters(SemanticAnalyzer *analyzer, ASTNode *field_type_node, ASTNodeList *type_params, SourceLocation location) {
    if (!analyzer || !field_type_node || !type_params) {
        return true;
    }
    
    // This is a simplified validation - in a full implementation, we would:
    // 1. Walk the type AST to find all referenced type names
    // 2. Check that any type parameter references are valid
    // 3. Ensure type constraints are satisfied
    
    // For now, just return true since the type has already been analyzed
    // in analyze_struct_declaration and we're not fully implementing generics yet
    return true;
} 
