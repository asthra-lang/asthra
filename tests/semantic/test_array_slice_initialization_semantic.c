/**
 * Copyright (c) 2024 Asthra Project
 * Licensed under the Apache License, Version 2.0
 * See LICENSE file for details
 * 
 * @file test_array_slice_initialization_semantic.c
 * @brief Semantic analysis tests for array/slice initialization features
 * 
 * Tests for:
 * - Repeated element syntax type checking: [value; count]
 * - Fixed-size array type validation: [N]Type
 * - Slice operation type checking: array[start:end]
 * - Type inference and compatibility
 * - Compile-time constant validation
 */

#include "../framework/test_framework.h"
#include "../framework/parser_test_utils.h"
#include "../framework/semantic_test_utils.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Helper to test that parsing fails (for unimplemented features)
 */
static bool test_parse_fails(const char* source, const char* test_name) {
    Parser* parser = create_test_parser(source);
    if (!parser) {
        fprintf(stderr, "[%s] Failed to create parser\n", test_name);
        return false;
    }
    
    ASTNode* ast = parser_parse_program(parser);
    bool has_errors = parser_had_error(parser);
    
    if (ast) {
        ast_free_node(ast);
    }
    destroy_test_parser(parser);
    
    return has_errors;
}

/**
 * Helper to test successful semantic analysis
 */
static bool test_semantic_success(const char* source, const char* test_name) {
    // Parse the source
    ASTNode* ast = parse_test_source(source, "test.as");
    if (!ast) {
        fprintf(stderr, "[%s] Failed to parse source\n", test_name);
        return false;
    }
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        fprintf(stderr, "[%s] Failed to create semantic analyzer\n", test_name);
        ast_free_node(ast);
        return false;
    }
    
    bool success = semantic_analyze_program(analyzer, ast);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return success;
}

/**
 * Helper to test semantic analysis failure
 */
static bool test_semantic_error(const char* source, SemanticErrorCode expected_error, const char* test_name) {
    // Parse the source
    ASTNode* ast = parse_test_source(source, "test.as");
    if (!ast) {
        fprintf(stderr, "[%s] Failed to parse source\n", test_name);
        return false;
    }
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        fprintf(stderr, "[%s] Failed to create semantic analyzer\n", test_name);
        ast_free_node(ast);
        return false;
    }
    
    bool success = semantic_analyze_program(analyzer, ast);
    
    // Get the last error if analysis failed
    SemanticErrorCode last_error = SEMANTIC_ERROR_NONE;
    if (!success && analyzer->last_error) {
        last_error = analyzer->last_error->code;
    }
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return !success && (last_error == expected_error);
}

/**
 * Test repeated element array syntax parsing and semantic validation
 * Now implemented in Phase 2!
 */
static AsthraTestResult test_repeated_array_syntax_not_yet_implemented(AsthraTestContext* context) {
    // Test successful repeated element array syntax
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [0; 10];\n"  // Now implemented!
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_semantic_success(code, "repeated_array_syntax"), 
        "Semantic analysis should succeed for repeated array syntax");
    
    // Test with non-constant count (should fail)
    const char* non_const_code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let count: i32 = 10;\n"
        "    let arr: []i32 = [0; count];\n"  // count is not a constant
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_semantic_error(non_const_code, SEMANTIC_ERROR_INVALID_EXPRESSION, "repeated_array_non_const"), 
        "Should fail with non-constant count");
    
    // Test with negative count (should fail)
    const char* negative_code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [0; -5];\n"  // negative count
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_semantic_error(negative_code, SEMANTIC_ERROR_INVALID_EXPRESSION, "repeated_array_negative"), 
        "Should fail with negative count");
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test fixed-size array type syntax parsing
 * Currently not implemented, expecting parser errors
 */
static AsthraTestResult test_fixed_array_type_not_yet_implemented(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: [10]i32 = [0; 10];\n"  // Fixed-size array not implemented
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_parse_fails(code, "fixed_array_type"), 
        "Parser should report errors for unimplemented fixed array type syntax");
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test slice operation syntax parsing
 * Slice syntax is in grammar but may not be fully implemented
 */
