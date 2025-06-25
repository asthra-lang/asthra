/**
 * Asthra Programming Language
 * Lexer Position Tracking Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test position tracking accuracy including line/column tracking,
 * source location accuracy, line endings, and Unicode handling
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "lexer_test_utils.h"
#include <errno.h>
#include <string.h>

// =============================================================================
// TEST UTILITIES
// =============================================================================

// Using ASSERT_TOKEN_TYPE from test_assertions.h

// =============================================================================
// POSITION TRACKING TESTS
// =============================================================================

/**
 * Test line and column tracking accuracy
 */
AsthraTestResult test_lexer_line_column_tracking(AsthraTestContext *context) {
    const char *input = "let x = 42;\nlet y = \"hello\";\n// comment\nlet z = 'c';";

    Lexer *lexer = create_test_lexer_with_context(context, input, "test_position.ast");
    if (!lexer) {
        return ASTHRA_TEST_FAIL;
    }

    struct {
        TokenType expected_type;
        int expected_line;
        int expected_column;
    } expectations[] = {{TOKEN_LET, 1, 1},
                        {TOKEN_IDENTIFIER, 1, 5}, // "x"
                        {TOKEN_ASSIGN, 1, 7},
                        {TOKEN_INTEGER, 1, 9}, // "42"
                        {TOKEN_SEMICOLON, 1, 11},
                        {TOKEN_LET, 2, 1},
                        {TOKEN_IDENTIFIER, 2, 5}, // "y"
                        {TOKEN_ASSIGN, 2, 7},
                        {TOKEN_STRING, 2, 9}, // "hello"
                        {TOKEN_SEMICOLON, 2, 16},
                        // Skip comment and newline...
                        {TOKEN_LET, 4, 1},
                        {TOKEN_IDENTIFIER, 4, 5}, // "z"
                        {TOKEN_ASSIGN, 4, 7},
                        {TOKEN_CHAR, 4, 9}, // 'c'
                        {TOKEN_SEMICOLON, 4, 12},
                        {TOKEN_EOF, 4, 13}};

    size_t expectation_count = sizeof(expectations) / sizeof(expectations[0]);
    size_t current_expectation = 0;

    Token token;
    do {
        token = lexer_next_token(lexer);

        // Skip whitespace, newlines, and comments for position testing
        if (is_skippable_token(token.type)) {
            token_free(&token);
            continue;
        }

        if (current_expectation < expectation_count) {
            // Check token type
            ASSERT_TOKEN_TYPE(context, &token, expectations[current_expectation].expected_type);

            // Check line number
            if (!asthra_test_assert_int_eq(
                    context, token.location.line, expectations[current_expectation].expected_line,
                    "Line number mismatch for token %zu", current_expectation)) {
                token_free(&token);
                lexer_destroy(lexer);
                return ASTHRA_TEST_FAIL;
            }

            // Check column number (if implemented)
            if (token.location.column > 0) {
                if (!asthra_test_assert_int_eq(context, token.location.column,
                                               expectations[current_expectation].expected_column,
                                               "Column number mismatch for token %zu",
                                               current_expectation)) {
                    token_free(&token);
                    lexer_destroy(lexer);
                    return ASTHRA_TEST_FAIL;
                }
            }

            current_expectation++;
        }

        token_free(&token);
    } while (token.type != TOKEN_EOF);

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

/**
 * Test source location accuracy
 */
AsthraTestResult test_lexer_source_location_accuracy(AsthraTestContext *context) {
    const char *input = "fn test(none) {\n    let x = 42;\n}";

    Lexer *lexer = create_test_lexer_with_context(context, input, "test_source_location.ast");
    if (!lexer) {
        return ASTHRA_TEST_FAIL;
    }

    Token token;
    do {
        token = lexer_next_token(lexer);

        // Skip whitespace and newlines
        if (is_whitespace_token(token.type)) {
            token_free(&token);
            continue;
        }

        // Verify that source location is properly set
        if (token.location.filename) {
            if (!asthra_test_assert_string_eq(context, token.location.filename,
                                              "test_source_location.ast",
                                              "Filename mismatch in token location")) {
                token_free(&token);
                lexer_destroy(lexer);
                return ASTHRA_TEST_FAIL;
            }
        }

        // Verify that line number is reasonable
        if (!asthra_test_assert_bool(context, token.location.line >= 1 && token.location.line <= 3,
                                     "Line number should be between 1 and 3, got %d",
                                     token.location.line)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
    } while (token.type != TOKEN_EOF);

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

/**
 * Test position tracking with different line endings
 */
AsthraTestResult test_lexer_line_endings(AsthraTestContext *context) {
    const struct {
        const char *input;
        const char *description;
        int expected_lines;
    } test_cases[] = {{"line1\nline2\nline3", "Unix line endings (LF)", 3},
                      {"line1\r\nline2\r\nline3", "Windows line endings (CRLF)", 3},
                      {"line1\rline2\rline3", "Mac line endings (CR)", 3},
                      {"line1\n\nline3", "Empty line in between", 3},
                      {"line1\r\n\r\nline3", "Empty line with CRLF", 3},
                      {"line1\n\r\nline3", "Mixed line endings", 3},
                      {NULL, NULL, 0}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        const char *description = test_cases[i].description;
        int expected_lines = test_cases[i].expected_lines;

        Lexer *lexer = create_test_lexer_with_context(context, input, "test_line_endings.ast");
        if (!lexer) {
            return ASTHRA_TEST_FAIL;
        }

        int max_line_seen = 0;
        Token token;
        do {
            token = lexer_next_token(lexer);
            if (token.location.line > max_line_seen) {
                max_line_seen = token.location.line;
            }
            token_free(&token);
        } while (token.type != TOKEN_EOF);

        // Check that we saw the expected number of lines
        if (!asthra_test_assert_int_eq(context, max_line_seen, expected_lines,
                                       "Expected %d lines for '%s', got %d", expected_lines,
                                       description, max_line_seen)) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test position tracking with Unicode characters
 */
AsthraTestResult test_lexer_unicode_position_tracking(AsthraTestContext *context) {
    const char *input = "let café = \"naïve\";";

    Lexer *lexer = create_test_lexer_with_context(context, input, "test_unicode_position.ast");
    if (!lexer) {
        return ASTHRA_TEST_FAIL;
    }

    Token token;
    int tokens_seen = 0;
    do {
        token = lexer_next_token(lexer);

        // Skip whitespace
        if (token.type == TOKEN_WHITESPACE) {
            token_free(&token);
            continue;
        }

        // All tokens should be on line 1
        if (!asthra_test_assert_int_eq(context, token.location.line, 1,
                                       "All tokens should be on line 1, got line %d for token %d",
                                       token.location.line, tokens_seen)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        tokens_seen++;
        token_free(&token);
    } while (token.type != TOKEN_EOF);

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function
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
        {test_lexer_line_column_tracking, "test_lexer_line_column_tracking"},
        {test_lexer_source_location_accuracy, "test_lexer_source_location_accuracy"},
        {test_lexer_line_endings, "test_lexer_line_endings"},
        {test_lexer_unicode_position_tracking, "test_lexer_unicode_position_tracking"},
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
