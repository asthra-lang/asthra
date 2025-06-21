#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "parser_token.h"

int main(void) {
    // Test just the 'none' keyword tokenization
    const char* test_code = "none";
    
    printf("Testing tokenization of: '%s'\n", test_code);
    
    Lexer* lexer = lexer_create(test_code, strlen(test_code), "test");
    if (!lexer) {
        printf("Failed to create lexer\n");
        return 1;
    }
    
    Token token = lexer_next_token(lexer);
    printf("Token type: %d (%s)\n", token.type, token_type_name(token.type));
    printf("Expected: %d (%s)\n", TOKEN_NONE, token_type_name(TOKEN_NONE));
    
    if (token.type == TOKEN_NONE) {
        printf("✅ 'none' correctly recognized as TOKEN_NONE\n");
    } else {
        printf("❌ 'none' incorrectly recognized as %s\n", token_type_name(token.type));
    }
    
    token_free(&token);
    lexer_destroy(lexer);
    
    // Now test function parameter context
    printf("\n--- Testing function parameter context ---\n");
    const char* func_code = "fn test(none)";
    
    Lexer* lexer2 = lexer_create(func_code, strlen(func_code), "test2");
    if (!lexer2) {
        printf("Failed to create lexer2\n");
        return 1;
    }
    
    // Advance through tokens
    Token tokens[4];
    for (int i = 0; i < 4; i++) {
        tokens[i] = lexer_next_token(lexer2);
        printf("Token %d: %d (%s)\n", i, tokens[i].type, token_type_name(tokens[i].type));
    }
    
    // Clean up
    for (int i = 0; i < 4; i++) {
        token_free(&tokens[i]);
    }
    lexer_destroy(lexer2);
    
    return 0;
} 
