/**
 * Real Program Test Suite Management Header
 * 
 * Contains core functions for managing test suites, test results,
 * and test execution.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_SUITE_H
#define REAL_PROGRAM_TEST_SUITE_H

#include <stddef.h>
#include <stdbool.h>
#include "real_program_test_utils.h"
#include "parser_string_interface.h"
#include "ast.h"

// =============================================================================
// TEST RESULT STRUCTURES
// =============================================================================

typedef struct {
    size_t parse_time_ms;
    size_t semantic_time_ms;
    size_t codegen_time_ms;
    size_t total_time_ms;
    size_t memory_used_kb;
    size_t ast_node_count;
    size_t symbol_count;
    size_t lines_of_code;
    size_t errors_detected;
    double parse_speed_loc_per_ms;
} PerformanceMetrics;

typedef struct {
    bool success;
    const char* error_message;
    const char* failure_reason;
    double execution_time_ms;
    PerformanceMetrics metrics;
    ParseResult* parse_result;
    ASTNode* ast;
} RealProgramTestResult;

// =============================================================================
// TEST DEFINITION STRUCTURES
// =============================================================================

typedef struct {
    const char* name;
    const char* source_code;
    bool should_parse;
    bool should_compile;
    bool should_execute;
    const char* expected_error_pattern;
    size_t max_parse_time_ms;
    size_t max_total_time_ms;
} RealProgramTest;

typedef struct {
    const char* suite_name;
    const char* description;
    RealProgramTest* tests;
    size_t test_count;
    size_t passed;
    size_t failed;
    size_t skipped;
    bool stop_on_failure;
    double total_parse_time_ms;
    double average_parse_time_ms;
} RealProgramTestSuite;

// =============================================================================
// CORE TEST SUITE FUNCTIONS
// =============================================================================

/**
 * Create a new test suite
 */
RealProgramTestSuite* create_real_program_test_suite(const char* suite_name, const char* description);

/**
 * Add a test to the suite
 */
bool add_test_to_suite(RealProgramTestSuite* suite, const RealProgramTest* test);

/**
 * Clean up test suite resources
 */
void cleanup_test_suite(RealProgramTestSuite* suite);

/**
 * Clean up test result resources
 */
void cleanup_test_result(RealProgramTestResult* result);

/**
 * Validate a complete program (parsing, semantic analysis, etc.)
 */
RealProgramTestResult validate_complete_program(const char* source, const char* test_name, const TestSuiteConfig* config);

/**
 * Run all tests in a suite
 */
bool run_test_suite(RealProgramTestSuite* suite, const TestSuiteConfig* config);

/**
 * Print test suite results
 */
void print_test_suite_results(const RealProgramTestSuite* suite, const TestSuiteConfig* config);

#endif // REAL_PROGRAM_TEST_SUITE_H