/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Enum Declaration Analysis Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of enum declarations and variant management
 */

#include "semantic_enums.h"
#include "../parser/ast.h"
#include "semantic_errors.h"
#include "semantic_scopes.h"
#include "semantic_symbols.h"
#include "semantic_symbols_entries.h"
#include "semantic_types.h"
#include "semantic_types_resolution.h"
#include "semantic_utilities.h"
#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// ENUM DECLARATION ANALYSIS
// =============================================================================

bool analyze_enum_declaration(SemanticAnalyzer *analyzer, ASTNode *enum_decl) {
    if (!analyzer || !enum_decl || enum_decl->type != AST_ENUM_DECL) {
        return false;
    }

    const char *enum_name = enum_decl->data.enum_decl.name;
    if (!enum_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_DECLARATION, enum_decl->location,
                              "Enum declaration missing name");
        return false;
    }

    // Check if enum name already exists in current scope
    SymbolEntry *existing = symbol_table_lookup_safe(analyzer->current_scope, enum_name);
    if (existing) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, enum_decl->location,
                              "Enum '%s' already declared", enum_name);
        return false;
    }

    // Validate type parameters if present
    if (enum_decl->data.enum_decl.type_params) {
        if (!validate_enum_type_parameters(analyzer, enum_decl->data.enum_decl.type_params,
                                           enum_decl->location)) {
            return false;
        }
    }

    // Save the current scope for registering the enum itself
    SymbolTable *enum_scope = analyzer->current_scope;

    // Enter a new scope for type parameter validation if generic
    bool is_generic = (enum_decl->data.enum_decl.type_params &&
                       ast_node_list_size(enum_decl->data.enum_decl.type_params) > 0);
    if (is_generic) {
        semantic_enter_scope(analyzer);

        // Register type parameters in scope for variant type validation
        if (!register_enum_type_parameters(analyzer, enum_decl->data.enum_decl.type_params,
                                           enum_decl->location)) {
            semantic_exit_scope(analyzer);
            return false;
        }
    }

    // Check for duplicate variants
    if (enum_decl->data.enum_decl.variants) {
        if (!check_duplicate_variants(analyzer, enum_decl->data.enum_decl.variants,
                                      enum_decl->location)) {
            return false;
        }
    }

    // Create enum type descriptor
    TypeDescriptor *enum_type = malloc(sizeof(TypeDescriptor));
    if (!enum_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, enum_decl->location,
                              "Failed to create enum type descriptor");
        return false;
    }

    // Initialize the enum type descriptor
    enum_type->category = TYPE_ENUM;
    enum_type->flags.is_mutable = false;
    enum_type->flags.is_owned = false;
    enum_type->flags.is_borrowed = false;
    enum_type->flags.is_constant = false;
    enum_type->flags.is_volatile = false;
    enum_type->flags.is_atomic = false;
    enum_type->flags.is_ffi_compatible = true;
    enum_type->flags.reserved = 0;
    enum_type->size = sizeof(int); // C-style enum size
    enum_type->alignment = _Alignof(int);
    enum_type->name = strdup(enum_name);
    enum_type->data.enum_type.variant_count =
        enum_decl->data.enum_decl.variants ? ast_node_list_size(enum_decl->data.enum_decl.variants)
                                           : 0;
    atomic_init(&enum_type->ref_count, 1);

    // Create a symbol table for enum variants
    enum_type->data.enum_type.variants = symbol_table_create(
        enum_type->data.enum_type.variant_count > 0 ? enum_type->data.enum_type.variant_count : 4);
    if (!enum_type->data.enum_type.variants) {
        free((char *)enum_type->name);
        free(enum_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, enum_decl->location,
                              "Failed to create variant symbol table for enum '%s'", enum_name);
        return false;
    }

    // Create enum symbol entry
    SymbolEntry *enum_symbol = symbol_entry_create(enum_name, SYMBOL_TYPE, enum_type, enum_decl);
    if (!enum_symbol) {
        free((char *)enum_type->name);
        free(enum_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, enum_decl->location,
                              "Failed to create enum symbol");
        return false;
    }

    // Mark the enum as generic if it has type parameters
    if (is_generic) {
        enum_symbol->is_generic = true;
        enum_symbol->type_param_count = ast_node_list_size(enum_decl->data.enum_decl.type_params);
        enum_symbol->generic_decl = enum_decl;
    }

    // Add enum to symbol table (use the outer scope, not the type parameter scope)
    if (!symbol_table_insert_safe(enum_scope, enum_name, enum_symbol)) {
        symbol_entry_destroy(enum_symbol);
        free((char *)enum_type->name);
        free(enum_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_SYMBOL_TABLE, enum_decl->location,
                              "Failed to insert enum '%s' into symbol table", enum_name);
        return false;
    }

    // Analyze each variant
    if (enum_decl->data.enum_decl.variants) {
        ASTNodeList *variants = enum_decl->data.enum_decl.variants;
        int64_t next_auto_value = 0;

        for (size_t i = 0; i < ast_node_list_size(variants); i++) {
            ASTNode *variant = ast_node_list_get(variants, i);
            if (!analyze_enum_variant_declaration(analyzer, variant, enum_name, enum_type)) {
                if (is_generic) {
                    semantic_exit_scope(analyzer);
                }
                return false;
            }

            // Track automatic value assignment for C-style enums
            if (variant->type == AST_ENUM_VARIANT_DECL) {
                if (variant->data.enum_variant_decl.value) {
                    // Explicit value provided
                    int64_t explicit_value;
                    if (!validate_enum_variant_value(
                            analyzer, variant->data.enum_variant_decl.value, &explicit_value)) {
                        return false;
                    }
                    next_auto_value = explicit_value + 1;
                } else {
                    // Use automatic value
                    next_auto_value++;
                }
                (void)next_auto_value; // Suppress unused variable warning
            }
        }
    }

    // Mark the enum declaration as analyzed
    enum_decl->flags.is_validated = true;

    // Exit scope if we entered one for generic type parameters
    if (is_generic) {
        semantic_exit_scope(analyzer);
    }

    return true;
}

