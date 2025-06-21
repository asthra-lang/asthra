/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Declaration Tests Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of declaration edge cases and syntax validation tests
 */

#include "generic_structs_test_declarations.h"

// =============================================================================
// DECLARATION VALIDATION TESTS IMPLEMENTATION
// =============================================================================

bool test_generic_struct_declaration_edge_cases(void) {
    printf("\n=== Test 1: Generic Struct Declaration Edge Cases ===\n");
    
    // Single character type parameters
    TEST_ASSERT(test_parse_success(
        "pub struct X<A> { value: A }",
        "Single character type parameter"
    ), "Single character type parameter parsing");
    
    // Long type parameter names
    TEST_ASSERT(test_parse_success(
        "pub struct Container<TypeParameter> { value: TypeParameter }",
        "Long type parameter name"
    ), "Long type parameter name parsing");
    
    // Maximum reasonable number of type parameters
    TEST_ASSERT(test_parse_success(
        "pub struct Multi<A, B, C, D, E, F> { a: A, b: B, c: C, d: D, e: E, f: F }",
        "Multiple type parameters (6)"
    ), "Multiple type parameters parsing");
    
    // Empty struct body with generics
    TEST_ASSERT(test_parse_success(
        "pub struct Empty<T> { }",
        "Empty generic struct"
    ), "Empty generic struct parsing");
    
    // Generic struct with complex field types
    TEST_ASSERT(test_parse_success(
        "pub struct Complex<T> { ptr: *mut T, array: [10]T, optional: Option<T> }",
        "Complex field types"
    ), "Complex field types parsing");
    
    TEST_SUCCESS();
}

bool test_invalid_generic_struct_syntax(void) {
    printf("\n=== Test 2: Invalid Generic Struct Syntax ===\n");
    
    // Missing closing angle bracket
    TEST_ASSERT(test_parse_failure(
        "pub struct Vec<T { data: T }",
        "Missing closing angle bracket"
    ), "Missing closing angle bracket correctly fails");
    
    // Missing opening angle bracket
    TEST_ASSERT(test_parse_failure(
        "pub struct VecT> { data: T }",
        "Missing opening angle bracket"
    ), "Missing opening angle bracket correctly fails");
    
    // Empty type parameter list
    TEST_ASSERT(test_parse_failure(
        "pub struct Vec<> { data: i32 }",
        "Empty type parameter list"
    ), "Empty type parameter list correctly fails");
    
    // Duplicate type parameters - this is a semantic error, not a parse error
    // The parser accepts it, but semantic analysis should reject it
    TEST_ASSERT(!test_semantic_success(
        "pub struct Dup<T, T> { first: T, second: T }",
        "Duplicate type parameters"
    ), "Duplicate type parameters correctly fails semantic analysis");
    
    // Invalid type parameter names
    TEST_ASSERT(test_parse_failure(
        "pub struct Bad<123> { value: 123 }",
        "Numeric type parameter name"
    ), "Numeric type parameter name correctly fails");
    
    // Missing comma between type parameters
    TEST_ASSERT(test_parse_failure(
        "pub struct Pair<A B> { first: A, second: B }",
        "Missing comma between type parameters"
    ), "Missing comma between type parameters correctly fails");
    
    TEST_SUCCESS();
} 
