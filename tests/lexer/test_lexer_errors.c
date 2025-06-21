/**
 * Asthra Programming Language
 * Lexer Error Handling Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test error handling, malformed literals, and boolean/special literals
 */

#include "../framework/test_framework.h"
#include "lexer.h"
#include <string.h>

// Simple token assertion macro for lexer tests only
#define ASSERT_TOKEN_TYPE_SIMPLE(context, token, expected_type) \
    do { \
        if (!asthra_test_assert_int_eq(context, (int)(token)->type, (int)expected_type, \
                                      "Token type mismatch: expected %d, got %d", \
                                      (int)expected_type, (int)(token)->type)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

// =============================================================================
// BOOLEAN AND SPECIAL LITERAL TESTS
// =============================================================================

/**
 * Test boolean literal parsing
 */
AsthraTestResult test_lexer_boolean_literals(AsthraTestContext *context) {
    // Test true
    Lexer* lexer = lexer_create("true", 4, "test_bool.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for 'true'")) {
        return ASTHRA_TEST_FAIL;
    }

    Token token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_BOOL_TRUE);
    
    // Boolean tokens don't store data - the value is determined by the token type
    
    token_free(&token);
    lexer_destroy(lexer);

    // Test false
    lexer = lexer_create("false", 5, "test_bool.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for 'false'")) {
        return ASTHRA_TEST_FAIL;
    }

    token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_BOOL_FALSE);
    
    // Boolean tokens don't store data - the value is determined by the token type

    token_free(&token);
    lexer_destroy(lexer);

    return ASTHRA_TEST_PASS;
}

/**
 * Test null and undefined literal parsing (if supported)
 */