bool analyze_enum_variant_declaration(SemanticAnalyzer *analyzer, ASTNode *variant_decl,
                                      const char *enum_name, TypeDescriptor *enum_type) {
    if (!analyzer || !variant_decl || !enum_name || !enum_type) {
        return false;
    }

    if (variant_decl->type != AST_ENUM_VARIANT_DECL) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_DECLARATION, variant_decl->location,
                              "Expected enum variant declaration");
        return false;
    }

    const char *variant_name = variant_decl->data.enum_variant_decl.name;
    if (!variant_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_DECLARATION, variant_decl->location,
                              "Enum variant missing name");
        return false;
    }

    // Create qualified variant name: EnumName.VariantName
    size_t qualified_name_len = strlen(enum_name) + strlen(variant_name) + 2; // +2 for '.' and '\0'
    char *qualified_name = malloc(qualified_name_len);
    if (!qualified_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, variant_decl->location,
                              "Failed to allocate memory for variant name");
        return false;
    }

    snprintf(qualified_name, qualified_name_len, "%s.%s", enum_name, variant_name);

    // Check if variant name already exists in enum's variant table
    SymbolEntry *existing =
        symbol_table_lookup_local(enum_type->data.enum_type.variants, variant_name);
    if (existing) {
        free(qualified_name);
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, variant_decl->location,
                              "Variant '%s' already declared in enum '%s'", variant_name,
                              enum_name);
        return false;
    }

    // Create variant type descriptor
    TypeDescriptor *variant_type = malloc(sizeof(TypeDescriptor));
    if (!variant_type) {
        free(qualified_name);
        semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, variant_decl->location,
                              "Failed to create variant type descriptor");
        return false;
    }

    // Initialize variant type
    variant_type->category = TYPE_PRIMITIVE; // Variants are primitive enum values
    variant_type->flags.is_mutable = false;
    variant_type->flags.is_owned = false;
    variant_type->flags.is_borrowed = false;
    variant_type->flags.is_constant = true;
    variant_type->flags.is_volatile = false;
    variant_type->flags.is_atomic = false;
    variant_type->flags.is_ffi_compatible = true;
    variant_type->flags.reserved = 0;
    variant_type->size = sizeof(int);
    variant_type->alignment = _Alignof(int);
    variant_type->name = strdup(qualified_name);
    variant_type->data.primitive.primitive_kind = PRIMITIVE_I32; // Enum variants are i32 by default
    atomic_init(&variant_type->ref_count, 1);

    // Create variant symbol
    SymbolEntry *variant_symbol =
        symbol_entry_create(variant_name, SYMBOL_ENUM_VARIANT, variant_type, variant_decl);
    if (!variant_symbol) {
        free(qualified_name);
        free((void *)variant_type->name);
        free(variant_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, variant_decl->location,
                              "Failed to create variant symbol");
        return false;
    }

    // Validate associated type if present
    if (variant_decl->data.enum_variant_decl.associated_type) {
        // Analyze the associated type
        TypeDescriptor *associated_type =
            analyze_type_node(analyzer, variant_decl->data.enum_variant_decl.associated_type);
        if (!associated_type) {
            symbol_entry_destroy(variant_symbol);
            free(qualified_name);
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                  variant_decl->data.enum_variant_decl.associated_type->location,
                                  "Invalid associated type for variant '%s'", variant_name);
            return false;
        }
        // TODO: Store associated type information in variant symbol if needed
        type_descriptor_release(associated_type);
    }

    // Validate explicit value if present
    if (variant_decl->data.enum_variant_decl.value) {
        int64_t value;
        if (!validate_enum_variant_value(analyzer, variant_decl->data.enum_variant_decl.value,
                                         &value)) {
            symbol_entry_destroy(variant_symbol);
            free(qualified_name);
            return false;
        }
        // Store the explicit value in the variant type for now
        // TODO: Add explicit_value field to enum variant type descriptor if needed
    }

    // Add variant to enum's variant symbol table
    if (!symbol_table_insert_safe(enum_type->data.enum_type.variants, variant_name,
                                  variant_symbol)) {
        symbol_entry_destroy(variant_symbol);
        free(qualified_name);
        semantic_report_error(analyzer, SEMANTIC_ERROR_SYMBOL_TABLE, variant_decl->location,
                              "Failed to insert variant '%s' into enum variant table",
                              variant_name);
        return false;
    }

    // Also add qualified variant to global scope for qualified access (EnumName.VariantName)
    // Create a copy of the symbol for the global scope to avoid linked list corruption
    SymbolEntry *qualified_symbol = symbol_entry_copy(variant_symbol);
    if (qualified_symbol) {
        free(qualified_symbol->name);                    // Free the copied name
        qualified_symbol->name = strdup(qualified_name); // Use the qualified name
        if (!symbol_table_insert_safe(analyzer->current_scope, qualified_name, qualified_symbol)) {
            // Not a fatal error - variant is still accessible through unqualified name within enum
            // context This mainly affects cross-module qualified access
            symbol_entry_destroy(qualified_symbol);
        }
    }

    // Mark the variant as analyzed
    variant_decl->flags.is_validated = true;

    free(qualified_name);
    return true;
}

