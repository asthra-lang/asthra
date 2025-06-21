/*
 * Struct Declaration Analysis Module
 * Core struct declaration analysis with generic type support
 * 
 * Part of semantic_structs.c split (578 lines -> 5 focused modules)
 * Handles struct declaration processing, field validation, and symbol table management
 */

#include "semantic_structs_common.h"

// ============================================================================
// STRUCT DECLARATION ANALYSIS
// ============================================================================

bool analyze_struct_declaration(SemanticAnalyzer *analyzer, ASTNode *struct_decl) {
    if (!analyzer || !struct_decl || struct_decl->type != AST_STRUCT_DECL) {
        return false;
    }
    
    const char *struct_name = struct_decl->data.struct_decl.name;
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;
    ASTNodeList *type_params = struct_decl->data.struct_decl.type_params; // Phase 3: Generic Structs
    
    if (!struct_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             struct_decl->location,
                             "Struct declaration missing name");
        return false;
    }
    
    // Check if struct name is already declared
    SymbolEntry *existing = symbol_table_lookup_local(analyzer->current_scope, struct_name);
    if (existing) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                             struct_decl->location,
                             "Struct '%s' is already declared", struct_name);
        return false;
    }
    
    // Phase 3: Validate type parameters if present (generic structs)
    if (type_params && !validate_struct_type_parameters(analyzer, type_params, struct_decl->location)) {
        return false;
    }
    
    // Create a new struct type descriptor
    size_t field_count = fields ? ast_node_list_size(fields) : 0;
    TypeDescriptor *struct_type = malloc(sizeof(TypeDescriptor));
    if (!struct_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE,
                             struct_decl->location,
                             "Memory allocation failed for struct type");
        return false;
    }
    
    // Initialize the struct type descriptor
    struct_type->category = TYPE_STRUCT;
    struct_type->flags.is_mutable = false;
    struct_type->flags.is_owned = false;
    struct_type->flags.is_borrowed = false;
    struct_type->flags.is_constant = false;
    struct_type->flags.is_volatile = false;
    struct_type->flags.is_atomic = false;
    struct_type->flags.is_ffi_compatible = true;
    struct_type->flags.reserved = 0;
    struct_type->size = 0;  // Will be calculated later
    struct_type->alignment = 0;  // Will be calculated later
    struct_type->name = strdup(struct_name);
    struct_type->data.struct_type.field_count = field_count;
    atomic_init(&struct_type->ref_count, 1);
    
    // Create a symbol table for the struct fields
    struct_type->data.struct_type.fields = symbol_table_create(field_count > 0 ? field_count : 4);
    if (!struct_type->data.struct_type.fields) {
        free((char*)struct_type->name);
        free(struct_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE,
                             struct_decl->location,
                             "Failed to create field symbol table for struct '%s'", struct_name);
        return false;
    }
    
    // Phase 3: If this is a generic struct, create method table for type parameter validation
    struct_type->data.struct_type.methods = symbol_table_create(4);
    if (!struct_type->data.struct_type.methods) {
        symbol_table_destroy(struct_type->data.struct_type.fields);
        free((char*)struct_type->name);
        free(struct_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE,
                             struct_decl->location,
                             "Failed to create method symbol table for struct '%s'", struct_name);
        return false;
    }
    
    // Phase 3: Enter a new scope for type parameter validation if generic
    bool is_generic = (type_params && ast_node_list_size(type_params) > 0);
    if (is_generic) {
        semantic_enter_scope(analyzer);
        
        // Add type parameters to scope for field type validation
        if (!register_struct_type_parameters(analyzer, type_params, struct_decl->location)) {
            semantic_exit_scope(analyzer);
            symbol_table_destroy(struct_type->data.struct_type.methods);
            symbol_table_destroy(struct_type->data.struct_type.fields);
            free((char*)struct_type->name);
            free(struct_type);
            return false;
        }
    }
    
    // Process each field
    if (fields && field_count > 0) {
        for (size_t i = 0; i < field_count; i++) {
            ASTNode *field = ast_node_list_get(fields, i);
            if (!field || field->type != AST_STRUCT_FIELD) {
                continue;
            }
            
            const char *field_name = field->data.struct_field.name;
            ASTNode *field_type_node = field->data.struct_field.type;
            
            if (!field_name) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     field->location,
                                     "Struct field missing name");
                continue;
            }
            
            // Check for duplicate field names
            if (symbol_table_lookup_local(struct_type->data.struct_type.fields, field_name)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                                     field->location,
                                     "Duplicate field '%s' in struct '%s'", field_name, struct_name);
                continue;
            }
            
            // Use analyze_type_node for proper semantic resolution
            TypeDescriptor *field_type = NULL;
            if (field_type_node) {
                field_type = analyze_type_node(analyzer, field_type_node);
                if (!field_type) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                         field_type_node->location,
                                         "Invalid type for field '%s' in struct '%s'", field_name, struct_name);
                    continue;
                }
                
                // Phase 3: For generic structs, validate that field types reference valid type parameters
                if (is_generic && !validate_field_type_parameters(analyzer, field_type_node, type_params, struct_decl->location)) {
                    type_descriptor_release(field_type);
                    continue;
                }
            } else {
                // Field missing type annotation - this should be an error
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     field->location,
                                     "Field '%s' in struct '%s' missing type annotation", field_name, struct_name);
                continue;
            }
            
            // Create a symbol entry for the field
            SymbolEntry *field_symbol = symbol_entry_create(field_name, SYMBOL_FIELD, field_type, field);
            if (field_symbol) {
                if (!symbol_table_insert_safe(struct_type->data.struct_type.fields, field_name, field_symbol)) {
                    symbol_entry_destroy(field_symbol);
                    if (field_type) type_descriptor_release(field_type);
                }
            }
        }
    }
    
    // Phase 3: Exit type parameter scope if generic
    if (is_generic) {
        semantic_exit_scope(analyzer);
    }
    
    // Create a symbol entry for the struct type
    SymbolEntry *struct_symbol = symbol_entry_create(struct_name, SYMBOL_TYPE, struct_type, struct_decl);
    if (!struct_symbol) {
        symbol_table_destroy(struct_type->data.struct_type.methods);
        symbol_table_destroy(struct_type->data.struct_type.fields);
        free((char*)struct_type->name);
        free(struct_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE,
                             struct_decl->location,
                             "Failed to create symbol entry for struct '%s'", struct_name);
        return false;
    }
    
    // Phase 3: Store type parameter information in the symbol entry for generic instantiation
    if (is_generic) {
        struct_symbol->is_generic = true;
        struct_symbol->type_param_count = ast_node_list_size(type_params);
        // Store the original AST node for later instantiation
        struct_symbol->generic_decl = struct_decl;
    }
    
    // Register the struct type in the symbol table
    if (!symbol_table_insert_safe(analyzer->current_scope, struct_name, struct_symbol)) {
        symbol_entry_destroy(struct_symbol);
        symbol_table_destroy(struct_type->data.struct_type.methods);
        symbol_table_destroy(struct_type->data.struct_type.fields);
        free((char*)struct_type->name);
        free(struct_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                             struct_decl->location,
                             "Failed to register struct '%s' in symbol table", struct_name);
        return false;
    }
    
    return true;
} 
