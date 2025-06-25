/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Error Handling Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Error reporting, management, and utilities
 */

#include "semantic_errors.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// C17 feature detection and compatibility
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
#define ASTHRA_HAS_C17 1
#else
#define ASTHRA_HAS_C17 0
#endif

#if ASTHRA_HAS_C17 && !defined(__STDC_NO_ATOMICS__)
#define ASTHRA_HAS_ATOMICS 1
#else
#define ASTHRA_HAS_ATOMICS 0
#endif

// Atomic operation wrappers with fallbacks
#if ASTHRA_HAS_ATOMICS
#define ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
#else
#define ATOMIC_FETCH_ADD(ptr, val) __sync_fetch_and_add(ptr, val)
#endif

// =============================================================================
// ERROR REPORTING
// =============================================================================

void semantic_report_error(SemanticAnalyzer *analyzer, SemanticErrorCode code,
                           SourceLocation location, const char *format, ...) {
    if (!analyzer || !format)
        return;

    if (analyzer->error_count >= analyzer->max_errors)
        return;

    SemanticError *error = malloc(sizeof(SemanticError));
    if (!error)
        return;

    // Format message
    va_list args;
    va_start(args, format);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    int len = vsnprintf(NULL, 0, format, args);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
    va_end(args);

    error->message = malloc((size_t)(len + 1));
    if (!error->message) {
        free(error);
        return;
    }

    va_start(args, format);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    vsnprintf(error->message, len + 1, format, args);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
    va_end(args);

    error->code = code;
    error->location = location;
    error->node = NULL;
    error->next = NULL;

    // Add to error list
    if (analyzer->last_error) {
        analyzer->last_error->next = error;
    } else {
        analyzer->errors = error;
    }
    analyzer->last_error = error;
    analyzer->error_count++;

    ATOMIC_FETCH_ADD(&analyzer->stats.errors_found, 1);
}

void semantic_report_warning(SemanticAnalyzer *analyzer, SourceLocation location,
                             const char *format, ...) {
    if (!analyzer || !format)
        return;

    // Check if warnings are enabled
    if (!analyzer->config.enable_warnings)
        return;

    // Format message
    va_list args;
    va_start(args, format);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    int len = vsnprintf(NULL, 0, format, args);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
    va_end(args);

    char *message = malloc((size_t)(len + 1));
    if (!message)
        return;

    va_start(args, format);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    vsnprintf(message, len + 1, format, args);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
    va_end(args);

    // Print warning to stderr
    fprintf(stderr, "Warning at %s:%d:%d: %s\n",
            location.filename ? location.filename : "<unknown>", location.line, location.column,
            message);

    free(message);

    ATOMIC_FETCH_ADD(&analyzer->stats.warnings_issued, 1);
}

void semantic_clear_errors(SemanticAnalyzer *analyzer) {
    if (!analyzer)
        return;

    SemanticError *error = analyzer->errors;
    while (error) {
        SemanticError *next = error->next;
        free(error->message);
        free(error);
        error = next;
    }

    analyzer->errors = NULL;
    analyzer->last_error = NULL;
    analyzer->error_count = 0;
}

const SemanticError *semantic_get_errors(const SemanticAnalyzer *analyzer) {
    return analyzer ? analyzer->errors : NULL;
}

size_t semantic_get_error_count(const SemanticAnalyzer *analyzer) {
    return analyzer ? analyzer->error_count : 0;
}

// =============================================================================
// ERROR UTILITIES
// =============================================================================

