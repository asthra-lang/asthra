/**
 * Minimal standalone codegen test to isolate issues
 */

#include "../codegen_backend_wrapper.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("Starting minimal codegen test...\n");

    // Create backend
    // Backend creation removed - LLVM is accessed directly
    void *backend = (void *)1; // Non-NULL placeholder for test compatibility

    printf("Backend created successfully\n");

    // Simple program source
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> int {\n"
                         "    return 0;\n"
                         "}\n";

    printf("Parsing program...\n");

    // Create lexer and parser
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("Failed to create lexer\n");
        // Backend cleanup not needed
        return 1;
    }

    Parser *parser = parser_create(lexer);
    if (!parser) {
        printf("Failed to create parser\n");
        lexer_destroy(lexer);
        // Backend cleanup not needed
        return 1;
    }

    ASTNode *program = parser_parse_program(parser);
    if (!program) {
        printf("Failed to parse program\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        // Backend cleanup not needed
        return 1;
    }

    parser_destroy(parser);
    lexer_destroy(lexer);

    printf("Parse successful, running semantic analysis...\n");

    // Run semantic analysis
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        ast_free_node(program);
        // Backend cleanup not needed
        return 1;
    }

    bool semantic_success = semantic_analyze_program(analyzer, program);
    if (!semantic_success) {
        printf("Semantic analysis failed\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        // Backend cleanup not needed
        return 1;
    }

    printf("Semantic analysis successful, generating code...\n");

    // Set semantic analyzer on backend
    asthra_backend_set_semantic_analyzer(backend, analyzer);

    // Generate code
    bool codegen_success = asthra_backend_generate_program(backend, program);

    if (codegen_success) {
        printf("Code generation successful!\n");
    } else {
        printf("Code generation failed\n");
    }

    // Cleanup
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    // Backend cleanup not needed

    return codegen_success ? 0 : 1;
}