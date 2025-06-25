/**
 * Asthra Programming Language Compiler - Const Expression Tests
 * Tests for const declarations with complex expressions
 *
 * This test verifies that const expressions are properly evaluated at compile time
 */

#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test arithmetic operations in const expressions
void test_const_arithmetic_expressions(void) {
    printf("Testing const arithmetic expressions...\n");

    const char *source = "package test;\n"
                         "\n"
                         "priv const BASE: i32 = 100;\n"
                         "priv const DOUBLE: i32 = BASE * 2;\n"
                         "priv const TRIPLE: i32 = BASE * 3;\n"
                         "priv const HALF: i32 = BASE / 2;\n"
                         "priv const SUM: i32 = BASE + 50;\n"
                         "priv const DIFF: i32 = BASE - 25;\n"
                         "priv const COMPLEX: i32 = (BASE + 50) * 2 - 10;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}\n";

    // Create lexer and parser
    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    assert(lexer != NULL);

    Parser *parser = parser_create(lexer);
    assert(parser != NULL);

    // Parse the source
    ASTNode *ast = parser_parse_program(parser);
    assert(ast != NULL);

    // Create semantic analyzer
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    // Analyze the AST
    bool result = semantic_analyze_program(analyzer, ast);
    assert(result);

    // Verify no errors occurred
    const SemanticError *errors = semantic_get_errors(analyzer);
    if (errors) {
        fprintf(stderr, "Unexpected errors in const arithmetic test:\n");
        while (errors) {
            fprintf(stderr, "  Error: %s\n", errors->message);
            errors = errors->next;
        }
        assert(false);
    }

    printf("✓ Const arithmetic expressions test passed\n");

    // Clean up
    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

// Test bitwise operations in const expressions
void test_const_bitwise_expressions(void) {
    printf("Testing const bitwise expressions...\n");

    const char *source = "package test;\n"
                         "\n"
                         "priv const BIT_AND: i32 = 0xFF & 0x0F;\n"
                         "priv const BIT_OR: i32 = 0xF0 | 0x0F;\n"
                         "priv const BIT_XOR: i32 = 0xFF ^ 0x0F;\n"
                         "priv const SHIFT_LEFT: i32 = 1 << 8;\n"
                         "priv const SHIFT_RIGHT: i32 = 256 >> 4;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}\n";

    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    assert(lexer != NULL);

    Parser *parser = parser_create(lexer);
    assert(parser != NULL);

    ASTNode *ast = parser_parse_program(parser);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool result = semantic_analyze_program(analyzer, ast);
    assert(result);

    const SemanticError *errors = semantic_get_errors(analyzer);
    if (errors) {
        fprintf(stderr, "Unexpected errors in const bitwise test:\n");
        while (errors) {
            fprintf(stderr, "  Error: %s\n", errors->message);
            errors = errors->next;
        }
        assert(false);
    }

    printf("✓ Const bitwise expressions test passed\n");

    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

// Test unary operations in const expressions
void test_const_unary_expressions(void) {
    printf("Testing const unary expressions...\n");

    const char *source = "package test;\n"
                         "\n"
                         "priv const BASE: i32 = 100;\n"
                         "priv const NEGATIVE: i32 = -BASE;\n"
                         "priv const NOT_TRUE: bool = !true;\n"
                         "priv const NOT_FALSE: bool = !false;\n"
                         "priv const BITWISE_NOT: i32 = ~5;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}\n";

    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    assert(lexer != NULL);

    Parser *parser = parser_create(lexer);
    assert(parser != NULL);

    ASTNode *ast = parser_parse_program(parser);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool result = semantic_analyze_program(analyzer, ast);
    assert(result);

    const SemanticError *errors = semantic_get_errors(analyzer);
    if (errors) {
        fprintf(stderr, "Unexpected errors in const unary test:\n");
        while (errors) {
            fprintf(stderr, "  Error: %s\n", errors->message);
            errors = errors->next;
        }
        assert(false);
    }

    printf("✓ Const unary expressions test passed\n");

    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

// Test comparison operations in const expressions
void test_const_comparison_expressions(void) {
    printf("Testing const comparison expressions...\n");

    const char *source = "package test;\n"
                         "\n"
                         "priv const BASE: i32 = 100;\n"
                         "priv const IS_EQUAL: bool = BASE == 100;\n"
                         "priv const NOT_EQUAL: bool = BASE != 50;\n"
                         "priv const LESS_THAN: bool = 5 < 10;\n"
                         "priv const GREATER_THAN: bool = 10 > 5;\n"
                         "priv const LESS_EQUAL: bool = 10 <= 10;\n"
                         "priv const GREATER_EQUAL: bool = 10 >= 10;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}\n";

    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    assert(lexer != NULL);

    Parser *parser = parser_create(lexer);
    assert(parser != NULL);

    ASTNode *ast = parser_parse_program(parser);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool result = semantic_analyze_program(analyzer, ast);
    assert(result);

    const SemanticError *errors = semantic_get_errors(analyzer);
    if (errors) {
        fprintf(stderr, "Unexpected errors in const comparison test:\n");
        while (errors) {
            fprintf(stderr, "  Error: %s\n", errors->message);
            errors = errors->next;
        }
        assert(false);
    }

    printf("✓ Const comparison expressions test passed\n");

    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

// Test logical operations in const expressions
void test_const_logical_expressions(void) {
    printf("Testing const logical expressions...\n");

    const char *source = "package test;\n"
                         "\n"
                         "priv const LOGICAL_AND_TT: bool = true && true;\n"
                         "priv const LOGICAL_AND_TF: bool = true && false;\n"
                         "priv const LOGICAL_OR_FF: bool = false || false;\n"
                         "priv const LOGICAL_OR_TF: bool = true || false;\n"
                         "priv const COMPLEX_LOGICAL: bool = (true && false) || (true && true);\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}\n";

    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    assert(lexer != NULL);

    Parser *parser = parser_create(lexer);
    assert(parser != NULL);

    ASTNode *ast = parser_parse_program(parser);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool result = semantic_analyze_program(analyzer, ast);
    assert(result);

    const SemanticError *errors = semantic_get_errors(analyzer);
    if (errors) {
        fprintf(stderr, "Unexpected errors in const logical test:\n");
        while (errors) {
            fprintf(stderr, "  Error: %s\n", errors->message);
            errors = errors->next;
        }
        assert(false);
    }

    printf("✓ Const logical expressions test passed\n");

    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

// Test error cases
void test_const_expression_errors(void) {
    printf("Testing const expression error cases...\n");

    // Test division by zero
    {
        const char *source = "package test;\n"
                             "\n"
                             "priv const ZERO: i32 = 0;\n"
                             "priv const DIV_BY_ZERO: i32 = 100 / ZERO;\n"
                             "\n"
                             "pub fn main(none) -> void {\n"
                             "    return ();\n"
                             "}\n";

        Lexer *lexer = lexer_create(source, strlen(source), "<test>");
        Parser *parser = parser_create(lexer);
        ASTNode *ast = parser_parse_program(parser);

        SemanticAnalyzer *analyzer = semantic_analyzer_create();
        bool result = semantic_analyze_program(analyzer, ast);

        // Should fail due to division by zero
        assert(!result);
        const SemanticError *errors = semantic_get_errors(analyzer);
        assert(errors != NULL);

        printf("✓ Division by zero error detected correctly\n");

        semantic_analyzer_destroy(analyzer);
        parser_destroy(parser);
        lexer_destroy(lexer);
    }

    // Test type mismatch
    {
        const char *source = "package test;\n"
                             "\n"
                             "priv const NUM: i32 = 100;\n"
                             "priv const INVALID: i32 = NUM && true;\n" // Can't use && on integer
                             "\n"
                             "pub fn main(none) -> void {\n"
                             "    return ();\n"
                             "}\n";

        Lexer *lexer = lexer_create(source, strlen(source), "<test>");
        Parser *parser = parser_create(lexer);
        ASTNode *ast = parser_parse_program(parser);

        SemanticAnalyzer *analyzer = semantic_analyzer_create();
        bool result = semantic_analyze_program(analyzer, ast);

        // Should fail due to type mismatch
        assert(!result);
        const SemanticError *errors = semantic_get_errors(analyzer);
        assert(errors != NULL);

        printf("✓ Type mismatch error detected correctly\n");

        semantic_analyzer_destroy(analyzer);
        parser_destroy(parser);
        lexer_destroy(lexer);
    }
}

// Test circular dependency detection
void test_const_circular_dependency(void) {
    printf("Testing const circular dependency detection...\n");

    const char *source = "package test;\n"
                         "\n"
                         "priv const A: i32 = B + 1;\n"
                         "priv const B: i32 = A + 1;\n" // Circular dependency
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}\n";

    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    Parser *parser = parser_create(lexer);
    ASTNode *ast = parser_parse_program(parser);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    bool result = semantic_analyze_program(analyzer, ast);

    // Should fail due to circular dependency
    assert(!result);
    const SemanticError *errors = semantic_get_errors(analyzer);
    assert(errors != NULL);

    printf("✓ Circular dependency error detected correctly\n");

    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

// Main function for standalone execution
int main(void) {
    printf("Running Const Expression Tests\n");
    printf("==============================\n\n");

    // Run all tests
    test_const_arithmetic_expressions();
    test_const_bitwise_expressions();
    test_const_unary_expressions();
    test_const_comparison_expressions();
    test_const_logical_expressions();
    test_const_expression_errors();
    test_const_circular_dependency();

    printf("\n==============================\n");
    printf("All Const Expression Tests PASSED\n");

    return 0;
}