bool check_variant_visibility(SemanticAnalyzer *analyzer, const char *enum_name,
                              const char *variant_name, SourceLocation location) {
    if (!analyzer || !enum_name || !variant_name) {
        return false;
    }

    // Create qualified variant name
    size_t qualified_name_len = strlen(enum_name) + strlen(variant_name) + 2;
    char *qualified_name = malloc(qualified_name_len);
    if (!qualified_name) {
        return false;
    }

    snprintf(qualified_name, qualified_name_len, "%s.%s", enum_name, variant_name);

    // Look up the variant symbol
    SymbolEntry *variant_symbol = symbol_table_lookup_safe(analyzer->current_scope, qualified_name);
    free(qualified_name);

    if (!variant_symbol || variant_symbol->kind != SYMBOL_ENUM_VARIANT) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, location,
                              "Variant '%s' not found in enum '%s'", variant_name, enum_name);
        return false;
    }

    // Check visibility (simplified - always allow for now)
    // TODO: Implement proper visibility checking based on module system
    return true;
}

bool validate_enum_variant_value(SemanticAnalyzer *analyzer, ASTNode *value_expr,
                                 int64_t *out_value) {
    if (!analyzer || !value_expr || !out_value) {
        return false;
    }

    // For now, only support integer literals as explicit values
    if (value_expr->type == AST_INTEGER_LITERAL) {
        *out_value = value_expr->data.integer_literal.value;
        return true;
    }

    // TODO: Support constant expressions and named constants
    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, value_expr->location,
                          "Enum variant values must be integer literals");
    return false;
}

