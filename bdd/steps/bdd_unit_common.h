#ifndef BDD_UNIT_COMMON_H
#define BDD_UNIT_COMMON_H

/**
 * Common BDD Unit Test Header
 * Consolidates all common includes and provides macros to eliminate duplication
 * across unit test step files.
 */

// Standard includes needed by all unit test step files
#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

/**
 * Macro to eliminate main function duplication across all unit step files.
 * 
 * Usage:
 *   BDD_UNIT_TEST_MAIN("Feature Name", feature_test_cases)
 * 
 * This expands to the standard main function that calls bdd_run_test_suite
 * with the appropriate parameters.
 */
#define BDD_UNIT_TEST_MAIN(feature_name, test_cases_array) \
    int main(void) { \
        return bdd_run_test_suite(feature_name, \
                                  test_cases_array, \
                                  sizeof(test_cases_array) / sizeof(test_cases_array[0]), \
                                  bdd_cleanup_temp_files); \
    }

/**
 * Helper macro to declare and define a BDD test case array.
 * Reduces boilerplate in test case array declarations.
 * 
 * Usage:
 *   BDD_DECLARE_TEST_CASES(feature_name)
 *   // ... test case definitions ...
 *   BDD_END_TEST_CASES()
 */
#define BDD_DECLARE_TEST_CASES(feature_name) \
    BddTestCase feature_name##_test_cases[] = {

#define BDD_END_TEST_CASES() \
    };

/**
 * Common error scenarios that can be used across multiple test files.
 * These provide standardized error testing patterns.
 */

/**
 * Run a type mismatch error scenario.
 * Common pattern for testing type system errors.
 */
static inline void bdd_run_type_mismatch_scenario(const char* scenario_name,
                                                  const char* filename,
                                                  const char* source_code) {
    bdd_run_compilation_scenario(scenario_name, filename, source_code, 0, "type mismatch");
}

/**
 * Run an undefined symbol error scenario.
 * Common pattern for testing semantic analysis errors.
 */
static inline void bdd_run_undefined_symbol_scenario(const char* scenario_name,
                                                     const char* filename,
                                                     const char* source_code) {
    bdd_run_compilation_scenario(scenario_name, filename, source_code, 0, "undefined");
}

/**
 * Run a syntax error scenario.
 * Common pattern for testing parser errors.
 */
static inline void bdd_run_syntax_error_scenario(const char* scenario_name,
                                                 const char* filename,
                                                 const char* source_code) {
    bdd_run_compilation_scenario(scenario_name, filename, source_code, 0, "syntax");
}

/**
 * Helper macros for common test patterns.
 * These reduce repetitive code in test functions.
 */

/**
 * Standard package header for test source code.
 * Most tests need this basic package declaration.
 */
#define BDD_TEST_PACKAGE_HEADER "package test;\n"

/**
 * Standard main function signature for test source code.
 * Most tests use this exact signature.
 */
#define BDD_TEST_MAIN_SIGNATURE "pub fn main(none) -> i32 {\n"

/**
 * Standard main function footer for test source code.
 */
#define BDD_TEST_MAIN_FOOTER "}\n"

/**
 * Macro to create a simple test source with just a return statement.
 * Useful for basic value tests.
 * 
 * Usage:
 *   BDD_SIMPLE_RETURN_TEST("return 42;")
 */
#define BDD_SIMPLE_RETURN_TEST(return_statement) \
    BDD_TEST_PACKAGE_HEADER \
    BDD_TEST_MAIN_SIGNATURE \
    "    " return_statement "\n" \
    BDD_TEST_MAIN_FOOTER

/**
 * Macro to create a test source with variable declarations and a return.
 * Useful for more complex value tests.
 * 
 * Usage:
 *   BDD_VARIABLE_TEST("let x: i32 = 5;", "return x;")
 */
#define BDD_VARIABLE_TEST(declarations, return_statement) \
    BDD_TEST_PACKAGE_HEADER \
    BDD_TEST_MAIN_SIGNATURE \
    "    " declarations "\n" \
    "    " return_statement "\n" \
    BDD_TEST_MAIN_FOOTER

/**
 * Macro to create a conditional test source.
 * Useful for boolean and comparison tests.
 * 
 * Usage:
 *   BDD_CONDITIONAL_TEST("let x = 5;", "x > 3", "return 1;", "return 0;")
 */
#define BDD_CONDITIONAL_TEST(setup, condition, true_branch, false_branch) \
    BDD_TEST_PACKAGE_HEADER \
    BDD_TEST_MAIN_SIGNATURE \
    "    " setup "\n" \
    "    if " condition " {\n" \
    "        " true_branch "\n" \
    "    } else {\n" \
    "        " false_branch "\n" \
    "    }\n" \
    BDD_TEST_MAIN_FOOTER

/**
 * Documentation for the refactoring:
 * 
 * BEFORE (per file):
 * - 4 include statements
 * - 5-line main function
 * - Manual test case array declaration
 * - Repetitive source code patterns
 * 
 * AFTER (per file):
 * - 1 include statement (this header)
 * - 1-line main function macro
 * - Optional helper macros for common patterns
 * 
 * BENEFITS:
 * - Eliminates ~310 duplicate lines across 31 files (10 lines × 31 files)
 * - Standardizes all unit test patterns
 * - Makes adding new tests faster and more consistent
 * - Centralizes common functionality for easier maintenance
 * - Reduces cognitive load when reading test files
 */

#endif // BDD_UNIT_COMMON_H