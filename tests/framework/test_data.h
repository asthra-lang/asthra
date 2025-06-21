/**
 * Asthra Programming Language
 * Test Data Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Centralized test data for consistent testing
 */

#ifndef ASTHRA_TEST_DATA_H
#define ASTHRA_TEST_DATA_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// VALID ASTHRA PROGRAMS
// =============================================================================

/**
 * Collection of valid Asthra programs for testing
 */
extern const char* VALID_ASTHRA_PROGRAMS[];

/**
 * Simple function declarations
 */
extern const char* SIMPLE_FUNCTION_PROGRAMS[];

/**
 * Struct definition programs
 */
extern const char* STRUCT_DEFINITION_PROGRAMS[];

/**
 * Variable declaration programs
 */
extern const char* VARIABLE_DECLARATION_PROGRAMS[];

/**
 * Expression testing programs
 */
extern const char* EXPRESSION_TEST_PROGRAMS[];

/**
 * Control flow programs
 */
extern const char* CONTROL_FLOW_PROGRAMS[];

/**
 * Pattern matching programs
 */
extern const char* PATTERN_MATCHING_PROGRAMS[];

// =============================================================================
// INVALID ASTHRA PROGRAMS
// =============================================================================

/**
 * Collection of invalid Asthra programs for error testing
 */
extern const char* INVALID_ASTHRA_PROGRAMS[];

/**
 * Syntax error programs
 */
extern const char* SYNTAX_ERROR_PROGRAMS[];

/**
 * Type error programs
 */
extern const char* TYPE_ERROR_PROGRAMS[];

/**
 * Semantic error programs
 */
extern const char* SEMANTIC_ERROR_PROGRAMS[];

// =============================================================================
// FFI TEST PROGRAMS
// =============================================================================

/**
 * FFI-related test programs
 */
extern const char* FFI_TEST_PROGRAMS[];

/**
 * Extern function declarations
 */
extern const char* EXTERN_FUNCTION_PROGRAMS[];

/**
 * FFI annotation programs
 */
extern const char* FFI_ANNOTATION_PROGRAMS[];

/**
 * Memory safety FFI programs
 */
extern const char* FFI_MEMORY_SAFETY_PROGRAMS[];

// =============================================================================
// CONCURRENCY TEST PROGRAMS
// =============================================================================

/**
 * Concurrency-related test programs
 */
extern const char* CONCURRENCY_TEST_PROGRAMS[];

/**
 * Spawn and async programs
 */
extern const char* SPAWN_ASYNC_PROGRAMS[];

/**
 * Channel communication programs
 */
extern const char* CHANNEL_PROGRAMS[];

/**
 * Worker pool programs
 */
extern const char* WORKER_POOL_PROGRAMS[];

// =============================================================================
// LEXER TEST DATA
// =============================================================================

/**
 * Token test cases for lexer testing
 */
typedef struct {
    const char* input;
    const char* expected_tokens[10]; // Up to 10 expected token types
    const char* expected_values[10]; // Up to 10 expected token values
} TokenTestCase;

/**
 * Collection of token test cases
 */
extern const TokenTestCase LEXER_TOKEN_TEST_CASES[];

/**
 * String literal test cases
 */
extern const struct {
    const char* input;
    const char* expected_value;
    bool should_be_valid;
} STRING_LITERAL_TEST_CASES[];

/**
 * Number literal test cases
 */
extern const struct {
    const char* input;
    long expected_int_value;
    double expected_float_value;
    bool is_integer;
    bool should_be_valid;
} NUMBER_LITERAL_TEST_CASES[];

/**
 * Character literal test cases
 */
extern const struct {
    const char* input;
    uint32_t expected_value;
    bool should_be_valid;
} CHARACTER_LITERAL_TEST_CASES[];

// =============================================================================
// PARSER TEST DATA
// =============================================================================

/**
 * Expression parsing test cases
 */
extern const struct {
    const char* input;
    const char* expected_ast_type;
    size_t expected_child_count;
    bool should_be_valid;
} EXPRESSION_PARSE_TEST_CASES[];

/**
 * Statement parsing test cases
 */
extern const struct {
    const char* input;
    const char* expected_ast_type;
    size_t expected_child_count;
    bool should_be_valid;
} STATEMENT_PARSE_TEST_CASES[];

/**
 * Declaration parsing test cases
 */
extern const struct {
    const char* input;
    const char* expected_ast_type;
    const char* expected_name;
    bool should_be_valid;
} DECLARATION_PARSE_TEST_CASES[];

// =============================================================================
// SEMANTIC ANALYSIS TEST DATA
// =============================================================================

/**
 * Type checking test cases
 */
extern const struct {
    const char* program;
    const char* expected_error_type;
    bool should_be_valid;
} TYPE_CHECK_TEST_CASES[];

