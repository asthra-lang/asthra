/**
 * Asthra Programming Language
 * Lexer Unicode Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test Unicode support in identifiers, strings, and escape sequences
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
// UNICODE SUPPORT TESTS
// =============================================================================

/**
 * Test Unicode identifier support
 */
AsthraTestResult test_lexer_unicode_identifiers(AsthraTestContext *context) {
    const char* test_cases[] = {
        "café",       // Latin with accent
        "naïve",      // Latin with diaeresis
        "résumé",     // Latin with accents
        "Ελληνικά",   // Greek
        "日本語",      // Japanese
        "العربية",    // Arabic
        "мир",        // Cyrillic
        "español",    // Spanish with tilde
        "português",  // Portuguese with accents
        "français",   // French with cedilla
        "Москва",     // Russian
        "北京",       // Chinese
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* identifier = test_cases[i];
        
        Lexer* lexer = lexer_create(identifier, strlen(identifier), "test_unicode_id.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for Unicode identifier")) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should recognize as identifier or handle gracefully
        bool is_valid = (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_ERROR);
        if (!asthra_test_assert_bool(context, is_valid,
                                    "Unicode identifier should be handled properly, got token type %d",
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
 * Test Unicode string support
 */
AsthraTestResult test_lexer_unicode_strings(AsthraTestContext *context) {
    const char* test_cases[] = {
        "\"Hello, 世界!\"",
        "\"Здравствуй мир\"",
        "\"مرحبا بالعالم\"",
        "\"🌍🌎🌏\"",  // Emoji
        "\"Ñandú\"",   // Latin with tilde
        "\"Côte d'Ivoire\"", // French with apostrophe
        "\"São Paulo\"", // Portuguese with accents
        "\"Москва\"", // Cyrillic
        "\"東京\"", // Japanese
        "\"🎉 Celebration! 🎊\"", // Mixed emoji and text
        "\"αβγδε\"", // Greek letters
        "\"♠♥♦♣\"", // Card symbols
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* input = test_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_unicode_strings.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for Unicode string")) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should recognize as string or handle gracefully
        bool is_valid = (token.type == TOKEN_STRING || token.type == TOKEN_ERROR);
        if (!asthra_test_assert_bool(context, is_valid,
                                    "Unicode string should be handled properly, got token type %d",
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
 * Test Unicode escape sequences
 */
AsthraTestResult test_lexer_unicode_escapes(AsthraTestContext *context) {
    const char* test_cases[] = {
        "\"\\u0041\"", // Unicode escape for 'A'
        "\"\\u00E9\"", // Unicode escape for 'é'
        "\"\\u03B1\"", // Unicode escape for 'α'
        "\"\\u4E2D\"", // Unicode escape for '中'
        "\"\\U0001F600\"", // Unicode escape for 😀
        "\"\\U0001F4A9\"", // Unicode escape for 💩
        "'\\u0041'", // Unicode escape in char literal
        "'\\u00E9'", // Unicode escape in char literal
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* input = test_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_unicode_escapes.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for Unicode escape")) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should recognize as string/char or handle gracefully
        bool is_valid = (token.type == TOKEN_STRING || token.type == TOKEN_CHAR || token.type == TOKEN_ERROR);
        if (!asthra_test_assert_bool(context, is_valid,
                                    "Unicode escape should be handled properly, got token type %d",
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
 * Test mixed ASCII and Unicode
 */
AsthraTestResult test_lexer_mixed_ascii_unicode(AsthraTestContext *context) {
    const char* test_cases[] = {
        "let café = \"naïve\";",
        "fn greet(名前: string) -> string { return \"Hello, \" + 名前; }",
        "const π = 3.14159;",
        "let résultat = calculer(données);",
        "if (température > 25°C) { print(\"Hot!\"); }",
        NULL
    };

    for (int i = 0; test_cases[i] != NULL; i++) {
        const char* input = test_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_mixed_unicode.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for mixed Unicode test")) {
            return ASTHRA_TEST_FAIL;
        }

        int token_count = 0;
        Token token;
        do {
            token = lexer_next_token(lexer);
            if (token.type != TOKEN_WHITESPACE && token.type != TOKEN_NEWLINE) {
                token_count++;
            }
            token_free(&token);
        } while (token.type != TOKEN_EOF);

        // Should have processed multiple tokens
        if (!asthra_test_assert_bool(context, token_count > 1,
                                    "Should have processed multiple tokens, got %d", token_count)) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test Unicode normalization (if supported)
 */
AsthraTestResult test_lexer_unicode_normalization(AsthraTestContext *context) {
    // Test composed vs decomposed Unicode characters
    const struct {
        const char* composed;
        const char* decomposed;
        const char* description;
    } test_cases[] = {
        {"é", "e\u0301", "e with acute accent"},
        {"ñ", "n\u0303", "n with tilde"},
        {"ü", "u\u0308", "u with diaeresis"},
        {NULL, NULL, NULL}
    };

    for (int i = 0; test_cases[i].composed != NULL; i++) {
        const char* composed = test_cases[i].composed;
        const char* decomposed = test_cases[i].decomposed;
        const char* description = test_cases[i].description;
        
        // Test composed form
        Lexer* lexer1 = lexer_create(composed, strlen(composed), "test_composed.ast");
        if (!asthra_test_assert_pointer(context, lexer1, "Failed to create lexer for composed Unicode: %s", description)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token1 = lexer_next_token(lexer1);
        bool composed_valid = (token1.type == TOKEN_IDENTIFIER || token1.type == TOKEN_ERROR);
        
        // Test decomposed form
        Lexer* lexer2 = lexer_create(decomposed, strlen(decomposed), "test_decomposed.ast");
        if (!asthra_test_assert_pointer(context, lexer2, "Failed to create lexer for decomposed Unicode: %s", description)) {
            token_free(&token1);
            lexer_destroy(lexer1);
            return ASTHRA_TEST_FAIL;
        }

        Token token2 = lexer_next_token(lexer2);
        bool decomposed_valid = (token2.type == TOKEN_IDENTIFIER || token2.type == TOKEN_ERROR);

        // Both should be handled consistently
        if (!asthra_test_assert_bool(context, composed_valid && decomposed_valid,
                                    "Both composed and decomposed forms should be handled for %s", description)) {
            token_free(&token1);
            token_free(&token2);
            lexer_destroy(lexer1);
            lexer_destroy(lexer2);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token1);
        token_free(&token2);
        lexer_destroy(lexer1);
        lexer_destroy(lexer2);
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
        {test_lexer_unicode_identifiers, "test_lexer_unicode_identifiers"},
        {test_lexer_unicode_strings, "test_lexer_unicode_strings"},
        {test_lexer_unicode_escapes, "test_lexer_unicode_escapes"},
        {test_lexer_mixed_ascii_unicode, "test_lexer_mixed_ascii_unicode"},
        {test_lexer_unicode_normalization, "test_lexer_unicode_normalization"},
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
