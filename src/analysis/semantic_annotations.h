/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Module
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Semantic validation for annotations including #[non_deterministic] support
 * as part of Phase 4: Semantic Analysis Updates for Pragmatic Concurrency Balance Plan
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_H

#include "semantic_core.h"
#include "../parser/ast.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ANNOTATION TYPE DEFINITIONS (Phase 1)
// =============================================================================

/**
 * Categories of semantic annotations for organization and validation
 */
typedef enum {
    SEMANTIC_CONCURRENCY,    // Concurrency-related annotations
    SEMANTIC_OPTIMIZATION,   // Optimization hints
    SEMANTIC_LIFECYCLE,      // Lifecycle annotations (deprecated, experimental, etc.)
    SEMANTIC_SECURITY,       // Security-related annotations
    SEMANTIC_MEMORY,         // Memory management annotations
    SEMANTIC_FFI            // Foreign Function Interface annotations
} SemanticTagCategory;

/**
 * Valid contexts where annotations can be applied
 */
typedef enum {
    CONTEXT_FUNCTION    = 1 << 0,   // Functions and methods
    CONTEXT_STRUCT      = 1 << 1,   // Struct definitions
    CONTEXT_STATEMENT   = 1 << 2,   // Statements
    CONTEXT_EXPRESSION  = 1 << 3,   // Expressions
    CONTEXT_PARAMETER   = 1 << 4,   // Function parameters
    CONTEXT_RETURN_TYPE = 1 << 5,   // Return types
    CONTEXT_ANY         = 0xFF      // Any context
} AnnotationContext;

/**
 * Types of annotation parameter values
 */
typedef enum {
    ANNOTATION_PARAM_STRING,    // String literal parameter
    ANNOTATION_PARAM_IDENT,     // Identifier parameter
    ANNOTATION_PARAM_INT,       // Integer parameter
    ANNOTATION_PARAM_BOOL       // Boolean parameter
} AnnotationParamType;

/**
 * Parameter definition for annotation validation
 */
typedef struct {
    AnnotationParamType type;
    bool required;
    const char *name;
    union {
        struct { int min, max; } int_range;
        struct { const char **values; } enum_values;
        struct { int min_len, max_len; } string_constraints;
    } validation;
} ParameterDefinition;

/**
 * Parameter schema for annotation validation
 */
typedef struct {
    const char *annotation_name;
    ParameterDefinition *parameters;
    size_t parameter_count;
} AnnotationParameterSchema;

/**
 * Definition of a known semantic annotation
 */
typedef struct {
    const char *name;
    SemanticTagCategory category;
    bool requires_params;
    AnnotationContext valid_contexts;
} SemanticTagDefinition;

/**
 * Types of annotation conflicts (Phase 2)
 */
typedef enum {
    CONFLICT_MUTUALLY_EXCLUSIVE,    // Annotations cannot coexist
    CONFLICT_REDUNDANT,             // One annotation supersedes another
    CONFLICT_DEPRECATED_COMBINATION // Combination is deprecated
} ConflictType;

/**
 * Definition of annotation conflicts (Phase 2)
 */
typedef struct {
    const char *annotation1;
    const char *annotation2;
    ConflictType type;
    const char *resolution_hint;
} AnnotationConflict;

/**
 * Warning severity levels (Phase 2)
 */
typedef enum {
    WARNING_LOW,        // Minor issue, suggestion only
    WARNING_MEDIUM,     // Potential problem, should be addressed
    WARNING_HIGH        // Likely problem, strongly recommend fixing
} WarningSeverity;

// =============================================================================
// ANNOTATION VALIDATION FUNCTIONS
// =============================================================================

/**
 * Analyze a semantic tag annotation
 * @param analyzer The semantic analyzer instance
 * @param tag_node The AST node representing the semantic tag
 * @return true if the annotation is valid, false otherwise
 */
bool analyze_semantic_tag(SemanticAnalyzer *analyzer, ASTNode *tag_node);

/**
 * Validate non-deterministic annotation
 * @param analyzer The semantic analyzer instance
 * @param tag_node The AST node representing the semantic tag
 * @param target_node The AST node that is annotated (function, statement, etc.)
 * @return true if the annotation is valid and properly applied, false otherwise
 */
