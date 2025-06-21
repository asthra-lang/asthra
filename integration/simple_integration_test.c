#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "src/parser/lexer.h"

/**
 * Simple Integration Test for String Escape Sequences
 * This test bypasses the test framework to verify core functionality
 */

int test_string_escape_sequences() {
    printf("=== String Escape Sequence Integration Test ===\n");
    
    // Test case 1: Basic escape sequences in string literals
    const char* input1 = "\"Hello\\nWorld\\t!\"";
    Lexer* lexer1 = lexer_create(input1, strlen(input1), "test1.ast");
    assert(lexer1 != NULL);
    
    Token token1 = lexer_next_token(lexer1);
    assert(token1.type == TOKEN_STRING);
    
    // Verify the escape sequences were processed
    const char* expected1 = "Hello\nWorld\t!";
    assert(strcmp(token1.data.string.value, expected1) == 0);
    printf("✓ Basic escape sequences: \\n and \\t processed correctly\n");
    
    token_free(&token1);
    lexer_destroy(lexer1);
    
    // Test case 2: Quote escaping
    const char* input2 = "\"She said \\\"Hello\\\"\"";
    Lexer* lexer2 = lexer_create(input2, strlen(input2), "test2.ast");
    assert(lexer2 != NULL);
    
    Token token2 = lexer_next_token(lexer2);
    assert(token2.type == TOKEN_STRING);
    
    const char* expected2 = "She said \"Hello\"";
    assert(strcmp(token2.data.string.value, expected2) == 0);
    printf("✓ Quote escaping: \\\" processed correctly\n");
    
    token_free(&token2);
    lexer_destroy(lexer2);
    
    // Test case 3: Backslash escaping
    const char* input3 = "\"Path: C:\\\\Users\\\\\"";
    Lexer* lexer3 = lexer_create(input3, strlen(input3), "test3.ast");
    assert(lexer3 != NULL);
    
    Token token3 = lexer_next_token(lexer3);
    assert(token3.type == TOKEN_STRING);
    
    const char* expected3 = "Path: C:\\Users\\";
    assert(strcmp(token3.data.string.value, expected3) == 0);
    printf("✓ Backslash escaping: \\\\ processed correctly\n");
    
    token_free(&token3);
    lexer_destroy(lexer3);
    
    // Test case 4: Mixed escape sequences
    const char* input4 = "\"Line1\\nTab:\\tQuote:\\\"\\\\\"";
    Lexer* lexer4 = lexer_create(input4, strlen(input4), "test4.ast");
    assert(lexer4 != NULL);
    
    Token token4 = lexer_next_token(lexer4);
    assert(token4.type == TOKEN_STRING);
    
    const char* expected4 = "Line1\nTab:\tQuote:\"\\";
    assert(strcmp(token4.data.string.value, expected4) == 0);
    printf("✓ Mixed escape sequences processed correctly\n");
    
    token_free(&token4);
    lexer_destroy(lexer4);
    
    return 1; // Success
}

int test_character_escape_sequences() {
    printf("\n=== Character Escape Sequence Integration Test ===\n");
    
    // Test case 1: Newline character
    const char* input1 = "'\\n'";
    Lexer* lexer1 = lexer_create(input1, strlen(input1), "char_test1.ast");
    assert(lexer1 != NULL);
    
    Token token1 = lexer_next_token(lexer1);
    assert(token1.type == TOKEN_CHAR);
    assert(token1.data.character.value == '\n');
    printf("✓ Character newline: '\\n' processed correctly\n");
    
    token_free(&token1);
    lexer_destroy(lexer1);
    
    // Test case 2: Quote character escaping
    const char* input2 = "'\\\"'";
    Lexer* lexer2 = lexer_create(input2, strlen(input2), "char_test2.ast");
    assert(lexer2 != NULL);
    
    Token token2 = lexer_next_token(lexer2);
    assert(token2.type == TOKEN_CHAR);
    assert(token2.data.character.value == '"');
    printf("✓ Character quote: '\\\"' processed correctly\n");
    
    token_free(&token2);
    lexer_destroy(lexer2);
    
    // Test case 3: Backslash character
    const char* input3 = "'\\\\'";
    Lexer* lexer3 = lexer_create(input3, strlen(input3), "char_test3.ast");
    assert(lexer3 != NULL);
    
    Token token3 = lexer_next_token(lexer3);
    assert(token3.type == TOKEN_CHAR);
    assert(token3.data.character.value == '\\');
    printf("✓ Character backslash: '\\\\' processed correctly\n");
    
    token_free(&token3);
    lexer_destroy(lexer3);
    
    return 1; // Success
}

int test_error_cases() {
    printf("\n=== Error Case Integration Test ===\n");
    
    // Test case 1: Invalid escape sequence
    const char* input1 = "\"invalid\\q\"";
    Lexer* lexer1 = lexer_create(input1, strlen(input1), "error_test1.ast");
    assert(lexer1 != NULL);
    
    Token token1 = lexer_next_token(lexer1);
    // Should be an error token due to invalid escape sequence
    assert(token1.type == TOKEN_ERROR || lexer1->error_message != NULL);
    printf("✓ Invalid escape sequence \\q produces error correctly\n");
    
    token_free(&token1);
    lexer_destroy(lexer1);
    
    return 1; // Success
}

int main() {
    printf("Running String Escape Sequence Integration Tests\n");
    printf("================================================\n\n");
    
    int tests_passed = 0;
    int total_tests = 3;
    
    if (test_string_escape_sequences()) {
        tests_passed++;
    }
    
    if (test_character_escape_sequences()) {
        tests_passed++;
    }
    
    if (test_error_cases()) {
        tests_passed++;
    }
    
    printf("\n================================================\n");
    printf("Integration Test Results: %d/%d tests passed\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("✅ All integration tests PASSED!\n");
        printf("✅ String escape sequence implementation is working correctly\n");
        return 0;
    } else {
        printf("❌ Some integration tests FAILED!\n");
        return 1;
    }
} 
