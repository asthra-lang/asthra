/**
 * Asthra Programming Language Compiler
 * Parser error handling types and functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_ERROR_H
#define ASTHRA_PARSER_ERROR_H

#include "parser_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// Structured error codes for enhanced error reporting
typedef enum {
    PARSE_ERROR_NONE = 0,
    
    // Lexical errors (1000-1999)
    PARSE_ERROR_INVALID_TOKEN = 1000,
    PARSE_ERROR_UNTERMINATED_STRING = 1001,
    PARSE_ERROR_INVALID_NUMBER = 1002,
    PARSE_ERROR_INVALID_CHARACTER = 1003,
    
    // Syntax errors (2000-2999)
    PARSE_ERROR_UNEXPECTED_TOKEN = 2000,
    PARSE_ERROR_MISSING_TOKEN = 2001,
    PARSE_ERROR_INVALID_EXPRESSION = 2002,
    PARSE_ERROR_INVALID_STATEMENT = 2003,
    PARSE_ERROR_INVALID_DECLARATION = 2004,
    PARSE_ERROR_UNMATCHED_DELIMITER = 2005,
    
    // Semantic errors (3000-3999)
    PARSE_ERROR_UNDEFINED_SYMBOL = 3000,
    PARSE_ERROR_DUPLICATE_SYMBOL = 3001,
    PARSE_ERROR_TYPE_MISMATCH = 3002,
    PARSE_ERROR_INVALID_ANNOTATION = 3003,
    PARSE_ERROR_OWNERSHIP_VIOLATION = 3004,
    
    // Grammar errors (4000-4999)
    PARSE_ERROR_AMBIGUOUS_GRAMMAR = 4000,
    PARSE_ERROR_PRECEDENCE_VIOLATION = 4001,
    PARSE_ERROR_PATTERN_EXHAUSTIVENESS = 4002,
    
    // Recovery errors (5000-5999)
    PARSE_ERROR_RECOVERY_FAILED = 5000,
    PARSE_ERROR_MAX_ERRORS_EXCEEDED = 5001,
    PARSE_ERROR_RECURSION_LIMIT = 5002,
    
    // Import-specific errors (6000-6999) - Phase 1: Import System Enhancement
    PARSE_ERROR_IMPORT_EMPTY_PATH = 6000,
    PARSE_ERROR_IMPORT_INVALID_FORMAT = 6001,
    PARSE_ERROR_IMPORT_INVALID_STDLIB = 6002,
    PARSE_ERROR_IMPORT_INVALID_GITHUB = 6003,
    PARSE_ERROR_IMPORT_INVALID_LOCAL = 6004,
    PARSE_ERROR_IMPORT_WHITESPACE = 6005,
    PARSE_ERROR_IMPORT_INTERNAL_ACCESS_DENIED = 6006,
    PARSE_ERROR_IMPORT_MALFORMED_DOMAIN = 6007,
    PARSE_ERROR_IMPORT_INCOMPLETE_PATH = 6008
} ParseErrorCode;

// Import-specific error types for enhanced reporting
typedef enum {
    IMPORT_ERROR_EMPTY_PATH,
    IMPORT_ERROR_INVALID_STDLIB,
    IMPORT_ERROR_INVALID_GITHUB,
    IMPORT_ERROR_INVALID_LOCAL,
    IMPORT_ERROR_WHITESPACE,
    IMPORT_ERROR_INTERNAL_ACCESS_DENIED,
    IMPORT_ERROR_MALFORMED_DOMAIN,
    IMPORT_ERROR_INCOMPLETE_PATH
} ImportPathErrorType;

// =============================================================================
// ERROR HANDLING FUNCTIONS
// =============================================================================

// Error handling with enhanced features
const ParseError *parser_get_errors(Parser *parser);
size_t parser_get_error_count(Parser *parser);
void parser_clear_errors(Parser *parser);
bool parser_had_error(Parser *parser);
bool parser_had_warning(Parser *parser);

// Enhanced error reporting
void parser_report_error_with_context(Parser *parser, const char *message, 
                                     TokenType expected, TokenType actual);
void parser_report_structured_error(Parser *parser, uint32_t error_code, 
                                   const char *message, SourceLocation location);

// Error reporting and recovery
void report_error(Parser *parser, const char *message);
void report_error_at(Parser *parser, SourceLocation location, const char *message);
void report_warning(Parser *parser, const char *message);
void synchronize(Parser *parser);
void enter_panic_mode(Parser *parser);
void exit_panic_mode(Parser *parser);

// Enhanced error recovery
bool recover_to_token(Parser *parser, TokenType target);
bool recover_to_statement_boundary(Parser *parser);
bool recover_to_declaration_boundary(Parser *parser);

// Import validation functions - Phase 1: Import System Enhancement
bool validate_import_path_format(const char* path);
bool validate_stdlib_path(const char* path);
bool validate_internal_path(const char* path);
bool validate_github_path(const char* path);
bool validate_local_path(const char* path);
void report_import_path_error(Parser* parser, ImportPathErrorType error_type, 
                             const char* path, int line_number);
void parser_error_with_suggestion(Parser* parser, int line_number,
                                const char* error_message, const char* suggestion);

// Error code utilities
const char *parse_error_code_name(ParseErrorCode code);
const char *parse_error_code_description(ParseErrorCode code);
bool is_recoverable_error(ParseErrorCode code);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_ERROR_H 
