/**
 * Asthra Programming Language
 * Lexer Buffer Boundary Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test buffer boundaries and edge cases including empty input,
 * single characters, large input, and complex structures
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "lexer_test_utils.h"
#include <errno.h>
#include <string.h>

// =============================================================================
// TEST UTILITIES
// =============================================================================

/**
 * Using ASSERT_TOKEN_TYPE from test_assertions.h
 */

// =============================================================================
// BUFFER BOUNDARY TESTS
// =============================================================================

/**
 * Test empty input handling
 */
AsthraTestResult test_lexer_empty_input(AsthraTestContext *context) {
    const char *input = "";

    Lexer *lexer = lexer_create(input, 0, "test_empty.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for empty input")) {
        return ASTHRA_TEST_FAIL;
    }

    Token token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &token, TOKEN_EOF);

    // Should still be EOF on subsequent calls
    Token token2 = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &token2, TOKEN_EOF);

    token_free(&token);
    token_free(&token2);
    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

/**
 * Test single character input
 */
AsthraTestResult test_lexer_single_character(AsthraTestContext *context) {
    const struct {
        const char *input;
        TokenType expected_type;
    } test_cases[] = {
        {"+", TOKEN_PLUS},          {"-", TOKEN_MINUS},       {"*", TOKEN_MULTIPLY},
        {"/", TOKEN_DIVIDE},        {"(", TOKEN_LEFT_PAREN},  {")", TOKEN_RIGHT_PAREN},
        {"{", TOKEN_LEFT_BRACE},    {"}", TOKEN_RIGHT_BRACE}, {"[", TOKEN_LEFT_BRACKET},
        {"]", TOKEN_RIGHT_BRACKET}, {";", TOKEN_SEMICOLON},   {",", TOKEN_COMMA},
        {".", TOKEN_DOT},           {":", TOKEN_COLON},       {"a", TOKEN_IDENTIFIER},
        {"5", TOKEN_INTEGER},       {NULL, TOKEN_EOF}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        TokenType expected = test_cases[i].expected_type;

        Lexer *lexer = lexer_create(input, 1, "test_single_char.ast");
        if (!asthra_test_assert_pointer(context, lexer,
                                        "Failed to create lexer for single character: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE(context, &token, expected);

        // Check position for single character
        if (!asthra_test_assert_int_eq(context, token.location.line, 1,
                                       "Single character should be on line 1")) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        // Next should be EOF
        Token eof_token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE(context, &eof_token, TOKEN_EOF);

        token_free(&token);
        token_free(&eof_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test large input handling
 */
AsthraTestResult test_lexer_large_input(AsthraTestContext *context) {
    // Create a large input with many tokens
    const size_t token_count = 1000;
    char *large_input = malloc(token_count * 10); // Rough estimate of space needed
    if (!asthra_test_assert_pointer(context, large_input,
                                    "Failed to allocate memory for large input test")) {
        return ASTHRA_TEST_FAIL;
    }

    // Build large input string
    strcpy(large_input, "");
    for (size_t i = 0; i < token_count; i++) {
        char token_str[20];
        snprintf(token_str, sizeof(token_str), "var%zu ", i);
        strcat(large_input, token_str);
    }

    size_t input_length = strlen(large_input);

    Lexer *lexer = create_test_lexer_with_context(context, large_input, "test_large_input.ast");
    if (!lexer) {
        free(large_input);
        return ASTHRA_TEST_FAIL;
    }

    // Count identifier tokens to verify they're all processed
    size_t tokens_processed = count_tokens_of_type(lexer, TOKEN_IDENTIFIER);

    // Should have processed all identifiers
    if (!asthra_test_assert_size_eq(context, tokens_processed, token_count,
                                    "Should have processed %zu tokens, got %zu", token_count,
                                    tokens_processed)) {
        lexer_destroy(lexer);
        free(large_input);
        return ASTHRA_TEST_FAIL;
    }

    lexer_destroy(lexer);
    free(large_input);
    return ASTHRA_TEST_PASS;
}

/**
 * Test position tracking with complex nested structures
 */
AsthraTestResult test_lexer_complex_structure_position(AsthraTestContext *context) {
    const char *input = "fn main(void) {\n"
                        "    let obj = {\n"
                        "        field1: \"value1\",\n"
                        "        field2: [\n"
                        "            1, 2, 3\n"
                        "        ],\n"
                        "        field3: {\n"
                        "            nested: true\n"
                        "        }\n"
                        "    };\n"
                        "}";

    Lexer *lexer = create_test_lexer_with_context(context, input, "test_complex_structure.ast");
    if (!lexer) {
        return ASTHRA_TEST_FAIL;
    }

    int max_line = 0;
    Token token;
    do {
        token = lexer_next_token(lexer);

        // Track maximum line number
        if (token.location.line > max_line) {
            max_line = token.location.line;
        }

        token_free(&token);
    } while (token.type != TOKEN_EOF);

    // Should have seen all lines
    if (!asthra_test_assert_bool(context, max_line >= 10,
                                 "Should have seen at least 10 lines, got %d", max_line)) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    // Create a new lexer to check brace balance
    Lexer *lexer2 = create_test_lexer_with_context(context, input, "test_complex_structure.ast");
    if (!lexer2) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    // Braces should be balanced
    int brace_balance = check_brace_balance(lexer2);
    if (!asthra_test_assert_int_eq(context, brace_balance, 0,
                                   "Braces should be balanced, final depth: %d", brace_balance)) {
        lexer_destroy(lexer);
        lexer_destroy(lexer2);
        return ASTHRA_TEST_FAIL;
    }

    lexer_destroy(lexer);
    lexer_destroy(lexer2);
    return ASTHRA_TEST_PASS;
}

/**
 * Test whitespace-only input
 */
AsthraTestResult test_lexer_whitespace_only_input(AsthraTestContext *context) {
    const struct {
        const char *input;
        const char *description;
    } test_cases[] = {{"   ", "Spaces only"},
                      {"\t\t\t", "Tabs only"},
                      {"\n\n\n", "Newlines only"},
                      {" \t \n \t\n", "Mixed whitespace"},
                      {NULL, NULL}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        const char *description = test_cases[i].description;

        Lexer *lexer = create_test_lexer_with_context(context, input, "test_whitespace.ast");
        if (!lexer) {
            return ASTHRA_TEST_FAIL;
        }

        bool found_non_whitespace = false;
        Token token;
        do {
            token = lexer_next_token(lexer);
            if (token.type != TOKEN_WHITESPACE && token.type != TOKEN_NEWLINE &&
                token.type != TOKEN_EOF) {
                found_non_whitespace = true;
            }
            token_free(&token);
        } while (token.type != TOKEN_EOF);

        // Should only find whitespace tokens (and EOF)
        if (!asthra_test_assert_bool(context, !found_non_whitespace,
                                     "Should only find whitespace tokens for: %s", description)) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

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
    } test_cases[] = {
        {test_lexer_empty_input, "test_lexer_empty_input"},
        {test_lexer_single_character, "test_lexer_single_character"},
        {test_lexer_large_input, "test_lexer_large_input"},
        {test_lexer_complex_structure_position, "test_lexer_complex_structure_position"},
        {test_lexer_whitespace_only_input, "test_lexer_whitespace_only_input"},
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
