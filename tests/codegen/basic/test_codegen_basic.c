/**
 * Basic Code Generation Test
 * 
 * This test verifies that the code generator can handle basic programs
 * without encountering the 0x1 parser issue.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Include the code generator header for struct definition
#include "code_generator_core.h"

// Direct declarations (only for types not in the included header)
typedef struct Lexer Lexer;
typedef struct Parser Parser;

// Function declarations
extern Lexer* lexer_create(const char* source, size_t length, const char* filename);
extern void lexer_destroy(Lexer* lexer);
extern Parser* parser_create(Lexer* lexer);
extern void parser_destroy(Parser* parser);
extern ASTNode* parser_parse_program(Parser* parser);
extern SemanticAnalyzer* semantic_analyzer_create(void);
extern void semantic_analyzer_destroy(SemanticAnalyzer* analyzer);
extern bool semantic_analyze_program(SemanticAnalyzer* analyzer, ASTNode* ast);
extern CodeGenerator* code_generator_create(TargetArchitecture arch, CallingConvention conv);
extern void code_generator_destroy(CodeGenerator* generator);
extern bool code_generate_program(CodeGenerator* generator, ASTNode* ast);

bool test_codegen(const char* name, const char* source) {
    printf("Testing %s...\n", name);
    
    // Parse
    Lexer* lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("  FAIL: Could not create lexer\n");
        return false;
    }
    
    Parser* parser = parser_create(lexer);
    if (!parser) {
        printf("  FAIL: Could not create parser\n");
        lexer_destroy(lexer);
        return false;
    }
    
    ASTNode* ast = parser_parse_program(parser);
    parser_destroy(parser);
    
    if (!ast || ast == (void*)0x1) {
        printf("  FAIL: Parser failed (returned %p)\n", (void*)ast);
        return false;
    }
    
    // Analyze
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("  FAIL: Could not create analyzer\n");
        return false;
    }
    
    bool analysis_result = semantic_analyze_program(analyzer, ast);
    
    if (!analysis_result) {
        printf("  FAIL: Semantic analysis failed\n");
        semantic_analyzer_destroy(analyzer);
        return false;
    }
    
    // Generate code
    CodeGenerator* generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator) {
        printf("  FAIL: Could not create code generator\n");
        semantic_analyzer_destroy(analyzer);
        return false;
    }
    
    // Set the semantic analyzer in the code generator
    // This is required for the code generator to access type information
    generator->semantic_analyzer = analyzer;
    
    bool codegen_result = code_generate_program(generator, ast);
    code_generator_destroy(generator);
    semantic_analyzer_destroy(analyzer);
    
    if (!codegen_result) {
        printf("  FAIL: Code generation failed\n");
        return false;
    }
    
    printf("  PASS\n");
    return true;
}

int main(void) {
    printf("=== Basic Code Generation Tests ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Minimal program
    {
        total++;
        if (test_codegen("minimal program", "package test;")) {
            passed++;
        }
    }
    
    // Test 2: Simple struct
    {
        total++;
        if (test_codegen("simple struct", 
            "package test;\n"
            "pub struct Point { pub x: i32, pub y: i32 }")) {
            passed++;
        }
    }
    
    // Test 3: Simple function
    {
        total++;
        if (test_codegen("simple function",
            "package test;\n"
            "pub fn add(a: i32, b: i32) -> i32 {\n"
            "    return a + b;\n"
            "}")) {
            passed++;
        }
    }
    
    // Test 4: Main function
    {
        total++;
        if (test_codegen("main function",
            "package test;\n"
            "pub fn main(none) -> void {\n"
            "    let x: i32 = 42;\n"
            "}")) {
            passed++;
        }
    }
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}