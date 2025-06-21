/**
 * Asthra Programming Language
 * Lexer Error Handling Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test error conditions and invalid input handling
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "lexer.h"
#include <string.h>
#include <errno.h>

// =============================================================================
// TEST FIXTURE AND UTILITIES
// =============================================================================

/**
 * Using ASSERT_TOKEN_TYPE from test_assertions.h
 */

// =============================================================================
// ERROR HANDLING TESTS
// =============================================================================

/**
 * Test handling of unterminated string literals
 */
AsthraTestResult test_lexer_unterminated_string(AsthraTestContext *context) {
    const char* test_cases[] = {
        "\"unterminated string",
        "\"string with\nnewline",
        "\"string with \\", // Unterminated escape
        "'unterminated char",
        "\"string with\rtab",
        "'char with\nnewline",
        "\"escape at end\\",
        "'escape at end\\",
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* input = test_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_unterminated.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for input: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should produce error token for unterminated strings
        ASSERT_TOKEN_TYPE(context, &token, TOKEN_ERROR);

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test handling of invalid characters
 */
AsthraTestResult test_lexer_invalid_character(AsthraTestContext *context) {
    const char* test_cases[] = {
        "\x01",  // Control character
        "\x02",  // STX character
        "\x03",  // ETX character
        "\x7F",  // DEL character
        "\x1B",  // ESC character
        "§",     // Invalid symbol
        "¿",     // Invalid punctuation
        "€",     // Currency symbol
        "™",     // Trademark symbol
        "©",     // Copyright symbol
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* input = test_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_invalid_char.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for invalid character test")) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should either skip invalid characters or produce error token
        bool is_valid_response = (token.type == TOKEN_ERROR || token.type == TOKEN_EOF);
        if (!asthra_test_assert_bool(context, is_valid_response,
                                    "Invalid character should produce error or EOF, got token type %d",
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
 * Test handling of invalid number formats
 */
AsthraTestResult test_lexer_invalid_number_format(AsthraTestContext *context) {
    const char* test_cases[] = {
        "0x",       // Incomplete hex
        "0b",       // Incomplete binary
        "0o",       // Incomplete octal
        "0b2",      // Invalid binary digit
        "0b8",      // Invalid binary digit
        "0xG",      // Invalid hex digit
        "0xZ",      // Invalid hex digit
        "0o8",      // Invalid octal digit
        "0o9",      // Invalid octal digit
        "1.2.3",    // Multiple decimal points
        "1..2",     // Double decimal point
        "1e",       // Incomplete scientific notation
        "1e+",      // Incomplete exponent
        "1e-",      // Incomplete negative exponent
        "1.e",      // Missing fraction digits
        "1E",       // Incomplete scientific notation (uppercase)
        "1E+",      // Incomplete exponent (uppercase)
        ".e5",      // Invalid decimal start with exponent
        "0x.5",     // Hex with decimal point
        "0b1.0",    // Binary with decimal point
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* input = test_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_invalid_numbers.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for invalid number: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should produce error token for invalid number formats
        ASSERT_TOKEN_TYPE(context, &token, TOKEN_ERROR);

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test handling of invalid escape sequences
 */
AsthraTestResult test_lexer_invalid_escape_sequences(AsthraTestContext *context) {
    const char* test_cases[] = {
        "\"\\q\"",    // Invalid escape character
        "\"\\z\"",    // Invalid escape character
        "'\\q'",      // Invalid escape in char literal
        "'\\z'",      // Invalid escape in char literal
        "\"\\x\"",    // Incomplete hex escape (if supported)
        "\"\\xG\"",   // Invalid hex escape (if supported)
        "\"\\u\"",    // Incomplete unicode escape (if supported)
        "\"\\uGGGG\"", // Invalid unicode escape (if supported)
        "\"\\U\"",    // Incomplete long unicode escape (if supported)
        "\"\\UGGGGGGGG\"", // Invalid long unicode escape (if supported)
        "\"\\777\"",  // Invalid octal escape (if supported)
        "\"\\888\"",  // Invalid octal digits (if supported)
        "\"incomplete\\",  // Incomplete escape at end
        "'incomplete\\",   // Incomplete escape at end (char)
        "\"{var}\\q\"",    // Invalid escape in interpolation
        "\"{var}\\z\"",    // Invalid escape in interpolation
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* input = test_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_invalid_escapes.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for invalid escape: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should produce error token for invalid escape sequences
        ASSERT_TOKEN_TYPE(context, &token, TOKEN_ERROR);

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test handling of buffer overflow conditions
 */
AsthraTestResult test_lexer_buffer_overflow(AsthraTestContext *context) {
    // Test very long identifier
    char long_identifier[1000];
    memset(long_identifier, 'a', sizeof(long_identifier) - 1);
    long_identifier[sizeof(long_identifier) - 1] = '\0';
    
    Lexer* lexer = lexer_create(long_identifier, strlen(long_identifier), "test_overflow.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for long identifier")) {
        return ASTHRA_TEST_FAIL;
    }

    Token token = lexer_next_token(lexer);
    
    // Should handle long identifiers gracefully (either truncate or error)
    bool is_valid_response = (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_ERROR);
    if (!asthra_test_assert_bool(context, is_valid_response,
                                "Long identifier should produce identifier or error, got token type %d",
                                token.type)) {
        token_free(&token);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    token_free(&token);
    lexer_destroy(lexer);

    // Test very long string literal
    char long_string[500];
    long_string[0] = '"';
    memset(long_string + 1, 'b', sizeof(long_string) - 3);
    long_string[sizeof(long_string) - 2] = '"';
    long_string[sizeof(long_string) - 1] = '\0';
    
    lexer = lexer_create(long_string, strlen(long_string), "test_overflow.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for long string")) {
        return ASTHRA_TEST_FAIL;
    }

    token = lexer_next_token(lexer);
    
    // Should handle long strings gracefully
    is_valid_response = (token.type == TOKEN_STRING || token.type == TOKEN_ERROR);
    if (!asthra_test_assert_bool(context, is_valid_response,
                                "Long string should produce string or error, got token type %d",
                                token.type)) {
        token_free(&token);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    token_free(&token);
    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

/**
 * Test handling of null and empty inputs
 */
AsthraTestResult test_lexer_null_empty_input(AsthraTestContext *context) {
    // Test null input
    Lexer* lexer = lexer_create(NULL, 0, "test_null.ast");
    if (lexer != NULL) {
        // If lexer creation succeeds with null input, it should handle it gracefully
        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE(context, &token, TOKEN_EOF);
        token_free(&token);
        lexer_destroy(lexer);
    }

    // Test empty input
    lexer = lexer_create("", 0, "test_empty.ast");
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

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for lexer error handling
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
        const char* test_name;
    } test_cases[] = {
        {test_lexer_unterminated_string, "test_lexer_unterminated_string"},
        {test_lexer_invalid_character, "test_lexer_invalid_character"},
        {test_lexer_invalid_number_format, "test_lexer_invalid_number_format"},
        {test_lexer_invalid_escape_sequences, "test_lexer_invalid_escape_sequences"},
        {test_lexer_buffer_overflow, "test_lexer_buffer_overflow"},
        {test_lexer_null_empty_input, "test_lexer_null_empty_input"},
        {NULL, NULL}
    };

    // Run tests
    for (int i = 0; test_cases[i].test_func != NULL; i++) {
        AsthraTestMetadata metadata = {
            .name = test_cases[i].test_name,
            .file = __FILE__,
            .line = __LINE__,
            .function = test_cases[i].test_name,
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
            .timeout_ns = 30000000000ULL,
            .skip = false,
            .skip_reason = NULL
        };

        AsthraTestResult result = asthra_test_run_single(test_cases[i].test_func, &metadata, stats);
        printf("[%s] %s\n", 
               result == ASTHRA_TEST_PASS ? "PASS" : "FAIL", 
               test_cases[i].test_name);
    }

    // Print results
    printf("\nTest Results: %llu/%llu passed\n", 
           (unsigned long long)stats->tests_passed, (unsigned long long)stats->tests_run);

    bool success = (stats->tests_failed == 0);
    asthra_test_statistics_destroy(stats);
    return success ? 0 : 1;
} 
