/**
 * Asthra Programming Language
 * String Escape Sequence Integration Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test escape sequences in complete parsing scenarios
 */

#include "../framework/test_framework.h"
#include "lexer.h"
#include "parser.h"
#include <string.h>

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

/**
 * Test that escape sequences work in variable assignments
 */
AsthraTestResult test_string_escapes_in_expressions(AsthraTestContext *context) {
    const char* source = 
        "let msg = \"Hello\\nWorld\";\n"
        "let path = \"C:\\\\Users\\\\file.txt\";\n"
        "let quote = \"She said \\\"Hi\\\"\";\n";
    
    Lexer* lexer = lexer_create(source, strlen(source), "test_escapes.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test first string literal
    Token token1 = lexer_next_token(lexer); // let
    if (!asthra_test_assert_int_eq(context, (int)token1.type, (int)TOKEN_LET, "Expected LET token")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    token_free(&token1);

    Token token2 = lexer_next_token(lexer); // msg
    if (!asthra_test_assert_int_eq(context, (int)token2.type, (int)TOKEN_IDENTIFIER, "Expected IDENTIFIER token")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    token_free(&token2);

    Token token3 = lexer_next_token(lexer); // =
    if (!asthra_test_assert_int_eq(context, (int)token3.type, (int)TOKEN_ASSIGN, "Expected ASSIGN token")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    token_free(&token3);

    Token token4 = lexer_next_token(lexer); // "Hello\nWorld"
    if (!asthra_test_assert_int_eq(context, (int)token4.type, (int)TOKEN_STRING, "Expected STRING token")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify the escape sequence was processed
    if (!asthra_test_assert_string_eq(context, token4.data.string.value, "Hello\nWorld",
                                     "String escape sequence not processed correctly")) {
        token_free(&token4);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    token_free(&token4);

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

// String interpolation tests removed - feature deprecated for AI generation efficiency

/**
 * Test that escape sequences work in character literals within arrays
 */
AsthraTestResult test_character_escapes_in_arrays(AsthraTestContext *context) {
    const char* source = "let chars = ['\\n', '\\t', '\\\\'];";
    
    Lexer* lexer = lexer_create(source, strlen(source), "test_char_escapes.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer")) {
        return ASTHRA_TEST_FAIL;
    }

    // Skip to the character literals
    Token token;
    do {
        token = lexer_next_token(lexer);
        if (token.type == TOKEN_CHAR) {
            // Verify first character is newline
            if (!asthra_test_assert_int_eq(context, (int)token.data.character.value, (int)'\n',
                                          "First character should be newline")) {
                token_free(&token);
                lexer_destroy(lexer);
                return ASTHRA_TEST_FAIL;
            }
            token_free(&token);
            break;
        }
        token_free(&token);
    } while (token.type != TOKEN_EOF);

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST REGISTRATION
// =============================================================================

/**
 * Register all integration tests
 */
void register_string_escape_integration_tests(void) {
    printf("String escape integration tests registered\n");
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    // FIXED: Use simple test counter instead of complex test context
    int tests_passed = 0;
    int tests_failed = 0;
    
    printf("=== String Escape Integration Tests ===\n");
    
    // Run tests directly
    AsthraTestContext context1 = {0};
    AsthraTestResult result1 = test_string_escapes_in_expressions(&context1);
    printf("String escapes in expressions: %s\n", 
           (result1 == ASTHRA_TEST_PASS) ? "PASS" : "FAIL");
    if (result1 == ASTHRA_TEST_PASS) tests_passed++; else tests_failed++;
    
    AsthraTestContext context2 = {0};
    AsthraTestResult result2 = test_character_escapes_in_arrays(&context2);
    printf("Character escapes in arrays: %s\n", 
           (result2 == ASTHRA_TEST_PASS) ? "PASS" : "FAIL");
    if (result2 == ASTHRA_TEST_PASS) tests_passed++; else tests_failed++;
    
    // Report results
    printf("\n=== Test Results ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    
    return tests_failed > 0 ? 1 : 0;
} 
