/**
 * Asthra Programming Language
 * Mutable Variable Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for mutable variable semantics with mut keyword
 */

#include "test_mutability_common.h"
#include "semantic_errors.h"
#include "../framework/test_framework.h"

// =============================================================================
// MUTABLE VARIABLE TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_mutable_variable_assignment, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: int = 42;\n"
        "    x = 43;      // OK: Can assign to mutable variable\n"
        "    x = x + 1;   // OK: Can use in expressions and reassign\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "mutable_variable_assignment"),
        "Should allow assignment to mutable variables");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_mutable_variable_compound_assignment, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut count: int = 0;\n"
        "    count = count + 1;  // OK: Can reassign mutable variable\n"
        "    count = count - 1;  // OK: Various operations\n"
        "    count = count * 2;\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "mutable_compound_assignment"),
        "Should allow compound assignment to mutable variables");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_mutable_variable_assignment_metadata,
        test_mutable_variable_compound_assignment_metadata
    };
    
    AsthraTestFunction test_functions[] = {
        test_mutable_variable_assignment,
        test_mutable_variable_compound_assignment
    };
    
    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);
    
    AsthraTestSuiteConfig config = {
        .name = "Mutable Variable Tests",
        .description = "Tests for mutable variable semantics with mut keyword",
        .parallel_execution = false,
        .default_timeout_ns = 30000000000ULL,
        .json_output = false,
        .verbose_output = true
    };
    
    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}