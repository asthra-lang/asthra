/**
 * Asthra Programming Language
 * Token Creation and Management Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test token creation, cloning, and basic management functions
 */

#include "../framework/test_assertions.h"
#include "../framework/test_framework.h"
#include "lexer.h"
#include <string.h>

// =============================================================================
// TOKEN CREATION AND MANAGEMENT TESTS
// =============================================================================

/**
 * Test token creation and destruction
 */
AsthraTestResult test_token_create_and_destroy(AsthraTestContext *context) {
    SourceLocation location = {.filename = (char *)"test.ast", .line = 1, .column = 1, .offset = 0};

    // Test creating various token types
    TokenType test_types[] = {TOKEN_INTEGER,    TOKEN_FLOAT,     TOKEN_STRING,
                              TOKEN_IDENTIFIER, TOKEN_CHAR,      TOKEN_PLUS,
                              TOKEN_LEFT_PAREN, TOKEN_SEMICOLON, TOKEN_EOF};

    size_t type_count = sizeof(test_types) / sizeof(test_types[0]);

    for (size_t i = 0; i < type_count; i++) {
        Token token = token_create(test_types[i], location);

        // Verify token type is set correctly
        ASTHRA_TEST_ASSERT_TRUE(context, token.type == test_types[i],
                                "Token type should be set correctly");

        // Verify location is set correctly
        ASTHRA_TEST_ASSERT_STR_EQ(context, token.location.filename, "test.ast",
                                  "Token filename should be set correctly");
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.location.line, 1,
                              "Token line should be set correctly");
        ASTHRA_TEST_ASSERT_EQ(context, (int)token.location.column, 1,
                              "Token column should be set correctly");

        // Clean up token
        token_free(&token);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test token cloning functionality
 */
AsthraTestResult test_token_clone(AsthraTestContext *context) {
    SourceLocation location = {
        .filename = (char *)"test_clone.ast", .line = 42, .column = 10, .offset = 100};

    // Create original token
    Token original = token_create(TOKEN_IDENTIFIER, location);

    // Set some data for the identifier
    original.data.identifier.name = malloc(10);
    strcpy(original.data.identifier.name, "test_var");
    original.data.identifier.length = 8;

    // Clone the token
    Token cloned = token_clone(&original);

    // Verify the clone matches the original
    ASTHRA_TEST_ASSERT_TRUE(context, cloned.type == TOKEN_IDENTIFIER,
                            "Cloned token type should match original");
    ASTHRA_TEST_ASSERT_STR_EQ(context, cloned.location.filename, "test_clone.ast",
                              "Cloned token filename should match original");
    ASTHRA_TEST_ASSERT_EQ(context, (int)cloned.location.line, 42,
                          "Cloned token line should match original");
    ASTHRA_TEST_ASSERT_EQ(context, (int)cloned.location.column, 10,
                          "Cloned token column should match original");

    // Verify identifier data was cloned
    if (cloned.data.identifier.name) {
        ASTHRA_TEST_ASSERT_STR_EQ(context, cloned.data.identifier.name, "test_var",
                                  "Cloned identifier name should match original");
    }
    ASTHRA_TEST_ASSERT_EQ(context, (int)cloned.data.identifier.length, 8,
                          "Cloned identifier length should match original");

    // Verify they are separate objects (modify original, check clone unchanged)
    strcpy(original.data.identifier.name, "modified");
    ASTHRA_TEST_ASSERT_STR_EQ(context, cloned.data.identifier.name, "test_var",
                              "Cloned token should be independent of original");

    token_free(&original);
    token_free(&cloned);
    return ASTHRA_TEST_PASS;
}

/**
 * Test token type name mapping
 */
AsthraTestResult test_token_type_name_mapping(AsthraTestContext *context) {
    const struct {
        TokenType type;
        const char *expected_name;
    } test_cases[] = {{TOKEN_INTEGER, "INTEGER"},
                      {TOKEN_FLOAT, "FLOAT"},
                      {TOKEN_STRING, "STRING"},
                      {TOKEN_IDENTIFIER, "IDENTIFIER"},
                      {TOKEN_LET, "LET"},
                      {TOKEN_FN, "FN"},
                      {TOKEN_IF, "IF"},
                      {TOKEN_PLUS, "PLUS"},
                      {TOKEN_MINUS, "MINUS"},
                      {TOKEN_EQUAL, "EQUAL"},
                      {TOKEN_LEFT_PAREN, "LEFT_PAREN"},
                      {TOKEN_RIGHT_PAREN, "RIGHT_PAREN"},
                      {TOKEN_SEMICOLON, "SEMICOLON"},
                      {TOKEN_EOF, "EOF"},
                      {TOKEN_ERROR, "ERROR"}};

    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < test_count; i++) {
        const char *name = token_type_name(test_cases[i].type);

        ASTHRA_TEST_ASSERT_NOT_NULL(context, name,
                                    "token_type_name should return non-NULL for valid token type");
        ASTHRA_TEST_ASSERT_STR_EQ(context, name, test_cases[i].expected_name,
                                  "Token type name mismatch for type %d", test_cases[i].type);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test token data integrity for various types
 */
AsthraTestResult test_token_data_integrity(AsthraTestContext *context) {
    SourceLocation location = {
        .filename = (char *)"test_data.ast", .line = 1, .column = 1, .offset = 0};

    // Test integer token data
    Token int_token = token_create(TOKEN_INTEGER, location);
    int_token.data.integer.value = 42;

    ASTHRA_TEST_ASSERT_EQ(context, (long)int_token.data.integer.value, 42L,
                          "Integer token data should be preserved");
    token_free(&int_token);

    // Test float token data
    Token float_token = token_create(TOKEN_FLOAT, location);
    float_token.data.float_val.value = 3.14;

    double diff = float_token.data.float_val.value - 3.14;
    if (diff < 0)
        diff = -diff;
    ASTHRA_TEST_ASSERT_TRUE(context, diff < 0.000001, "Float token data should be preserved");
    token_free(&float_token);

    // Test string token data
    Token string_token = token_create(TOKEN_STRING, location);
    string_token.data.string.value = malloc(10);
    strcpy(string_token.data.string.value, "test");
    string_token.data.string.length = 4;

    ASTHRA_TEST_ASSERT_STR_EQ(context, string_token.data.string.value, "test",
                              "String token data should be preserved");
    ASTHRA_TEST_ASSERT_EQ(context, (int)string_token.data.string.length, 4,
                          "String token length should be preserved");
    token_free(&string_token);

    // Test character token data
    Token char_token = token_create(TOKEN_CHAR, location);
    char_token.data.character.value = 'A';

    ASTHRA_TEST_ASSERT_EQ(context, (int)char_token.data.character.value, (int)'A',
                          "Character token data should be preserved");
    token_free(&char_token);

    return ASTHRA_TEST_PASS;
}

/**
 * Test token memory management
 */
AsthraTestResult test_token_memory_management(AsthraTestContext *context) {
    SourceLocation location = {
        .filename = (char *)"test_memory.ast", .line = 1, .column = 1, .offset = 0};

    // Test that tokens properly manage their memory
    Token *tokens = malloc(sizeof(Token) * 100);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, tokens, "Failed to allocate memory for token array");

    // Create many tokens with allocated data
    for (int i = 0; i < 100; i++) {
        tokens[i] = token_create(TOKEN_STRING, location);
        tokens[i].data.string.value = malloc(20);
        snprintf(tokens[i].data.string.value, 20, "string_%d", i);
        tokens[i].data.string.length = strlen(tokens[i].data.string.value);
    }

    // Verify all tokens have valid data
    for (int i = 0; i < 100; i++) {
        char expected[20];
        snprintf(expected, 20, "string_%d", i);

        ASTHRA_TEST_ASSERT_STR_EQ(context, tokens[i].data.string.value, expected,
                                  "Token %d data should be preserved", i);
    }

    // Clean up all tokens
    for (int i = 0; i < 100; i++) {
        token_free(&tokens[i]);
    }
    free(tokens);

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for token creation and management
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
    } test_cases[] = {{test_token_create_and_destroy, "test_token_create_and_destroy"},
                      {test_token_clone, "test_token_clone"},
                      {test_token_type_name_mapping, "test_token_type_name_mapping"},
                      {test_token_data_integrity, "test_token_data_integrity"},
                      {test_token_memory_management, "test_token_memory_management"},
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
