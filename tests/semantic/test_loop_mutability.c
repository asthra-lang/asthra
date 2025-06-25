/**
 * Asthra Programming Language
 * Loop Mutability Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for loop variable mutability semantics
 */

#include "../framework/test_framework.h"
#include "semantic_errors.h"
#include "test_mutability_common.h"

// =============================================================================
// MUTABILITY WITH LOOPS
// =============================================================================

ASTHRA_TEST_DEFINE(test_loop_variable_mutability, ASTHRA_TEST_SEVERITY_LOW) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let mut sum: int = 0;\n"
                         "    let arr: []int = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];\n"
                         "    for i in arr {\n"
                         "        // i is immutable within loop\n"
                         "        sum = sum + i;  // OK: sum is mutable\n"
                         "    }\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_mutability_success(source, "loop_variable_mutability"),
                            "Should handle loop variable mutability correctly");

    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_loop_variable_assignment_error, ASTHRA_TEST_SEVERITY_LOW) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []int = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];\n"
                         "    for i in arr {\n"
                         "        i = i + 1;  // ERROR: Loop variable is immutable\n"
                         "    }\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context,
                            test_mutability_error(source, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION,
                                                  "loop_variable_assignment"),
                            "Should error on assignment to loop variable");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {test_loop_variable_mutability_metadata,
                                          test_loop_variable_assignment_error_metadata};

    AsthraTestFunction test_functions[] = {test_loop_variable_mutability,
                                           test_loop_variable_assignment_error};

    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);

    AsthraTestSuiteConfig config = {.name = "Loop Mutability Tests",
                                    .description = "Tests for loop variable mutability semantics",
                                    .parallel_execution = false,
                                    .default_timeout_ns = 30000000000ULL,
                                    .json_output = false,
                                    .verbose_output = true};

    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}