/**
 * Asthra Programming Language
 * pub/impl/self Lexer Token Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for pub, impl, and self keyword token recognition
 */

#include "../framework/test_assertions.h"
#include "../framework/test_framework.h"
#include "lexer.h"
#include <string.h>

// Use the framework's ASSERT_TOKEN_TYPE macro

// =============================================================================
// LEXER TOKEN TESTS
// =============================================================================

AsthraTestResult test_pub_token(AsthraTestContext *context) {
    const char *source = "pub";

    Lexer *lexer = lexer_create(source, strlen(source), "test_pub.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for pub test");

    Token token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &token, TOKEN_PUB);

    token_free(&token);
    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_impl_token(AsthraTestContext *context) {
    const char *source = "impl";

    Lexer *lexer = lexer_create(source, strlen(source), "test_impl.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for impl test");

    Token token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &token, TOKEN_IMPL);

    token_free(&token);
    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_self_token(AsthraTestContext *context) {
    const char *source = "self";

    Lexer *lexer = lexer_create(source, strlen(source), "test_self.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for self test");

    Token token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &token, TOKEN_SELF);

    token_free(&token);
    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pub_impl_self_sequence(AsthraTestContext *context) {
    const char *source = "pub impl MyStruct { fn method(self) -> i32 { return 42; } }";

    Lexer *lexer = lexer_create(source, strlen(source), "test_sequence.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for sequence test");

    // Expected token sequence
    TokenType expected[] = {TOKEN_PUB,        TOKEN_IMPL,
                            TOKEN_IDENTIFIER, // MyStruct
                            TOKEN_LEFT_BRACE, TOKEN_FN,
                            TOKEN_IDENTIFIER, // method
                            TOKEN_LEFT_PAREN, TOKEN_SELF,        TOKEN_RIGHT_PAREN, TOKEN_ARROW,
                            TOKEN_I32,        TOKEN_LEFT_BRACE,  TOKEN_RETURN,      TOKEN_INTEGER,
                            TOKEN_SEMICOLON,  TOKEN_RIGHT_BRACE, TOKEN_RIGHT_BRACE, TOKEN_EOF};

    size_t expected_count = sizeof(expected) / sizeof(expected[0]);

    for (size_t i = 0; i < expected_count; i++) {
        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE(context, &token, expected[i]);
        token_free(&token);
    }

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_combined_pub_impl_self_patterns(AsthraTestContext *context) {
    const char *source = "pub fn public_function() {";

    Lexer *lexer = lexer_create(source, strlen(source), "test_combined.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for combined test");

    // Test public function declaration
    Token pub_token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &pub_token, TOKEN_PUB);

    Token fn_token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &fn_token, TOKEN_FN);

    Token identifier_token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE(context, &identifier_token, TOKEN_IDENTIFIER);

    // Check identifier value
    ASTHRA_TEST_ASSERT_STR_EQ(context, identifier_token.data.identifier.name, "public_function",
                              "Expected 'public_function' identifier");

    token_free(&pub_token);
    token_free(&fn_token);
    token_free(&identifier_token);
    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

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
        {test_pub_token, "test_pub_token"},
        {test_impl_token, "test_impl_token"},
        {test_self_token, "test_self_token"},
        {test_pub_impl_self_sequence, "test_pub_impl_self_sequence"},
        {test_combined_pub_impl_self_patterns, "test_combined_pub_impl_self_patterns"},
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
