/**
 * Simple test to verify basic Option functionality without pattern matching
 */

#include "../../framework/test_framework.h"
#include "../../framework/compiler_test_utils.h"
#include "codegen/code_generator.h"
#include "analysis/semantic_analyzer.h"
#include "parser/parser.h"
#include "parser/parser_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test helper
static bool compile_test(const char* source) {
    Parser* parser = create_test_parser(source);
    if (!parser) return false;
    
    ASTNode* ast = parser_parse_program(parser);
    if (!ast) {
        fprintf(stderr, "Parser failed\n");
        if (parser_had_error(parser)) {
            fprintf(stderr, "Parser had errors\n");
        }
        destroy_test_parser(parser);
        return false;
    }
    
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }
    
    bool semantic_success = semantic_analyze_program(analyzer, ast);
    if (!semantic_success) {
        // Print semantic errors for debugging
        fprintf(stderr, "Semantic analysis failed\n");
        fprintf(stderr, "Semantic analysis returned false\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }
    
    CodeGenerator* generator = code_generator_create(TARGET_ARCH_AARCH64, CALLING_CONV_AARCH64_AAPCS);
    if (!generator) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }
    
    bool codegen_success = code_generate_program(generator, ast);
    
    code_generator_destroy(generator);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    destroy_test_parser(parser);
    
    return codegen_success;
}

// Main test
int main(void) {
    printf("=== Testing Simple Option Usage ===\n\n");
    
    // Test 1: Basic Option type declaration
    printf("Test 1: Option type declaration...\n");
    const char* test1 = 
        "package test;\n"
        "pub fn test_option(none) -> void {\n"
        "    let opt: Option<i32>;\n"
        "    return ();\n"
        "}\n";
    
    if (compile_test(test1)) {
        printf("✓ Option type declaration works\n");
    } else {
        printf("✗ Option type declaration failed\n");
    }
    
    // Test 2: Option.Some construction
    printf("\nTest 2: Option.Some construction...\n");
    const char* test2 = 
        "package test;\n"
        "pub fn test_some(none) -> void {\n"
        "    let opt: Option<i32> = Option.Some(42);\n"
        "    return ();\n"
        "}\n";
    
    if (compile_test(test2)) {
        printf("✓ Option.Some construction works\n");
    } else {
        printf("✗ Option.Some construction failed\n");
    }
    
    // Test 3: Option.None construction
    printf("\nTest 3: Option.None construction...\n");
    const char* test3 = 
        "package test;\n"
        "pub fn test_none(none) -> void {\n"
        "    let opt: Option<i32> = Option.None;\n"
        "    return ();\n"
        "}\n";
    
    if (compile_test(test3)) {
        printf("✓ Option.None construction works\n");
    } else {
        printf("✗ Option.None construction failed\n");
    }
    
    // Test 4: Simple match statement (no Option)
    printf("\nTest 4: Simple match statement...\n");
    const char* test4 = 
        "package test;\n"
        "pub fn test_match(x: i32) -> i32 {\n"
        "    let result: i32 = 0;\n"
        "    match x {\n"
        "        42 => { result = 1; },\n"
        "        _ => { result = 0; }\n"
        "    }\n"
        "    return result;\n"
        "}\n";
    
    if (compile_test(test4)) {
        printf("✓ Simple match statement works\n");
    } else {
        printf("✗ Simple match statement failed\n");
    }
    
    return 0;
}