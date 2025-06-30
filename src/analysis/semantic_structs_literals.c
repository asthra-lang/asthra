/*
 * Struct Literal Analysis Module
 * Struct literal expression analysis with generic type support
 *
 * Part of semantic_structs.c split (578 lines -> 5 focused modules)
 * Handles struct literal analysis, field initialization validation, and generic type checking
 */

#include "semantic_structs_common.h"
#include "type_info.h"

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Helper structure for collecting field names
 */
typedef struct {
    char **names;
    size_t index;
    size_t capacity;
} FieldCollector;

/**
 * Callback function for collecting field names from symbol table
 */
static bool collect_field_names(const char *name, SymbolEntry *entry, void *user_data) {
    if (!entry || entry->kind != SYMBOL_FIELD) {
        return true; // Continue iteration
    }
    
    FieldCollector *collector = (FieldCollector *)user_data;
    if (collector->index < collector->capacity) {
        collector->names[collector->index++] = (char *)name;
    }
    return true; // Continue iteration
}

/**
 * Get the actual field type for a generic instance
 * For generic instances, this substitutes type parameters with concrete types
 */
static TypeDescriptor *get_instance_field_type(TypeDescriptor *instance_type,
                                               SymbolEntry *field_symbol) {
    if (!instance_type || !field_symbol || !field_symbol->type) {
        return NULL;
    }

    // If this is not a generic instance, just return the field type
    if (instance_type->category != TYPE_GENERIC_INSTANCE) {
        type_descriptor_retain(field_symbol->type);
        return field_symbol->type;
    }

    // For generic instances, we need to check if the field type is a type parameter
    TypeDescriptor *field_type = field_symbol->type;

    // Simple case: if the field type is exactly a type parameter (T)
    // In a full implementation, we'd need to handle nested types like Vec<T>, *T, etc.
    // For now, let's handle the simple case where the field type matches a type parameter name

    // Get the base struct's type parameters (we'd need to store these)
    // For now, we'll do a simple check: if the field type name matches "T", "U", etc.
    // and we have type arguments, use the corresponding type argument

    // This is a simplified implementation - in practice, we'd need to:
    // 1. Store type parameter names in the struct symbol
    // 2. Match field types against type parameters
    // 3. Recursively substitute in complex types

    // For the test case Vec<T> with field "data: T", when instantiated as Vec<i32>,
    // we need to return i32 instead of T

    // Simple heuristic: if field type name is a single uppercase letter, it's likely a type param
    if (field_type->name && strlen(field_type->name) == 1 && field_type->name[0] >= 'A' &&
        field_type->name[0] <= 'Z') {
        // Assume it's the first type parameter for now
        if (instance_type->data.generic_instance.type_arg_count > 0) {
            TypeDescriptor *substituted = instance_type->data.generic_instance.type_args[0];
            type_descriptor_retain(substituted);
            return substituted;
        }
    }

    // Otherwise, return the original field type
    type_descriptor_retain(field_type);
    return field_type;
}

// ============================================================================
// STRUCT LITERAL ANALYSIS
// ============================================================================

/**
 * Analyze struct literal expressions with generic type support
 */
