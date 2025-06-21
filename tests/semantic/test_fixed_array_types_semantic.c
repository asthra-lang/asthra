/**
 * Test fixed-size array type syntax and semantic analysis
 * Tests the implementation of [N]Type syntax
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
 * Test basic fixed-size array type declarations
 */
static AsthraTestResult test_basic_fixed_array_types(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr1: [10]i32 = [0; 10];\n"      // Fixed array of 10 i32s
        "    let arr2: [256]u8 = [0; 256];\n"    // Fixed array of 256 bytes
        "    let arr3: [5]bool = [false; 5];\n"  // Fixed array of 5 bools
        "    return ();\n"
        "}\n";
    
    // Parse the source
    ASTNode* ast = parse_test_source(code, "test.as");
    ASTHRA_TEST_ASSERT(context, ast != NULL, "Failed to parse fixed array types");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Failed to create semantic analyzer");
    
    bool success = semantic_analyze_program(analyzer, ast);
    ASTHRA_TEST_ASSERT(context, success, "Semantic analysis should succeed for fixed array types");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test const expressions in array sizes
 */
static AsthraTestResult test_const_expr_array_sizes(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "const SIZE: i32 = 100;\n"
        "const DOUBLE_SIZE: i32 = SIZE * 2;\n"
        "pub fn test(none) -> void {\n"
        "    let arr1: [SIZE]i32 = [0; SIZE];\n"
        "    let arr2: [DOUBLE_SIZE]u8 = [0; DOUBLE_SIZE];\n"
        "    let arr3: [10 + 5]i32 = [0; 15];\n"  // Const expression
        "    return ();\n"
        "}\n";
    
    // Parse the source
    ASTNode* ast = parse_test_source(code, "test.as");
    ASTHRA_TEST_ASSERT(context, ast != NULL, "Failed to parse const array sizes");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Failed to create semantic analyzer");
    
    bool success = semantic_analyze_program(analyzer, ast);
    ASTHRA_TEST_ASSERT(context, success, "Semantic analysis should succeed for const array sizes");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test invalid array sizes
 */
static AsthraTestResult test_invalid_array_sizes(AsthraTestContext* context) {
    // Test non-constant size
    const char* code1 = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let size: i32 = 10;\n"
        "    let arr: [size]i32 = [0; 10];\n"  // Error: non-constant size
        "    return ();\n"
        "}\n";
    
    ASTNode* ast1 = parse_test_source(code1, "test.as");
    ASTHRA_TEST_ASSERT(context, ast1 != NULL, "Failed to parse non-constant size");
    
    SemanticAnalyzer* analyzer1 = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer1 != NULL, "Failed to create semantic analyzer");
    
    bool success1 = semantic_analyze_program(analyzer1, ast1);
    ASTHRA_TEST_ASSERT(context, !success1, "Semantic analysis should fail for non-constant size");
    
    semantic_analyzer_destroy(analyzer1);
    ast_free_node(ast1);
    
    // Test negative size
    const char* code2 = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: [-5]i32 = [0; 5];\n"  // Error: negative size
        "    return ();\n"
        "}\n";
    
    ASTNode* ast2 = parse_test_source(code2, "test.as");
    ASTHRA_TEST_ASSERT(context, ast2 != NULL, "Failed to parse negative size");
    
    SemanticAnalyzer* analyzer2 = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer2 != NULL, "Failed to create semantic analyzer");
    
    bool success2 = semantic_analyze_program(analyzer2, ast2);
    ASTHRA_TEST_ASSERT(context, !success2, "Semantic analysis should fail for negative size");
    
    semantic_analyzer_destroy(analyzer2);
    ast_free_node(ast2);
    
    // Test non-integer size
    const char* code3 = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: [5.5]i32 = [0; 5];\n"  // Error: float size
        "    return ();\n"
        "}\n";
    
    ASTNode* ast3 = parse_test_source(code3, "test.as");
    ASTHRA_TEST_ASSERT(context, ast3 != NULL, "Failed to parse float size");
    
    SemanticAnalyzer* analyzer3 = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer3 != NULL, "Failed to create semantic analyzer");
    
    bool success3 = semantic_analyze_program(analyzer3, ast3);
    ASTHRA_TEST_ASSERT(context, !success3, "Semantic analysis should fail for non-integer size");
    
    semantic_analyzer_destroy(analyzer3);
    ast_free_node(ast3);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test multi-dimensional fixed arrays
 */
static AsthraTestResult test_multidimensional_fixed_arrays(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let matrix: [3][4]i32 = [[0; 4]; 3];\n"     // 3x4 matrix
        "    let cube: [2][3][4]i32 = [[[0; 4]; 3]; 2];\n" // 2x3x4 cube
        "    return ();\n"
        "}\n";
    
    // Parse the source
    ASTNode* ast = parse_test_source(code, "test.as");
    ASTHRA_TEST_ASSERT(context, ast != NULL, "Failed to parse multidimensional arrays");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Failed to create semantic analyzer");
    
    bool success = semantic_analyze_program(analyzer, ast);
    ASTHRA_TEST_ASSERT(context, success, "Semantic analysis should succeed for multidimensional arrays");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test type compatibility with fixed arrays
 */
static AsthraTestResult test_fixed_array_type_compatibility(AsthraTestContext* context) {
    const char* code = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr1: [5]i32 = [0; 5];\n"
        "    let arr2: [5]i32 = arr1;\n"       // OK: same type
        "    let slice: []i32 = arr1[:];\n"    // OK: array to slice
        "    return ();\n"
        "}\n";
    
    // Parse the source
    ASTNode* ast = parse_test_source(code, "test.as");
    ASTHRA_TEST_ASSERT(context, ast != NULL, "Failed to parse array compatibility");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Failed to create semantic analyzer");
    
    bool success = semantic_analyze_program(analyzer, ast);
    ASTHRA_TEST_ASSERT(context, success, "Semantic analysis should succeed for array compatibility");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    
    return ASTHRA_TEST_PASS;
}

// Main function
int main(int argc, char **argv) {
    printf("Running test suite: Fixed-Size Array Types\n");
    printf("Description: Tests for [N]Type fixed-size array syntax\n\n");
    
    AsthraTestContext context = {0};
    int total_tests = 0;
    int passed_tests = 0;
    
    // Run each test
    AsthraTestResult result;
    
    result = test_basic_fixed_array_types(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_basic_fixed_array_types\n");
    } else {
        printf("[FAIL] test_basic_fixed_array_types\n");
    }
    
    result = test_const_expr_array_sizes(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_const_expr_array_sizes\n");
    } else {
        printf("[FAIL] test_const_expr_array_sizes\n");
    }
    
    result = test_invalid_array_sizes(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_invalid_array_sizes\n");
    } else {
        printf("[FAIL] test_invalid_array_sizes\n");
    }
    
    result = test_multidimensional_fixed_arrays(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_multidimensional_fixed_arrays\n");
    } else {
        printf("[FAIL] test_multidimensional_fixed_arrays\n");
    }
    
    result = test_fixed_array_type_compatibility(&context);
    total_tests++;
    if (result == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("[PASS] test_fixed_array_type_compatibility\n");
    } else {
        printf("[FAIL] test_fixed_array_type_compatibility\n");
    }
    
    printf("\nTest Results: %d/%d passed\n", passed_tests, total_tests);
    
    return (passed_tests == total_tests) ? 0 : 1;
}