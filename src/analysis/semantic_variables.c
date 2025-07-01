/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Variable Declarations and Assignments
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of variable declarations (let statements) and assignment validation
 */

#include "semantic_variables.h"
#include "../parser/ast.h"
#include "../parser/ast_node_list.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_type_helpers.h"
#include "semantic_utilities.h"
#include "type_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
static bool validate_assignment_target_mutability(SemanticAnalyzer *analyzer, ASTNode *target);

// =============================================================================
// VARIABLE DECLARATION STATEMENTS
// =============================================================================

bool analyze_let_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_LET_STMT) {
        return false;
    }

    const char *var_name = stmt->data.let_stmt.name;
    ASTNode *type_node = stmt->data.let_stmt.type;
    ASTNode *initializer = stmt->data.let_stmt.initializer;
    bool is_mutable = stmt->data.let_stmt.is_mutable; // Read mutability from AST

    if (!var_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Variable declaration missing name");
        return false;
    }

    // Check if variable is already declared in current scope
    SymbolEntry *existing = symbol_table_lookup_local(analyzer->current_scope, var_name);
    if (existing) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, stmt->location,
                              "Variable '%s' is already declared", var_name);
        return false;
    }

    // Resolve the variable type using semantic resolution
    TypeDescriptor *var_type = NULL;

    // Type annotation is now always required
    if (!type_node) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Variable declaration missing required type annotation. "
                              "Use 'let %s: Type = value;' syntax",
                              var_name);
        return false;
    }

    // Use analyze_type_node for proper semantic resolution
    // This will correctly resolve AST_STRUCT_TYPE to actual type (enum vs struct)
    var_type = analyze_type_node(analyzer, type_node);
    if (!var_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, type_node->location,
                              "Invalid type annotation in variable declaration for '%s'", var_name);
        return false;
    }

    // Analyze initializer expression BEFORE declaring the variable
    // This prevents the variable from being visible during its own initialization
    if (initializer) {
        // Set expected type context for type inference
        TypeDescriptor *old_expected_type = analyzer->expected_type;
        analyzer->expected_type = var_type;

        bool analyze_success = semantic_analyze_expression(analyzer, initializer);

        // Restore previous expected type context
        analyzer->expected_type = old_expected_type;

        if (!analyze_success) {
            type_descriptor_release(var_type);
            return false;
        }
        // Get the type of the initializer expression
        TypeDescriptor *init_type = semantic_get_expression_type(analyzer, initializer);
        if (init_type) {
            // Check type compatibility between declared type and initializer
            if (!semantic_check_type_compatibility(analyzer, init_type, var_type)) {
                semantic_report_error(
                    analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, initializer->location,
                    "Type mismatch in variable initialization: cannot assign %s to %s",
                    init_type->name ? init_type->name : "unknown type",
                    var_type->name ? var_type->name : "unknown type");
                type_descriptor_release(init_type);
                type_descriptor_release(var_type);
                return false;
            }
            type_descriptor_release(init_type);
        }
    }

    // Now create a symbol entry for the variable with mutability information
    SymbolEntry *var_symbol = symbol_entry_create(var_name, SYMBOL_VARIABLE, var_type, stmt);
    if (!var_symbol) {
        type_descriptor_release(var_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, stmt->location,
                              "Failed to create symbol entry for variable '%s'", var_name);
        return false;
    }

    // Set mutability flag in symbol entry
    var_symbol->flags.is_mutable = is_mutable;
    var_symbol->flags.is_initialized = (initializer != NULL);

    // Process ownership annotations on the variable
    if (stmt->data.let_stmt.annotations) {
        ASTNodeList *annotations = stmt->data.let_stmt.annotations;
        for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
            ASTNode *ann = ast_node_list_get(annotations, i);
            if (ann && ann->type == AST_OWNERSHIP_TAG) {
                // Process ownership annotation
                OwnershipType ownership = ann->data.ownership_tag.ownership;

                // Validate ownership type (already validated by parser, but double-check)
                if (ownership != OWNERSHIP_GC && ownership != OWNERSHIP_C &&
                    ownership != OWNERSHIP_PINNED) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION,
                                          ann->location, "Invalid ownership type");
                    symbol_entry_destroy(var_symbol);
                    type_descriptor_release(var_type);
                    return false;
                }

                // Store ownership information in symbol
                // For now, we just validate it's correct
                // The actual ownership tracking would be handled by the memory manager
                // TODO: Add ownership field to SymbolEntry to store this information
            }
        }
    }

    // Register the variable in the symbol table
    if (!symbol_table_insert_safe(analyzer->current_scope, var_name, var_symbol)) {
        symbol_entry_destroy(var_symbol);
        type_descriptor_release(var_type);
        semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, stmt->location,
                              "Failed to register variable '%s' in symbol table", var_name);
        return false;
    }

    // Attach TypeInfo to the AST node for codegen
    TypeInfo *type_info = type_info_from_descriptor(var_type);
    if (type_info) {
        ast_node_set_type_info(stmt, type_info);
        type_info_release(type_info); // ast_node_set_type_info retains it
    }

    return true;
}