bool validate_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *tag_node, ASTNode *target_node);

/**
 * Check if a node requires non-deterministic annotation
 * @param analyzer The semantic analyzer instance
 * @param node The AST node to check
 * @return true if the node requires #[non_deterministic] annotation, false otherwise
 */
bool requires_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Check if a node has non-deterministic annotation
 * @param node The AST node to check
 * @return true if the node has #[non_deterministic] annotation, false otherwise
 */
bool has_non_deterministic_annotation(ASTNode *node);

/**
 * Validate that Tier 2 concurrency features have non-deterministic annotation
 * @param analyzer The semantic analyzer instance
 * @param node The AST node to validate
 * @return true if validation passes, false if annotation is missing or invalid
 */
bool validate_tier2_concurrency_annotation(SemanticAnalyzer *analyzer, ASTNode *node);

// =============================================================================
// PHASE 1: ENHANCED VALIDATION FUNCTIONS
// =============================================================================

/**
 * Validate annotation parameters according to schema
 * @param analyzer The semantic analyzer instance
 * @param tag_node The semantic tag node with parameters
 * @param schema The parameter schema to validate against
 * @return true if parameters are valid, false otherwise
 */
bool validate_annotation_parameters(SemanticAnalyzer *analyzer, ASTNode *tag_node, const AnnotationParameterSchema *schema);

/**
 * Validate annotation context appropriateness
 * @param analyzer The semantic analyzer instance
 * @param annotation_name The name of the annotation
 * @param target_node The node being annotated
 * @return true if context is appropriate, false otherwise
 */
bool validate_annotation_context(SemanticAnalyzer *analyzer, const char *annotation_name, ASTNode *target_node);

/**
 * Find semantic tag definition by name
 * @param annotation_name The name of the annotation to find
 * @return Pointer to definition if found, NULL otherwise
 */
const SemanticTagDefinition *find_semantic_tag_definition(const char *annotation_name);

/**
 * Find parameter schema for annotation
 * @param annotation_name The name of the annotation
 * @return Pointer to schema if found, NULL otherwise
 */
const AnnotationParameterSchema *find_parameter_schema(const char *annotation_name);

/**
 * Convert AST node type to annotation context
 * @param node_type The AST node type
 * @return The corresponding annotation context flags
 */
AnnotationContext ast_node_type_to_context(ASTNodeType node_type);

/**
 * Get annotations list from AST node
 * @param node The AST node to check
 * @return Pointer to annotations list if present, NULL otherwise
 */
ASTNodeList *get_node_annotations(ASTNode *node);

// =============================================================================
// ENHANCED ERROR REPORTING FUNCTIONS (Phase 1)
// =============================================================================

/**
 * Report unknown annotation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with the unknown annotation
 * @param annotation_name The unknown annotation name
 */
void report_unknown_annotation_error(SemanticAnalyzer *analyzer, ASTNode *node, const char *annotation_name);

/**
 * Report parameter validation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with invalid parameters
 * @param annotation_name The annotation name
 * @param param_name The parameter name with issues
 * @param issue Description of the validation issue
 */
void report_parameter_validation_error(SemanticAnalyzer *analyzer, ASTNode *node, const char *annotation_name, const char *param_name, const char *issue);

/**
 * Report context validation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with inappropriate annotation
 * @param annotation_name The annotation name
 * @param expected_context The expected context for this annotation
 */
void report_context_validation_error(SemanticAnalyzer *analyzer, ASTNode *node, const char *annotation_name, AnnotationContext expected_context);

/**
 * Report missing non-deterministic annotation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node missing the annotation
 * @param feature_name The name of the feature requiring annotation
 */
void report_missing_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *node, const char *feature_name);

// =============================================================================
// PHASE 2: ADVANCED VALIDATION FUNCTIONS
// =============================================================================

/**
 * Check for annotation conflicts in a list of annotations
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with annotations
 * @param annotations The list of annotations to check
 * @return true if no conflicts found, false if conflicts detected
 */
