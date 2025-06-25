/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - FFI Annotations Validation Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of FFI annotation validation for SafeFFIAnnotation
 * Phase 3: Semantic Analysis Enhancement for FFI Annotation Ambiguity Fix
 */

#include "semantic_annotations_ffi.h"
#include "../parser/ast_node_list.h"
#include "semantic_annotations_errors.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// FFI ANNOTATION VALIDATION FUNCTIONS
// =============================================================================

bool is_ffi_transfer_annotation(const char *annotation_name) {
    if (!annotation_name)
        return false;

    return (strcmp(annotation_name, "transfer_full") == 0 ||
            strcmp(annotation_name, "transfer_none") == 0 ||
            strcmp(annotation_name, "borrowed") == 0);
}

AnnotationContext get_ffi_annotation_context(ASTNode *node) {
    if (!node)
        return CONTEXT_ANY;

    switch (node->type) {
    case AST_PARAM_DECL:
        return CONTEXT_PARAMETER;
    case AST_FUNCTION_DECL:
    case AST_EXTERN_DECL:
        // For function/extern nodes, we need to determine if this is
        // for return type annotation or parameter annotation based on context
        return CONTEXT_RETURN_TYPE;
    default:
        return CONTEXT_ANY;
    }
}

bool validate_return_type_ffi_annotation(SemanticAnalyzer *analyzer, const char *annotation_name,
                                         ASTNode *function_node) {
    if (!analyzer || !annotation_name || !function_node)
        return false;

    // Only transfer_full and transfer_none are allowed on return types
    if (strcmp(annotation_name, "borrowed") == 0) {
        report_invalid_ffi_annotation_context_error(analyzer, function_node, annotation_name,
                                                    CONTEXT_RETURN_TYPE);
        return false;
    }

    // transfer_full and transfer_none are valid
    if (strcmp(annotation_name, "transfer_full") == 0 ||
        strcmp(annotation_name, "transfer_none") == 0) {
        return true;
    }

    // Unknown FFI annotation
    return false;
}

bool validate_parameter_ffi_annotation(SemanticAnalyzer *analyzer, const char *annotation_name,
                                       ASTNode *param_node) {
    if (!analyzer || !annotation_name || !param_node)
        return false;

    // All FFI transfer annotations are allowed on parameters
    if (is_ffi_transfer_annotation(annotation_name)) {
        return true;
    }

    // Unknown FFI annotation
    return false;
}

bool validate_ffi_transfer_annotation_context(SemanticAnalyzer *analyzer,
                                              const char *annotation_name, ASTNode *target_node,
                                              AnnotationContext context) {
    if (!analyzer || !annotation_name || !target_node)
        return false;

    if (!is_ffi_transfer_annotation(annotation_name)) {
        return true; // Not an FFI transfer annotation, let other validators handle it
    }

    switch (context) {
    case CONTEXT_RETURN_TYPE:
        return validate_return_type_ffi_annotation(analyzer, annotation_name, target_node);

    case CONTEXT_PARAMETER:
        return validate_parameter_ffi_annotation(analyzer, annotation_name, target_node);

    default:
        // FFI transfer annotations are only valid on parameters and return types
        report_invalid_ffi_annotation_context_error(analyzer, target_node, annotation_name,
                                                    context);
        return false;
    }
}

bool validate_ffi_annotation_mutual_exclusivity(SemanticAnalyzer *analyzer, ASTNode *node,
                                                ASTNodeList *annotations) {
    if (!analyzer || !node || !annotations)
        return true;

    const char *found_ffi_annotation = NULL;

    // Iterate through annotations to check for multiple FFI transfer annotations
    for (size_t i = 0; i < annotations->count; i++) {
        ASTNode *annotation = annotations->nodes[i];
        if (!annotation || annotation->type != AST_SEMANTIC_TAG)
            continue;

        const char *annotation_name = annotation->data.semantic_tag.name;
        if (!is_ffi_transfer_annotation(annotation_name))
            continue;

        if (found_ffi_annotation != NULL) {
            // Multiple FFI transfer annotations found - this violates mutual exclusivity
            report_ffi_annotation_conflict_error(analyzer, node, found_ffi_annotation,
                                                 annotation_name);
            return false;
        }

        found_ffi_annotation = annotation_name;
    }

    return true; // No conflicts found
}

bool validate_function_ffi_annotations(SemanticAnalyzer *analyzer, ASTNode *func_node) {
    if (!analyzer || !func_node)
        return true;

    if (func_node->type != AST_FUNCTION_DECL)
        return true;

    bool valid = true;

    // Validate return type annotations
    ASTNodeList *return_annotations = func_node->data.function_decl.annotations;
    if (return_annotations) {
        // Check mutual exclusivity for return type annotations
        if (!validate_ffi_annotation_mutual_exclusivity(analyzer, func_node, return_annotations)) {
            valid = false;
        }

        // Validate each return type annotation
        for (size_t i = 0; i < return_annotations->count; i++) {
            ASTNode *annotation = return_annotations->nodes[i];
            if (!annotation || annotation->type != AST_SEMANTIC_TAG)
                continue;

            const char *annotation_name = annotation->data.semantic_tag.name;
            if (is_ffi_transfer_annotation(annotation_name)) {
                if (!validate_ffi_transfer_annotation_context(analyzer, annotation_name, func_node,
                                                              CONTEXT_RETURN_TYPE)) {
                    valid = false;
                }
            }
        }
    }

    // Validate parameter annotations
    ASTNodeList *params = func_node->data.function_decl.params;
    if (params) {
        for (size_t i = 0; i < params->count; i++) {
            ASTNode *param = params->nodes[i];
            if (!param || param->type != AST_PARAM_DECL)
                continue;

            ASTNodeList *param_annotations = param->data.param_decl.annotations;
            if (!param_annotations)
                continue;

            // Check mutual exclusivity for parameter annotations
            if (!validate_ffi_annotation_mutual_exclusivity(analyzer, param, param_annotations)) {
                valid = false;
            }

            // Validate each parameter annotation
            for (size_t j = 0; j < param_annotations->count; j++) {
                ASTNode *annotation = param_annotations->nodes[j];
                if (!annotation || annotation->type != AST_SEMANTIC_TAG)
                    continue;

                const char *annotation_name = annotation->data.semantic_tag.name;
                if (is_ffi_transfer_annotation(annotation_name)) {
                    if (!validate_ffi_transfer_annotation_context(analyzer, annotation_name, param,
                                                                  CONTEXT_PARAMETER)) {
                        valid = false;
                    }
                }
            }
        }
    }

    return valid;
}

