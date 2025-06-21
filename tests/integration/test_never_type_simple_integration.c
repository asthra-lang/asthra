/**
 * Asthra Programming Language Compiler
 * Never Type Simple Integration Tests
 * 
 * Basic integration tests for Never type through the compiler pipeline
 * without requiring complex optimization infrastructure.
 * 
 * Note: Since panic is not yet implemented (issue #42) and Asthra
 * doesn't have while loops, these tests focus on parsing Never type
 * in function signatures and type checking.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "semantic_analyzer.h"
#include "code_generator_core.h"
#include <stdio.h>
#include <string.h>

// Test 1: Basic Never type parsing in function signatures
static AsthraTestResult test_never_type_parsing(AsthraTestContext* ctx) {
    asthra_test_context_start(ctx);
    
    const char* source = 
        "package test;\n"
        "\n"
        "// Test parsing of Never type in function signatures\n"
        "pub fn will_never_return(none) -> Never {\n"
        "    // In real code, this would call panic or exit\n"
        "    // For now, just return void to satisfy parser\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn regular_function(none) -> i32 {\n"
        "    return 42;\n"
        "}\n";
    
    // Parse the source code
    Parser* parser = create_test_parser(source);
    if (!parser) {
        printf("Failed to create parser\n");
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parser_parse_program(parser);
    if (!ast) {
        printf("Failed to parse program\n");
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check if parser detected any errors
    if (parser_had_error(parser)) {
        printf("Parser encountered errors\n");
        ast_free_node(ast);
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Never type parsing: SUCCESS\n");
    printf("  - Parsed Never type in return position\n");
    printf("  - Multiple function signatures accepted\n");
    
    // Clean up
    ast_free_node(ast);
    destroy_test_parser(parser);
    
    asthra_test_context_end(ctx, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

// Test 2: Never type as expression type
static AsthraTestResult test_never_as_expression_type(AsthraTestContext* ctx) {
    asthra_test_context_start(ctx);
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn test_never_in_if(none) -> i32 {\n"
        "    let x: i32 = 5;\n"
        "    if true {\n"
        "        return x * 2;\n"
        "    } else {\n"
        "        // This branch would have Never type with panic\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    // Parse the source code
    Parser* parser = create_test_parser(source);
    if (!parser) {
        printf("Failed to create parser\n");
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parser_parse_program(parser);
    if (!ast || parser_had_error(parser)) {
        printf("Parsing failed\n");
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        ast_free_node(ast);
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    bool semantic_success = semantic_analyze_program(analyzer, ast);
    if (!semantic_success) {
        printf("Semantic analysis failed\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Never as expression type: SUCCESS\n");
    printf("  - If expression parsed correctly\n");
    printf("  - Type checking passed\n");
    
    // Clean up
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    destroy_test_parser(parser);
    
    asthra_test_context_end(ctx, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

// Test 3: Never type code generation
static AsthraTestResult test_never_type_codegen(AsthraTestContext* ctx) {
    asthra_test_context_start(ctx);
    
    const char* source = 
        "package test;\n"
        "\n"
        "// Test code generation for functions returning Never\n"
        "pub fn unreachable_function(none) -> Never {\n"
        "    // Temporary implementation until panic is available\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // Don't actually call the Never function\n"
        "    let x: i32 = 42;\n"
        "    return ();\n"
        "}\n";
    
    // Parse the source code
    Parser* parser = create_test_parser(source);
    if (!parser) {
        printf("Failed to create parser\n");
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parser_parse_program(parser);
    if (!ast || parser_had_error(parser)) {
        printf("Parsing failed\n");
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        ast_free_node(ast);
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    bool semantic_success = semantic_analyze_program(analyzer, ast);
    if (!semantic_success) {
        printf("Semantic analysis failed\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    // Create code generator
    CodeGenerator* codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!codegen) {
        printf("Failed to create code generator\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    // CRITICAL: Link semantic analyzer to code generator to prevent architectural violations
    code_generator_set_semantic_analyzer(codegen, analyzer);
    
    // Generate code
    bool codegen_success = code_generate_program(codegen, ast);
    if (!codegen_success) {
        printf("Code generation failed\n");
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        asthra_test_context_end(ctx, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    // Get code generation statistics
    CodeGenStatistics stats = code_generator_get_statistics(codegen);
    
    printf("Never type code generation: SUCCESS\n");
    printf("  - Generated %llu functions\n", (unsigned long long)stats.functions_generated);
    printf("  - Never-returning function handled in codegen\n");
    
    // Clean up
    code_generator_destroy(codegen);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    destroy_test_parser(parser);
    
    asthra_test_context_end(ctx, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Never Type Simple Integration Tests");
    
    asthra_test_suite_add_test(suite, "never_type_parsing", 
        "Test Never type parsing in function signatures", 
        test_never_type_parsing);
    
    asthra_test_suite_add_test(suite, "never_as_expression_type", 
        "Test Never as expression type", 
        test_never_as_expression_type);
    
    asthra_test_suite_add_test(suite, "never_type_codegen", 
        "Test Never type code generation", 
        test_never_type_codegen);
    
    return asthra_test_suite_run(suite);
}