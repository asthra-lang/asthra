/**
 * Test for GitHub Issue #16: Type alias as constraint for generic type
 * This test reproduces the panic when trying to use a type alias as a constraint
 */

#include "../framework/parser_test_utils.h"
#include "../framework/semantic_test_utils.h"
#include "../framework/test_framework.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <string.h>

// Test that type alias as constraint should fail gracefully
static AsthraTestResult
test_type_alias_as_constraint_should_fail_gracefully(AsthraTestContext *context) {
    // This syntax is not supported in the grammar
    const char *code = "package test;\n"
                       "\n"
                       "// Type alias (not implemented yet)\n"
                       "type Numeric = i32;\n"
                       "\n"
                       "// Trying to use type alias as constraint (not supported)\n"
                       "pub struct Container<T: Numeric> {\n"
                       "    value: T\n"
                       "}\n";

    // Create parser to check for errors
    Parser *parser = create_test_parser(code);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Failed to create parser");

    // Parse the source - with error recovery, it might return a partial AST
    ASTNode *ast = parser_parse_program(parser);

    // The parser should have reported errors about type aliases not being implemented
    bool has_errors = parser_had_error(parser);
    ASTHRA_TEST_ASSERT_TRUE(context, has_errors,
                            "Parser should report errors for unsupported type alias syntax");

    if (ast) {
        ast_free_node(ast);
    }

    destroy_test_parser(parser);

    return ASTHRA_TEST_PASS;
}

// Test that generic with constraint syntax should fail
static AsthraTestResult
test_generic_with_constraint_syntax_should_fail(AsthraTestContext *context) {
    // Even without type alias, constraint syntax is not supported
    const char *code = "package test;\n"
                       "\n"
                       "// Trying to use constraint syntax directly (not supported)\n"
                       "pub struct Container<T: i32> {\n"
                       "    value: T\n"
                       "}\n";

    // Create parser to check for errors
    Parser *parser = create_test_parser(code);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Failed to create parser");

    // Parse the source - with error recovery, it might return a partial AST
    ASTNode *ast = parser_parse_program(parser);

    // The parser should have reported errors about constraints not being supported
    bool has_errors = parser_had_error(parser);
    ASTHRA_TEST_ASSERT_TRUE(context, has_errors,
                            "Parser should report errors for constraint syntax");

    if (ast) {
        ast_free_node(ast);
    }

    destroy_test_parser(parser);

    return ASTHRA_TEST_PASS;
}

// Test that valid generic syntax should work
static AsthraTestResult test_valid_generic_syntax_should_work(AsthraTestContext *context) {
    // This is the currently supported syntax
    const char *code = "package test;\n"
                       "\n"
                       "pub struct Container<T> {\n"
                       "    value: T\n"
                       "}\n";

    ASTNode *ast = parse_test_source(code, "test.as");

    // This should parse successfully
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ast, "Valid generic syntax should parse");

    if (ast) {
        // Perform semantic analysis
        SemanticAnalyzer *analyzer = setup_semantic_analyzer();
        ASTHRA_TEST_ASSERT_NOT_NULL(context, analyzer, "Failed to create semantic analyzer");

        if (analyzer) {
            bool analysis_success = analyze_test_ast(analyzer, ast);
            ASTHRA_TEST_ASSERT_TRUE(context, analysis_success,
                                    "Semantic analysis should succeed for valid generics");

            destroy_semantic_analyzer(analyzer);
        }

        // AST cleanup is handled by the framework
    }

    return ASTHRA_TEST_PASS;
}

// Test that type keyword is not recognized
static AsthraTestResult test_type_keyword_not_recognized(AsthraTestContext *context) {
    // The 'type' keyword itself is not in the grammar
    const char *code = "package test;\n"
                       "\n"
                       "type MyInt = i32;\n";

    // Create parser to check for errors
    Parser *parser = create_test_parser(code);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Failed to create parser");

    // Parse the source - the parser should report errors about type aliases
    ASTNode *ast = parser_parse_program(parser);

    // The parser should have reported errors about type aliases not being implemented
    bool has_errors = parser_had_error(parser);
    ASTHRA_TEST_ASSERT_TRUE(context, has_errors, "Parser should report errors for 'type' keyword");

    if (ast) {
        ast_free_node(ast);
    }

    destroy_test_parser(parser);

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(int argc, char *argv[]) {
    AsthraTestContext context = {0};
    int failed = 0;

    printf("Running Type Alias Constraint Tests...\n");
    printf("=====================================\n\n");

    // Run each test
    AsthraTestResult result;

    printf("Test 1: Type alias as constraint should fail gracefully\n");
    result = test_type_alias_as_constraint_should_fail_gracefully(&context);
    if (result != ASTHRA_TEST_PASS) {
        printf("  FAILED\n");
        failed++;
    } else {
        printf("  PASSED\n");
    }

    printf("Test 2: Generic with constraint syntax should fail\n");
    result = test_generic_with_constraint_syntax_should_fail(&context);
    if (result != ASTHRA_TEST_PASS) {
        printf("  FAILED\n");
        failed++;
    } else {
        printf("  PASSED\n");
    }

    printf("Test 3: Valid generic syntax should work\n");
    result = test_valid_generic_syntax_should_work(&context);
    if (result != ASTHRA_TEST_PASS) {
        printf("  FAILED\n");
        failed++;
    } else {
        printf("  PASSED\n");
    }

    printf("Test 4: Type keyword not recognized\n");
    result = test_type_keyword_not_recognized(&context);
    if (result != ASTHRA_TEST_PASS) {
        printf("  FAILED\n");
        failed++;
    } else {
        printf("  PASSED\n");
    }

    printf("\n=====================================\n");
    printf("Total tests: 4\n");
    printf("Passed: %d\n", 4 - failed);
    printf("Failed: %d\n", failed);

    return failed > 0 ? 1 : 0;
}