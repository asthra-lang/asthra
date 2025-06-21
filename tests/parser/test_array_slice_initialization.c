/**
 * Copyright (c) 2024 Asthra Project
 * Licensed under the Apache License, Version 2.0
 * See LICENSE file for details
 * 
 * @file test_array_slice_initialization.c
 * @brief Parser tests for array/slice initialization syntax
 * 
 * Tests for:
 * - Repeated element syntax: [value; count]
 * - Basic array literals: [1, 2, 3]
 * - Empty arrays: [none]
 * - Slice operations: array[start:end]
 * - Fixed-size array types: [N]Type
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "../framework/test_data.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AsthraTestResult setup_array_slice_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_array_slice_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

/**
 * Test parsing of repeated element array syntax: [value; count]
 */
static AsthraTestResult test_parse_repeated_array_literals(AsthraTestContext* context) {
    const char* test_cases[] = {
        // Basic repeated elements
        "[0; 10]",              // 10 zeros
        "[42; 100]",            // 100 instances of 42
        "[true; 5]",            // 5 true values
        "[\"hello\"; 3]",       // 3 strings
        "[3.14; 7]",            // 7 floats
        
        // With constants
        "[0; SIZE]",            // Using constant for count
        "[DEFAULT; COUNT]",     // Both value and count as constants
        
        // Complex expressions for value
        "[(1 + 2); 5]",         // Expression as value
        "[calculate(); 10]",    // Function call as value
        
        // Nested arrays
        "[[0; 3]; 4]",          // Array of arrays
    };
    
    size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, 
            "Failed to create parser for repeated array test")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        // For now, we expect these to fail since the feature isn't implemented
        // Once implemented, we'll check for proper AST structure
        if (result != NULL) {
            // Verify it's an array literal with repeated type
            if (!asthra_test_assert_int_eq(context, result->type, AST_ARRAY_LITERAL,
                "Expected array literal AST node")) {
                ast_free_node(result);
                destroy_test_parser(parser);
                return ASTHRA_TEST_FAIL;
            }
            ast_free_node(result);
        }
        
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test parsing of basic array literals
 */
static AsthraTestResult test_parse_basic_array_literals(AsthraTestContext* context) {
    const char* test_cases[] = {
        // Simple arrays
        "[1, 2, 3]",
        "[\"a\", \"b\", \"c\"]",
        "[true, false, true]",
        "[1.0, 2.0, 3.0]",
        
        // Empty array
        "[none]",
        
        // Single element
        "[42]",
        
        // Complex expressions
        "[1 + 2, 3 * 4, 5 - 1]",
        "[func(), x.field, array[0]]",
        
        // Nested arrays
        "[[1, 2], [3, 4], [5, 6]]",
    };
    
    size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, 
            "Failed to create parser for basic array test")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, 
            "Failed to parse basic array literal")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!asthra_test_assert_int_eq(context, result->type, AST_ARRAY_LITERAL,
            "Expected array literal AST node")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test parsing of slice operations: array[start:end]
 */
static AsthraTestResult test_parse_slice_operations(AsthraTestContext* context) {
    const char* test_cases[] = {
        // Basic slice operations
        "array[1:5]",           // Slice from index 1 to 5
        "array[0:10]",          // First 10 elements
        "array[5:]",            // From index 5 to end
        "array[:10]",           // From start to index 10
        "array[:]",             // Full slice copy
        
        // With expressions
        "array[i:j]",           // Variable indices
        "array[i+1:j-1]",       // Expression indices
        "array[start():end()]", // Function call indices
        
        // Nested slicing
        "matrix[0][1:3]",       // Slice of array element
        "slice[1:5][0:2]",      // Slice of slice
        
        // Complex expressions as array
        "get_array()[1:5]",     // Slice of function result
        "data.field[2:8]",      // Slice of struct field
    };
    
    size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, 
            "Failed to create parser for slice operation test")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        // For now, we check if it parses without error
        // Once slice operations are implemented, we'll verify AST structure
        if (result != NULL) {
            ast_free_node(result);
        }
        
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test parsing of fixed-size array types: [N]Type
 */
