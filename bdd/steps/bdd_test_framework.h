#ifndef BDD_TEST_FRAMEWORK_H
#define BDD_TEST_FRAMEWORK_H

#include "bdd_support.h"

/**
 * Common BDD Test Framework
 * Provides reusable test patterns and structures
 */

// Test function pointer type
typedef void (*BddTestFunction)(void);

// Test registry structure
typedef struct {
    const char* name;
    BddTestFunction function;
    int is_wip;  // 1 if marked with @wip, 0 otherwise
} BddTestCase;

// Common result structure for mock implementations
typedef struct {
    int success;
    char* error_message;
    int error_count;
    void* specific_data;  // For parser AST, semantic symbols, etc.
} BddResult;

// Test framework functions
int bdd_run_test_suite(const char* feature_name, 
                       BddTestCase* test_cases,
                       int test_count,
                       void (*cleanup_function)(void));

// Result utilities
void bdd_init_result(BddResult* result);
void bdd_cleanup_result(BddResult* result);

// Test case utilities
void bdd_run_test_case(BddTestCase* test_case);
int bdd_should_skip_test_case(BddTestCase* test_case);

// Common scenario patterns
void bdd_run_compilation_scenario(const char* scenario_name,
                                  const char* filename,
                                  const char* source_code,
                                  int should_succeed,
                                  const char* expected_output_or_error);

void bdd_run_execution_scenario(const char* scenario_name,
                                const char* filename,
                                const char* source_code,
                                const char* expected_output,
                                int expected_exit_code);

// Macro for defining test cases
#define BDD_TEST_CASE(name, func) { #name, func, 0 }
#define BDD_WIP_TEST_CASE(name, func) { #name, func, 1 }

#endif // BDD_TEST_FRAMEWORK_H