/**
 * Test file for Struct Pattern Error Handling
 * Tests error cases and malformed struct patterns in match expressions
 */

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test helper macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("FAIL: %s\n", message);                                                         \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST_SUCCESS()                                                                             \
    do {                                                                                           \
        printf("PASS\n");                                                                          \
        return true;                                                                               \
    } while (0)

// Helper function to create a parser from source code
static Parser *create_parser_from_source(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    return parser;
}

// Helper function to cleanup parser and lexer
static void cleanup_parser(Parser *parser) {
    if (parser) {
        Lexer *lexer = parser->lexer;
        parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
    }
}

// Test error handling for malformed struct patterns
static bool test_struct_pattern_error_handling(void) {
    printf("Testing struct pattern error handling... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point { x: } => 0\n" // Missing binding after colon
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for malformed struct pattern");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test missing closing brace
static bool test_missing_closing_brace(void) {
    printf("Testing missing closing brace error... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point { x, y => 0\n" // Missing closing brace
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for missing closing brace");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test missing opening brace
static bool test_missing_opening_brace(void) {
    printf("Testing missing opening brace error... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point x, y } => 0\n" // Missing opening brace
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for missing opening brace");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test empty struct name
static bool test_empty_struct_name(void) {
    printf("Testing empty struct name error... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        { x: _, y: _ } => 0\n" // Missing struct name
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for missing struct name");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test invalid field name
static bool test_invalid_field_name(void) {
    printf("Testing invalid field name error... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point { 123, y: y } => 0\n" // Invalid field name (number)
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for invalid field name");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test trailing comma without field
static bool test_trailing_comma_without_field(void) {
    printf("Testing trailing comma without field error... ");

    const char *source =
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> i32 {\n"
        "    match point {\n"
        "        Point { x: x, } => 0\n" // Trailing comma not allowed in v1.12 grammar
        "    }\n"
        "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    // Note: v1.12 grammar disallows trailing commas, so this should fail
    TEST_ASSERT(program == NULL,
                "Trailing comma should not be allowed in struct patterns (v1.12 grammar)");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test multiple colons in field pattern
static bool test_multiple_colons_in_field(void) {
    printf("Testing multiple colons in field pattern error... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point { x: y: z } => 0\n" // Multiple colons
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for multiple colons in field pattern");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test nested patterns without proper syntax
static bool test_invalid_nested_patterns(void) {
    printf("Testing invalid nested pattern syntax error... ");

    const char *source =
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> i32 {\n"
        "    match data {\n"
        "        Container { inner: { value: value } } => 0\n" // Missing struct name in nested
                                                               // pattern
        "    }\n"
        "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for invalid nested pattern syntax");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test duplicate field names in pattern
static bool test_duplicate_field_names(void) {
    printf("Testing duplicate field names in pattern... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point { x: _, x: _ } => 0\n" // Duplicate field name
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    // Note: The parser might successfully parse this, but semantic analysis should catch it
    // For now, we test that the parser doesn't crash
    if (program != NULL) {
        ast_free_node(program);
    }

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test mixing patterns incorrectly
static bool test_mixed_pattern_syntax_error(void) {
    printf("Testing mixed pattern syntax error... ");

    const char *source =
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> i32 {\n"
        "    match data {\n"
        "        Point { x: x } | Circle { radius: radius } => 0\n" // Invalid pattern combination
        "    }\n"
        "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program == NULL, "Expected parsing to fail for invalid pattern combination");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Main test runner
int main(void) {
    printf("Running struct pattern error handling tests...\n\n");

    bool all_passed = true;

    all_passed &= test_struct_pattern_error_handling();
    all_passed &= test_missing_closing_brace();
    all_passed &= test_missing_opening_brace();
    all_passed &= test_empty_struct_name();
    all_passed &= test_invalid_field_name();
    all_passed &= test_trailing_comma_without_field();
    all_passed &= test_multiple_colons_in_field();
    all_passed &= test_invalid_nested_patterns();
    all_passed &= test_duplicate_field_names();
    all_passed &= test_mixed_pattern_syntax_error();

    printf("\n");
    if (all_passed) {
        printf("🎉 All struct pattern error handling tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
}
