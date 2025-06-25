/**
 * Asthra Programming Language
 * Lexer Token Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test all token types including keywords, operators, and punctuation
 */

#include "../framework/test_assertions.h"
#include "../framework/test_framework.h"
#include "lexer.h"
#include <string.h>

// =============================================================================
// TOKEN TYPE TESTS
// =============================================================================

/**
 * Test all keyword token types
 */
AsthraTestResult test_lexer_keywords(AsthraTestContext *context) {
    const char *test_cases[][2] = {
        {"package", "TOKEN_PACKAGE"},
        {"import", "TOKEN_IMPORT"},
        {"as", "TOKEN_AS"},
        {"pub", "TOKEN_PUB"},
        {"fn", "TOKEN_FN"},
        {"struct", "TOKEN_STRUCT"},
        {"extern", "TOKEN_EXTERN"},
        {"let", "TOKEN_LET"},
        {"if", "TOKEN_IF"},
        {"else", "TOKEN_ELSE"},
        {"for", "TOKEN_FOR"},
        {"in", "TOKEN_IN"},
        {"return", "TOKEN_RETURN"},
        {"match", "TOKEN_MATCH"},
        {"spawn", "TOKEN_SPAWN"},
        {"unsafe", "TOKEN_UNSAFE"},
        {"sizeof", "TOKEN_SIZEOF"},
        {"impl", "TOKEN_IMPL"},
        {"self", "TOKEN_SELF"},
        {"true", "TOKEN_BOOL_TRUE"},
        {"false", "TOKEN_BOOL_FALSE"},
        // Tier 1 Concurrency Features (Core & Simple)
        {"spawn_with_handle", "TOKEN_SPAWN_WITH_HANDLE"},
        {"await", "TOKEN_AWAIT"},
        // Note: Tier 3 concurrency tokens moved to stdlib:
        // {"channel", "TOKEN_CHANNEL"}, {"send", "TOKEN_SEND"}, {"recv", "TOKEN_RECV"},
        // {"select", "TOKEN_SELECT"}, {"default", "TOKEN_DEFAULT"},
        // {"worker_pool", "TOKEN_WORKER_POOL"}, {"timeout", "TOKEN_TIMEOUT"}, {"close",
        // "TOKEN_CLOSE"},
        {NULL, NULL}};

    for (int i = 0; test_cases[i][0] != NULL; i++) {
        const char *keyword = test_cases[i][0];

        Lexer *lexer = lexer_create(keyword, strlen(keyword), "test_keywords.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for keyword: %s",
                                    keyword);

        Token token = lexer_next_token(lexer);

        // Check that the keyword is properly recognized
        TokenType expected_type = keyword_lookup(keyword, strlen(keyword));
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.type, (int)expected_type,
                              "Token type mismatch: expected %d, got %d", (int)expected_type,
                              (int)token.type);

        // Verify token value matches keyword
        if (token.data.identifier.name) {
            ASTHRA_TEST_ASSERT_STR_EQ(context, token.data.identifier.name, keyword,
                                      "Keyword value mismatch for '%s'", keyword);
        }

        // Check that next token is EOF
        Token eof_token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)eof_token.type, (int)TOKEN_EOF,
                              "Expected EOF after keyword");

        token_free(&token);
        token_free(&eof_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test all operator token types
 */
AsthraTestResult test_lexer_operators(AsthraTestContext *context) {
    const struct {
        const char *operator;
        TokenType expected_type;
    } test_cases[] = {
        {"+", TOKEN_PLUS},         {"-", TOKEN_MINUS},        {"*", TOKEN_MULTIPLY},
        {"/", TOKEN_DIVIDE},       {"%", TOKEN_MODULO},       {"=", TOKEN_ASSIGN},
        {"==", TOKEN_EQUAL},       {"!=", TOKEN_NOT_EQUAL},   {"<", TOKEN_LESS_THAN},
        {"<=", TOKEN_LESS_EQUAL},  {">", TOKEN_GREATER_THAN}, {">=", TOKEN_GREATER_EQUAL},
        {"&&", TOKEN_LOGICAL_AND}, {"||", TOKEN_LOGICAL_OR},  {"!", TOKEN_LOGICAL_NOT},
        {"&", TOKEN_BITWISE_AND},  {"|", TOKEN_BITWISE_OR},   {"^", TOKEN_BITWISE_XOR},
        {"~", TOKEN_BITWISE_NOT},  {"<<", TOKEN_LEFT_SHIFT},  {">>", TOKEN_RIGHT_SHIFT},
        {"->", TOKEN_ARROW},       {NULL, TOKEN_EOF}};

    for (int i = 0; test_cases[i].operator!= NULL; i++) {
        const char *op = test_cases[i].operator;
        TokenType expected = test_cases[i].expected_type;

        Lexer *lexer = lexer_create(op, strlen(op), "test_operators.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for operator: %s", op);

        Token token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.type, (int)expected,
                              "Operator token type mismatch for '%s'", op);

        Token eof_token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)eof_token.type, (int)TOKEN_EOF,
                              "Expected EOF after operator");

        token_free(&token);
        token_free(&eof_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test all punctuation token types
 */
AsthraTestResult test_lexer_punctuation(AsthraTestContext *context) {
    const struct {
        const char *punctuation;
        TokenType expected_type;
    } test_cases[] = {
        {"(", TOKEN_LEFT_PAREN},  {")", TOKEN_RIGHT_PAREN},   {"{", TOKEN_LEFT_BRACE},
        {"}", TOKEN_RIGHT_BRACE}, {"[", TOKEN_LEFT_BRACKET},  {"]", TOKEN_RIGHT_BRACKET},
        {";", TOKEN_SEMICOLON},   {",", TOKEN_COMMA},         {".", TOKEN_DOT},
        {":", TOKEN_COLON},       {"::", TOKEN_DOUBLE_COLON}, {"#", TOKEN_HASH},
        {NULL, TOKEN_EOF}};

    for (int i = 0; test_cases[i].punctuation != NULL; i++) {
        const char *punct = test_cases[i].punctuation;
        TokenType expected = test_cases[i].expected_type;

        Lexer *lexer = lexer_create(punct, strlen(punct), "test_punctuation.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for punctuation: %s",
                                    punct);

        Token token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.type, (int)expected,
                              "Punctuation token type mismatch for '%s'", punct);

        Token eof_token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)eof_token.type, (int)TOKEN_EOF,
                              "Expected EOF after punctuation");

        token_free(&token);
        token_free(&eof_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test basic type tokens
 */
AsthraTestResult test_lexer_types(AsthraTestContext *context) {
    const struct {
        const char *type_name;
        TokenType expected_type;
    } test_cases[] = {{"i8", TOKEN_I8},     {"i16", TOKEN_I16},     {"i32", TOKEN_I32},
                      {"i64", TOKEN_I64},   {"u8", TOKEN_U8},       {"u16", TOKEN_U16},
                      {"u32", TOKEN_U32},   {"u64", TOKEN_U64},     {"f32", TOKEN_F32},
                      {"f64", TOKEN_F64},   {"bool", TOKEN_BOOL},   {"string", TOKEN_STRING_TYPE},
                      {"void", TOKEN_VOID}, {"usize", TOKEN_USIZE}, {"isize", TOKEN_ISIZE},
                      {NULL, TOKEN_EOF}};

    for (int i = 0; test_cases[i].type_name != NULL; i++) {
        const char *type_name = test_cases[i].type_name;
        TokenType expected = test_cases[i].expected_type;

        Lexer *lexer = lexer_create(type_name, strlen(type_name), "test_types.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for type: %s",
                                    type_name);

        Token token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.type, (int)expected,
                              "Type token type mismatch for '%s'", type_name);

        Token eof_token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)eof_token.type, (int)TOKEN_EOF,
                              "Expected EOF after type");

        token_free(&token);
        token_free(&eof_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test identifier recognition
 */
AsthraTestResult test_lexer_identifiers(AsthraTestContext *context) {
    const struct {
        const char *input;
        const char *expected_value;
    } test_cases[] = {{"identifier", "identifier"}, {"variable_name", "variable_name"},
                      {"CamelCase", "CamelCase"},   {"snake_case", "snake_case"},
                      {"name123", "name123"},       {"_private", "_private"},
                      {"__internal", "__internal"}, {NULL, NULL}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        const char *expected = test_cases[i].expected_value;

        Lexer *lexer = lexer_create(input, strlen(input), "test_identifiers.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for identifier: %s",
                                    input);

        Token token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.type, (int)TOKEN_IDENTIFIER,
                              "Expected identifier token for '%s'", input);

        ASTHRA_TEST_ASSERT_STR_EQ(context, token.data.identifier.name, expected,
                                  "Identifier value mismatch for '%s'", input);

        Token eof_token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)eof_token.type, (int)TOKEN_EOF,
                              "Expected EOF after identifier");

        token_free(&token);
        token_free(&eof_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test whitespace and comment handling
 */
AsthraTestResult test_lexer_whitespace_comments(AsthraTestContext *context) {
    const char *input = "  /* comment */  identifier";

    Lexer *lexer = lexer_create(input, strlen(input), "test_whitespace.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer");

    Token token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT_EQ(context, (int)token.type, (int)TOKEN_IDENTIFIER,
                          "Expected identifier after whitespace and comments");
    ASTHRA_TEST_ASSERT_STR_EQ(context, token.data.identifier.name, "identifier",
                              "Expected 'identifier' token value");

    token_free(&token);
    lexer_destroy(lexer);

    return ASTHRA_TEST_PASS;
}

/**
 * Test token sequence parsing
 */
AsthraTestResult test_lexer_token_sequence(AsthraTestContext *context) {
    const char *input = "fn main(none) { let x = 42; }";
    TokenType expected[] = {TOKEN_FN,
                            TOKEN_IDENTIFIER, // main
                            TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_LET,
                            TOKEN_IDENTIFIER, // x
                            TOKEN_ASSIGN,
                            TOKEN_INTEGER, // 42
                            TOKEN_SEMICOLON,  TOKEN_RIGHT_BRACE, TOKEN_EOF};

    Lexer *lexer = lexer_create(input, strlen(input), "test_sequence.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer");

    for (int i = 0; expected[i] != TOKEN_EOF; i++) {
        Token token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.type, (int)expected[i],
                              "Token sequence mismatch at position %d", i);
        token_free(&token);
    }

    // Check final EOF
    Token eof_token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT_EQ(context, (int)eof_token.type, (int)TOKEN_EOF,
                          "Expected EOF at end of sequence");
    token_free(&eof_token);

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
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
    } test_cases[] = {{test_lexer_keywords, "test_lexer_keywords"},
                      {test_lexer_operators, "test_lexer_operators"},
                      {test_lexer_punctuation, "test_lexer_punctuation"},
                      {test_lexer_types, "test_lexer_types"},
                      {test_lexer_identifiers, "test_lexer_identifiers"},
                      {test_lexer_whitespace_comments, "test_lexer_whitespace_comments"},
                      {test_lexer_token_sequence, "test_lexer_token_sequence"},
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
