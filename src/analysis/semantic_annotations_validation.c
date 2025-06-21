/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Validation Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of core validation functions for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#include "semantic_annotations_validation.h"
#include "semantic_annotations_errors.h"
#include "semantic_annotations_parameters.h"
#include "semantic_annotations_conflicts.h"
#include "semantic_annotations_inheritance.h"
#include "semantic_annotations_concurrency.h"
#include "semantic_annotations_ffi.h"
#include "../parser/ast_node_list.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// CORE VALIDATION FUNCTIONS
// =============================================================================

bool analyze_semantic_tag(SemanticAnalyzer *analyzer, ASTNode *tag_node) {
    if (!analyzer || !tag_node || tag_node->type != AST_SEMANTIC_TAG) {
        return false;
    }
    
    const char *annotation_name = tag_node->data.semantic_tag.name;
    if (!annotation_name) {
        return false;
    }
    
    // Find annotation definition
    const SemanticTagDefinition *def = find_semantic_tag_definition(annotation_name);
    if (!def) {
        report_unknown_annotation_error(analyzer, tag_node, annotation_name);
        return false;
    }
    
    // Validate parameters if annotation requires them
    if (def->requires_params) {
        const AnnotationParameterSchema *schema = find_parameter_schema(annotation_name);
        if (schema && !validate_annotation_parameters(analyzer, tag_node, schema)) {
            return false;
        }
    }
    
    // Note: Context validation is performed when the annotation is attached to a target node
    // This function only validates the annotation structure itself
    
    return true;
}

bool validate_annotation_context(SemanticAnalyzer *analyzer, const char *annotation_name, ASTNode *target_node) {
    if (!analyzer || !annotation_name || !target_node) {
        return false;
    }
    
    // Special handling for FFI transfer annotations
    if (is_ffi_transfer_annotation(annotation_name)) {
        AnnotationContext context = get_ffi_annotation_context(target_node);
        return validate_ffi_transfer_annotation_context(analyzer, annotation_name, target_node, context);
    }
    
    const SemanticTagDefinition *def = find_semantic_tag_definition(annotation_name);
    if (!def) {
        report_unknown_annotation_error(analyzer, target_node, annotation_name);
        return false;
    }
    
    AnnotationContext node_context = ast_node_type_to_context(target_node->type);
    if ((def->valid_contexts & node_context) == 0) {
        report_context_validation_error(analyzer, target_node, annotation_name, def->valid_contexts);
        return false;
    }
    
    return true;
}

bool validate_annotation_context_with_inheritance(SemanticAnalyzer *analyzer, const char *annotation_name, 
                                                ASTNode *target_node, bool allow_inheritance) {
    if (!analyzer || !annotation_name || !target_node) {
        return false;
    }
    
    // First check direct context validation
    if (validate_annotation_context(analyzer, annotation_name, target_node)) {
        return true;
    }
    
    // If inheritance is allowed, perform relaxed context checking
    // Note: Without parent pointers in AST, we do relaxed validation for certain contexts
    if (allow_inheritance) {
        AnnotationContext target_context = ast_node_type_to_context(target_node->type);
        const SemanticTagDefinition *def = find_semantic_tag_definition(annotation_name);
        
        if (def) {
            // Allow some context flexibility for common cases
            if (target_context == CONTEXT_STATEMENT && (def->valid_contexts & CONTEXT_FUNCTION)) {
                issue_annotation_warning(analyzer, target_node, WARNING_LOW, 
                    "Annotation applied to statement context, typically used in function context");
                return true;
            }
            
            if (target_context == CONTEXT_EXPRESSION && (def->valid_contexts & CONTEXT_STATEMENT)) {
                issue_annotation_warning(analyzer, target_node, WARNING_LOW, 
                    "Annotation applied to expression context, typically used in statement context");
                return true;
            }
        }
    }
    
    return false;
}

bool has_non_deterministic_annotation(ASTNode *node) {
    if (!node) {
        return false;
    }
    
    // Get the annotations list for this node
    ASTNodeList *annotations = get_node_annotations(node);
    if (!annotations) {
        return false;
    }
    
    // Search for non_deterministic annotation
    for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
        ASTNode *annotation = ast_node_list_get(annotations, i);
        if (annotation && annotation->type == AST_SEMANTIC_TAG) {
            const char *name = annotation->data.semantic_tag.name;
            if (name && strcmp(name, "non_deterministic") == 0) {
                return true;
            }
        }
    }
    
    return false;
}

// =============================================================================
// ANNOTATION ANALYSIS INTEGRATION
// =============================================================================

bool analyze_declaration_annotations(SemanticAnalyzer *analyzer, ASTNode *decl) {
    
    if (!analyzer || !decl) {
        return false;
    }
    
    ASTNodeList *annotations = get_node_annotations(decl);
    
    if (!annotations) {
        return true; // No annotations to validate
    }
    
    // Check for duplicate annotations
    if (!check_duplicate_annotations(analyzer, decl, annotations)) {
        return false;
    }
    
    // Check for annotation conflicts
    if (!check_annotation_conflicts(analyzer, decl, annotations)) {
        return false;
    }
    
    // Validate each annotation
    for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
        ASTNode *annotation = ast_node_list_get(annotations, i);
        if (!annotation) continue;
        
        if (annotation->type == AST_SEMANTIC_TAG) {
            // Validate the annotation itself
            if (!analyze_semantic_tag(analyzer, annotation)) {
                return false;
            }
            
            // Enhanced context validation with inheritance
            const char *annotation_name = annotation->data.semantic_tag.name;
            if (annotation_name && !validate_annotation_context_with_inheritance(analyzer, annotation_name, decl, true)) {
                return false;
            }
        }
    }
    
    // Check annotation inheritance and dependencies
    if (!analyze_annotation_inheritance(analyzer, decl)) {
        return false;
    }
    
    if (!validate_annotation_dependencies(analyzer, decl, annotations)) {
        return false;
    }
    
    // Validate FFI annotations for function and extern declarations
    bool ffi_valid = true;
    if (decl->type == AST_FUNCTION_DECL) {
        ffi_valid = validate_function_ffi_annotations(analyzer, decl);
    } else if (decl->type == AST_EXTERN_DECL) {
        ffi_valid = validate_extern_ffi_annotations(analyzer, decl);
    }
    
    // Also validate Tier 2 concurrency annotations
    bool concurrency_valid = validate_tier2_concurrency_annotation(analyzer, decl);
    
    return ffi_valid && concurrency_valid;
}

bool analyze_statement_annotations(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt) {
        return false;
    }
    
    // For now, statements don't typically have annotations in our AST structure
    // But we still validate Tier 2 concurrency annotations
    return validate_tier2_concurrency_annotation(analyzer, stmt);
}

bool analyze_expression_annotations(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return false;
    }
    
    // For now, expressions don't typically have annotations in our AST structure
    // But we still validate Tier 2 concurrency annotations
    return validate_tier2_concurrency_annotation(analyzer, expr);
} 
