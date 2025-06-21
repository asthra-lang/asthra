/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Registry Module
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Registry of known semantic annotations, parameter schemas, and conflict definitions
 * Split from semantic_annotations.c for better modularity
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_REGISTRY_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_REGISTRY_H

#include "semantic_core.h"
#include "../parser/ast.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ANNOTATION TYPE DEFINITIONS
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
    const ParameterDefinition *parameters;
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
 * Types of annotation conflicts
 */
typedef enum {
    CONFLICT_MUTUALLY_EXCLUSIVE,    // Annotations cannot coexist
    CONFLICT_REDUNDANT,             // One annotation supersedes another
    CONFLICT_DEPRECATED_COMBINATION // Combination is deprecated
} ConflictType;

/**
 * Definition of annotation conflicts
 */
typedef struct {
    const char *annotation1;
    const char *annotation2;
    ConflictType type;
    const char *resolution_hint;
} AnnotationConflict;

// =============================================================================
// REGISTRY ACCESS FUNCTIONS
// =============================================================================

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
 * Get all known semantic tag definitions
 * @return Pointer to array of definitions (null-terminated)
 */
const SemanticTagDefinition *get_all_semantic_tag_definitions(void);

/**
 * Get all annotation conflict definitions
 * @return Pointer to array of conflicts (null-terminated)
 */
const AnnotationConflict *get_all_annotation_conflicts(void);

/**
 * Convert AST node type to annotation context
 * @param type The AST node type
 * @return The corresponding annotation context
 */
AnnotationContext ast_node_type_to_context(ASTNodeType type);

/**
 * Get annotations list from an AST node
 * @param node The AST node
 * @return Pointer to annotations list, or NULL if none
 */
ASTNodeList *get_node_annotations(ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_REGISTRY_H 
