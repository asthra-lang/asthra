/**
 * Asthra Programming Language Grammar Test Helpers
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common helpers and utilities for grammar disambiguation,
 * pattern matching syntax, string operations, and slice syntax parsing tests.
 */

#ifndef ASTHRA_TEST_GRAMMAR_HELPERS_H
#define ASTHRA_TEST_GRAMMAR_HELPERS_H

#include "../framework/test_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// GRAMMAR TEST HELPER FUNCTIONS
// =============================================================================

/**
 * Mock parser function for testing expression parsing
 * In a real implementation, this would use the actual parser
 */
bool grammar_test_parse_expression(const char *expression);

/**
 * Mock function to validate AST disambiguation
 * In a real implementation, this would check actual AST structure
 */
bool grammar_test_validate_disambiguation(const char *expression);

/**
 * Mock function to check operator precedence handling
 * In a real implementation, this would verify precedence in AST
 */
bool grammar_test_check_precedence(const char *expression);

/**
 * Mock function to detect parsing conflicts
 * In a real implementation, this would check parser tables
 */
bool grammar_test_has_parsing_conflicts(int *shift_reduce, int *reduce_reduce);

/**
 * Mock function to parse semantic tags
 * In a real implementation, this would use the semantic tag parser
 */
bool grammar_test_parse_semantic_tag(const char *tag_expression);

/**
 * Mock function to validate semantic tag semantics
 * In a real implementation, this would check tag validity
 */
bool grammar_test_validate_semantic_tag(const char *tag_expression);

/**
 * Mock function to parse FFI annotations
 * In a real implementation, this would use the FFI annotation parser
 */
bool grammar_test_parse_ffi_annotation(const char *annotation);

/**
 * Mock function to validate FFI annotation semantics
 * In a real implementation, this would check annotation validity
 */
bool grammar_test_validate_ffi_annotation(const char *annotation);

/**
 * Mock function to get the last error message from parsing
 * In a real implementation, this would return actual parser error
 */
const char *grammar_test_get_last_error(void);

/**
 * Mock function to check if expression parses as associated function call
 * In a real implementation, this would check AST node type
 */
bool grammar_test_is_associated_function_call(const char *expression);

/**
 * Mock function to check if expression has type arguments (generic types)
 * In a real implementation, this would check AST structure
 */
bool grammar_test_has_type_arguments(const char *expression);

/**
 * Mock function to check if type arguments field is populated in AST
 * In a real implementation, this would verify type_args field
 */
bool grammar_test_type_args_populated(const char *expression);

/**
 * Mock function to check if parsing behavior is deterministic
 * In a real implementation, this would verify parser consistency
 */
bool grammar_test_check_deterministic_parsing(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_GRAMMAR_HELPERS_H
