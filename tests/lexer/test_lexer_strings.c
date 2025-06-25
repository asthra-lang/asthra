/**
 * Asthra Programming Language
 * Lexer String and Character Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test string literals, character literals, interpolation, and escape sequences
 */

#include "../framework/test_framework.h"
#include "lexer.h"
#include <string.h>

// Simple token assertion macro for lexer tests only
#define ASSERT_TOKEN_TYPE_SIMPLE(context, token, expected_type)                                    \
    do {                                                                                           \
        if (!asthra_test_assert_int_eq(context, (int)(token)->type, (int)expected_type,            \
                                       "Token type mismatch: expected %d, got %d",                 \
                                       (int)expected_type, (int)(token)->type)) {                  \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

// =============================================================================
// STRING LITERAL TESTS
// =============================================================================

/**
 * Test string literal parsing
 */
static AsthraTestResult test_lexer_string_literals(AsthraTestContext *context) {
    const struct {
        const char *input;
        const char *expected_value;
    } test_cases[] = {{"\"hello\"", "hello"},
                      {"\"world\"", "world"},
                      {"\"\"", ""},
                      {"\"Hello, World!\"", "Hello, World!"},
                      {"\"String with spaces\"", "String with spaces"},
                      {"\"123abc\"", "123abc"},
                      {"\"Special chars: @#$%\"", "Special chars: @#$%"},
                      // Escape sequence tests
                      {"\"Hello\\nWorld\"", "Hello\nWorld"},
                      {"\"Tab\\tSeparated\"", "Tab\tSeparated"},
                      {"\"Quote: \\\"Hello\\\"\"", "Quote: \"Hello\""},
                      {"\"Backslash: \\\\\"", "Backslash: \\"},
                      {"\"Null\\0Term\"", "Null\0Term"},
                      {"\"Carriage\\rReturn\"", "Carriage\rReturn"},
                      {"\"Single quote: \\'\"", "Single quote: '"},
                      {"\"Mixed\\tEscape\\nSequences\"", "Mixed\tEscape\nSequences"},
                      {NULL, NULL}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        const char *expected = test_cases[i].expected_value;

        Lexer *lexer = lexer_create(input, strlen(input), "test_strings.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for string: %s",
                                       input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_STRING);

        // Verify string value
        if (!asthra_test_assert_string_eq(context, token.data.string.value, expected,
                                          "String value mismatch for input '%s'", input)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// String interpolation tests removed - feature deprecated for AI generation efficiency

// String interpolation escape tests removed - feature deprecated for AI generation efficiency

// =============================================================================
// CHARACTER LITERAL TESTS
// =============================================================================

/**
 * Test character literal parsing
 */
static AsthraTestResult test_lexer_character_literals(AsthraTestContext *context) {
    const struct {
        const char *input;
        uint32_t expected_value;
    } test_cases[] = {{"'a'", 'a'}, {"'Z'", 'Z'}, {"'1'", '1'}, {"' '", ' '}, {"'@'", '@'},
                      {"'#'", '#'}, {"'$'", '$'}, {"'%'", '%'}, {NULL, 0}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        uint32_t expected = test_cases[i].expected_value;

        Lexer *lexer = lexer_create(input, strlen(input), "test_chars.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for character: %s",
                                       input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_CHAR);

        // Verify character value
        if (!asthra_test_assert_int_eq(context, (int)token.data.character.value, (int)expected,
                                       "Character value mismatch for input '%s'", input)) {
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
 * Test escape sequence parsing
 */
static AsthraTestResult test_lexer_escape_sequences(AsthraTestContext *context) {
    const struct {
        const char *input;
        uint32_t expected_value;
    } test_cases[] = {{"'\\n'", '\n'},  {"'\\t'", '\t'},  {"'\\r'", '\r'}, {"'\\''", '\''},
                      {"'\\\"'", '\"'}, {"'\\\\'", '\\'}, {"'\\0'", '\0'}, {NULL, 0}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        uint32_t expected = test_cases[i].expected_value;

        Lexer *lexer = lexer_create(input, strlen(input), "test_escape.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer,
                                       "Failed to create lexer for escape sequence: %s", input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_CHAR);

        // Verify character value
        if (!asthra_test_assert_int_eq(context, (int)token.data.character.value, (int)expected,
                                       "Character value mismatch for input '%s'", input)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

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
    } test_cases[] = {{test_lexer_string_literals, "test_lexer_string_literals"},
                      {test_lexer_character_literals, "test_lexer_character_literals"},
                      {test_lexer_escape_sequences, "test_lexer_escape_sequences"},
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
