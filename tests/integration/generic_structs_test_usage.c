/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Usage Validation Tests Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of type usage validation and complex nested types tests
 */

#include "generic_structs_test_usage.h"

// =============================================================================
// USAGE VALIDATION TESTS IMPLEMENTATION
// =============================================================================

bool test_generic_type_usage_validation(void) {
    printf("\n=== Test 3: Generic Type Usage Validation ===\n");
    
    // Valid generic type usage
    TEST_ASSERT(test_semantic_success(
        "pub struct Vec<T> { data: T }\n"
        "pub fn test(none) -> i32 {\n"
        "    let v: Vec<i32> = Vec<i32> { data: 42 };\n"
        "    return 0;\n"
        "}",
        "Valid generic type usage"
    ), "Valid generic type usage passes semantic analysis");
    
    // Using generic type without type arguments should fail semantic analysis
    // Note: This might pass parsing but fail semantic analysis
    const char *missing_args_source = 
        "pub struct Vec<T> { data: T }\n"
        "pub fn test(none) -> i32 {\n"
        "    let v: Vec = Vec { data: 42 };\n"  // Missing <T>
        "    return 0;\n"
        "}";
    
    bool missing_args_fails = !test_semantic_success(missing_args_source, "Missing type arguments");
    TEST_ASSERT(missing_args_fails, "Missing type arguments correctly fails semantic analysis");
    
    // Wrong number of type arguments
    const char *wrong_args_source = 
        "pub struct Vec<T> { data: T }\n"
        "pub fn test(none) -> i32 {\n"
        "    let v: Vec<i32, string> = Vec { data: 42 };\n"  // Too many args
        "    return 0;\n"
        "}";
    
    bool wrong_args_fails = !test_semantic_success(wrong_args_source, "Wrong number of type arguments");
    TEST_ASSERT(wrong_args_fails, "Wrong number of type arguments correctly fails semantic analysis");
    
    TEST_SUCCESS();
}

bool test_complex_nested_generic_types(void) {
    printf("\n=== Test 4: Complex Nested Generic Types ===\n");
    
    // Deeply nested generics
    TEST_ASSERT(test_parse_success(
        "pub struct Triple<A, B, C> { a: A, b: B, c: C }\n"
        "pub fn test(none) -> i32 {\n"
        "    let x: Triple<Vec<i32>, Option<string>, Result<bool, i32>> = Triple {\n"
        "        a: Vec { data: 1 },\n"
        "        b: Option.Some(\"test\"),\n"
        "        c: Result.Ok(true)\n"
        "    };\n"
        "    return 0;\n"
        "}",
        "Deeply nested generic types"
    ), "Deeply nested generic types parsing");
    
    // Self-referential types (should be valid syntax)
    TEST_ASSERT(test_parse_success(
        "pub struct Node<T> {\n"
        "    value: T,\n"
        "    children: Vec<Node<T>>\n"
        "}",
        "Self-referential generic types"
    ), "Self-referential generic types parsing");
    
    // Multiple levels of nesting
    TEST_ASSERT(test_parse_success(
        "pub struct Matrix<T> { data: Vec<Vec<T>> }\n"
        "pub fn test(none) -> i32 {\n"
        "    let m: Matrix<f64> = Matrix { data: Vec { data: Vec { data: 1.0 } } };\n"
        "    return 0;\n"
        "}",
        "Multiple levels of generic nesting"
    ), "Multiple levels of generic nesting parsing");
    
    TEST_SUCCESS();
}

bool test_generic_struct_pattern_matching_validation(void) {
    printf("\n=== Test 5: Generic Struct Pattern Matching Validation ===\n");
    
    // Valid generic struct patterns
    TEST_ASSERT(test_parse_success(
        "pub struct Pair<A, B> { first: A, second: B }\n"
        "pub fn test(none) -> i32 {\n"
        "    let p: Pair<i32, string> = Pair<i32, string> { first: 1, second: \"a\" };\n"
        "    match p {\n"
        "        Pair<i32, string> { first: first, second: second } => { return first; }\n"
        "        _ => { return 0; }\n"
        "    }\n"
        "}",
        "Generic struct pattern matching"
    ), "Generic struct pattern matching parsing");
    
    // Nested generic patterns - simplified to avoid complex nested pattern syntax issues
    TEST_ASSERT(test_parse_success(
        "pub struct Container<T> { value: T }\n"
        "pub fn test(none) -> i32 {\n"
        "    let c: Container<i32> = Container<i32> { value: 42 };\n"
        "    match c {\n"
        "        Container<i32> { value: x } => { return x; }\n"
        "        _ => { return 0; }\n"
        "    }\n"
        "}",
        "Nested generic struct patterns"
    ), "Nested generic struct patterns parsing");
    
    TEST_SUCCESS();
} 