bool validate_extern_ffi_annotations(SemanticAnalyzer *analyzer, ASTNode *extern_node) {
    if (!analyzer || !extern_node)
        return true;

    if (extern_node->type != AST_EXTERN_DECL)
        return true;

    bool valid = true;

    // Validate return type annotations
    ASTNodeList *return_annotations = extern_node->data.extern_decl.annotations;
    if (return_annotations) {
        // Check mutual exclusivity for return type annotations
        if (!validate_ffi_annotation_mutual_exclusivity(analyzer, extern_node,
                                                        return_annotations)) {
            valid = false;
        }

        // Validate each return type annotation
        for (size_t i = 0; i < return_annotations->count; i++) {
            ASTNode *annotation = return_annotations->nodes[i];
            if (!annotation || annotation->type != AST_SEMANTIC_TAG)
                continue;

            const char *annotation_name = annotation->data.semantic_tag.name;
            if (is_ffi_transfer_annotation(annotation_name)) {
                if (!validate_ffi_transfer_annotation_context(analyzer, annotation_name,
                                                              extern_node, CONTEXT_RETURN_TYPE)) {
                    valid = false;
                }
            }
        }
    }

    // Validate parameter annotations
    ASTNodeList *params = extern_node->data.extern_decl.params;
    if (params) {
        for (size_t i = 0; i < params->count; i++) {
            ASTNode *param = params->nodes[i];
            if (!param || param->type != AST_PARAM_DECL)
                continue;

            ASTNodeList *param_annotations = param->data.param_decl.annotations;
            if (!param_annotations)
                continue;

            // Check mutual exclusivity for parameter annotations
            if (!validate_ffi_annotation_mutual_exclusivity(analyzer, param, param_annotations)) {
                valid = false;
            }

            // Validate each parameter annotation
            for (size_t j = 0; j < param_annotations->count; j++) {
                ASTNode *annotation = param_annotations->nodes[j];
                if (!annotation || annotation->type != AST_SEMANTIC_TAG)
                    continue;

                const char *annotation_name = annotation->data.semantic_tag.name;
                if (is_ffi_transfer_annotation(annotation_name)) {
                    if (!validate_ffi_transfer_annotation_context(analyzer, annotation_name, param,
                                                                  CONTEXT_PARAMETER)) {
                        valid = false;
                    }
                }
            }
        }
    }

    return valid;
}

// =============================================================================
// FFI ANNOTATION ERROR REPORTING
// =============================================================================

void report_invalid_ffi_annotation_context_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                                 const char *annotation_name,
                                                 AnnotationContext context) {
    if (!analyzer || !node || !annotation_name)
        return;

    char error_message[256];
    switch (context) {
    case CONTEXT_RETURN_TYPE:
        if (strcmp(annotation_name, "borrowed") == 0) {
            snprintf(error_message, sizeof(error_message),
                     "FFI annotation '#[%s]' is not allowed on return types. "
                     "Only '#[transfer_full]' or '#[transfer_none]' are valid for return types.",
                     annotation_name);
        } else {
            snprintf(error_message, sizeof(error_message),
                     "FFI annotation '#[%s]' is not valid for return types.", annotation_name);
        }
        break;

    case CONTEXT_PARAMETER:
        snprintf(error_message, sizeof(error_message),
                 "FFI annotation '#[%s]' is not valid for parameters.", annotation_name);
        break;

    default:
        snprintf(
            error_message, sizeof(error_message),
            "FFI annotation '#[%s]' is not valid in this context. "
            "FFI transfer annotations are only allowed on function parameters and return types.",
            annotation_name);
        break;
    }

    report_context_validation_error(analyzer, node, annotation_name, context);
}

void report_ffi_annotation_conflict_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                          const char *first_annotation,
                                          const char *second_annotation) {
    if (!analyzer || !node || !first_annotation || !second_annotation)
        return;

    char error_message[256];
    snprintf(error_message, sizeof(error_message),
             "Conflicting FFI annotations: '#[%s]' and '#[%s]' are mutually exclusive. "
             "Only one FFI transfer annotation is allowed per declaration.",
             first_annotation, second_annotation);

    // Find the conflict definition for more specific error reporting
    const AnnotationConflict *conflicts = get_all_annotation_conflicts();
    for (size_t i = 0; conflicts[i].annotation1 != NULL; i++) {
        if ((strcmp(conflicts[i].annotation1, first_annotation) == 0 &&
             strcmp(conflicts[i].annotation2, second_annotation) == 0) ||
            (strcmp(conflicts[i].annotation1, second_annotation) == 0 &&
             strcmp(conflicts[i].annotation2, first_annotation) == 0)) {
            report_annotation_conflict_error(analyzer, node, &conflicts[i]);
            return;
        }
    }

    // Fallback to general conflict error
    // TODO: Add a generic conflict error function if not available
}
