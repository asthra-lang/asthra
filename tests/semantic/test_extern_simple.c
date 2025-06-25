/**
 * Simple test to demonstrate extern function semantic analysis
 * This test shows how extern declarations should be handled
 */

#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("Testing extern function semantic analysis...\n");

    // Simple extern function declarations
    const char *test_code =
        "package test;\n"
        "pub extern \"C\" fn malloc(size: usize) -> *mut void;\n"
        "pub extern \"C\" fn strlen(s: *const u8) -> usize;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // Don't call extern functions - they aren't registered as symbols\n"
        "    // let ptr: *mut void = malloc(100);  // This would fail\n"
        "    return ();\n"
        "}\n";

    // Create lexer first
    Lexer *lexer = lexer_create(test_code, strlen(test_code), "test.astra");
    assert(lexer != NULL);

    // Create parser with lexer
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);

    // Parse the program
    ASTNode *program = parser_parse_program(parser);
    if (!program) {
        fprintf(stderr, "Failed to parse test code\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 1;
    }
    assert(program->type == AST_PROGRAM);

    // Create semantic analyzer
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    // Analyze the program
    bool analysis_result = semantic_analyze_program(analyzer, program);

    if (!analysis_result) {
        fprintf(stderr, "Semantic analysis failed\n");
        const SemanticError *errors = semantic_get_errors(analyzer);
        while (errors) {
            fprintf(stderr, "Error: %s\n", errors->message);
            errors = errors->next;
        }
        semantic_analyzer_destroy(analyzer);
        parser_destroy(parser);
        return 1;
    }

    printf("Semantic analysis passed!\n");

    // Check if extern functions were registered as symbols
    SymbolEntry *malloc_symbol = semantic_resolve_identifier(analyzer, "malloc");
    SymbolEntry *strlen_symbol = semantic_resolve_identifier(analyzer, "strlen");

    if (malloc_symbol) {
        printf("✓ malloc symbol found: kind=%d\n", malloc_symbol->kind);
    } else {
        printf("✗ malloc symbol NOT found (expected with current implementation)\n");
    }

    if (strlen_symbol) {
        printf("✓ strlen symbol found: kind=%d\n", strlen_symbol->kind);
    } else {
        printf("✗ strlen symbol NOT found (expected with current implementation)\n");
    }

    // The main function should have been analyzed
    SymbolEntry *main_symbol = semantic_resolve_identifier(analyzer, "main");
    if (main_symbol) {
        printf("✓ main function found and analyzed\n");
    } else {
        printf("✗ main function NOT found\n");
    }

    // Cleanup
    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    lexer_destroy(lexer);

    printf("\nTest completed.\n");
    printf(
        "Note: With the current implementation, extern functions are NOT registered as symbols\n");
    printf("because analyze_extern_declaration() just returns true without processing.\n");

    return 0;
}