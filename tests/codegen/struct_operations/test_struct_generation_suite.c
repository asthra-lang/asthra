/**
 * Asthra Programming Language Compiler
 * Struct Generation Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test suite for struct generation code tests, coordinating all
 * individual struct test modules
 */

#include "struct_generation_test_fixtures.h"

// Include all test modules
#include "test_struct_access.c"
#include "test_struct_assignment.c"
#include "test_struct_complex_types.c"
#include "test_struct_copy_move.c"
#include "test_struct_instantiation.c"
#include "test_struct_methods.c"

/**
 * Main test function for struct generation suite
 */
int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();

    AsthraTestSuiteConfig config = {.name = "Struct Generation Tests",
                                    .description = "Test code generation for structs including "
                                                   "field access, instantiation, and method calls",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    AsthraTestFunction tests[] = {
        test_generate_struct_access,    test_generate_struct_instantiation,
        test_generate_struct_methods,   test_generate_struct_assignment,
        test_generate_struct_copy_move, test_generate_struct_complex_types};

    AsthraTestMetadata metadata[] = {
        {"test_generate_struct_access", __FILE__, __LINE__, "test_generate_struct_access",
         ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_struct_instantiation", __FILE__, __LINE__,
         "test_generate_struct_instantiation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
         NULL},
        {"test_generate_struct_methods", __FILE__, __LINE__, "test_generate_struct_methods",
         ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_struct_assignment", __FILE__, __LINE__, "test_generate_struct_assignment",
         ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_struct_copy_move", __FILE__, __LINE__, "test_generate_struct_copy_move",
         ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_generate_struct_complex_types", __FILE__, __LINE__,
         "test_generate_struct_complex_types", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false,
         NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
