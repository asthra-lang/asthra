/**
 * Minimal standalone codegen test to isolate issues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "code_generator.h"
#include "parser.h"
#include "lexer.h"
#include "semantic_analyzer.h"

int main(void) {
    printf("Starting minimal codegen test...\n");
    
    // Create code generator
    CodeGenerator* generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator) {
        printf("Failed to create code generator\n");
        return 1;
    }
    
    printf("Code generator created successfully\n");
    
    // Simple program source
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> int {\n"
        "    return 0;\n"
        "}\n";
    
    printf("Parsing program...\n");
    
    // Create lexer and parser
    Lexer* lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("Failed to create lexer\n");
        code_generator_destroy(generator);
        return 1;
    }
    
    Parser* parser = parser_create(lexer);
    if (!parser) {
        printf("Failed to create parser\n");
        lexer_destroy(lexer);
        code_generator_destroy(generator);
        return 1;
    }
    
    ASTNode* program = parser_parse_program(parser);
    if (!program) {
        printf("Failed to parse program\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        code_generator_destroy(generator);
        return 1;
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    printf("Parse successful, running semantic analysis...\n");
    
    // Run semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        ast_free_node(program);
        code_generator_destroy(generator);
        return 1;
    }
    
    bool semantic_success = semantic_analyze_program(analyzer, program);
    if (!semantic_success) {
        printf("Semantic analysis failed\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        code_generator_destroy(generator);
        return 1;
    }
    
    printf("Semantic analysis successful, generating code...\n");
    
    // Set semantic analyzer on generator
    generator->semantic_analyzer = analyzer;
    
    // Generate code
    bool codegen_success = code_generate_program(generator, program);
    
    if (codegen_success) {
        printf("Code generation successful!\n");
    } else {
        printf("Code generation failed\n");
    }
    
    // Cleanup
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    code_generator_destroy(generator);
    
    return codegen_success ? 0 : 1;
}