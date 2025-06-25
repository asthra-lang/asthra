/**
 * Asthra Programming Language
 * Immutable Variable Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for immutable variable semantics
 */

#include "../framework/test_framework.h"
#include "semantic_errors.h"
#include "test_mutability_common.h"

// =============================================================================
// IMMUTABLE VARIABLE TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_immutable_variable_read, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: int = 42;\n"
                         "    let y: int = x + 1;  // Reading immutable variable is allowed\n"
                         "    let z: int = x * 2;  // Multiple reads are fine\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_mutability_success(source, "immutable_variable_read"),
                            "Should allow reading immutable variables");

    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_immutable_variable_assignment_error, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: int = 42;\n"
                         "    x = 43;  // ERROR: Cannot assign to immutable variable\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context,
                            test_mutability_error(source, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION,
                                                  "immutable_variable_assignment"),
                            "Should error on assignment to immutable variable");

    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_immutable_variable_compound_assignment_error,
                   ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let count: int = 0;\n"
                         "    count = count + 1;  // ERROR: Cannot modify immutable variable\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context,
                            test_mutability_error(source, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION,
                                                  "immutable_compound_assignment"),
                            "Should error on compound assignment to immutable variable");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_immutable_variable_read_metadata, test_immutable_variable_assignment_error_metadata,
        test_immutable_variable_compound_assignment_error_metadata};

    AsthraTestFunction test_functions[] = {test_immutable_variable_read,
                                           test_immutable_variable_assignment_error,
                                           test_immutable_variable_compound_assignment_error};

    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);

    AsthraTestSuiteConfig config = {.name = "Immutable Variable Tests",
                                    .description = "Tests for immutable variable semantics",
                                    .parallel_execution = false,
                                    .default_timeout_ns = 30000000000ULL,
                                    .json_output = false,
                                    .verbose_output = true};

    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}