bool check_duplicate_variants(SemanticAnalyzer *analyzer, ASTNodeList *variants,
                              SourceLocation enum_location) {
    if (!analyzer || !variants) {
        return true; // No variants to check
    }

    (void)enum_location; // Suppress unused parameter warning

    size_t variant_count = ast_node_list_size(variants);

    // Check for duplicate variant names
    for (size_t i = 0; i < variant_count; i++) {
        ASTNode *variant_i = ast_node_list_get(variants, i);
        if (variant_i->type != AST_ENUM_VARIANT_DECL) {
            continue;
        }

        const char *name_i = variant_i->data.enum_variant_decl.name;
        if (!name_i) {
            continue;
        }

        for (size_t j = i + 1; j < variant_count; j++) {
            ASTNode *variant_j = ast_node_list_get(variants, j);
            if (variant_j->type != AST_ENUM_VARIANT_DECL) {
                continue;
            }

            const char *name_j = variant_j->data.enum_variant_decl.name;
            if (!name_j) {
                continue;
            }

            if (strcmp(name_i, name_j) == 0) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                                      variant_j->location, "Duplicate variant name '%s' in enum",
                                      name_i);
                return false;
            }
        }
    }

    return true;
}

bool validate_enum_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params,
                                   SourceLocation location) {
    if (!analyzer || !type_params) {
        return true; // No type parameters to validate
    }

    (void)location; // Suppress unused parameter warning

    size_t param_count = ast_node_list_size(type_params);

    // Check for duplicate type parameter names
    for (size_t i = 0; i < param_count; i++) {
        ASTNode *param_i = ast_node_list_get(type_params, i);
        if (param_i->type != AST_IDENTIFIER) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, param_i->location,
                                  "Type parameter must be an identifier");
            return false;
        }

        const char *name_i = param_i->data.identifier.name;
        if (!name_i) {
            continue;
        }

        for (size_t j = i + 1; j < param_count; j++) {
            ASTNode *param_j = ast_node_list_get(type_params, j);
            if (param_j->type != AST_IDENTIFIER) {
                continue;
            }

            const char *name_j = param_j->data.identifier.name;
            if (!name_j) {
                continue;
            }

            if (strcmp(name_i, name_j) == 0) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, param_j->location,
                                      "Duplicate type parameter '%s'", name_i);
                return false;
            }
        }
    }

    return true;
}

/**
 * Register type parameters in the current scope for variant type validation
 */
bool register_enum_type_parameters(SemanticAnalyzer *analyzer, ASTNodeList *type_params,
                                   SourceLocation location) {
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
            semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, location,
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
        SymbolEntry *param_symbol =
            symbol_entry_create(param_name, SYMBOL_TYPE_PARAMETER, param_type, param);
        if (!param_symbol) {
            free((char *)param_type->name);
            free(param_type);
            semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, location,
                                  "Failed to create type parameter symbol");
            return false;
        }

        // Add to current scope
        if (!symbol_table_insert_safe(analyzer->current_scope, param_name, param_symbol)) {
            symbol_entry_destroy(param_symbol);
            semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, param->location,
                                  "Failed to register type parameter '%s'", param_name);
            return false;
        }
    }

    return true;
}