AsthraTestResult test_lexer_null_literals(AsthraTestContext *context) {
    const char* null_keywords[] = {
        "null",
        "nil",
        "None",
        "undefined",
        NULL
    };

    for (int i = 0; null_keywords[i] != NULL; i++) {
        const char* keyword = null_keywords[i];
        
        Lexer* lexer = lexer_create(keyword, strlen(keyword), "test_null.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for null keyword: %s", keyword)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should be either an error token, identifier, or specific keyword tokens depending on language design
        bool is_valid = (token.type == TOKEN_ERROR || 
                        token.type == TOKEN_IDENTIFIER || 
                        token.type == TOKEN_BOOL_FALSE ||  // null might be parsed as false
                        token.type == TOKEN_BOOL_TRUE ||
                        token.type == TOKEN_EOF);
        
        if (!asthra_test_assert_bool(context, is_valid,
                                    "Null keyword '%s' should be handled properly, got token type %d",
                                    keyword, token.type)) {
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
// ERROR HANDLING TESTS
// =============================================================================

/**
 * Test malformed literal handling
 */
AsthraTestResult test_lexer_malformed_literals(AsthraTestContext *context) {
    const char* malformed_cases[] = {
        "\"unterminated string",  // Unterminated string
        "'unterminated char",     // Unterminated character
        "0x",                     // Incomplete hex number
        "0b",                     // Incomplete binary number
        "1.2.3",                  // Multiple decimal points
        "1e",                     // Incomplete exponential
        NULL
    };

    for (int i = 0; malformed_cases[i] != NULL; i++) {
        const char* input = malformed_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_malformed.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for malformed case: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should either be an error token or the lexer should have an error state
        bool has_error = (token.type == TOKEN_ERROR) || (lexer_get_error(lexer) != NULL);
        if (!asthra_test_assert_bool(context, has_error, 
                                    "Expected error for malformed input: %s", input)) {
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
 * Test invalid escape sequences
 */
AsthraTestResult test_lexer_invalid_escapes(AsthraTestContext *context) {
    const char* invalid_escapes[] = {
        "'\\z'",      // Invalid escape character
        "'\\q'",      // Invalid escape character
        "'\\777'",    // Invalid octal (too large)
        "'\\x'",      // Incomplete hex escape
        "'\\u'",      // Incomplete unicode escape
        "'\\u12'",    // Incomplete unicode escape (too short)
        "'\\U1234'",  // Incomplete unicode escape (too short)
        "\"\\z\"",    // Invalid escape in string
        "\"\\u12G4\"", // Invalid hex digit in unicode escape
        NULL
    };

    for (int i = 0; invalid_escapes[i] != NULL; i++) {
        const char* input = invalid_escapes[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_invalid_escapes.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for invalid escape: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should either be an error token or handle gracefully
        bool handled = (token.type == TOKEN_ERROR || 
                       token.type == TOKEN_CHAR || 
                       token.type == TOKEN_STRING ||
                       lexer_get_error(lexer) != NULL);
        
        if (!asthra_test_assert_bool(context, handled,
                                    "Invalid escape sequence should be handled: %s", input)) {
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
 * Test lexer error recovery
 */
AsthraTestResult test_lexer_error_recovery(AsthraTestContext *context) {
    const char* error_recovery_cases[] = {
        "\"bad string\n good_identifier",  // Unterminated string followed by valid token
        "'bad' 'good'",                    // Bad character followed by good character
        "123.456.789 valid_var",           // Invalid float followed by identifier
        "0xZZZ valid_token",               // Invalid hex followed by valid token
        NULL
    };

    for (int i = 0; error_recovery_cases[i] != NULL; i++) {
        const char* input = error_recovery_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_error_recovery.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for error recovery: %s", input)) {
            return ASTHRA_TEST_FAIL;
        }

        // First token might be an error
        Token first_token = lexer_next_token(lexer);
        
        // Should be able to continue and get more tokens
        Token second_token = lexer_next_token(lexer);
        
        // At least one token should be valid or we should reach EOF
        bool has_valid_tokens = (first_token.type != TOKEN_ERROR || 
                               second_token.type != TOKEN_ERROR ||
                               second_token.type == TOKEN_EOF);
        
        if (!asthra_test_assert_bool(context, has_valid_tokens,
                                    "Lexer should recover from errors: %s", input)) {
            token_free(&first_token);
            token_free(&second_token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        token_free(&first_token);
        token_free(&second_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test edge cases in literal boundaries
 */
AsthraTestResult test_lexer_literal_boundaries(AsthraTestContext *context) {
    const struct {
        const char* input;
        const char* description;
    } boundary_cases[] = {
        {"''", "Empty character literal"},
        {"'ab'", "Multi-character literal"},
        {"\"", "Single quote character"},
        {"'\"'", "Quote in character literal"},
        {"\"'\"", "Apostrophe in string literal"},
        {"'\\\\'", "Escaped backslash"},
        {"\"\\\"\"", "Escaped quote in string"},
        {"0x", "Hex prefix only"},
        {"0b", "Binary prefix only"},
        {"0o", "Octal prefix only"},
        {".", "Lone decimal point"},
        {"e", "Lone exponent character"},
        {"0e", "Zero with incomplete exponent"},
        {NULL, NULL}
    };

    for (int i = 0; boundary_cases[i].input != NULL; i++) {
        const char* input = boundary_cases[i].input;
        const char* description = boundary_cases[i].description;
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_boundaries.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for boundary case: %s", description)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should handle gracefully - either parse successfully or generate appropriate error
        // The lexer should not crash and should return a reasonable token (including error tokens)
        bool handled = (token.type == TOKEN_ERROR || 
                       token.type == TOKEN_EOF || 
                       token.type == TOKEN_CHAR ||
                       token.type == TOKEN_STRING ||
                       token.type == TOKEN_INTEGER ||
                       token.type == TOKEN_FLOAT ||
                       token.type == TOKEN_DOT ||
                       token.type == TOKEN_IDENTIFIER);
        
        if (!asthra_test_assert_bool(context, handled,
                                    "Boundary case should be handled: %s (%s)", description, input)) {
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
        const char* test_name;
    } test_cases[] = {
        {test_lexer_boolean_literals, "test_lexer_boolean_literals"},
        {test_lexer_null_literals, "test_lexer_null_literals"},
        {test_lexer_malformed_literals, "test_lexer_malformed_literals"},
        {test_lexer_invalid_escapes, "test_lexer_invalid_escapes"},
        {test_lexer_error_recovery, "test_lexer_error_recovery"},
        {test_lexer_literal_boundaries, "test_lexer_literal_boundaries"},
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
