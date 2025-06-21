/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Error Reporting Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of error reporting and warning systems for annotation validation
 * Split from semantic_annotations.c for better modularity
 */

#include "semantic_annotations_errors.h"
#include "semantic_errors.h"
#include "semantic_utilities.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// ERROR REPORTING FUNCTIONS
// =============================================================================

void report_unknown_annotation_error(SemanticAnalyzer *analyzer, ASTNode *node, const char *annotation_name) {
    if (!analyzer || !node || !annotation_name) {
        return;
    }
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_UNKNOWN_ANNOTATION,
                         node->location,
                         "Unknown semantic annotation '#[%s]'. "
                         "Valid annotations include: non_deterministic, inline, deprecated, etc. "
                         "Check the documentation for a complete list of supported annotations.",
                         annotation_name);
}

void report_parameter_validation_error(SemanticAnalyzer *analyzer, ASTNode *node, 
                                     const char *annotation_name, const char *param_name, 
                                     const char *issue) {
    if (!analyzer || !node || !annotation_name || !param_name || !issue) {
        return;
    }
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION_PARAM,
                         node->location,
                         "Invalid parameter '%s' in annotation '#[%s]': %s",
                         param_name, annotation_name, issue);
}

void report_context_validation_error(SemanticAnalyzer *analyzer, ASTNode *node, 
                                   const char *annotation_name, AnnotationContext expected_context) {
    if (!analyzer || !node || !annotation_name) {
        return;
    }
    
    // Build context description string
    char context_desc[256] = {0};
    bool first = true;
    
    if (expected_context & CONTEXT_FUNCTION) {
        strcat(context_desc, first ? "functions" : ", functions");
        first = false;
    }
    if (expected_context & CONTEXT_STRUCT) {
        strcat(context_desc, first ? "structs" : ", structs");
        first = false;
    }
    if (expected_context & CONTEXT_STATEMENT) {
        strcat(context_desc, first ? "statements" : ", statements");
        first = false;
    }
    if (expected_context & CONTEXT_EXPRESSION) {
        strcat(context_desc, first ? "expressions" : ", expressions");
        first = false;
    }
    if (expected_context & CONTEXT_PARAMETER) {
        strcat(context_desc, first ? "parameters" : ", parameters");
        first = false;
    }
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION_CONTEXT,
                         node->location,
                         "Annotation '#[%s]' cannot be used in this context. "
                         "This annotation is valid for: %s",
                         annotation_name, context_desc);
}

void report_missing_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *node, 
                                                const char *feature_name) {
    if (!analyzer || !node || !feature_name) {
        return;
    }
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_NON_DETERMINISTIC_WITHOUT_ANNOTATION,
                         node->location,
                         "Tier 2 concurrency feature '%s' requires #[non_deterministic] annotation. "
                         "This annotation explicitly marks non-deterministic behavior as per "
                         "Asthra's Pragmatic Concurrency Balance Plan.",
                         feature_name);
}

void report_annotation_conflict_error(SemanticAnalyzer *analyzer, ASTNode *node, 
                                    const AnnotationConflict *conflict) {
    if (!analyzer || !node || !conflict) {
        return;
    }
    
    char error_message[512];
    snprintf(error_message, sizeof(error_message), 
             "Conflicting annotations '%s' and '%s': %s", 
             conflict->annotation1, conflict->annotation2, conflict->resolution_hint);
    
    SemanticErrorCode error_code;
    switch (conflict->type) {
        case CONFLICT_MUTUALLY_EXCLUSIVE:
            error_code = SEMANTIC_ERROR_MUTUALLY_EXCLUSIVE_ANNOTATIONS;
            break;
        case CONFLICT_REDUNDANT:
            error_code = SEMANTIC_ERROR_REDUNDANT_ANNOTATION;
            break;
        case CONFLICT_DEPRECATED_COMBINATION:
            error_code = SEMANTIC_ERROR_DEPRECATED_COMBINATION;
            break;
        default:
            error_code = SEMANTIC_ERROR_CONFLICTING_ANNOTATIONS;
            break;
    }
    
    semantic_report_error(analyzer, error_code, node->location, error_message);
}

void report_duplicate_annotation_error(SemanticAnalyzer *analyzer, ASTNode *node, 
                                     const char *annotation_name) {
    if (!analyzer || !node || !annotation_name) {
        return;
    }
    
    char error_message[256];
    snprintf(error_message, sizeof(error_message), 
             "Duplicate annotation '%s' found on the same declaration", annotation_name);
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_ANNOTATION, node->location, error_message);
}

void report_missing_dependency_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                   const char *annotation_name, const char *required_annotation,
                                   const char *reason) {
    if (!analyzer || !node || !annotation_name || !required_annotation) {
        return;
    }
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION_PARAM,
                         node->location,
                         "Annotation '#[%s]' requires '#[%s]' to be present. %s",
                         annotation_name, required_annotation, reason ? reason : "");
}

// =============================================================================
// WARNING FUNCTIONS
// =============================================================================

void issue_annotation_warning(SemanticAnalyzer *analyzer, ASTNode *node, 
                            WarningSeverity severity, const char *message) {
    if (!analyzer || !node || !message) {
        return;
    }
    
    // Create a warning message with severity prefix
    char warning_message[512];
    const char *severity_prefix;
    
    switch (severity) {
        case WARNING_LOW:
            severity_prefix = "NOTE";
            break;
        case WARNING_MEDIUM:
            severity_prefix = "WARNING";
            break;
        case WARNING_HIGH:
            severity_prefix = "STRONG WARNING";
            break;
        default:
            severity_prefix = "WARNING";
            break;
    }
    
    snprintf(warning_message, sizeof(warning_message), "%s: %s", severity_prefix, message);
    
    // Report as a warning using semantic analyzer warning capability
    semantic_report_warning(analyzer, node->location, warning_message);
} 
