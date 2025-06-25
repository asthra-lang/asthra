/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Inheritance Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of inheritance and dependency validation for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#include "semantic_annotations_inheritance.h"
#include "../parser/ast_node_list.h"
#include "semantic_annotations_errors.h"
#include <string.h>

// =============================================================================
// FORWARD DECLARATIONS FOR STATIC HELPER FUNCTIONS
// =============================================================================

static bool validate_function_annotation_inheritance(SemanticAnalyzer *analyzer,
                                                     ASTNode *func_decl);
static bool validate_method_annotation_inheritance(SemanticAnalyzer *analyzer,
                                                   ASTNode *method_decl);
static bool validate_struct_annotation_inheritance(SemanticAnalyzer *analyzer,
                                                   ASTNode *struct_decl);
static bool validate_single_annotation_dependencies(SemanticAnalyzer *analyzer, ASTNode *node,
                                                    const char *annotation_name,
                                                    ASTNodeList *annotations);

// =============================================================================
// INHERITANCE ANALYSIS FUNCTIONS
// =============================================================================

bool analyze_annotation_inheritance(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!analyzer || !node) {
        return false;
    }

    // For Phase 3, we implement basic inheritance checking
    // In a full implementation, this would traverse parent scopes and
    // check for inherited annotations based on inheritance rules

    // Example inheritance rules:
    // 1. Functions can inherit security annotations from their containing struct
    // 2. Methods can inherit optimization annotations from their impl block
    // 3. Statements can inherit certain annotations from their containing function

    switch (node->type) {
    case AST_FUNCTION_DECL:
        // Functions can inherit annotations from module or struct context
        return validate_function_annotation_inheritance(analyzer, node);

    case AST_METHOD_DECL:
        // Methods can inherit from impl block or struct
        return validate_method_annotation_inheritance(analyzer, node);

    case AST_STRUCT_DECL:
        // Structs can inherit module-level annotations
        return validate_struct_annotation_inheritance(analyzer, node);

    default:
        // No special inheritance rules for other node types
        return true;
    }
}

static bool validate_function_annotation_inheritance(SemanticAnalyzer *analyzer,
                                                     ASTNode *func_decl) {
    if (!analyzer || !func_decl) {
        return false;
    }

    // Check for inherited security annotations
    // In a full implementation, this would check parent scopes

    // For now, just validate that the function's annotations are consistent
    // with any inherited context
    return true;
}

static bool validate_method_annotation_inheritance(SemanticAnalyzer *analyzer,
                                                   ASTNode *method_decl) {
    if (!analyzer || !method_decl) {
        return false;
    }

    // Methods can inherit certain annotations from their impl block
    // For example, if an impl block has #[performance_critical],
    // methods might inherit optimization hints

    return true;
}

static bool validate_struct_annotation_inheritance(SemanticAnalyzer *analyzer,
                                                   ASTNode *struct_decl) {
    if (!analyzer || !struct_decl) {
        return false;
    }

    // Structs can inherit module-level memory management or security annotations
    return true;
}

// =============================================================================
// DEPENDENCY VALIDATION FUNCTIONS
// =============================================================================

bool validate_annotation_dependencies(SemanticAnalyzer *analyzer, ASTNode *node,
                                      ASTNodeList *annotations) {
    if (!analyzer || !node) {
        return true; // No annotations means no dependencies to check
    }

    if (!annotations) {
        return true;
    }

    // Check each annotation for its dependencies
    for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
        ASTNode *annotation = ast_node_list_get(annotations, i);
        if (!annotation || annotation->type != AST_SEMANTIC_TAG) {
            continue;
        }

        const char *annotation_name = annotation->data.semantic_tag.name;
        if (!annotation_name) {
            continue;
        }

        if (!validate_single_annotation_dependencies(analyzer, node, annotation_name,
                                                     annotations)) {
            return false;
        }
    }

    return true;
}

static bool validate_single_annotation_dependencies(SemanticAnalyzer *analyzer, ASTNode *node,
                                                    const char *annotation_name,
                                                    ASTNodeList *annotations) {
    if (!analyzer || !node || !annotation_name || !annotations) {
        return false;
    }

    // Define annotation dependencies
    static const struct {
        const char *annotation;
        const char *required_annotation;
        const char *reason;
    } ANNOTATION_DEPENDENCIES[] = {
        {"dll_export", "c_abi", "DLL export requires C ABI compatibility"},
        {"dll_import", "c_abi", "DLL import requires C ABI compatibility"},
        {"cache_friendly", "performance_critical",
         "Cache optimization requires performance critical marking"},
        {"constant_time", "security_critical",
         "Constant time operations should be security critical"},
        {NULL, NULL, NULL}};

    // Check if this annotation has any dependencies
    for (size_t i = 0; ANNOTATION_DEPENDENCIES[i].annotation != NULL; i++) {
        if (strcmp(annotation_name, ANNOTATION_DEPENDENCIES[i].annotation) == 0) {
            const char *required = ANNOTATION_DEPENDENCIES[i].required_annotation;

            // Check if the required annotation is present
            bool found_required = false;
            for (size_t j = 0; j < ast_node_list_size(annotations); j++) {
                ASTNode *check_annotation = ast_node_list_get(annotations, j);
                if (check_annotation && check_annotation->type == AST_SEMANTIC_TAG) {
                    const char *check_name = check_annotation->data.semantic_tag.name;
                    if (check_name && strcmp(check_name, required) == 0) {
                        found_required = true;
                        break;
                    }
                }
            }

            if (!found_required) {
                report_missing_dependency_error(analyzer, node, annotation_name, required,
                                                ANNOTATION_DEPENDENCIES[i].reason);
                return false;
            }
        }
    }

    return true;
}

// =============================================================================
// SCOPE RESOLUTION FUNCTIONS
// =============================================================================

bool check_annotation_scope_resolution(SemanticAnalyzer *analyzer, ASTNode *node,
                                       const char *annotation_name) {
    if (!analyzer || !node || !annotation_name) {
        return false;
    }

    // Check if the annotation is accessible in the current scope
    // This would involve checking module visibility, import statements, etc.

    // For Phase 3, we implement basic visibility checking

    // Check if this is a built-in annotation (always accessible)
    const SemanticTagDefinition *def = find_semantic_tag_definition(annotation_name);
    if (def) {
        return true; // Built-in annotations are always accessible
    }

    // Check for custom annotations (would be defined in imports or modules)
    // For now, we'll assume unknown annotations are errors (handled elsewhere)

    return true;
}
