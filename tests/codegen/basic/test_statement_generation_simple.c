/**
 * Asthra Programming Language Compiler
 * Simple Statement Generation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test statement generation without complex wrappers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Define to use real parser
#define ASTHRA_PARSER_REAL_IMPLEMENTATION

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic_analyzer.h"
#include "code_generator.h"
#include "code_generator_core.h"

bool test_variable_declaration(void) {
    printf("Testing variable declaration generation...\n");
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub const PI: float = 3.14159;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: int = 42;\n"
        "    let y: int = x + 5;\n"
        "    let z: int;\n"
        "    return ();\n"
        "}\n";
    
    // Create lexer
    Lexer* lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("  FAIL: Could not create lexer\n");
        return false;
    }
    
    // Create parser
    Parser* parser = parser_create(lexer);
    if (!parser) {
        printf("  FAIL: Could not create parser\n");
        lexer_destroy(lexer);
        return false;
    }
    
    // Parse program
    ASTNode* ast = parser_parse_program(parser);
    if (!ast) {
        printf("  FAIL: Could not parse program\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Create semantic analyzer
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("  FAIL: Could not create semantic analyzer\n");
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Run semantic analysis
    bool semantic_ok = semantic_analyze_program(analyzer, ast);
    if (!semantic_ok) {
        printf("  FAIL: Semantic analysis failed\n");
        if (analyzer->error_count > 0) {
            for (size_t i = 0; i < analyzer->error_count && i < 3; i++) {
                printf("    Error: %s\n", analyzer->errors[i].message);
            }
        }
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Create code generator
    CodeGenerator* codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!codegen) {
        printf("  FAIL: Could not create code generator\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Set semantic analyzer
    codegen->semantic_analyzer = analyzer;
    
    // Generate code
    bool gen_ok = code_generate_program(codegen, ast);
    if (!gen_ok) {
        printf("  FAIL: Code generation failed\n");
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    printf("  PASS: Variable declaration generation successful\n");
    
    // Cleanup
    code_generator_destroy(codegen);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return true;
}

bool test_control_flow_statements(void) {
    printf("Testing control flow statement generation...\n");
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: int = 10;\n"
        "    let condition: bool = true;\n"
        "    if condition {\n"
        "        x = x + 1;\n"
        "    } else {\n"
        "        x = x - 1;\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    // Create lexer
    Lexer* lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("  FAIL: Could not create lexer\n");
        return false;
    }
    
    // Create parser
    Parser* parser = parser_create(lexer);
    if (!parser) {
        printf("  FAIL: Could not create parser\n");
        lexer_destroy(lexer);
        return false;
    }
    
    // Parse program
    ASTNode* ast = parser_parse_program(parser);
    if (!ast) {
        printf("  FAIL: Could not parse program\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Create semantic analyzer
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("  FAIL: Could not create semantic analyzer\n");
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Run semantic analysis
    bool semantic_ok = semantic_analyze_program(analyzer, ast);
    if (!semantic_ok) {
        printf("  FAIL: Semantic analysis failed\n");
        if (analyzer->error_count > 0) {
            for (size_t i = 0; i < analyzer->error_count && i < 3; i++) {
                printf("    Error: %s\n", analyzer->errors[i].message);
            }
        }
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Create code generator
    CodeGenerator* codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!codegen) {
        printf("  FAIL: Could not create code generator\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Set semantic analyzer
    codegen->semantic_analyzer = analyzer;
    
    // Generate code
    bool gen_ok = code_generate_program(codegen, ast);
    if (!gen_ok) {
        printf("  FAIL: Code generation failed\n");
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    printf("  PASS: Control flow statement generation successful\n");
    
    // Cleanup
    code_generator_destroy(codegen);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return true;
}

int main(void) {
    printf("=== Simple Statement Generation Tests ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Run tests
    total++;
    if (test_variable_declaration()) passed++;
    
    total++;
    if (test_control_flow_statements()) passed++;
    
    printf("\n=== Summary ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (100.0 * passed) / total);
    
    return (passed == total) ? 0 : 1;
}