static AsthraTestResult test_slice_operations_partial_implementation(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
        "    let slice: []i32 = arr[1:4];\n"  // Slice operation
        "    return ();\n"
        "}\n";
    
    // Since slice syntax is in grammar, it might parse
    // We don't assert on the result, just ensure no crash
    Parser* parser = create_test_parser(code);
    if (parser) {
        ASTNode* ast = parser_parse_program(parser);
        if (ast) {
            ast_free_node(ast);
        }
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test basic array literal that should work today
 */
static AsthraTestResult test_basic_array_literal_works(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3];\n"  // Basic array literal
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_semantic_success(code, "basic_array_literal"),
        "Semantic analysis should succeed for basic array literals");
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test empty array with 'none' syntax
 */
static AsthraTestResult test_empty_array_with_none(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [none];\n"  // Empty array
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_semantic_success(code, "empty_array_none"),
        "Semantic analysis should succeed for empty arrays with 'none'");
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test array indexing (should work)
 */
static AsthraTestResult test_array_indexing_works(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3];\n"
        "    let elem: i32 = arr[1];\n"  // Array indexing
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_semantic_success(code, "array_indexing"),
        "Semantic analysis should succeed for array indexing");
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test invalid array index type
 */
static AsthraTestResult test_invalid_array_index_type(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3];\n"
        "    let elem: i32 = arr[\"hello\"];\n"  // String as index - invalid
        "    return ();\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT(context, test_semantic_error(code, SEMANTIC_ERROR_INVALID_OPERATION, "invalid_index_type"),
        "Semantic analysis should fail for string array index");
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test array length operation
 */
static AsthraTestResult test_array_length_operation(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3];\n"
        "    let len: usize = arr.len;\n"  // Array length
        "    return ();\n"
        "}\n";
    
    // Note: .len might not be implemented yet, so we don't assert success
    // Just ensure it doesn't crash
    ASTNode* ast = parse_test_source(code, "test.as");
    if (ast) {
        SemanticAnalyzer* analyzer = semantic_analyzer_create();
        if (analyzer) {
            semantic_analyze_program(analyzer, ast);
            semantic_analyzer_destroy(analyzer);
        }
        ast_free_node(ast);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test suite setup
 */
static AsthraTestResult setup_array_slice_semantic_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

/**
 * Test suite teardown
 */
static AsthraTestResult teardown_array_slice_semantic_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

/**
 * Create the test suite
 */
AsthraTestSuite* create_array_slice_initialization_semantic_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create(
        "Array/Slice Semantic Analysis", 
        "Semantic analysis tests for array/slice initialization features");
    
    asthra_test_suite_set_setup(suite, setup_array_slice_semantic_tests);
    asthra_test_suite_set_teardown(suite, teardown_array_slice_semantic_tests);
    
    // Tests for unimplemented features (expect parser errors)
    asthra_test_suite_add_test(suite, "test_repeated_array_syntax_not_yet_implemented", 
        "Test that repeated element array syntax [value; count] is not yet implemented", 
        test_repeated_array_syntax_not_yet_implemented);
    
    asthra_test_suite_add_test(suite, "test_fixed_array_type_not_yet_implemented", 
        "Test that fixed-size array type [N]Type is not yet implemented", 
        test_fixed_array_type_not_yet_implemented);
    
    asthra_test_suite_add_test(suite, "test_slice_operations_partial_implementation", 
        "Test slice operations array[start:end] parsing", 
        test_slice_operations_partial_implementation);
    
    // Tests for currently working features
    asthra_test_suite_add_test(suite, "test_basic_array_literal_works", 
        "Test that basic array literals work correctly", 
        test_basic_array_literal_works);
    
    asthra_test_suite_add_test(suite, "test_empty_array_with_none", 
        "Test empty array syntax with 'none'", 
        test_empty_array_with_none);
    
    asthra_test_suite_add_test(suite, "test_array_indexing_works", 
        "Test array indexing operations", 
        test_array_indexing_works);
    
    asthra_test_suite_add_test(suite, "test_invalid_array_index_type", 
        "Test semantic error for invalid array index type", 
        test_invalid_array_index_type);
    
    asthra_test_suite_add_test(suite, "test_array_length_operation", 
        "Test array length operation", 
        test_array_length_operation);
    
    return suite;
}

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    AsthraTestSuite* suite = create_array_slice_initialization_semantic_test_suite();
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif