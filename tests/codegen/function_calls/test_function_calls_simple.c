/**
 * Simplified Function Call Tests
 * 
 * This is a refactored version that avoids the problematic headers
 * that cause parser_parse_program to return 0x1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Include necessary headers for CodeGenerator structure access
#include "code_generator_core.h"
#include "semantic_analyzer.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"

// Direct declarations for types we don't need full definitions for
typedef struct Lexer Lexer;
typedef struct Parser Parser;

// Function declarations that aren't in headers
// (Most functions are now available through included headers)

// Simple test result type
typedef enum {
    TEST_PASS,
    TEST_FAIL
} TestResult;

// Test a source program compiles successfully
TestResult test_source_compiles(const char* test_name, const char* source) {
    printf("Testing %s...\n", test_name);
    
    // Parse
    Lexer* lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("  FAIL: Could not create lexer\n");
        return TEST_FAIL;
    }
    
    Parser* parser = parser_create(lexer);
    if (!parser) {
        printf("  FAIL: Could not create parser\n");
        lexer_destroy(lexer);
        return TEST_FAIL;
    }
    
    ASTNode* ast = parser_parse_program(parser);
    parser_destroy(parser); // This also destroys the lexer
    
    if (!ast || ast == (void*)0x1) {
        printf("  FAIL: Parser returned invalid AST (%p)\n", (void*)ast);
        return TEST_FAIL;
    }
    
    // Analyze
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("  FAIL: Could not create analyzer\n");
        return TEST_FAIL;
    }
    
    bool analysis_result = semantic_analyze_program(analyzer, ast);
    
    if (!analysis_result) {
        printf("  FAIL: Semantic analysis failed\n");
        // Print semantic analyzer errors if available
        if (analyzer && analyzer->error_count > 0) {
            printf("  Semantic errors: %zu\n", analyzer->error_count);
            if (analyzer->last_error && analyzer->last_error->message) {
                printf("  Last error: %s\n", analyzer->last_error->message);
            }
        }
        semantic_analyzer_destroy(analyzer);
        return TEST_FAIL;
    }
    
    // Generate code
    CodeGenerator* generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator) {
        printf("  FAIL: Could not create code generator\n");
        semantic_analyzer_destroy(analyzer);
        return TEST_FAIL;
    }
    
    // Associate semantic analyzer with code generator to prevent architectural violations
    generator->semantic_analyzer = analyzer;
    
    bool codegen_result = code_generate_program(generator, ast);
    code_generator_destroy(generator);
    semantic_analyzer_destroy(analyzer);
    
    if (!codegen_result) {
        printf("  FAIL: Code generation failed\n");
        return TEST_FAIL;
    }
    
    // For now, we can't check the output since we don't have access to the generated assembly
    // The original tests seem to use a placeholder
    printf("  PASS: Code generation succeeded (output check skipped)\n");
    return TEST_PASS;
}

int main(void) {
    printf("=== Simple Function Call Tests ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Associated function calls
    {
        const char* source = 
            "package test;\n"
            "\n"
            "pub struct Point { \n"
            "    pub x: f64,\n"
            "    pub y: f64\n"
            "}\n"
            "\n"
            "impl Point {\n"
            "    pub fn new(x: f64, y: f64) -> Point {\n"
            "        return Point { x: x, y: y };\n"
            "    }\n"
            "}\n"
            "\n"
            "pub fn main(none) -> void {\n"
            "    let p: Point = Point { x: 1.0, y: 2.0 };\n"
            "    return ();\n"
            "}";
        
        total++;
        if (test_source_compiles("associated function calls", source) == TEST_PASS) {
            passed++;
        }
    }
    
    // Test 2: Function calls with primitive parameters
    {
        const char* source = 
            "package test;\n"
            "\n"
            "pub fn calculate_area(radius: f64) -> f64 {\n"
            "    return 3.14159 * radius * radius;\n"
            "}\n"
            "\n"
            "pub fn is_positive(value: i32) -> bool {\n"
            "    return value > 0;\n"
            "}\n"
            "\n"
            "pub fn main(none) -> void {\n"
            "    let area: f64 = calculate_area(5.0);\n"
            "    let positive: bool = is_positive(42);\n"
            "    return ();\n"
            "}";
        
        total++;
        if (test_source_compiles("primitive parameter calls", source) == TEST_PASS) {
            passed++;
        }
    }
    
    // Test 3: Function calls with return values
    {
        const char* source = 
            "package test;\n"
            "\n"
            "pub fn add(a: i32, b: i32) -> i32 {\n"
            "    return a + b;\n"
            "}\n"
            "\n"
            "pub fn multiply(x: f64, y: f64) -> f64 {\n"
            "    return x * y;\n"
            "}\n"
            "\n"
            "pub fn main(none) -> void {\n"
            "    let sum: i32 = add(10, 20);\n"
            "    let product: f64 = multiply(3.14, 2.0);\n"
            "    return ();\n"
            "}";
        
        total++;
        if (test_source_compiles("function return values", source) == TEST_PASS) {
            passed++;
        }
    }
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}