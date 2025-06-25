/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Error Reporting Module
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Error reporting and warning systems for annotation validation
 * Split from semantic_annotations.c for better modularity
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_ERRORS_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_ERRORS_H

#include "../parser/ast.h"
#include "semantic_annotations_registry.h"
#include "semantic_core.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// WARNING SYSTEM
// =============================================================================

/**
 * Warning severity levels
 */
typedef enum {
    WARNING_LOW,    // Minor issue, suggestion only
    WARNING_MEDIUM, // Potential problem, should be addressed
    WARNING_HIGH    // Likely problem, strongly recommend fixing
} WarningSeverity;

// =============================================================================
// ERROR REPORTING FUNCTIONS
// =============================================================================

/**
 * Report unknown annotation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the error occurred
 * @param annotation_name The unknown annotation name
 */
void report_unknown_annotation_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                     const char *annotation_name);

/**
 * Report parameter validation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the error occurred
 * @param annotation_name The annotation name
 * @param param_name The parameter name that failed validation
 * @param issue Description of the validation issue
 */
void report_parameter_validation_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                       const char *annotation_name, const char *param_name,
                                       const char *issue);

/**
 * Report context validation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the error occurred
 * @param annotation_name The annotation name
 * @param expected_context The expected annotation context
 */
void report_context_validation_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                     const char *annotation_name,
                                     AnnotationContext expected_context);

/**
 * Report missing non-deterministic annotation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node that requires the annotation
 * @param feature_name The name of the feature requiring the annotation
 */
void report_missing_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *node,
                                                 const char *feature_name);

/**
 * Report annotation conflict error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the conflict occurred
 * @param conflict The conflict definition
 */
void report_annotation_conflict_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                      const AnnotationConflict *conflict);

/**
 * Report duplicate annotation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the duplicate was found
 * @param annotation_name The duplicated annotation name
 */
void report_duplicate_annotation_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                       const char *annotation_name);

/**
 * Report missing annotation dependency error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the dependency is missing
 * @param annotation_name The annotation that has dependencies
 * @param required_annotation The required annotation that is missing
 * @param reason The reason why the dependency is required
 */
void report_missing_dependency_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                     const char *annotation_name, const char *required_annotation,
                                     const char *reason);

// =============================================================================
// WARNING FUNCTIONS
// =============================================================================

/**
 * Issue annotation warning with specified severity
 * @param analyzer The semantic analyzer instance
 * @param node The AST node where the warning applies
 * @param severity The warning severity level
 * @param message The warning message
 */
void issue_annotation_warning(SemanticAnalyzer *analyzer, ASTNode *node, WarningSeverity severity,
                              const char *message);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_ERRORS_H