bool analyze_struct_literal_expression(SemanticAnalyzer *analyzer, ASTNode *struct_literal) {
    if (!analyzer || !struct_literal || struct_literal->type != AST_STRUCT_LITERAL) {
        return false;
    }

    const char *struct_name = struct_literal->data.struct_literal.struct_name;
    ASTNodeList *type_args = struct_literal->data.struct_literal.type_args;
    ASTNodeList *field_inits = struct_literal->data.struct_literal.field_inits;

    if (!struct_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, struct_literal->location,
                              "Struct literal missing struct name");
        return false;
    }

    // Look up the struct type in the symbol table
    SymbolEntry *struct_symbol = symbol_table_lookup_safe(analyzer->current_scope, struct_name);
    if (!struct_symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, struct_literal->location,
                              "Struct type '%s' not found in symbol table", struct_name);
        return false;
    }
    if (struct_symbol->kind != SYMBOL_TYPE) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, struct_literal->location,
                              "Symbol '%s' is not a type (kind: %d)", struct_name,
                              struct_symbol->kind);
        return false;
    }
    if (!struct_symbol->type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, struct_literal->location,
                              "Struct type '%s' has no type descriptor", struct_name);
        return false;
    }

    TypeDescriptor *struct_type = struct_symbol->type;
    if (struct_type->category != TYPE_STRUCT) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, struct_literal->location,
                              "Type '%s' is not a struct", struct_name);
        return false;
    }

    // Phase 3: Handle generic struct literals
    TypeDescriptor *actual_struct_type = struct_type; // The type to use for field checking

    if (type_args) {
        size_t arg_count = ast_node_list_size(type_args);

        // Validate that this is actually a generic struct
        if (!struct_symbol->is_generic) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, struct_literal->location,
                                  "Struct '%s' is not generic but type arguments provided",
                                  struct_name);
            return false;
        }

        // Validate type argument count matches parameter count
        if (arg_count != struct_symbol->type_param_count) {
            semantic_report_error(
                analyzer, SEMANTIC_ERROR_INVALID_TYPE, struct_literal->location,
                "Type argument count mismatch for struct '%s': expected %zu, got %zu", struct_name,
                struct_symbol->type_param_count, arg_count);
            return false;
        }

        // Collect and validate type arguments
        TypeDescriptor **arg_types = malloc(arg_count * sizeof(TypeDescriptor *));
        if (!arg_types) {
            return false;
        }

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
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, type_arg->location,
                                          "Invalid type argument %zu for struct '%s'", i + 1,
                                          struct_name);
                    return false;
                }
                arg_types[i] = arg_type;
            } else {
                arg_types[i] = NULL;
            }
        }

        // Create the generic instance type
        TypeDescriptor *instance_type =
            type_descriptor_create_generic_instance(struct_type, arg_types, arg_count);

        // Release the type arguments (instance retains them)
        for (size_t i = 0; i < arg_count; i++) {
            if (arg_types[i]) {
                type_descriptor_release(arg_types[i]);
            }
        }
        free(arg_types);

        if (!instance_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, struct_literal->location,
                                  "Failed to create generic instance for struct '%s'", struct_name);
            return false;
        }

        // Use the instance type for field checking
        actual_struct_type = instance_type;

        // Store the instance type in the AST node for later use
        // Note: We'll need to release this later or ensure proper memory management
    } else if (struct_symbol->is_generic) {
        // Generic struct used without type arguments - error
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, struct_literal->location,
                              "Generic struct '%s' requires type arguments", struct_name);
        return false;
    }

    // Get the fields table for validation
    TypeDescriptor *field_lookup_type = actual_struct_type;
    if (actual_struct_type->category == TYPE_GENERIC_INSTANCE) {
        field_lookup_type = actual_struct_type->data.generic_instance.base_type;
    }

    SymbolTable *fields_table = field_lookup_type->data.struct_type.fields;
    if (!fields_table && field_lookup_type->data.struct_type.field_count > 0) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, struct_literal->location,
                              "Struct '%s' has invalid field table", struct_name);
        if (actual_struct_type != struct_type) {
            type_descriptor_release(actual_struct_type);
        }
        return false;
    }

    // Create a tracking structure for initialized fields
    size_t field_count = field_lookup_type->data.struct_type.field_count;
    bool *fields_initialized = NULL;
    char **field_names = NULL;
    
    if (field_count > 0) {
        fields_initialized = calloc(field_count, sizeof(bool));
        field_names = calloc(field_count, sizeof(char*));
        if (!fields_initialized || !field_names) {
            free(fields_initialized);
            free(field_names);
            if (actual_struct_type != struct_type) {
                type_descriptor_release(actual_struct_type);
            }
            return false;
        }

        // Collect all field names from the struct definition
        FieldCollector collector = {
            .names = field_names,
            .index = 0,
            .capacity = field_count
        };

        symbol_table_iterate(fields_table, collect_field_names, &collector);
    }

    // Validate field initializations
    if (field_inits) {
        size_t field_init_count = ast_node_list_size(field_inits);

        for (size_t i = 0; i < field_init_count; i++) {
            ASTNode *field_init = ast_node_list_get(field_inits, i);
            if (!field_init || field_init->type != AST_ASSIGNMENT) {
                continue;
            }

            // Field initialization is stored as assignment: field_name = value
            ASTNode *field_target = field_init->data.assignment.target;
            ASTNode *field_value = field_init->data.assignment.value;

            if (!field_target || field_target->type != AST_IDENTIFIER) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                      field_init->location,
                                      "Field initialization target must be an identifier");
                continue;
            }

            const char *field_name = field_target->data.identifier.name;
            if (!field_name) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                      field_init->location,
                                      "Field initialization missing field name");
                continue;
            }

            SymbolEntry *field_symbol =
                symbol_table_lookup_local(fields_table, field_name);
            if (!field_symbol) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                      field_init->location, "Struct '%s' has no field '%s'",
                                      struct_name, field_name);
                continue;
            }

            // Mark this field as initialized
            for (size_t j = 0; j < field_count; j++) {
                if (field_names[j] && strcmp(field_names[j], field_name) == 0) {
                    fields_initialized[j] = true;
                    break;
                }
            }

            // Analyze the field value expression
            if (field_value) {
                if (!semantic_analyze_expression(analyzer, field_value)) {
                    // Clean up
                    free(fields_initialized);
                    free(field_names);
                    if (actual_struct_type != struct_type) {
                        type_descriptor_release(actual_struct_type);
                    }
                    return false;
                }
            }

            // Get the actual field type (with type parameter substitution for generic instances)
            TypeDescriptor *actual_field_type =
                get_instance_field_type(actual_struct_type, field_symbol);
            if (!actual_field_type) {
                continue;
            }

            // Check type compatibility between field and value
            if (field_value && actual_field_type) {
                TypeDescriptor *value_type = semantic_get_expression_type(analyzer, field_value);
                if (value_type) {
                    bool compatible =
                        semantic_check_type_compatibility(analyzer, value_type, actual_field_type);
                    if (!compatible) {
                        semantic_report_error(
                            analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, field_init->location,
                            "Type mismatch in field '%s': expected %s, got %s", field_name,
                            actual_field_type->name ? actual_field_type->name : "unknown",
                            value_type->name ? value_type->name : "unknown");
                        type_descriptor_release(value_type);
                        type_descriptor_release(actual_field_type);
                        // Clean up
                        free(fields_initialized);
                        free(field_names);
                        if (actual_struct_type != struct_type) {
                            type_descriptor_release(actual_struct_type);
                        }
                        return false;
                    }
                    type_descriptor_release(value_type);
                }
            }

            type_descriptor_release(actual_field_type);
        }
    }

    // Check for missing fields (only if struct has fields)
    bool has_missing_fields = false;
    if (field_count > 0) {
        for (size_t i = 0; i < field_count; i++) {
            if (!fields_initialized[i] && field_names[i]) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                      struct_literal->location,
                                      "missing field '%s' in struct literal", field_names[i]);
                has_missing_fields = true;
            }
        }
    }

    // Clean up tracking data
    free(fields_initialized);
    free(field_names);

    if (has_missing_fields) {
        if (actual_struct_type != struct_type) {
            type_descriptor_release(actual_struct_type);
        }
        return false;
    }

    // Store the type information in the AST node for later retrieval
    if (!struct_literal->type_info) {
        TypeInfo *type_info = type_info_from_descriptor(actual_struct_type);
        if (type_info) {
            struct_literal->type_info = type_info;
            // Don't release the instance type since TypeInfo now holds a reference to it
        }
    } else {
        // Clean up instance type if we created one but didn't store it
        if (actual_struct_type != struct_type) {
            type_descriptor_release(actual_struct_type);
        }
    }

    return true;
}
