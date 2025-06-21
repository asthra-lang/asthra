/**
 * Asthra Programming Language
 * Complex Mutability Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for complex mutability scenarios and patterns
 */

#include "test_mutability_common.h"
#include "semantic_errors.h"
#include "../framework/test_framework.h"

// =============================================================================
// COMPLEX MUTABILITY SCENARIOS
// =============================================================================

ASTHRA_TEST_DEFINE(test_mutable_reference_through_immutable, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: int = 42;\n"
        "    let ref_x: *const int = &x;     // Immutable reference to mutable variable\n"
        "    let y: int = *ref_x;     // OK: Can read through reference\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "reference_through_immutable"),
        "Should allow reading through immutable reference");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_mutability_in_match_patterns, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum IntOption {\n"
        "    Some(int),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let opt: IntOption = IntOption.Some(42);\n"
        "    match opt {\n"
        "        IntOption.Some(x) => {\n"
        "            // x is immutable here\n"
        "            let y: int = x + 1;  // OK: Can read\n"
        "        },\n"
        "        IntOption.None => {}\n"
        "    }\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "match_pattern_mutability"),
        "Should handle mutability in match patterns correctly");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_mutability_in_match_pattern_error, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum IntOption {\n"
        "    Some(int),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let opt: IntOption = IntOption.Some(42);\n"
        "    match opt {\n"
        "        IntOption.Some(x) => {\n"
        "            x = 100;  // ERROR: Pattern bindings are immutable\n"
        "        },\n"
        "        IntOption.None => {}\n"
        "    }\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_error(source, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION, "match_pattern_assignment_error"),
        "Should error on assignment to pattern binding");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_method_receiver_mutability, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    pub x: int,\n"
        "    pub y: int\n"
        "}\n"
        "\n"
        "pub fn point_distance(p: Point) -> float {\n"
        "    // p is immutable parameter\n"
        "    let x_sq: int = p.x * p.x;\n"
        "    let y_sq: int = p.y * p.y;\n"
        "    return 0.0;  // Simplified for test\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "method_receiver_mutability"),
        "Functions can read immutable struct parameters");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_immutable_method_on_immutable_receiver, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    pub x: int,\n"
        "    pub y: int\n"
        "}\n"
        "\n"
        "pub fn get_x(p: Point) -> int {\n"
        "    return p.x;  // OK: Reading field from immutable parameter\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Point = Point { x: 10, y: 20 };\n"
        "    let x: int = get_x(p);  // OK: Passing immutable struct to function\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "mut_method_on_immutable"),
        "Can pass immutable structs to functions");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_mutable_reference_through_immutable_metadata,
        test_mutability_in_match_patterns_metadata,
        test_mutability_in_match_pattern_error_metadata,
        test_method_receiver_mutability_metadata,
        test_immutable_method_on_immutable_receiver_metadata
    };
    
    AsthraTestFunction test_functions[] = {
        test_mutable_reference_through_immutable,
        test_mutability_in_match_patterns,
        test_mutability_in_match_pattern_error,
        test_method_receiver_mutability,
        test_immutable_method_on_immutable_receiver
    };
    
    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);
    
    AsthraTestSuiteConfig config = {
        .name = "Complex Mutability Tests",
        .description = "Tests for complex mutability scenarios and patterns",
        .parallel_execution = false,
        .default_timeout_ns = 30000000000ULL,
        .json_output = false,
        .verbose_output = true
    };
    
    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}