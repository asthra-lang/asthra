/**
 * Basic Code Generation Test
 *
 * This test verifies that the code generator can handle basic programs
 * without encountering the 0x1 parser issue.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include backend interface and necessary headers
#include "../../../src/analysis/semantic_analyzer.h"
#include "../../../src/compiler.h"
#include "../../../src/parser/ast.h"
#include "../../framework/backend_stubs.h"
#include "../codegen_backend_wrapper.h"

// Direct declarations for parser types
typedef struct Lexer Lexer;
typedef struct Parser Parser;

// Function declarations
extern Lexer *lexer_create(const char *source, size_t length, const char *filename);
extern void lexer_destroy(Lexer *lexer);
extern Parser *parser_create(Lexer *lexer);
extern void parser_destroy(Parser *parser);
extern ASTNode *parser_parse_program(Parser *parser);
extern SemanticAnalyzer *semantic_analyzer_create(void);
extern void semantic_analyzer_destroy(SemanticAnalyzer *analyzer);
extern bool semantic_analyze_program(SemanticAnalyzer *analyzer, ASTNode *ast);
// Backend-related functions are provided by backend_interface.h

bool test_codegen(const char *name, const char *source) {
    printf("Testing %s...\n", name);

    // Parse
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("  FAIL: Could not create lexer\n");
        return false;
    }

    Parser *parser = parser_create(lexer);
    if (!parser) {
        printf("  FAIL: Could not create parser\n");
        lexer_destroy(lexer);
        return false;
    }

    ASTNode *ast = parser_parse_program(parser);
    parser_destroy(parser);

    if (!ast || ast == (void *)0x1) {
        printf("  FAIL: Parser failed (returned %p)\n", (void *)ast);
        return false;
    }

    // Analyze
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
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

    // Generate code using backend interface
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.target_arch = ASTHRA_TARGET_X86_64;

    // Backend creation removed - LLVM is accessed directly
    void *backend = asthra_backend_create(&options);
    if (!backend) {
        printf("  FAIL: Could not create backend\n");
        semantic_analyzer_destroy(analyzer);
        return false;
    }

    if (asthra_backend_initialize(backend, &options) != 0) {
        printf("  FAIL: Could not initialize backend\n");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        return false;
    }

    // Create a minimal compiler context for the backend
    AsthraCompilerContext ctx = {0};
    ctx.options = options;
    ctx.ast = ast;
    ctx.symbol_table = analyzer->global_scope;
    ctx.type_checker = analyzer;

    bool codegen_result = asthra_backend_generate(backend, &ctx, ast, "test.ll") == 0;
    asthra_backend_destroy(backend);
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
        if (test_codegen("simple struct", "package test;\n"
                                          "pub struct Point { pub x: i32, pub y: i32 }")) {
            passed++;
        }
    }

    // Test 3: Simple function
    {
        total++;
        if (test_codegen("simple function", "package test;\n"
                                            "pub fn add(a: i32, b: i32) -> i32 {\n"
                                            "    return a + b;\n"
                                            "}")) {
            passed++;
        }
    }

    // Test 4: Main function
    {
        total++;
        if (test_codegen("main function", "package test;\n"
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