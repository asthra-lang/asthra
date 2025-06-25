/**
 * Asthra Programming Language
 * Lexer Comment Handling Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test comment processing including single-line, multi-line, and nested comments
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "lexer.h"
#include <errno.h>
#include <string.h>

// =============================================================================
// TEST FIXTURE AND UTILITIES
// =============================================================================

/**
 * Using ASSERT_TOKEN_TYPE from test_assertions.h
 */

// =============================================================================
// COMMENT HANDLING TESTS
// =============================================================================

/**
 * Test single-line comment handling
 */
AsthraTestResult test_lexer_single_line_comments(AsthraTestContext *context) {
    const struct {
        const char *input;
        TokenType expected_after_comment;
    } test_cases[] = {{"// single line comment", TOKEN_EOF},
                      {"// comment\nidentifier", TOKEN_IDENTIFIER},
                      {"42 // comment at end", TOKEN_EOF},
                      {"let x = 5; // end of line comment", TOKEN_EOF},
                      {"// comment with symbols !@#$%^&*()", TOKEN_EOF},
                      {"// comment with unicode: 你好世界", TOKEN_EOF},
                      {"// empty comment after this:\n//\nlet y = 10;", TOKEN_LET},
                      {"identifier1 // comment\nidentifier2", TOKEN_IDENTIFIER},
                      {"123 // number comment\n456", TOKEN_INTEGER},
                      {NULL, TOKEN_EOF}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        TokenType expected = test_cases[i].expected_after_comment;

        Lexer *lexer = lexer_create(input, strlen(input), "test_single_comments.ast");
        if (!asthra_test_assert_pointer(context, lexer,
                                        "Failed to create lexer for comment test: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token;

        // Skip tokens until we find the expected one or EOF
        do {
            token = lexer_next_token(lexer);
            if (token.type != TOKEN_COMMENT && token.type != TOKEN_WHITESPACE &&
                token.type != TOKEN_NEWLINE) {
                break;
            }
            token_free(&token);
        } while (token.type != TOKEN_EOF);

        // Check that we got the expected token after comment processing
        if (expected != TOKEN_EOF) {
            ASSERT_TOKEN_TYPE(context, &token, expected);
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test multi-line comment handling
 */
AsthraTestResult test_lexer_multi_line_comments(AsthraTestContext *context) {
    const struct {
        const char *input;
        bool should_be_valid;
    } test_cases[] = {{"/* simple comment */", true},
                      {"/* multi\nline\ncomment */", true},
                      {"/* comment with /* nested */ */", true},
                      {"/* unterminated comment", false},
                      {"/* comment with\0null character */", false},
                      {"/* /* double nested */ */", true},
                      {"/* comment with symbols !@#$%^&*() */", true},
                      {"/* empty comment: */ let x = 5;", true},
                      {"/**/", true},         // Empty comment
                      {"/* \n\n\n */", true}, // Comment with only whitespace
                      {"/* comment\r\nwith\r\nwindows\r\nline\r\nendings */", true},
                      {NULL, false}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        bool should_be_valid = test_cases[i].should_be_valid;

        Lexer *lexer = lexer_create(input, strlen(input), "test_multi_comments.ast");
        if (!asthra_test_assert_pointer(context, lexer,
                                        "Failed to create lexer for multi-line comment test")) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);

        if (should_be_valid) {
            // Should either be comment token or EOF (if comment is skipped)
            bool is_valid =
                (token.type == TOKEN_COMMENT || token.type == TOKEN_EOF || token.type == TOKEN_LET);
            if (!asthra_test_assert_bool(
                    context, is_valid,
                    "Valid multi-line comment should not produce error, got token type %d",
                    token.type)) {
                token_free(&token);
                lexer_destroy(lexer);
                return ASTHRA_TEST_FAIL;
            }
        } else {
            // Should produce error token for invalid comments
            ASSERT_TOKEN_TYPE(context, &token, TOKEN_ERROR);
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test nested comment handling
 */
AsthraTestResult test_lexer_nested_comments(AsthraTestContext *context) {
    const char *test_cases[] = {
        "/* outer /* inner */ still outer */",
        "/* /* /* triple nested */ */ */",
        "/* comment /* with */ code after */",
        "/* start /* middle /* deep */ middle */ end */",
        "/* /* nested */ and /* another nested */ */",
        "/* level1 /* level2 /* level3 /* level4 */ level3 */ level2 */ level1 */",
        "/* /* /* /* /* five levels deep */ */ */ */ */",
        NULL};

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char *input = test_cases[i];

        Lexer *lexer = lexer_create(input, strlen(input), "test_nested_comments.ast");
        if (!asthra_test_assert_pointer(context, lexer,
                                        "Failed to create lexer for nested comment test")) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);

        // Should handle nested comments correctly (either comment token or EOF)
        bool is_valid = (token.type == TOKEN_COMMENT || token.type == TOKEN_EOF);
        if (!asthra_test_assert_bool(
                context, is_valid, "Nested comment should be handled correctly, got token type %d",
                token.type)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test comment edge cases and special scenarios
 */
AsthraTestResult test_lexer_comment_edge_cases(AsthraTestContext *context) {
    const struct {
        const char *input;
        const char *description;
        bool should_be_valid;
    } test_cases[] = {
        {"/* comment */ /* another comment */", "Multiple comments", true},
        {"// first line\n// second line\n// third line", "Multiple single-line comments", true},
        {"/* /* unterminated nested", "Unterminated nested comment", false},
        {"*/ orphaned close", "Orphaned close comment", false},
        {"/* comment with // inside */", "Single-line comment inside multi-line", true},
        {"// comment with /* inside", "Multi-line start inside single-line", true},
        {"/* comment with */ code /* more comment */", "Code between comments", true},
        {"/*", "Just comment start", false},
        {"*/", "Just comment end", false},
        {"/*/**/*/", "Complex nesting", true},
        {"/* */ // mixed", "Multi-line followed by single-line", true},
        {"// single /* multi-line start\n still in single-line", "Multi-line start in single-line",
         true},
        {NULL, NULL, false}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        const char *description = test_cases[i].description;
        bool should_be_valid = test_cases[i].should_be_valid;

        Lexer *lexer = lexer_create(input, strlen(input), "test_comment_edge_cases.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for test: %s",
                                        description)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);

        if (should_be_valid) {
            // Should not produce error token
            bool is_valid = (token.type != TOKEN_ERROR);
            if (!asthra_test_assert_bool(context, is_valid,
                                         "Test '%s' should not produce error, got token type %d",
                                         description, token.type)) {
                token_free(&token);
                lexer_destroy(lexer);
                return ASTHRA_TEST_FAIL;
            }
        } else {
            // Should produce error token
            ASSERT_TOKEN_TYPE(context, &token, TOKEN_ERROR);
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test comments with various content types
 */
AsthraTestResult test_lexer_comment_content_types(AsthraTestContext *context) {
    const char *test_cases[] = {"/* comment with numbers: 123 456 789 */",
                                "/* comment with operators: + - * / % == != < > */",
                                "/* comment with keywords: let const fn if else while for */",
                                "/* comment with strings: \"hello\" 'world' */",
                                "/* comment with escapes: \\n \\t \\r \\\" \\' */",
                                "/* comment with unicode: α β γ δ ε */",
                                "/* comment with emoji: 🚀 🎉 💻 🔥 */",
                                "/* comment with mixed: let x = 42; // nested single-line? */",
                                "// comment with code: fn test(void) { return 42; }",
                                "// comment with special chars: !@#$%^&*()_+-={}[]|\\:;\"'<>,.?/~`",
                                NULL};

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char *input = test_cases[i];

        Lexer *lexer = lexer_create(input, strlen(input), "test_comment_content.ast");
        if (!asthra_test_assert_pointer(context, lexer,
                                        "Failed to create lexer for comment content test")) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);

        // Should handle all content types gracefully
        bool is_valid = (token.type == TOKEN_COMMENT || token.type == TOKEN_EOF);
        if (!asthra_test_assert_bool(
                context, is_valid,
                "Comment with various content should be handled correctly, got token type %d",
                token.type)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test comment interaction with other tokens
 */
AsthraTestResult test_lexer_comment_token_interaction(AsthraTestContext *context) {
    const struct {
        const char *input;
        TokenType first_token;
        TokenType second_token;
    } test_cases[] = {{"let/* comment */x", TOKEN_LET, TOKEN_IDENTIFIER},
                      {"42/* comment */+", TOKEN_INTEGER, TOKEN_PLUS},
                      {"\"string\"/* comment */;", TOKEN_STRING, TOKEN_SEMICOLON},
                      {"identifier// comment\n+", TOKEN_IDENTIFIER, TOKEN_PLUS},
                      {"(/* comment */)", TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN},
                      {"[/* comment */]", TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET},
                      {"{/* comment */}", TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE},
                      {"=/* comment */=", TOKEN_ASSIGN, TOKEN_ASSIGN},
                      {"+/* comment */+", TOKEN_PLUS, TOKEN_PLUS},
                      {"</* comment */=", TOKEN_LESS_THAN, TOKEN_ASSIGN},
                      {NULL, TOKEN_EOF, TOKEN_EOF}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        TokenType first_expected = test_cases[i].first_token;
        TokenType second_expected = test_cases[i].second_token;

        Lexer *lexer = lexer_create(input, strlen(input), "test_comment_interaction.ast");
        if (!asthra_test_assert_pointer(context, lexer,
                                        "Failed to create lexer for comment interaction test")) {
            return ASTHRA_TEST_FAIL;
        }

        // Get first token
        Token first_token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE(context, &first_token, first_expected);

        // Skip comments and whitespace
        Token second_token;
        do {
            second_token = lexer_next_token(lexer);
        } while (second_token.type == TOKEN_COMMENT || second_token.type == TOKEN_WHITESPACE ||
                 second_token.type == TOKEN_NEWLINE);

        // Check second token
        ASSERT_TOKEN_TYPE(context, &second_token, second_expected);

        token_free(&first_token);
        token_free(&second_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for lexer comment handling
 */
int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Define test cases
    const struct {
        AsthraTestFunction test_func;
        const char *test_name;
    } test_cases[] = {
        {test_lexer_single_line_comments, "test_lexer_single_line_comments"},
        {test_lexer_multi_line_comments, "test_lexer_multi_line_comments"},
        {test_lexer_nested_comments, "test_lexer_nested_comments"},
        {test_lexer_comment_edge_cases, "test_lexer_comment_edge_cases"},
        {test_lexer_comment_content_types, "test_lexer_comment_content_types"},
        {test_lexer_comment_token_interaction, "test_lexer_comment_token_interaction"},
        {NULL, NULL}};

    // Run tests
    for (int i = 0; test_cases[i].test_func != NULL; i++) {
        AsthraTestMetadata metadata = {.name = test_cases[i].test_name,
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = test_cases[i].test_name,
                                       .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                       .timeout_ns = 30000000000ULL,
                                       .skip = false,
                                       .skip_reason = NULL};

        AsthraTestResult result = asthra_test_run_single(test_cases[i].test_func, &metadata, stats);
        printf("[%s] %s\n", result == ASTHRA_TEST_PASS ? "PASS" : "FAIL", test_cases[i].test_name);
    }

    // Print results
    printf("\nTest Results: %llu/%llu passed\n", (unsigned long long)stats->tests_passed,
           (unsigned long long)stats->tests_run);

    bool success = (stats->tests_failed == 0);
    asthra_test_statistics_destroy(stats);
    return success ? 0 : 1;
}
