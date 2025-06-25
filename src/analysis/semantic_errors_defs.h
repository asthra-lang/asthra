/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Error Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Error codes, error structures, and statistics definitions
 */

#ifndef ASTHRA_SEMANTIC_ERRORS_DEFS_H
#define ASTHRA_SEMANTIC_ERRORS_DEFS_H

#include "../parser/ast.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SEMANTIC ERROR HANDLING
// =============================================================================

// Semantic error codes
typedef enum {
    SEMANTIC_ERROR_NONE,                   // No error
    SEMANTIC_ERROR_UNDEFINED_SYMBOL,       // Symbol not found in scope
    SEMANTIC_ERROR_DUPLICATE_SYMBOL,       // Symbol already defined in this scope
    SEMANTIC_ERROR_INCOMPATIBLE_TYPES,     // Type mismatch in operation
    SEMANTIC_ERROR_INVALID_TYPE,           // Invalid type for operation
    SEMANTIC_ERROR_INVALID_OPERATION,      // Invalid operation (e.g. arithmetic on non-numeric)
    SEMANTIC_ERROR_INVALID_ARGUMENTS,      // Invalid argument types for function call
    SEMANTIC_ERROR_INVALID_RETURN,         // Invalid return type for function
    SEMANTIC_ERROR_MISMATCHED_TYPES,       // Mismatched types in binary operation
    SEMANTIC_ERROR_NOT_CALLABLE,           // Trying to call a non-function
    SEMANTIC_ERROR_IMMUTABLE_MODIFICATION, // Trying to modify an immutable value
    SEMANTIC_ERROR_INVALID_MODULE,         // Invalid module or missing export
    SEMANTIC_ERROR_SHADOWED_SYMBOL,        // Symbol shadows an existing symbol (warning)
    SEMANTIC_ERROR_UNUSED_SYMBOL,          // Unused symbol (warning)
    SEMANTIC_ERROR_UNINITIALIZED_VARIABLE, // Variable used before initialization
    SEMANTIC_ERROR_CIRCULAR_DEPENDENCY,    // Circular dependency detected
    SEMANTIC_ERROR_INVALID_DECLARATION,    // Invalid declaration structure
    SEMANTIC_ERROR_MEMORY_ALLOCATION,      // Memory allocation failure
    SEMANTIC_ERROR_SYMBOL_TABLE,           // Symbol table operation failure
    SEMANTIC_ERROR_INVALID_EXPRESSION,     // Invalid expression

    // Type-specific errors
    SEMANTIC_ERROR_INVALID_STRUCT_FIELD,  // Invalid field for struct type
    SEMANTIC_ERROR_UNDEFINED_TYPE,        // Undefined type (added back)
    SEMANTIC_ERROR_TYPE_MISMATCH,         // Type mismatch (added back)
    SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, // Failed to infer type
    SEMANTIC_ERROR_UNSUPPORTED_OPERATION, // Unsupported operation
    SEMANTIC_ERROR_DIVISION_BY_ZERO,      // Division by zero

    // Literal validation errors (Phase 4: Character Literals)
    SEMANTIC_ERROR_INVALID_LITERAL,          // Invalid literal value
    SEMANTIC_ERROR_TYPE_ANNOTATION_REQUIRED, // Type annotation required for literal
    SEMANTIC_ERROR_INTERNAL,                 // Internal semantic analyzer error

    // Pattern matching errors
    SEMANTIC_ERROR_NON_EXHAUSTIVE_MATCH, // Match is not exhaustive

    // Memory management errors
    SEMANTIC_ERROR_INVALID_OWNERSHIP, // Invalid ownership annotation
    SEMANTIC_ERROR_INVALID_LIFETIME,  // Invalid lifetime annotation

    // Security annotation errors
    SEMANTIC_ERROR_INVALID_ANNOTATION,         // Invalid annotation type or value
    SEMANTIC_ERROR_INVALID_ANNOTATION_CONTEXT, // Annotation used in wrong context
    SEMANTIC_ERROR_CONFLICTING_ANNOTATIONS,    // Conflicting annotations on same declaration
    SEMANTIC_ERROR_SECURITY_VIOLATION,         // Security constraint violation
    SEMANTIC_ERROR_MISSING_ANNOTATION, // Missing required annotation (e.g. #[non_deterministic])

    // Phase 1: Enhanced semantic annotation errors
    SEMANTIC_ERROR_UNKNOWN_ANNOTATION,                   // Unknown semantic annotation
    SEMANTIC_ERROR_INVALID_ANNOTATION_PARAM,             // Invalid annotation parameter
    SEMANTIC_ERROR_MISSING_REQUIRED_PARAM,               // Missing required parameter
    SEMANTIC_ERROR_PARAM_TYPE_MISMATCH,                  // Parameter type mismatch
    SEMANTIC_ERROR_PARAM_VALUE_OUT_OF_RANGE,             // Parameter value out of range
    SEMANTIC_ERROR_NON_DETERMINISTIC_WITHOUT_ANNOTATION, // Non-deterministic operation without
                                                         // annotation

    // Phase 2: Advanced validation errors
    SEMANTIC_ERROR_DUPLICATE_ANNOTATION,           // Duplicate annotation on same node
    SEMANTIC_ERROR_MUTUALLY_EXCLUSIVE_ANNOTATIONS, // Mutually exclusive annotations
    SEMANTIC_ERROR_REDUNDANT_ANNOTATION,           // Redundant annotation combination
    SEMANTIC_ERROR_DEPRECATED_COMBINATION,         // Deprecated annotation combination

    SEMANTIC_ERROR_COUNT
} SemanticErrorCode;

// Semantic error information
typedef struct SemanticError {
    SemanticErrorCode code;
    SourceLocation location;
    char *message;
    ASTNode *node;
    struct SemanticError *next;
} SemanticError;

// =============================================================================
// SEMANTIC ANALYSIS STATISTICS WITH ATOMICS
// =============================================================================

typedef struct SemanticStatistics {
    atomic_uint_fast64_t nodes_analyzed;
    atomic_uint_fast64_t types_checked;
    atomic_uint_fast64_t symbols_resolved;
    atomic_uint_fast64_t errors_found;
    atomic_uint_fast64_t warnings_issued;
    atomic_uint_fast32_t max_scope_depth;
    atomic_uint_fast32_t current_scope_depth;
} SemanticStatistics;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ERRORS_DEFS_H
