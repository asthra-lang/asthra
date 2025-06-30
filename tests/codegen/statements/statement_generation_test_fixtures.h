/**
 * Asthra Programming Language Compiler
 * Statement Generation Test Fixtures
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common fixtures and utilities for statement generation tests
 */

#ifndef STATEMENT_GENERATION_TEST_FIXTURES_H
#define STATEMENT_GENERATION_TEST_FIXTURES_H

// Ensure we use real parser implementation, not stubs
#define ASTHRA_PARSER_REAL_IMPLEMENTATION

#ifdef TEST_FRAMEWORK_MINIMAL
#include "../framework/test_framework_minimal.h"
#else
#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#endif
// Backend interface removed - using LLVM directly
#include "compiler.h"
// code_generator_types.h is already included by code_generator_core.h
// code_generator_instructions.h also includes code_generator_types.h, so we avoid double inclusion
#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURE STRUCTURES
// =============================================================================

/**
 * Test fixture for code generator testing
 */
typedef struct {
    void *backend; // Backend abstraction removed
    struct SemanticAnalyzer *analyzer;
    ASTNode *test_ast;
    char *output_buffer;
    size_t output_buffer_size;
} CodeGenTestFixture;

// =============================================================================
// FIXTURE MANAGEMENT FUNCTIONS
// =============================================================================

CodeGenTestFixture *setup_codegen_fixture(void);
void cleanup_codegen_fixture(CodeGenTestFixture *fixture);
bool generate_and_verify_statement(AsthraTestContext *context, CodeGenTestFixture *fixture,
                                   ASTNode *ast, const char *test_name);

/**
 * Setup test fixture with a code generator
 *
 * @return Allocated and initialized test fixture, or NULL on failure
 */
CodeGenTestFixture *setup_codegen_fixture(void);

/**
 * Cleanup test fixture and free all resources
 *
 * @param fixture The fixture to cleanup
 */
void cleanup_codegen_fixture(CodeGenTestFixture *fixture);

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Generate code for a statement and verify basic success
 *
 * @param context Test context for assertions
 * @param fixture Test fixture containing generator
 * @param ast AST node to generate code for
 * @param test_name Name of test for error messages
 * @return true if generation succeeded, false otherwise
 */
bool generate_and_verify_statement(AsthraTestContext *context, CodeGenTestFixture *fixture,
                                   ASTNode *ast, const char *test_name);

/**
 * Parse test source and verify parsing succeeded
 *
 * @param context Test context for assertions
 * @param source Source code to parse
 * @param filename Filename for parsing context
 * @param test_name Name of test for error messages
 * @return Parsed AST node, or NULL on failure
 */
ASTNode *parse_and_verify_source(AsthraTestContext *context, const char *source,
                                 const char *filename, const char *test_name);

/**
 * Common test pattern: parse source, generate code, cleanup
 *
 * @param context Test context for assertions
 * @param fixture Test fixture containing generator
 * @param source Source code to test
 * @param test_name Name of test for error messages
 * @return ASTHRA_TEST_PASS on success, ASTHRA_TEST_FAIL on failure
 */
AsthraTestResult test_statement_generation_pattern(AsthraTestContext *context,
                                                   CodeGenTestFixture *fixture, const char *source,
                                                   const char *test_name);

/**
 * Parse a statement fragment by wrapping it in a valid Asthra program
 *
 * @param fragment Code fragment to parse (e.g., "if (x > 0) { y = 1; }")
 * @param test_name Name of test for error messages
 * @return Parsed AST of the complete program containing the fragment, or NULL on failure
 */
ASTNode *parse_statement_fragment(const char *fragment, const char *test_name);

// =============================================================================
// TEST SUITE CONFIGURATION
// =============================================================================

/**
 * Create standard test suite configuration for statement generation tests
 *
 * @param suite_name Name of the test suite
 * @param description Description of the test suite
 * @param stats Test statistics object
 * @return Configured test suite config
 */
AsthraTestSuiteConfig create_statement_test_suite_config(const char *suite_name,
                                                         const char *description,
                                                         AsthraTestStatistics *stats);

/**
 * Standard test metadata template for statement generation tests
 *
 * @param test_name Name of the test function
 * @param severity Test severity level
 * @param timeout_ns Test timeout in nanoseconds
 * @return Configured test metadata
 */
AsthraTestMetadata create_statement_test_metadata(const char *test_name,
                                                  AsthraTestSeverity severity, uint64_t timeout_ns);

#endif /* STATEMENT_GENERATION_TEST_FIXTURES_H */
