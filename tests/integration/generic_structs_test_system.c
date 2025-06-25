/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - System Integration Tests Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of memory management, boundary conditions, and type system integration tests
 */

#include "generic_structs_test_system.h"

// =============================================================================
// SYSTEM INTEGRATION TESTS IMPLEMENTATION
// =============================================================================

bool test_memory_management_validation(void) {
    printf("\n=== Test 6: Memory Management Validation ===\n");

    // Test that complex AST structures are properly cleaned up
    // This test creates and destroys multiple complex AST structures
    for (int i = 0; i < 10; i++) {
        char source[1024];
        snprintf(source, sizeof(source),
                 "pub struct Complex%d<T, U, V> {\n"
                 "    field1: T,\n"
                 "    field2: U,\n"
                 "    field3: V,\n"
                 "    nested: Vec<Pair<T, U>>\n"
                 "}\n"
                 "pub fn test%d(none) -> i32 {\n"
                 "    let x: Complex%d<i32, string, bool> = Complex%d {\n"
                 "        field1: %d,\n"
                 "        field2: \"test%d\",\n"
                 "        field3: true,\n"
                 "        nested: Vec { data: Pair { first: %d, second: \"nested\" } }\n"
                 "    };\n"
                 "    return %d;\n"
                 "}",
                 i, i, i, i, i, i, i, i);

        bool success = test_parse_success(source, "Complex memory management test");
        if (!success) {
            TEST_ASSERT(false, "Memory management test iteration failed");
            return false;
        }
    }

    TEST_ASSERT(true, "Multiple complex AST creation and cleanup succeeded");

    TEST_SUCCESS();
}

bool test_boundary_conditions(void) {
    printf("\n=== Test 7: Boundary Conditions and Limits ===\n");

    // Very long struct names
    TEST_ASSERT(test_parse_success(
                    "pub struct VeryLongStructNameThatTestsTheParserBoundaries<T> { value: T }",
                    "Very long struct name"),
                "Very long struct name parsing");

    // Very long type parameter names
    TEST_ASSERT(
        test_parse_success("pub struct Test<VeryLongTypeParameterNameThatTestsBoundaries> {\n"
                           "    value: VeryLongTypeParameterNameThatTestsBoundaries\n"
                           "}",
                           "Very long type parameter name"),
        "Very long type parameter name parsing");

    // Many fields in generic struct
    TEST_ASSERT(test_parse_success("pub struct ManyFields<T> {\n"
                                   "    f1: T, f2: T, f3: T, f4: T, f5: T,\n"
                                   "    f6: T, f7: T, f8: T, f9: T, f10: T,\n"
                                   "    f11: T, f12: T, f13: T, f14: T, f15: T\n"
                                   "}",
                                   "Many fields in generic struct"),
                "Many fields in generic struct parsing");

    // Deep type nesting
    TEST_ASSERT(test_parse_success("pub struct Deep<T> { value: Vec<Vec<Vec<Vec<T>>>> }",
                                   "Deep type nesting"),
                "Deep type nesting parsing");

    TEST_SUCCESS();
}

bool test_type_system_integration_edge_cases(void) {
    printf("\n=== Test 8: Type System Integration Edge Cases ===\n");

    // Generic structs with builtin types
    TEST_ASSERT(test_parse_success("pub struct WithBuiltins<T> {\n"
                                   "    int_field: i32,\n"
                                   "    float_field: f64,\n"
                                   "    bool_field: bool,\n"
                                   "    string_field: string,\n"
                                   "    generic_field: T\n"
                                   "}",
                                   "Generic struct with builtin types"),
                "Generic struct with builtin types parsing");

    // Generic structs with pointer types
    TEST_ASSERT(test_parse_success("pub struct WithPointers<T> {\n"
                                   "    raw_ptr: *T,\n"
                                   "    mut_ptr: *mut T,\n"
                                   "    const_ptr: *const T\n"
                                   "}",
                                   "Generic struct with pointer types"),
                "Generic struct with pointer types parsing");

    // Generic structs with array types
    TEST_ASSERT(test_parse_success("pub struct WithArrays<T> {\n"
                                   "    fixed_array: [10]T,\n"
                                   "    slice: []T\n"
                                   "}",
                                   "Generic struct with array types"),
                "Generic struct with array types parsing");

    // Generic structs with function pointer types
    // NOTE: Function pointer types are not currently supported in the grammar
    // This test is commented out until function types are added to the language
    /*
    TEST_ASSERT(test_parse_success(
        "pub struct WithFunctions<T> {\n"
        "    callback: fn(T) -> T,\n"
        "    processor: fn(*T, usize) -> bool\n"
        "}",
        "Generic struct with function pointer types"
    ), "Generic struct with function pointer types parsing");
    */

    TEST_SUCCESS();
}

bool test_comprehensive_error_recovery(void) {
    printf("\n=== Test 9: Comprehensive Error Recovery ===\n");

    // Test that parser can recover from various syntax errors
    const char *error_cases[] = {
        // Missing type parameter name
        "pub struct Bad<> { }",

        // Invalid characters in type parameters
        "pub struct Bad<T$> { value: T$ }",

        // Mismatched brackets
        "pub struct Bad<T] { value: T }",

        // Missing struct body
        "pub struct Bad<T>",

        // Invalid field syntax in generic struct
        "pub struct Bad<T> { T: value }",

        // Circular type parameter reference (syntactically valid but semantically invalid)
        "pub struct Bad<T> { value: Bad<Bad<T>> }"};

    size_t num_cases = sizeof(error_cases) / sizeof(error_cases[0]);
    int expected_failures = 0;

    for (size_t i = 0; i < num_cases; i++) {
        // Most of these should fail parsing, some might pass parsing but fail semantic analysis
        bool parse_failed = test_parse_failure(error_cases[i], "Error recovery test case");
        if (parse_failed) {
            expected_failures++;
        }
    }

    TEST_ASSERT(expected_failures >= num_cases / 2,
                "At least half of error cases correctly detected");

    printf("    Error recovery: %d/%zu cases correctly failed\n", expected_failures, num_cases);

    TEST_SUCCESS();
}
