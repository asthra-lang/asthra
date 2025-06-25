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

#include "../../../src/parser/lexer.h"
#include "../../../src/parser/parser.h"
#include "../../../src/parser/ast.h"
#include "../../../src/analysis/semantic_analyzer.h"
#include "../../../src/codegen/backend_interface.h"
#include "../../../src/compiler.h"

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
    
    // Create backend for code generation
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.target_arch = ASTHRA_TARGET_X86_64;
    options.backend_type = ASTHRA_BACKEND_LLVM_IR;
    options.output_file = "test_output.ll";
    
    AsthraBackend* backend = asthra_backend_create(&options);
    if (!backend) {
        printf("  FAIL: Could not create backend\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    if (asthra_backend_initialize(backend, &options) != 0) {
        printf("  FAIL: Could not initialize backend\n");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Create compiler context
    AsthraCompilerContext ctx = {0};
    ctx.options = options;
    ctx.ast = ast;
    ctx.symbol_table = analyzer->global_scope;
    ctx.type_checker = analyzer;
    
    // Generate code
    bool gen_ok = asthra_backend_generate(backend, &ctx, ast, options.output_file) == 0;
    if (!gen_ok) {
        printf("  FAIL: Code generation failed\n");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    printf("  PASS: Variable declaration generation successful\n");
    
    // Cleanup
    asthra_backend_destroy(backend);
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
    
    // Create backend for code generation
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.target_arch = ASTHRA_TARGET_X86_64;
    options.backend_type = ASTHRA_BACKEND_LLVM_IR;
    options.output_file = "test_output.ll";
    
    AsthraBackend* backend = asthra_backend_create(&options);
    if (!backend) {
        printf("  FAIL: Could not create backend\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    if (asthra_backend_initialize(backend, &options) != 0) {
        printf("  FAIL: Could not initialize backend\n");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    // Create compiler context
    AsthraCompilerContext ctx = {0};
    ctx.options = options;
    ctx.ast = ast;
    ctx.symbol_table = analyzer->global_scope;
    ctx.type_checker = analyzer;
    
    // Generate code
    bool gen_ok = asthra_backend_generate(backend, &ctx, ast, options.output_file) == 0;
    if (!gen_ok) {
        printf("  FAIL: Code generation failed\n");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
    printf("  PASS: Control flow statement generation successful\n");
    
    // Cleanup
    asthra_backend_destroy(backend);
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