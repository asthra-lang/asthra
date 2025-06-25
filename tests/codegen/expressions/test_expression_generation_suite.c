/**
 * Asthra Programming Language Compiler
 * Expression Generation Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test suite for expression generation code tests, coordinating all
 * individual expression type test modules
 */

#include "expression_generation_test_fixtures.h"

// Include all test modules
#include "test_arithmetic_expressions.c"
#include "test_array_slice_codegen.c"
#include "test_bitwise_expressions.c"
#include "test_call_expressions.c"
#include "test_comparison_expressions.c"
#include "test_logical_expressions.c"
#include "test_unary_expressions.c"

/**
 * Main test function for expression generation suite
 */
int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();

    AsthraTestSuiteConfig config = {.name = "Expression Generation Tests",
                                    .description =
                                        "Test code generation for expressions including "
                                        "arithmetic, logical, comparison, and function calls",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    AsthraTestFunction tests[] = {
        test_generate_arithmetic_expressions, test_generate_logical_expressions,
        test_generate_comparison_expressions, test_generate_call_expressions,
        test_generate_unary_expressions,      test_generate_bitwise_expressions,
        test_generate_array_slice_expressions};

    AsthraTestMetadata metadata[] = {
        {"test_generate_arithmetic_expressions", __FILE__, __LINE__,
         "test_generate_arithmetic_expressions", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
         NULL},
        {"test_generate_logical_expressions", __FILE__, __LINE__,
         "test_generate_logical_expressions", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
         NULL},
        {"test_generate_comparison_expressions", __FILE__, __LINE__,
         "test_generate_comparison_expressions", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
         NULL},
        {"test_generate_call_expressions", __FILE__, __LINE__, "test_generate_call_expressions",
         ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_unary_expressions", __FILE__, __LINE__, "test_generate_unary_expressions",
         ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_bitwise_expressions", __FILE__, __LINE__,
         "test_generate_bitwise_expressions", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false,
         NULL},
        {"test_generate_array_slice_expressions", __FILE__, __LINE__,
         "test_generate_array_slice_expressions", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
         NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
