/**
 * Integration test for slice operations
 * Tests the implementation of array[start:end] slicing with all variants
 */

#include "../framework/test_framework.h"
#include "../framework/parser_test_utils.h"
#include "../framework/semantic_test_utils.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Test basic slice operations
 */
static AsthraTestResult test_basic_slice_operations(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
        "    let slice1: []i32 = arr[1:4];\n"     // [2, 3, 4]
        "    let slice2: []i32 = arr[:3];\n"      // [1, 2, 3]
        "    let slice3: []i32 = arr[2:];\n"      // [3, 4, 5]
        "    let slice4: []i32 = arr[:];\n"       // [1, 2, 3, 4, 5]
        "    return ();\n"
        "}\n";
    
    // Parse the source
    ASTNode* ast = parse_test_source(code, "test.as");
    ASTHRA_TEST_ASSERT(context, ast != NULL, "Failed to parse slice operations");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Failed to create semantic analyzer");
    
    bool success = semantic_analyze_program(analyzer, ast);
    ASTHRA_TEST_ASSERT(context, success, "Semantic analysis should succeed for slice operations");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test slice operations on fixed arrays
 */
static AsthraTestResult test_slice_on_fixed_arrays(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [0; 10];\n"        // Fixed array of 10 zeros
        "    let slice: []i32 = arr[2:8];\n"     // Slice from index 2 to 8
        "    return ();\n"
        "}\n";
    
    // Parse the source
    ASTNode* ast = parse_test_source(code, "test.as");
    ASTHRA_TEST_ASSERT(context, ast != NULL, "Failed to parse fixed array slice");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Failed to create semantic analyzer");
    
    bool success = semantic_analyze_program(analyzer, ast);
    ASTHRA_TEST_ASSERT(context, success, "Semantic analysis should succeed for fixed array slice");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test nested slice operations
 */
static AsthraTestResult test_nested_slice_operations(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];\n"
        "    let slice1: []i32 = arr[2:8];\n"    // [3, 4, 5, 6, 7, 8]
        "    let slice2: []i32 = slice1[1:4];\n" // [4, 5, 6]
        "    return ();\n"
        "}\n";
    
    // Parse the source
    ASTNode* ast = parse_test_source(code, "test.as");
    ASTHRA_TEST_ASSERT(context, ast != NULL, "Failed to parse nested slice operations");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Failed to create semantic analyzer");
    
    bool success = semantic_analyze_program(analyzer, ast);
    ASTHRA_TEST_ASSERT(context, success, "Semantic analysis should succeed for nested slices");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test invalid slice operations
 */
static AsthraTestResult test_invalid_slice_operations(AsthraTestContext* context) {
    // Test slicing non-array type
    const char* code1 = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let slice: []i32 = x[1:3];\n"  // Error: cannot slice i32
        "    return ();\n"
        "}\n";
    
    ASTNode* ast1 = parse_test_source(code1, "test.as");
    ASTHRA_TEST_ASSERT(context, ast1 != NULL, "Failed to parse invalid slice");
    
    SemanticAnalyzer* analyzer1 = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer1 != NULL, "Failed to create semantic analyzer");
    
    bool success1 = semantic_analyze_program(analyzer1, ast1);
    ASTHRA_TEST_ASSERT(context, !success1, "Semantic analysis should fail for slicing non-array type");
    
    semantic_analyzer_destroy(analyzer1);
    ast_free_node(ast1);
    
    // Test non-integer slice indices
    const char* code2 = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
        "    let slice: []i32 = arr[1.5:3.7];\n"  // Error: float indices
        "    return ();\n"
        "}\n";
    
    ASTNode* ast2 = parse_test_source(code2, "test.as");
    ASTHRA_TEST_ASSERT(context, ast2 != NULL, "Failed to parse float indices");
    
    SemanticAnalyzer* analyzer2 = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer2 != NULL, "Failed to create semantic analyzer");
    
    bool success2 = semantic_analyze_program(analyzer2, ast2);
    ASTHRA_TEST_ASSERT(context, !success2, "Semantic analysis should fail for non-integer indices");
    
    semantic_analyzer_destroy(analyzer2);
    ast_free_node(ast2);
    
    return ASTHRA_TEST_PASS;
}

// Main function
int main(int argc, char **argv) {
    printf("Running test suite: Slice Operations Integration\n");
    printf("Description: Integration tests for slice operations implementation\n\n");
    
    AsthraTestContext context = {0};
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run each test
    AsthraTestResult result;
    
    result = test_basic_slice_operations(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_basic_slice_operations\n");
    } else {
        printf("[FAIL] test_basic_slice_operations\n");
    }
    
    result = test_slice_on_fixed_arrays(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_slice_on_fixed_arrays\n");
    } else {
        printf("[FAIL] test_slice_on_fixed_arrays\n");
    }
    
    result = test_nested_slice_operations(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_nested_slice_operations\n");
    } else {
        printf("[FAIL] test_nested_slice_operations\n");
    }
    
    result = test_invalid_slice_operations(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_invalid_slice_operations\n");
    } else {
        printf("[FAIL] test_invalid_slice_operations\n");
    }
    
    printf("\nTest Results: %d/%d passed\n", passed_tests, total_tests);
    
    return (passed_tests == total_tests) ? 0 : 1;
}