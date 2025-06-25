/**
 * Asthra Programming Language Compiler
 * String-based Parser Interface Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This header defines the string-based parsing interface that replaces
 * the stub implementations used in test files. It provides real parsing
 * functionality integrated with the existing parser infrastructure.
 */

#ifndef PARSER_STRING_INTERFACE_H
#define PARSER_STRING_INTERFACE_H

#include "ast_types.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// =============================================================================
// PARSE RESULT STRUCTURES
// =============================================================================

/**
 * Parse result structure for string-based parsing
 * Compatible with test framework expectations
 */
typedef struct {
    bool success;    // Whether parsing succeeded
    ASTNode *ast;    // Resulting AST (NULL on failure)
    int error_count; // Number of parse errors
    char **errors;   // Array of error messages

    // Extended statistics (optional)
    struct {
        size_t nodes_created;
        size_t tokens_consumed;
        size_t errors_reported;
        size_t parse_time_ms;
    } statistics;
} ParseResult;

/**
 * Parse options for configurable parsing behavior
 */
typedef struct {
    const char *filename;    // Source filename for error reporting
    bool strict_mode;        // Enable strict parsing mode
    bool enable_recovery;    // Enable error recovery
    size_t max_errors;       // Maximum errors before stopping
    bool collect_statistics; // Whether to collect parsing statistics
} ParseOptions;

// Forward declaration - ParseStatistics is defined in parser_core.h
typedef struct ParseStatistics ParseStatistics;

// =============================================================================
// CORE PARSING FUNCTIONS
// =============================================================================

/**
 * Parse Asthra source code from string
 *
 * This is the main entry point that replaces stub implementations.
 * Creates a real lexer and parser to generate a valid AST.
 *
 * @param code Source code string to parse
 * @return ParseResult with AST or error information
 */
ParseResult parse_string(const char *code);

/**
 * Parse Asthra source code with custom options
 *
 * Enhanced version that allows configuration of parsing behavior.
 *
 * @param code Source code string to parse
 * @param options Parse configuration options
 * @return ParseResult with AST or error information
 */
ParseResult parse_string_with_options(const char *code, const ParseOptions *options);

// =============================================================================
// SPECIALIZED PARSING FUNCTIONS
// =============================================================================

/**
 * Parse a single expression from string
 *
 * @param code Expression string to parse
 * @return ParseResult containing expression AST
 */
ParseResult parse_expression_string(const char *code);

/**
 * Parse a single statement from string
 *
 * @param code Statement string to parse
 * @return ParseResult containing statement AST
 */
ParseResult parse_statement_string(const char *code);

/**
 * Parse a single declaration from string
 *
 * @param code Declaration string to parse
 * @return ParseResult containing declaration AST
 */
ParseResult parse_declaration_string(const char *code);

// =============================================================================
// RESULT MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * Clean up ParseResult and free all associated memory
 *
 * @param result ParseResult to clean up
 */
void cleanup_parse_result(ParseResult *result);

/**
 * Check if ParseResult contains errors
 *
 * @param result ParseResult to check
 * @return true if errors are present
 */
bool parse_result_has_errors(const ParseResult *result);

/**
 * Get specific error message from ParseResult
 *
 * @param result ParseResult containing errors
 * @param index Error index (0-based)
 * @return Error message string or NULL if index invalid
 */
const char *parse_result_get_error(const ParseResult *result, size_t index);

/**
 * Print all error messages from ParseResult
 *
 * @param result ParseResult containing errors
 * @param output Output stream for error messages
 */
void parse_result_print_errors(const ParseResult *result, FILE *output);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Validate that a string contains valid Asthra syntax
 *
 * @param code Source code string to validate
 * @return true if syntax is valid
 */
bool validate_syntax(const char *code);

/**
 * Get parse statistics from last parsing operation
 *
 * @return ParseStatistics structure with performance data
 */
ParseStatistics get_last_parse_statistics(void);

#endif // PARSER_STRING_INTERFACE_H