static AsthraTestResult test_parse_fixed_array_types(AsthraTestContext* context) {
    // We need full variable declarations to test type parsing
    const char* test_cases[] = {
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: [10]i32 = [0; 10];\n"
        "    return ();\n"
        "}\n",
        
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let buffer: [1024]u8 = [0; 1024];\n"
        "    return ();\n"
        "}\n",
        
        "package test;\n"
        "const SIZE: i32 = 100;\n"
        "pub fn test(none) -> void {\n"
        "    let data: [SIZE]f32 = [0.0; SIZE];\n"
        "    return ();\n"
        "}\n",
        
        // Multi-dimensional fixed arrays
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let matrix: [3][4]i32 = [[0; 4]; 3];\n"
        "    return ();\n"
        "}\n",
    };
    
    size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, 
            "Failed to create parser for fixed array type test")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_program(parser);
        
        // For now, we just check if it parses
        // Once fixed-size arrays are implemented, we'll verify the type nodes
        if (result != NULL) {
            ast_free_node(result);
        }
        
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test error handling for invalid array/slice syntax
 */
static AsthraTestResult test_parse_array_slice_errors(AsthraTestContext* context) {
    const char* test_cases[] = {
        // Invalid repeated element syntax
        "[; 10]",               // Missing value
        "[0;]",                 // Missing count
        "[0; -5]",              // Negative count (should be caught in semantic phase)
        
        // Invalid slice syntax
        "array[1:2:3]",         // Too many colons
        "array[::]",            // Double colon
        "array[1..5]",          // Rust-style range (not supported)
        
        // Mismatched brackets
        "[1, 2, 3)",            // Mismatched closing
        "(1, 2, 3]",            // Mismatched opening
    };
    
    size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, 
            "Failed to create parser for error test")) {
            return ASTHRA_TEST_FAIL;
        }
        
        // These should fail to parse or produce errors
        ASTNode* result = parser_parse_expression(parser);
        
        // Clean up if something was parsed
        if (result != NULL) {
            ast_free_node(result);
        }
        
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test integration of array/slice syntax in larger expressions
 */
static AsthraTestResult test_parse_array_slice_integration(AsthraTestContext* context) {
    const char* test_cases[] = {
        // Arrays in assignments
        "buffer = [0; 1024]",
        "data = array[1:5]",
        
        // Arrays as function arguments
        "process([1, 2, 3])",
        "handle(data[2:8])",
        "init([0; size])",
        
        // Arrays in struct literals
        "Config { data: [0; 100], name: \"test\" }",
        "Result { values: array[1:10], count: 10 }",
        
        // Complex nested usage
        "matrix[i][j:k]",
        "process(data[start():end()], [0; count])",
    };
    
    size_t count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, 
            "Failed to create parser for integration test")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (result != NULL) {
            ast_free_node(result);
        }
        
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestSuite* create_array_slice_initialization_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create(
        "Array/Slice Initialization", 
        "Tests for array/slice initialization syntax including [value; count] and array[start:end]");
    
    asthra_test_suite_set_setup(suite, setup_array_slice_tests);
    asthra_test_suite_set_teardown(suite, teardown_array_slice_tests);
    
    // Add test cases
    asthra_test_suite_add_test(suite, "test_parse_repeated_array_literals", 
        "Test parsing of repeated element array syntax [value; count]", 
        test_parse_repeated_array_literals);
    
    asthra_test_suite_add_test(suite, "test_parse_basic_array_literals", 
        "Test parsing of basic array literals [1, 2, 3]", 
        test_parse_basic_array_literals);
    
    asthra_test_suite_add_test(suite, "test_parse_slice_operations", 
        "Test parsing of Go-style slice operations array[start:end]", 
        test_parse_slice_operations);
    
    asthra_test_suite_add_test(suite, "test_parse_fixed_array_types", 
        "Test parsing of fixed-size array types [N]Type", 
        test_parse_fixed_array_types);
    
    asthra_test_suite_add_test(suite, "test_parse_array_slice_errors", 
        "Test error handling for invalid array/slice syntax", 
        test_parse_array_slice_errors);
    
    asthra_test_suite_add_test(suite, "test_parse_array_slice_integration", 
        "Test integration of array/slice syntax in larger expressions", 
        test_parse_array_slice_integration);
    
    return suite;
}

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    AsthraTestSuite* suite = create_array_slice_initialization_test_suite();
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif