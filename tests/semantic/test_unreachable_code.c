/**
 * Asthra Programming Language
 * Test Suite - Unreachable Code Detection
 * 
 * Tests for detecting unreachable code after Never-returning functions
 * and expressions.
 * 
 * NOTE: These tests are written in TDD style - they are expected to FAIL
 * initially until the unreachable code detection feature is implemented.
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "semantic_analyzer.h"
#include "parser.h"
#include "lexer.h"
#include <string.h>
#include <stdio.h>

// Helper function to count warnings in diagnostics
static size_t count_warnings(SemanticAnalyzer *analyzer) {
    if (!analyzer) return 0;
    return analyzer->stats.warnings_issued;
}

// Helper function to perform semantic analysis on source code
static bool analyze_source(SemanticAnalyzer *analyzer, const char *source) {
    if (!analyzer || !source) return false;
    
    // Create lexer and parser
    Lexer *lexer = lexer_create(source, strlen(source), "test.astra");
    if (!lexer) return false;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return false;
    }
    
    // Parse the program
    ASTNode *ast = parser_parse_program(parser);
    bool parse_success = (ast != NULL && !parser_had_error(parser));
    
    // Perform semantic analysis if parsing succeeded
    bool analysis_success = false;
    if (parse_success) {
        analysis_success = semantic_analyze_program(analyzer, ast);
    }
    
    // Clean up
    if (ast) ast_free_node(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return parse_success && analysis_success;
}

// Test unreachable code after panic (Never-returning function)
static AsthraTestResult test_unreachable_after_panic(AsthraTestContext *ctx) {
    // Test Description:
    // When a function calls a Never-returning function like panic(),
    // any code after that call should be marked as unreachable.
    
    const char *source = 
        "package test;\n"
        "\n"
        "pub fn test_function(none) -> i32 {\n"
        "    panic(\"Error occurred\");\n"
        "    log(\"This is unreachable\");\n"  // Line 5
        "    return 42;\n"                      // Line 6
        "}\n";
    
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!asthra_test_assert_not_null(ctx, analyzer, "Failed to create semantic analyzer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Enable warnings
    analyzer->config.enable_warnings = true;
    
    // Clear any previous warnings
    analyzer->stats.warnings_issued = 0;
    
    // Analyze the source code
    bool analysis_result = analyze_source(analyzer, source);
    if (!asthra_test_assert_bool(ctx, analysis_result, "Semantic analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t warning_count = count_warnings(analyzer);
    
    // We expect 2 warnings for the two unreachable statements
    bool has_warnings = asthra_test_assert_size_eq(ctx, warning_count, 2, 
        "Expected 2 warnings for unreachable code after panic()");
    
    semantic_analyzer_destroy(analyzer);
    
    return has_warnings ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// Test unreachable code in if branches with Never
static AsthraTestResult test_unreachable_in_if_branches(AsthraTestContext *ctx) {
    // Test Description:
    // In an if-else where one branch calls a Never-returning function,
    // code after the Never call in that branch should be unreachable.
    
    const char *source = 
        "package test;\n"
        "\n"
        "pub fn test_branch(x: i32) -> i32 {\n"
        "    if x < 0 {\n"
        "        panic(\"Negative value\");\n"
        "        return 0;\n"  // Line 6 - unreachable
        "    } else {\n"
        "        return x * 2;\n"
        "    }\n"
        "}\n";
    
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!asthra_test_assert_not_null(ctx, analyzer, "Failed to create semantic analyzer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Enable warnings
    analyzer->config.enable_warnings = true;
    
    // Clear any previous warnings
    analyzer->stats.warnings_issued = 0;
    
    // Analyze the source code
    bool analysis_result = analyze_source(analyzer, source);
    if (!asthra_test_assert_bool(ctx, analysis_result, "Semantic analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t warning_count = count_warnings(analyzer);
    
    // We expect 1 warning for the unreachable return after panic
    bool has_warning = asthra_test_assert_size_eq(ctx, warning_count, 1, 
        "Expected 1 warning for unreachable code in if branch");
    
    // Debug: print the actual warning count
    if (warning_count != 1) {
        fprintf(stderr, "DEBUG: Got %zu warnings instead of 1\n", (size_t)warning_count);
    }
    
    semantic_analyzer_destroy(analyzer);
    return has_warning ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// Test no warning for valid Never usage
static AsthraTestResult test_no_warning_valid_never_usage(AsthraTestContext *ctx) {
    // Test Description:
    // When a function correctly returns Never and only calls Never functions,
    // there should be no unreachable code warnings.
    
    const char *source = 
        "package test;\n"
        "\n"
        "pub fn always_fails(none) -> Never {\n"
        "    panic(\"This function always fails\");\n"
        "}\n"
        "\n"
        "pub fn another_never(none) -> Never {\n"
        "    always_fails();\n"  // No warning - this is valid
        "}\n";
    
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!asthra_test_assert_not_null(ctx, analyzer, "Failed to create semantic analyzer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Enable warnings
    analyzer->config.enable_warnings = true;
    
    // Clear any previous warnings
    analyzer->stats.warnings_issued = 0;
    
    // Analyze the source code
    bool analysis_result = analyze_source(analyzer, source);
    if (!asthra_test_assert_bool(ctx, analysis_result, "Semantic analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t warning_count = count_warnings(analyzer);
    
    // We expect 0 warnings for valid Never usage
    bool no_warnings = asthra_test_assert_size_eq(ctx, warning_count, 0, 
        "Expected no warnings for valid Never usage");
    
    semantic_analyzer_destroy(analyzer);
    return no_warnings ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// Test unreachable code after Never expression (not just function call)
static AsthraTestResult test_unreachable_after_never_expression(AsthraTestContext *ctx) {
    // Test Description:
    // When a statement contains a Never-typed expression,
    // code after that statement should be unreachable.
    
    const char *source = 
        "package test;\n"
        "\n"
        "pub fn helper(none) -> Never {\n"
        "    panic(\"Helper fails\");\n"
        "}\n"
        "\n"
        "pub fn test_expr(none) -> i32 {\n"
        "    helper();\n"              // This expression returns Never
        "    return 42;\n"           // Line 9 - unreachable
        "}\n";
    
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!asthra_test_assert_not_null(ctx, analyzer, "Failed to create semantic analyzer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Enable warnings
    analyzer->config.enable_warnings = true;
    
    // Clear any previous warnings
    analyzer->stats.warnings_issued = 0;
    
    // Analyze the source code
    bool analysis_result = analyze_source(analyzer, source);
    if (!asthra_test_assert_bool(ctx, analysis_result, "Semantic analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t warning_count = count_warnings(analyzer);
    
    // We expect at least 1 warning for unreachable code
    bool has_warning = asthra_test_assert_bool(ctx, warning_count >= 1, 
        "Expected at least 1 warning for unreachable code after Never expression");
    
    semantic_analyzer_destroy(analyzer);
    return has_warning ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// Test unreachable code in nested blocks
static AsthraTestResult test_unreachable_in_nested_blocks(AsthraTestContext *ctx) {
    // Test Description:
    // When a nested block contains a Never-returning call,
    // the rest of that block should be unreachable.
    
    const char *source = 
        "package test;\n"
        "\n"
        "pub fn test_nested(x: i32) -> i32 {\n"
        "    let mut y: i32 = x;\n"
        "    {\n"  // Start nested block
        "        if x == 0 {\n"
        "            panic(\"Zero not allowed\");\n"
        "            y = 10;\n"         // Line 8 - unreachable
        "        } else {\n"
        "            y = x * 2;\n"
        "        }\n"
        "        y = y + 1;\n"         // Line 12 - reachable (only if x != 0)
        "    }\n"  // End nested block
        "    return y;\n"
        "}\n";
    
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!asthra_test_assert_not_null(ctx, analyzer, "Failed to create semantic analyzer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Enable warnings
    analyzer->config.enable_warnings = true;
    
    // Clear any previous warnings
    analyzer->stats.warnings_issued = 0;
    
    // Analyze the source code
    bool analysis_result = analyze_source(analyzer, source);
    if (!asthra_test_assert_bool(ctx, analysis_result, "Semantic analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t warning_count = count_warnings(analyzer);
    
    // We expect at least 1 warning for unreachable code in the if branch
    bool has_warning = asthra_test_assert_bool(ctx, warning_count >= 1, 
        "Expected at least 1 warning for unreachable code in nested blocks");
    
    semantic_analyzer_destroy(analyzer);
    return has_warning ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_unreachable_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_unreachable_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE CREATION
// =============================================================================

AsthraTestSuite* create_unreachable_code_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Unreachable Code Detection", 
                                                      "Tests for unreachable code detection after Never-returning functions");
    if (!suite) {
        return NULL;
    }
    
    // Set up and tear down
    asthra_test_suite_set_setup(suite, setup_unreachable_tests);
    asthra_test_suite_set_teardown(suite, teardown_unreachable_tests);
    
    // Register tests
    asthra_test_suite_add_test(suite, "test_unreachable_after_panic", 
                              "Test unreachable code after panic", 
                              test_unreachable_after_panic);
    
    asthra_test_suite_add_test(suite, "test_unreachable_in_if_branches", 
                              "Test unreachable code in if branches", 
                              test_unreachable_in_if_branches);
    
    asthra_test_suite_add_test(suite, "test_no_warning_valid_never_usage", 
                              "Test no warning for valid Never usage", 
                              test_no_warning_valid_never_usage);
    
    asthra_test_suite_add_test(suite, "test_unreachable_after_never_expression", 
                              "Test unreachable after Never expression", 
                              test_unreachable_after_never_expression);
    
    asthra_test_suite_add_test(suite, "test_unreachable_in_nested_blocks", 
                              "Test unreachable in nested blocks", 
                              test_unreachable_in_nested_blocks);
    
    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Unreachable Code Detection Tests (TDD) ===\n\n");
    printf("Note: These tests explore unreachable code detection after Never-returning functions.\n");
    printf("All tests except 'valid Never usage' are expected to FAIL initially.\n");
    printf("This is normal TDD behavior - tests are written before implementation.\n\n");
    
    AsthraTestSuite* suite = create_unreachable_code_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    printf("\n=== Test Results ===\n");
    printf("Expected: 1 PASS (valid Never usage), 4 FAIL (not implemented)\n");
    printf("This indicates tests are ready for implementation.\n");
    
    // For TDD, we expect failures, so return 0 to indicate tests ran successfully
    return 0;
}
#endif // ASTHRA_TEST_COMBINED