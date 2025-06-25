/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Common Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared test framework and utilities for generic structs validation
 */

#ifndef GENERIC_STRUCTS_TEST_COMMON_H
#define GENERIC_STRUCTS_TEST_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include all necessary headers
#include "ast.h"
#include "backend_interface.h"
#include "generic_instantiation.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST FRAMEWORK MACROS
// =============================================================================

extern size_t tests_run;
extern size_t tests_passed;

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("  ✅ PASS: %s\n", message);                                                    \
        } else {                                                                                   \
            printf("  ❌ FAIL: %s\n", message);                                                    \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST_SUCCESS()                                                                             \
    do {                                                                                           \
        printf("  🎉 Test completed successfully!\n");                                             \
        return true;                                                                               \
    } while (0)

// =============================================================================
// VALIDATION HELPER FUNCTIONS
// =============================================================================

/**
 * Test if parsing succeeds and produces valid AST
 * @param source Source code to parse
 * @param description Description of the test case
 * @return true if parsing succeeds, false otherwise
 */
bool test_parse_success(const char *source, const char *description);

/**
 * Test if parsing fails as expected
 * @param source Source code to parse
 * @param description Description of the test case
 * @return true if parsing fails as expected, false otherwise
 */
bool test_parse_failure(const char *source, const char *description);

/**
 * Test semantic analysis on valid source
 * @param source Source code to analyze
 * @param description Description of the test case
 * @return true if semantic analysis succeeds, false otherwise
 */
bool test_semantic_success(const char *source, const char *description);

/**
 * Initialize test framework counters
 */
void init_test_framework(void);

/**
 * Print test summary and return exit code
 * @return 0 if all tests passed, 1 otherwise
 */
int print_test_summary(void);

#ifdef __cplusplus
}
#endif

#endif // GENERIC_STRUCTS_TEST_COMMON_H
