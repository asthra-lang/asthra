/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Field and Module Access
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of field access and module access expressions
 */

#include "semantic_field_access.h"
#include "semantic_builtins.h"
#include "semantic_core.h"
#include "semantic_diagnostics.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// FIELD AND MODULE ACCESS
// =============================================================================

bool analyze_field_access(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!analyzer || !node || node->type != AST_FIELD_ACCESS) {
        return false;
    }

    ASTNode *object = node->data.field_access.object;
    const char *field_name = node->data.field_access.field_name;

    if (!object || !field_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, node->location,
                              "Invalid field access: missing object or field name");
        return false;
    }

    // First analyze the object expression (could be nested field access, array access, etc.)
    if (!semantic_analyze_expression(analyzer, object)) {
        return false;
    }

    // Get the type of the object expression
    TypeDescriptor *object_type = semantic_get_expression_type(analyzer, object);
    if (!object_type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, object->location,
                              "Cannot determine type of object in field access");
        return false;
    }

    // Check if this is tuple element access
    if (object_type->category == TYPE_TUPLE) {
        // Check if field_name is a valid numeric index
        char *endptr;
        long index = strtol(field_name, &endptr, 10);

        if (*endptr != '\0' || index < 0) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, node->location,
                                  "Invalid tuple element access: '%s' is not a valid index",
                                  field_name);
            type_descriptor_release(object_type);
            return false;
        }

        if ((size_t)index >= object_type->data.tuple.element_count) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, node->location,
                                  "Tuple index %ld out of bounds for tuple with %zu elements",
                                  index, object_type->data.tuple.element_count);
            type_descriptor_release(object_type);
            return false;
        }

        // Get the type of the tuple element
        TypeDescriptor *element_type = object_type->data.tuple.element_types[index];
        if (!element_type) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, node->location,
                                  "Invalid tuple element type at index %ld", index);
            type_descriptor_release(object_type);
            return false;
        }

        // Set the type info for this node
        node->type_info = type_info_from_descriptor(element_type);
        if (!node->type_info) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, node->location,
                                  "Failed to create type info for tuple element at index %ld",
                                  index);
            type_descriptor_release(object_type);
            return false;
        }

        type_descriptor_release(object_type);
        return true;
    }

    // Check if the object type is a struct or generic instance
    TypeDescriptor *struct_type = object_type;
    SymbolTable *fields_table = NULL;

    if (object_type->category == TYPE_GENERIC_INSTANCE) {
        // For generic instances, we need to look at the base type
        struct_type = object_type->data.generic_instance.base_type;
        if (!struct_type || struct_type->category != TYPE_STRUCT) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, node->location,
                                  "Generic instance base type is not a struct");
            type_descriptor_release(object_type);
            return false;
        }
        fields_table = struct_type->data.struct_type.fields;
    } else if (object_type->category == TYPE_STRUCT) {
        fields_table = object_type->data.struct_type.fields;
    } else if (object_type->category == TYPE_ENUM) {
        // Handle enum constructors: Color.Red, Option.Some, etc.
        // For enum types accessed via dot notation, we're creating an enum variant
        // Transform the AST node from field access to enum variant
        node->type = AST_ENUM_VARIANT;
        node->data.enum_variant.enum_name = strdup(object_type->name);
        node->data.enum_variant.variant_name = strdup(field_name);
        node->data.enum_variant.value = NULL;

        // For generic enums, defer type setting to allow for type inference in call expressions
        // For non-generic enums, set the type immediately
        SymbolEntry *enum_symbol = semantic_resolve_identifier(analyzer, object_type->name);
        if (enum_symbol && enum_symbol->is_generic) {
            // Leave type_info as NULL for generic enum variants - will be set during call analysis
            node->type_info = NULL;
        } else {
            // Set the type info to the enum type for non-generic enums
            node->type_info = type_info_from_descriptor(object_type);
        }

        type_descriptor_release(object_type);
        return true;
    } else {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, node->location,
                              "Cannot access field '%s' on non-struct type %s", field_name,
                              object_type->name ? object_type->name : "unknown");
        type_descriptor_release(object_type);
        return false;
    }

    // Check if the struct has the requested field
    if (fields_table) {
        SymbolEntry *field_symbol = symbol_table_lookup_safe(fields_table, field_name);
        if (!field_symbol) {
            // Try to find similar field names
            size_t suggestion_count = 0;
            char **suggestions =
                find_similar_identifiers(field_name, fields_table, 3, &suggestion_count);

            if (suggestion_count > 0) {
                char msg[256];
                snprintf(msg, sizeof(msg), "no field named '%s' in struct '%s'", field_name,
                         object_type->name ? object_type->name : "unknown");
                semantic_report_error_with_suggestion(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                                                      node->location, msg, field_name,
                                                      (const char **)suggestions, suggestion_count);
                for (size_t i = 0; i < suggestion_count; i++) {
                    free(suggestions[i]);
                }
                free(suggestions);
            } else {
                semantic_report_error_enhanced(
                    analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, node->location, "field not found",
                    "check the struct definition for available fields",
                    "fields must be declared with 'pub' to be accessible");
            }

            if (object_type != NULL) {
                type_descriptor_release(object_type);
            }
            return false;
        }

        // Field access is valid - set type information on the node
        if (field_symbol->type) {
            TypeDescriptor *field_type = field_symbol->type;

            // For generic instances, we might need to substitute type parameters
            if (object_type->category == TYPE_GENERIC_INSTANCE) {
                // Simple type substitution for type parameters
                // This is a simplified version - a full implementation would need proper
                // substitution
                if (field_type->name && strlen(field_type->name) == 1 &&
                    field_type->name[0] >= 'A' && field_type->name[0] <= 'Z') {
                    // Likely a type parameter, use the first type argument
                    if (object_type->data.generic_instance.type_arg_count > 0) {
                        field_type = object_type->data.generic_instance.type_args[0];
                    }
                }
            }

            node->type_info = type_info_from_descriptor(field_type);
            if (!node->type_info) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, node->location,
                                      "Failed to create type info for field '%s'", field_name);
                if (object_type != NULL) {
                    type_descriptor_release(object_type);
                }
                return false;
            }
        }

        if (object_type != NULL) {
            type_descriptor_release(object_type);
        }
        return true;
    } else {
        semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, node->location,
                              "Struct type '%s' has no fields defined",
                              object_type->name ? object_type->name : "unknown");
        if (object_type != NULL) {
            type_descriptor_release(object_type);
        }
        return false;
    }
}

ASTNode *analyze_regular_field_access(SemanticAnalyzer *analyzer, ASTNode *node) {
    // Regular field access is the same as general field access for now
    return analyze_field_access(analyzer, node) ? node : NULL;
}

ASTNode *analyze_module_access(SemanticAnalyzer *analyzer, ASTNode *node) {
    (void)analyzer;
    // TODO: Implement module access analysis
    return node;
}