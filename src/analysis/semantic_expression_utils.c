/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Expression Utilities and Struct/Enum Analysis
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Utility functions for expression analysis and struct/enum expression handling
 */

#include "semantic_expression_utils.h"
#include "semantic_core.h"
#include "semantic_utilities.h"
#include "semantic_types.h"
#include "semantic_type_helpers.h"
#include "semantic_diagnostics.h"
#include "semantic_builtins.h"
#include "type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// STRUCT AND ENUM EXPRESSIONS
// =============================================================================

bool analyze_struct_instantiation(SemanticAnalyzer *analyzer, ASTNode *expr) {
    (void)analyzer;
    (void)expr;
    // TODO: Implement struct instantiation analysis
    return true;
}

bool analyze_enum_variant(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return false;
    }
    
    if (expr->type == AST_ENUM_VARIANT) {
        const char *enum_name = expr->data.enum_variant.enum_name;
        const char *variant_name = expr->data.enum_variant.variant_name;
        
        
        if (!enum_name || !variant_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION,
                                 expr->location,
                                 "Invalid enum constructor: missing enum or variant name");
            return false;
        }
        
        // Check if enum exists (use recursive lookup to check parent scopes)
        SymbolEntry *enum_symbol = semantic_resolve_identifier(analyzer, enum_name);
        if (!enum_symbol || enum_symbol->kind != SYMBOL_TYPE || 
            !enum_symbol->type || enum_symbol->type->category != TYPE_ENUM) {
            
            // WORKAROUND: Check if this is actually a field access misrepresented as enum variant
            SymbolEntry *var_symbol = symbol_table_lookup_safe(analyzer->current_scope, enum_name);
            if (var_symbol && var_symbol->kind == SYMBOL_VARIABLE) {
                
                // Treat this as a field access: variable.field
                if (!var_symbol->type) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                                         expr->location,
                                         "Cannot access field on variable '%s' with unknown type", enum_name);
                    return false;
                }
                
                // Check if the variable type is a struct
                if (var_symbol->type->category != TYPE_STRUCT) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                                         expr->location,
                                         "Cannot access field '%s' on non-struct type", variant_name);
                    return false;
                }
                
                // Look up the field in the struct type
                if (!var_symbol->type->data.struct_type.fields) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                                         expr->location,
                                         "Struct type has no fields");
                    return false;
                }
                
                SymbolEntry *field_symbol = symbol_table_lookup_safe(var_symbol->type->data.struct_type.fields, variant_name);
                if (!field_symbol) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                         expr->location,
                                         "Struct has no field '%s'", variant_name);
                    return false;
                }
                
                // Set the type of the expression to the field type
                if (field_symbol->type) {
                    semantic_set_expression_type(analyzer, expr, field_symbol->type);
                }
                
                return true;
            }
            
            // Not a variable either, so it's genuinely an undefined enum
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                 expr->location,
                                 "Undefined enum type: %s", enum_name);
            return false;
        }
        
        // Check if variant exists in the enum
        SymbolEntry *variant_symbol = symbol_table_lookup_safe(
            enum_symbol->type->data.enum_type.variants, variant_name);
        if (!variant_symbol || variant_symbol->kind != SYMBOL_ENUM_VARIANT) {
            // Debug: Also try looking up by qualified name in global scope
            size_t qualified_len = strlen(enum_name) + strlen(variant_name) + 2;
            char *qualified_name = malloc(qualified_len);
            if (qualified_name) {
                snprintf(qualified_name, qualified_len, "%s.%s", enum_name, variant_name);
                SymbolEntry *qualified_variant = semantic_resolve_identifier(analyzer, qualified_name);
                free(qualified_name);
                
                if (qualified_variant && qualified_variant->kind == SYMBOL_ENUM_VARIANT) {
                    // Found it via qualified name, use this instead
                    variant_symbol = qualified_variant;
                } else {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                         expr->location,
                                         "Enum '%s' has no variant '%s'", enum_name, variant_name);
                    return false;
                }
            } else {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                     expr->location,
                                     "Enum '%s' has no variant '%s'", enum_name, variant_name);
                return false;
            }
        }
        
        // Analyze constructor value if present
        if (expr->data.enum_variant.value) {
            if (!semantic_analyze_expression(analyzer, expr->data.enum_variant.value)) {
                return false;
            }
        }
        
        // Set the type of the enum variant expression to the enum type
        semantic_set_expression_type(analyzer, expr, enum_symbol->type);
        
        return true;
    }
    
    return true;
}

// =============================================================================
// TYPE CHECKING UTILITIES
// =============================================================================

bool semantic_is_lvalue_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return false;
    }

    switch (expr->type) {
        case AST_IDENTIFIER:
            return true; // Variables are lvalues
        case AST_FIELD_ACCESS:
            return true; // Field access yields lvalue if base is lvalue
        case AST_INDEX_ACCESS:
            return true; // Index access yields lvalue
        case AST_UNARY_EXPR:
            // Dereference yields an lvalue
            return expr->data.unary_expr.operator == UNOP_DEREF;
        default:
            return false; // Most expressions are rvalues
    }
}

bool semantic_is_constant_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return false;
    }

    switch (expr->type) {
        case AST_INTEGER_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_STRING_LITERAL:
        case AST_BOOL_LITERAL:
        case AST_CHAR_LITERAL:
        case AST_UNIT_LITERAL:
            return true;
        case AST_IDENTIFIER:
            {
                SymbolEntry *symbol = semantic_resolve_identifier(analyzer, expr->data.identifier.name);
                return symbol && (symbol->kind == SYMBOL_CONST);
            }
        case AST_BINARY_EXPR:
            return semantic_is_constant_expression(analyzer, expr->data.binary_expr.left) &&
                   semantic_is_constant_expression(analyzer, expr->data.binary_expr.right);
        case AST_UNARY_EXPR:
            return semantic_is_constant_expression(analyzer, expr->data.unary_expr.operand);
        default:
            return false;
    }
}

bool semantic_has_side_effects(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return false;
    }

    switch (expr->type) {
        case AST_ASSIGNMENT:
            return true;
        case AST_CALL_EXPR:
            return true;
        case AST_BINARY_EXPR:
            return semantic_has_side_effects(analyzer, expr->data.binary_expr.left) ||
                   semantic_has_side_effects(analyzer, expr->data.binary_expr.right);
        case AST_UNARY_EXPR:
            return semantic_has_side_effects(analyzer, expr->data.unary_expr.operand);
        case AST_STRUCT_LITERAL:
            if (expr->type == AST_STRUCT_LITERAL) {
                if (expr->data.struct_literal.field_inits) {
                    for (size_t i = 0; i < expr->data.struct_literal.field_inits->count; i++) {
                        // For now, assume no side effects in field initialization
                        // This would need more sophisticated analysis
                    }
                }
            }
            return false;
        default:
            return false;
    }
}