/**
 * Symbol resolution test cases
 */
extern const struct {
    const char* program;
    const char* symbol_name;
    bool should_resolve;
} SYMBOL_RESOLUTION_TEST_CASES[];

/**
 * Scope testing programs
 */
extern const struct {
    const char* program;
    const char* symbol_name;
    const char* expected_scope;
    bool should_be_accessible;
} SCOPE_TEST_CASES[];

// =============================================================================
// CODE GENERATION TEST DATA
// =============================================================================

/**
 * Code generation test cases
 */
extern const struct {
    const char* input_program;
    const char* expected_output_pattern;
    bool should_compile;
} CODEGEN_TEST_CASES[];

/**
 * Expression code generation tests
 */
extern const struct {
    const char* expression;
    const char* expected_instructions;
} EXPRESSION_CODEGEN_TEST_CASES[];

/**
 * Function code generation tests
 */
extern const struct {
    const char* function_def;
    const char* expected_prologue;
    const char* expected_epilogue;
} FUNCTION_CODEGEN_TEST_CASES[];

// =============================================================================
// ANNOTATION TEST DATA
// =============================================================================

/**
 * Annotation parsing test cases
 */
extern const struct {
    const char* input;
    const char* expected_annotation_type;
    const char* expected_parameters;
    bool should_be_valid;
} ANNOTATION_TEST_CASES[];

/**
 * Security annotation test cases
 */
extern const char* SECURITY_ANNOTATION_PROGRAMS[];

/**
 * Ownership annotation test cases
 */
extern const char* OWNERSHIP_ANNOTATION_PROGRAMS[];

/**
 * FFI annotation test cases
 */
extern const char* FFI_ANNOTATION_TEST_PROGRAMS[];

// =============================================================================
// ERROR MESSAGE TEST DATA
// =============================================================================

/**
 * Expected error messages for various error conditions
 */
extern const struct {
    const char* input_program;
    const char* expected_error_message;
    const char* expected_error_location;
} ERROR_MESSAGE_TEST_CASES[];

/**
 * Lexer error test cases
 */
extern const struct {
    const char* input;
    const char* expected_error;
} LEXER_ERROR_TEST_CASES[];

/**
 * Parser error test cases
 */
extern const struct {
    const char* input;
    const char* expected_error;
} PARSER_ERROR_TEST_CASES[];

/**
 * Semantic error test cases
 */
extern const struct {
    const char* input;
    const char* expected_error;
} SEMANTIC_ERROR_TEST_CASES[];

// =============================================================================
// PERFORMANCE TEST DATA
// =============================================================================

/**
 * Performance test programs of varying sizes
 */
extern const struct {
    const char* name;
    const char* program;
    size_t expected_max_compile_time_ms;
    size_t expected_max_memory_usage_kb;
} PERFORMANCE_TEST_CASES[];

/**
 * Large program generators for stress testing
 */
typedef struct {
    size_t token_count;
    size_t function_count;
    size_t struct_count;
    size_t max_nesting_depth;
} ProgramGeneratorConfig;

/**
 * Generate a large test program based on configuration
 * @param config Generation configuration
 * @return Generated program string (caller must free)
 */
char* generate_large_test_program(const ProgramGeneratorConfig* config);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Load test data from file
 * @param filename The file to load
 * @return The file contents (caller must free)
 */
char* load_test_data_file(const char* filename);

/**
 * Save test data to file
 * @param filename The file to save to
 * @param content The content to save
 * @return true if save succeeded
 */
bool save_test_data_file(const char* filename, const char* content);

/**
 * Compare two test data files for equality
 * @param expected_file The expected output file
 * @param actual_file The actual output file
 * @return true if files are equal
 */
bool compare_test_data_files(const char* expected_file, const char* actual_file);

/**
 * Create a temporary test data file with content
 * @param content The content to write
 * @param suffix File suffix (e.g., ".ast")
 * @return The temporary filename (caller must free)
 */
char* create_temp_test_data_file(const char* content, const char* suffix);

/**
 * Clean up temporary test data files
 * @param filename The temporary file to remove
 */
void cleanup_temp_test_data_file(const char* filename);

/**
 * Get the count of test cases in an array
 * @param test_array Pointer to test array
 * @param element_size Size of each element
 * @return Number of test cases
 */
size_t get_test_case_count(const void* test_array, size_t element_size);

/**
 * Validate test data integrity
 * @return true if all test data is valid
 */
bool validate_test_data_integrity(void);

/**
 * Initialize test data (load from files, validate, etc.)
 * @return true if initialization succeeded
 */
bool initialize_test_data(void);

/**
 * Cleanup test data (free memory, close files, etc.)
 */
void cleanup_test_data(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_DATA_H
