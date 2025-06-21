/**
 * Asthra Programming Language
 * Function Parameter Mutability Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for function parameter immutability semantics
 */

#include "test_mutability_common.h"
#include "semantic_errors.h"
#include "../framework/test_framework.h"

// =============================================================================
// FUNCTION PARAMETER IMMUTABILITY TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_function_parameter_immutable, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn modify_param(x: int) -> void {\n"
        "    x = 42;  // ERROR: Function parameters are immutable\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_error(source, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION, "function_parameter_immutable"),
        "Should error on assignment to function parameter");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_function_parameter_read, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn use_param(x: int) -> int {\n"
        "    return x + 1;  // OK: Can read parameter\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "function_parameter_read"),
        "Should allow reading function parameters");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_function_slice_parameter_modification, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn modify_slice(arr: []int) -> void {\n"
        "    arr[0] = 42;  // ERROR: Cannot modify through immutable parameter\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_error(source, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION, "function_slice_parameter"),
        "Should error on modification through immutable slice parameter");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_function_parameter_immutable_metadata,
        test_function_parameter_read_metadata,
        test_function_slice_parameter_modification_metadata
    };
    
    AsthraTestFunction test_functions[] = {
        test_function_parameter_immutable,
        test_function_parameter_read,
        test_function_slice_parameter_modification
    };
    
    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);
    
    AsthraTestSuiteConfig config = {
        .name = "Function Parameter Mutability Tests",
        .description = "Tests for function parameter immutability semantics",
        .parallel_execution = false,
        .default_timeout_ns = 30000000000ULL,
        .json_output = false,
        .verbose_output = true
    };
    
    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}