// =============================================================================
// ASSIGNMENT VALIDATION (Immutable-by-Default)
// =============================================================================

/**
 * Validate assignment expressions to ensure immutable variables aren't modified
 */
bool analyze_assignment_validation(SemanticAnalyzer *analyzer, ASTNode *assignment) {
    if (!analyzer || !assignment || assignment->type != AST_ASSIGNMENT) {
        return false;
    }

    ASTNode *target = assignment->data.assignment.target;
    ASTNode *value = assignment->data.assignment.value;

    if (!target || !value) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, assignment->location,
                              "Assignment missing target or value");
        return false;
    }

    // Validate the assignment target is mutable
    if (!validate_assignment_target_mutability(analyzer, target)) {
        return false;
    }

    // Analyze both sides of the assignment
    if (!semantic_analyze_expression(analyzer, target)) {
        return false;
    }

    if (!semantic_analyze_expression(analyzer, value)) {
        return false;
    }

    // Check type compatibility between target and value
    TypeDescriptor *target_type = semantic_get_expression_type(analyzer, target);
    TypeDescriptor *value_type = semantic_get_expression_type(analyzer, value);

    if (target_type && value_type) {
        if (!semantic_check_type_compatibility(analyzer, value_type, target_type)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INCOMPATIBLE_TYPES, assignment->location,
                                  "Cannot assign %s to %s",
                                  value_type->name ? value_type->name : "unknown",
                                  target_type->name ? target_type->name : "unknown");
            type_descriptor_release(target_type);
            type_descriptor_release(value_type);
            return false;
        }
        type_descriptor_release(target_type);
        type_descriptor_release(value_type);
    }

    return true;
}

/**
 * Validate that an assignment target is mutable
 */
static bool validate_assignment_target_mutability(SemanticAnalyzer *analyzer, ASTNode *target) {
    if (!analyzer || !target) {
        return false;
    }

    switch (target->type) {
    case AST_IDENTIFIER: {
        // Simple variable assignment: check if variable is mutable
        const char *var_name = target->data.identifier.name;
        if (!var_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                                  "Assignment target missing variable name");
            return false;
        }

        SymbolEntry *symbol = symbol_table_lookup_safe(analyzer->current_scope, var_name);
        if (!symbol) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, target->location,
                                  "Undefined variable '%s' in assignment", var_name);
            return false;
        }

        if (symbol->kind != SYMBOL_VARIABLE && symbol->kind != SYMBOL_PARAMETER) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                                  "Cannot assign to '%s' - not a variable", var_name);
            return false;
        }

        // Check mutability flag
        if (!symbol->flags.is_mutable) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION, target->location,
                                  "Cannot assign to immutable variable '%s'. "
                                  "Declare with 'let mut %s = ...' if mutation is needed.",
                                  var_name, var_name);
            return false;
        }

        return true;
    }

    case AST_FIELD_ACCESS: {
        // Field assignment: check if base object is mutable
        ASTNode *base = target->data.field_access.object;
        const char *field_name = target->data.field_access.field_name;

        if (!base || !field_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                                  "Field access assignment missing base or field name");
            return false;
        }

        // Field mutability follows the containing variable's mutability
        // So we need to check if the base object is mutable
        return validate_assignment_target_mutability(analyzer, base);
    }

    case AST_INDEX_ACCESS: {
        // Array/slice element assignment: check if array is mutable
        ASTNode *array = target->data.index_access.array;

        if (!array) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                                  "Array access assignment missing array");
            return false;
        }

        // Array element mutability follows the array variable's mutability
        return validate_assignment_target_mutability(analyzer, array);
    }

    case AST_UNARY_EXPR: {
        // Pointer dereference assignment: check if pointer is mutable
        if (target->data.unary_expr.operator== UNOP_DEREF) {
            ASTNode *pointer = target->data.unary_expr.operand;

            if (!pointer) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                                      "Pointer dereference assignment missing pointer");
                return false;
            }

            // For pointer dereference, we need to check if the pointer type is mutable
            // First, analyze the pointer expression to get its type
            if (!semantic_analyze_expression(analyzer, pointer)) {
                return false;
            }

            // Get the pointer's type
            TypeDescriptor *pointer_type = semantic_get_expression_type(analyzer, pointer);
            if (!pointer_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, pointer->location,
                                      "Cannot determine type of pointer in dereference assignment");
                return false;
            }

            // Check if it's a pointer type
            if (!is_pointer_type(pointer_type)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pointer->location,
                                      "Cannot dereference non-pointer type in assignment");
                type_descriptor_release(pointer_type);
                return false;
            }

            // Check if it's a mutable pointer (*mut T)
            bool is_mutable_pointer = pointer_type->flags.is_mutable;
            type_descriptor_release(pointer_type);
            
            if (!is_mutable_pointer) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                                      "Cannot assign through const pointer. Use '*mut' pointer type for mutable access.");
                return false;
            }

            // If it's a mutable pointer, the assignment is allowed
            return true;
        }

        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                              "Invalid assignment target: unary operator %d",
                              target->data.unary_expr.operator);
        return false;
    }

    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, target->location,
                              "Invalid assignment target: expression type %d", target->type);
        return false;
    }
}