/**
 * Asthra Programming Language
 * If-Let Statement Testing - Parser Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 4.1: Parser implementation tests for if-let statements
 */

#ifndef IF_LET_TEST_PARSER_H
#define IF_LET_TEST_PARSER_H

#include "if_let_test_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PHASE 4.1: PARSER TESTS
// =============================================================================

/**
 * Test: Basic if-let parsing
 * Verifies that basic if-let statements are parsed correctly
 * @return true if test passes, false otherwise
 */
bool test_basic_if_let_parsing(void);

/**
 * Test: If-let with else clause
 * Verifies that if-let statements with else clauses are parsed correctly
 * @return true if test passes, false otherwise
 */
bool test_if_let_with_else(void);

/**
 * Test: If statement integration
 * Verifies that parse_if_stmt correctly delegates to if-let parsing
 * @return true if test passes, false otherwise
 */
bool test_if_statement_integration(void);

/**
 * Test: Regular if statement still works
 * Verifies that regular if statements are not broken by if-let integration
 * @return true if test passes, false otherwise
 */
bool test_regular_if_statement(void);

/**
 * Test: Nested if-let statements
 * Verifies that nested if-let statements are parsed correctly
 * @return true if test passes, false otherwise
 */
bool test_nested_if_let(void);

/**
 * Test: Error handling for malformed if-let
 * Verifies that malformed if-let statements are rejected
 * @return true if test passes, false otherwise
 */
bool test_malformed_if_let_error_handling(void);

/**
 * Run all Phase 4.1 parser tests
 * @return true if all tests pass, false otherwise
 */
bool run_parser_tests(void);

#ifdef __cplusplus
}
#endif

#endif // IF_LET_TEST_PARSER_H 
