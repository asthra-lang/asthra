/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Registry Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of annotation registry with definitions, schemas, and lookup functions
 * Split from semantic_annotations.c for better modularity
 */

#include "semantic_annotations_registry.h"
#include "../parser/ast_node_list.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// ANNOTATION REGISTRY DATA
// =============================================================================

/**
 * Registry of known semantic annotations with their properties
 */
static const SemanticTagDefinition KNOWN_SEMANTIC_TAGS[] = {
    // Concurrency annotations
    {"non_deterministic", SEMANTIC_CONCURRENCY, false, CONTEXT_FUNCTION | CONTEXT_STATEMENT},
    {"atomic", SEMANTIC_CONCURRENCY, false, CONTEXT_FUNCTION | CONTEXT_STATEMENT},
    {"thread_safe", SEMANTIC_CONCURRENCY, false, CONTEXT_FUNCTION | CONTEXT_STRUCT},
    
    // Optimization annotations
    {"inline", SEMANTIC_OPTIMIZATION, false, CONTEXT_FUNCTION},
    {"no_inline", SEMANTIC_OPTIMIZATION, false, CONTEXT_FUNCTION},
    {"hot", SEMANTIC_OPTIMIZATION, false, CONTEXT_FUNCTION},
    {"cold", SEMANTIC_OPTIMIZATION, false, CONTEXT_FUNCTION},
    {"cache_friendly", SEMANTIC_OPTIMIZATION, true, CONTEXT_FUNCTION | CONTEXT_STRUCT},
    {"performance_critical", SEMANTIC_OPTIMIZATION, false, CONTEXT_FUNCTION},
    
    // Lifecycle annotations
    {"deprecated", SEMANTIC_LIFECYCLE, true, CONTEXT_ANY},
    {"experimental", SEMANTIC_LIFECYCLE, false, CONTEXT_ANY},
    {"stable", SEMANTIC_LIFECYCLE, false, CONTEXT_ANY},
    
    // Security annotations (complement existing SecurityTag)
    {"security_critical", SEMANTIC_SECURITY, false, CONTEXT_FUNCTION},
    {"audit_required", SEMANTIC_SECURITY, false, CONTEXT_FUNCTION},
    
    // Memory annotations (complement existing OwnershipTag)
    {"no_gc", SEMANTIC_MEMORY, false, CONTEXT_FUNCTION | CONTEXT_STRUCT},
    {"stack_only", SEMANTIC_MEMORY, false, CONTEXT_STRUCT},
    
    // FFI annotations (complement existing FFITransferTag)
    {"c_abi", SEMANTIC_FFI, false, CONTEXT_FUNCTION},
    {"dll_export", SEMANTIC_FFI, false, CONTEXT_FUNCTION},
    {"dll_import", SEMANTIC_FFI, true, CONTEXT_FUNCTION},
    
    // FFI Transfer annotations (SafeFFIAnnotation support)
    {"transfer_full", SEMANTIC_FFI, false, CONTEXT_PARAMETER | CONTEXT_RETURN_TYPE},
    {"transfer_none", SEMANTIC_FFI, false, CONTEXT_PARAMETER | CONTEXT_RETURN_TYPE},
    {"borrowed", SEMANTIC_FFI, false, CONTEXT_PARAMETER},
    
    {NULL, 0, false, 0} // Sentinel
};

/**
 * Parameter schemas for annotations that accept parameters
 */
static const ParameterDefinition DEPRECATED_PARAMS[] = {
    {ANNOTATION_PARAM_STRING, true, "message", .validation.string_constraints = {1, 256}},
    {ANNOTATION_PARAM_STRING, false, "since", .validation.string_constraints = {1, 32}},
    {ANNOTATION_PARAM_STRING, false, "replacement", .validation.string_constraints = {1, 128}}
};

static const ParameterDefinition CACHE_FRIENDLY_PARAMS[] = {
    {ANNOTATION_PARAM_INT, false, "level", .validation.int_range = {1, 3}},
    {ANNOTATION_PARAM_IDENT, false, "strategy", .validation.enum_values = {(const char*[]){"temporal", "spatial", "both", NULL}}}
};

static const ParameterDefinition DLL_IMPORT_PARAMS[] = {
    {ANNOTATION_PARAM_STRING, true, "library", .validation.string_constraints = {1, 64}}
};

/**
 * Registry of parameter schemas
 */
static const AnnotationParameterSchema PARAMETER_SCHEMAS[] = {
    {"deprecated", DEPRECATED_PARAMS, 3},
    {"cache_friendly", CACHE_FRIENDLY_PARAMS, 2},
    {"dll_import", DLL_IMPORT_PARAMS, 1},
    {NULL, NULL, 0} // Sentinel
};

/**
 * Registry of known annotation conflicts
 */