bool check_annotation_conflicts(SemanticAnalyzer *analyzer, ASTNode *node, ASTNodeList *annotations);

/**
 * Check for duplicate annotations in a list
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with annotations
 * @param annotations The list of annotations to check
 * @return true if no duplicates found, false if duplicates detected
 */
bool check_duplicate_annotations(SemanticAnalyzer *analyzer, ASTNode *node, ASTNodeList *annotations);

/**
 * Validate annotation context with inheritance rules
 * @param analyzer The semantic analyzer instance
 * @param annotation_name The name of the annotation
 * @param target_node The node being annotated
 * @param allow_inheritance Whether to check parent contexts
 * @return true if context is appropriate, false otherwise
 */
bool validate_annotation_context_with_inheritance(SemanticAnalyzer *analyzer, const char *annotation_name, ASTNode *target_node, bool allow_inheritance);

/**
 * Issue a warning for non-critical annotation issues
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with the issue
 * @param severity The warning severity level
 * @param message The warning message
 */
void issue_annotation_warning(SemanticAnalyzer *analyzer, ASTNode *node, WarningSeverity severity, const char *message);

// =============================================================================
// PHASE 2: ENHANCED ERROR REPORTING FUNCTIONS
// =============================================================================

/**
 * Report annotation conflict error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with conflicting annotations
 * @param conflict The conflict definition
 */
void report_annotation_conflict_error(SemanticAnalyzer *analyzer, ASTNode *node, const AnnotationConflict *conflict);

/**
 * Report duplicate annotation error
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with duplicate annotation
 * @param annotation_name The duplicated annotation name
 */
void report_duplicate_annotation_error(SemanticAnalyzer *analyzer, ASTNode *node, const char *annotation_name);

// =============================================================================
// ANNOTATION ANALYSIS INTEGRATION
// =============================================================================

/**
 * Analyze all annotations on a declaration
 * @param analyzer The semantic analyzer instance
 * @param decl The declaration node with potential annotations
 * @return true if all annotations are valid, false otherwise
 */
bool analyze_declaration_annotations(SemanticAnalyzer *analyzer, ASTNode *decl);

/**
 * Analyze all annotations on a statement
 * @param analyzer The semantic analyzer instance
 * @param stmt The statement node with potential annotations
 * @return true if all annotations are valid, false otherwise
 */
bool analyze_statement_annotations(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Analyze all annotations on an expression
 * @param analyzer The semantic analyzer instance
 * @param expr The expression node with potential annotations
 * @return true if all annotations are valid, false otherwise
 */
bool analyze_expression_annotations(SemanticAnalyzer *analyzer, ASTNode *expr);

/**
 * Analyze Tier 1 concurrency features (spawn, spawn_with_handle, await)
 * @param analyzer The semantic analyzer instance
 * @param node The AST node representing the concurrency feature
 * @return true if analysis passes, false otherwise
 */
bool analyze_tier1_concurrency_feature(SemanticAnalyzer *analyzer, ASTNode *node);

// =============================================================================
// PHASE 3: ADVANCED FEATURES FUNCTION DECLARATIONS
// =============================================================================

/**
 * Perform comprehensive annotation inheritance analysis
 * Checks for inherited annotations from parent scopes
 * @param analyzer The semantic analyzer instance
 * @param node The AST node to analyze
 * @return true if inheritance analysis passes, false otherwise
 */
bool analyze_annotation_inheritance(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Validate annotation dependencies and requirements
 * Ensures required annotation combinations are present
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with annotations
 * @param annotations The list of annotations to validate
 * @return true if all dependencies are satisfied, false otherwise
 */
bool validate_annotation_dependencies(SemanticAnalyzer *analyzer, ASTNode *node, ASTNodeList *annotations);

/**
 * Check for annotation scope resolution
 * Validates annotation visibility and accessibility
 * @param analyzer The semantic analyzer instance
 * @param node The AST node with the annotation
 * @param annotation_name The name of the annotation to check
 * @return true if annotation is accessible, false otherwise
 */
bool check_annotation_scope_resolution(SemanticAnalyzer *analyzer, ASTNode *node, const char *annotation_name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_H
