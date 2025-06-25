/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Parameters Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of parameter validation for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#include "semantic_annotations_parameters.h"
#include "../parser/ast_node_list.h"
#include "semantic_annotations_errors.h"
#include <string.h>

// =============================================================================
// PARAMETER VALIDATION FUNCTIONS
// =============================================================================

bool validate_annotation_parameters(SemanticAnalyzer *analyzer, ASTNode *tag_node,
                                    const AnnotationParameterSchema *schema) {
    if (!analyzer || !tag_node || !schema || tag_node->type != AST_SEMANTIC_TAG) {
        return false;
    }

    ASTNodeList *params = tag_node->data.semantic_tag.params;
    size_t param_count = params ? ast_node_list_size(params) : 0;

    // Check required parameters
    for (size_t i = 0; i < schema->parameter_count; i++) {
        const ParameterDefinition *param_def = &schema->parameters[i];
        if (param_def->required && i >= param_count) {
            report_parameter_validation_error(analyzer, tag_node, schema->annotation_name,
                                              param_def->name, "required parameter is missing");
            return false;
        }
    }

    // Validate provided parameters
    for (size_t i = 0; i < param_count && i < schema->parameter_count; i++) {
        ASTNode *param = ast_node_list_get(params, i);
        const ParameterDefinition *param_def = &schema->parameters[i];

        if (!param)
            continue;

        // Type validation based on parameter definition
        bool type_valid = false;
        switch (param_def->type) {
        case ANNOTATION_PARAM_STRING:
            type_valid = (param->type == AST_STRING_LITERAL);
            break;
        case ANNOTATION_PARAM_IDENT:
            type_valid = (param->type == AST_IDENTIFIER);
            break;
        case ANNOTATION_PARAM_INT:
            type_valid = (param->type == AST_INTEGER_LITERAL);
            if (type_valid &&
                param_def->validation.int_range.min != param_def->validation.int_range.max) {
                int64_t value = param->data.integer_literal.value;
                if (value < param_def->validation.int_range.min ||
                    value > param_def->validation.int_range.max) {
                    report_parameter_validation_error(analyzer, tag_node, schema->annotation_name,
                                                      param_def->name, "value out of valid range");
                    return false;
                }
            }
            break;
        case ANNOTATION_PARAM_BOOL:
            type_valid = (param->type == AST_BOOL_LITERAL);
            break;
        }

        if (!type_valid) {
            report_parameter_validation_error(analyzer, tag_node, schema->annotation_name,
                                              param_def->name, "parameter type mismatch");
            return false;
        }

        // String length validation
        if (param_def->type == ANNOTATION_PARAM_STRING && param->type == AST_STRING_LITERAL) {
            const char *str_value = param->data.string_literal.value;
            if (str_value) {
                size_t len = strlen(str_value);
                if ((int)len < param_def->validation.string_constraints.min_len ||
                    (int)len > param_def->validation.string_constraints.max_len) {
                    report_parameter_validation_error(analyzer, tag_node, schema->annotation_name,
                                                      param_def->name,
                                                      "string length out of valid range");
                    return false;
                }
            }
        }

        // Enum value validation
        if (param_def->type == ANNOTATION_PARAM_IDENT && param->type == AST_IDENTIFIER &&
            param_def->validation.enum_values.values) {
            const char *ident_value = param->data.identifier.name;
            bool valid_enum = false;
            for (const char **valid_value = param_def->validation.enum_values.values; *valid_value;
                 valid_value++) {
                if (strcmp(ident_value, *valid_value) == 0) {
                    valid_enum = true;
                    break;
                }
            }
            if (!valid_enum) {
                report_parameter_validation_error(analyzer, tag_node, schema->annotation_name,
                                                  param_def->name, "invalid enum value");
                return false;
            }
        }
    }

    return true;
}