const char *semantic_error_code_name(SemanticErrorCode code) {
    switch (code) {
    case SEMANTIC_ERROR_NONE:
        return "NONE";
    case SEMANTIC_ERROR_TYPE_MISMATCH:
        return "TYPE_MISMATCH";
    case SEMANTIC_ERROR_UNDEFINED_TYPE:
        return "UNDEFINED_TYPE";
    case SEMANTIC_ERROR_UNDEFINED_SYMBOL:
        return "UNDEFINED_SYMBOL";
    case SEMANTIC_ERROR_DUPLICATE_SYMBOL:
        return "DUPLICATE_SYMBOL";
    case SEMANTIC_ERROR_INCOMPATIBLE_TYPES:
        return "INCOMPATIBLE_TYPES";
    case SEMANTIC_ERROR_INVALID_TYPE:
        return "INVALID_TYPE";
    case SEMANTIC_ERROR_INVALID_OPERATION:
        return "INVALID_OPERATION";
    case SEMANTIC_ERROR_INVALID_ARGUMENTS:
        return "INVALID_ARGUMENTS";
    case SEMANTIC_ERROR_INVALID_RETURN:
        return "INVALID_RETURN";
    case SEMANTIC_ERROR_MISMATCHED_TYPES:
        return "MISMATCHED_TYPES";
    case SEMANTIC_ERROR_NOT_CALLABLE:
        return "NOT_CALLABLE";
    case SEMANTIC_ERROR_IMMUTABLE_MODIFICATION:
        return "IMMUTABLE_MODIFICATION";
    case SEMANTIC_ERROR_INVALID_MODULE:
        return "INVALID_MODULE";
    case SEMANTIC_ERROR_SHADOWED_SYMBOL:
        return "SHADOWED_SYMBOL";
    case SEMANTIC_ERROR_UNUSED_SYMBOL:
        return "UNUSED_SYMBOL";
    case SEMANTIC_ERROR_UNINITIALIZED_VARIABLE:
        return "UNINITIALIZED_VARIABLE";
    case SEMANTIC_ERROR_CIRCULAR_DEPENDENCY:
        return "CIRCULAR_DEPENDENCY";
    case SEMANTIC_ERROR_INVALID_STRUCT_FIELD:
        return "INVALID_STRUCT_FIELD";
    case SEMANTIC_ERROR_NON_EXHAUSTIVE_MATCH:
        return "NON_EXHAUSTIVE_MATCH";
    case SEMANTIC_ERROR_INVALID_OWNERSHIP:
        return "INVALID_OWNERSHIP";
    case SEMANTIC_ERROR_INVALID_LIFETIME:
        return "INVALID_LIFETIME";
    case SEMANTIC_ERROR_DUPLICATE_ANNOTATION:
        return "DUPLICATE_ANNOTATION";
    case SEMANTIC_ERROR_INVALID_ANNOTATION:
        return "INVALID_ANNOTATION";
    case SEMANTIC_ERROR_INVALID_ANNOTATION_CONTEXT:
        return "INVALID_ANNOTATION_CONTEXT";
    case SEMANTIC_ERROR_CONFLICTING_ANNOTATIONS:
        return "CONFLICTING_ANNOTATIONS";
    case SEMANTIC_ERROR_SECURITY_VIOLATION:
        return "SECURITY_VIOLATION";
    default:
        return "UNKNOWN";
    }
}

const char *semantic_error_code_description(SemanticErrorCode code) {
    switch (code) {
    case SEMANTIC_ERROR_NONE:
        return "No error";
    case SEMANTIC_ERROR_TYPE_MISMATCH:
        return "Type mismatch";
    case SEMANTIC_ERROR_UNDEFINED_TYPE:
        return "Undefined type";
    case SEMANTIC_ERROR_UNDEFINED_SYMBOL:
        return "Undefined symbol";
    case SEMANTIC_ERROR_DUPLICATE_SYMBOL:
        return "Duplicate symbol";
    case SEMANTIC_ERROR_INCOMPATIBLE_TYPES:
        return "Incompatible types";
    case SEMANTIC_ERROR_INVALID_TYPE:
        return "Invalid type";
    case SEMANTIC_ERROR_INVALID_OPERATION:
        return "Invalid operation";
    case SEMANTIC_ERROR_INVALID_ARGUMENTS:
        return "Invalid arguments";
    case SEMANTIC_ERROR_INVALID_RETURN:
        return "Invalid return";
    case SEMANTIC_ERROR_MISMATCHED_TYPES:
        return "Mismatched types";
    case SEMANTIC_ERROR_NOT_CALLABLE:
        return "Not callable";
    case SEMANTIC_ERROR_IMMUTABLE_MODIFICATION:
        return "Immutable modification";
    case SEMANTIC_ERROR_INVALID_MODULE:
        return "Invalid module";
    case SEMANTIC_ERROR_SHADOWED_SYMBOL:
        return "Shadowed symbol";
    case SEMANTIC_ERROR_UNUSED_SYMBOL:
        return "Unused symbol";
    case SEMANTIC_ERROR_UNINITIALIZED_VARIABLE:
        return "Uninitialized variable";
    case SEMANTIC_ERROR_CIRCULAR_DEPENDENCY:
        return "Circular dependency";
    case SEMANTIC_ERROR_INVALID_STRUCT_FIELD:
        return "Invalid struct field";
    case SEMANTIC_ERROR_NON_EXHAUSTIVE_MATCH:
        return "Non-exhaustive match";
    case SEMANTIC_ERROR_INVALID_OWNERSHIP:
        return "Invalid ownership";
    case SEMANTIC_ERROR_INVALID_LIFETIME:
        return "Invalid lifetime";
    case SEMANTIC_ERROR_DUPLICATE_ANNOTATION:
        return "Duplicate annotation";
    case SEMANTIC_ERROR_INVALID_ANNOTATION:
        return "Invalid annotation";
    case SEMANTIC_ERROR_INVALID_ANNOTATION_CONTEXT:
        return "Invalid annotation context";
    case SEMANTIC_ERROR_CONFLICTING_ANNOTATIONS:
        return "Conflicting annotations";
    case SEMANTIC_ERROR_SECURITY_VIOLATION:
        return "Security violation";
    default:
        return "Unknown error";
    }
}