static const AnnotationConflict ANNOTATION_CONFLICTS[] = {
    // Optimization conflicts
    {"inline", "no_inline", CONFLICT_MUTUALLY_EXCLUSIVE, "Choose either inline or no_inline, not both"},
    {"hot", "cold", CONFLICT_MUTUALLY_EXCLUSIVE, "Function cannot be both hot and cold"},
    
    // Lifecycle conflicts
    {"deprecated", "experimental", CONFLICT_REDUNDANT, "Deprecated supersedes experimental"},
    {"stable", "experimental", CONFLICT_MUTUALLY_EXCLUSIVE, "Stable and experimental are contradictory"},
    {"stable", "deprecated", CONFLICT_REDUNDANT, "Stable should not be deprecated"},
    
    // Concurrency conflicts
    {"atomic", "non_deterministic", CONFLICT_DEPRECATED_COMBINATION, "Atomic operations with non_deterministic may indicate design issues"},
    
    // Memory conflicts
    {"no_gc", "stack_only", CONFLICT_REDUNDANT, "stack_only implies no_gc"},
    
    // FFI conflicts
    {"dll_export", "dll_import", CONFLICT_MUTUALLY_EXCLUSIVE, "Function cannot be both exported and imported"},
    {"c_abi", "dll_export", CONFLICT_REDUNDANT, "dll_export implies c_abi"},
    {"c_abi", "dll_import", CONFLICT_REDUNDANT, "dll_import implies c_abi"},
    
    // FFI Transfer conflicts (enforced by grammar but validated here for completeness)
    {"transfer_full", "transfer_none", CONFLICT_MUTUALLY_EXCLUSIVE, "Cannot specify both transfer_full and transfer_none"},
    {"transfer_full", "borrowed", CONFLICT_MUTUALLY_EXCLUSIVE, "Cannot specify both transfer_full and borrowed"},
    {"transfer_none", "borrowed", CONFLICT_MUTUALLY_EXCLUSIVE, "Cannot specify both transfer_none and borrowed"},
    
    {NULL, NULL, 0, NULL} // Sentinel
};

// =============================================================================
// REGISTRY ACCESS FUNCTIONS
// =============================================================================

const SemanticTagDefinition *find_semantic_tag_definition(const char *annotation_name) {
    if (!annotation_name) return NULL;
    
    for (size_t i = 0; KNOWN_SEMANTIC_TAGS[i].name != NULL; i++) {
        if (strcmp(KNOWN_SEMANTIC_TAGS[i].name, annotation_name) == 0) {
            return &KNOWN_SEMANTIC_TAGS[i];
        }
    }
    
    return NULL;
}

const AnnotationParameterSchema *find_parameter_schema(const char *annotation_name) {
    if (!annotation_name) return NULL;
    
    for (size_t i = 0; PARAMETER_SCHEMAS[i].annotation_name != NULL; i++) {
        if (strcmp(PARAMETER_SCHEMAS[i].annotation_name, annotation_name) == 0) {
            return &PARAMETER_SCHEMAS[i];
        }
    }
    
    return NULL;
}

const SemanticTagDefinition *get_all_semantic_tag_definitions(void) {
    return KNOWN_SEMANTIC_TAGS;
}

const AnnotationConflict *get_all_annotation_conflicts(void) {
    return ANNOTATION_CONFLICTS;
}

AnnotationContext ast_node_type_to_context(ASTNodeType type) {
    switch (type) {
        case AST_FUNCTION_DECL:
        case AST_EXTERN_DECL:
        case AST_METHOD_DECL:
            return CONTEXT_FUNCTION;
            
        case AST_STRUCT_DECL:
            return CONTEXT_STRUCT;
            
        case AST_LET_STMT:
        case AST_RETURN_STMT:
        case AST_IF_STMT:
        case AST_FOR_STMT:
        case AST_MATCH_STMT:
        case AST_SPAWN_STMT:
        case AST_SPAWN_WITH_HANDLE_STMT:
        case AST_EXPR_STMT:
        case AST_BLOCK:
        case AST_IF_LET_STMT:
        case AST_UNSAFE_BLOCK:
            return CONTEXT_STATEMENT;
            
        case AST_CALL_EXPR:
        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR:
        case AST_POSTFIX_EXPR:
        case AST_FIELD_ACCESS:
        case AST_INDEX_ACCESS:
        case AST_ASSIGNMENT:
        case AST_AWAIT_EXPR:
            return CONTEXT_EXPRESSION;
            
        case AST_PARAM_DECL:
            return CONTEXT_PARAMETER;
            
        default:
            return CONTEXT_ANY;
    }
}

ASTNodeList *get_node_annotations(ASTNode *node) {
    if (!node) {
        return NULL;
    }
    
    
    switch (node->type) {
        case AST_FUNCTION_DECL:
            return node->data.function_decl.annotations;
        case AST_STRUCT_DECL:
            return node->data.struct_decl.annotations;
        case AST_EXTERN_DECL:
            return node->data.extern_decl.annotations;
        case AST_PARAM_DECL:
            return node->data.param_decl.annotations;
        case AST_METHOD_DECL:
            return node->data.method_decl.annotations;
        case AST_IMPL_BLOCK:
            return node->data.impl_block.annotations;
        default:
            return NULL;
    }
} 
