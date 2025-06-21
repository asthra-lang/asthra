/**
 * Simple If-Let Token Sequence Test
 * 
 * Tests the token sequence for if-let without full parser
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include essential headers
#include "lexer.h"
#include "parser_token.h"

int main(void) {
    printf("=== Simple If-Let Token Sequence Test ===\n\n");
    
    const char* source = "if let Option.Some(x) = opt { return x; }";
    printf("Source: %s\n\n", source);
    
    // Create lexer
    Lexer* lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) {
        printf("❌ Failed to create lexer\n");
        return 1;
    }
    
    printf("Step 1: Check first token (should be IF)\n");
    Token token1 = lexer_next_token(lexer);
    printf("Token 1: %s\n", token_type_name(token1.type));
    
    if (token1.type != TOKEN_IF) {
        printf("❌ Expected IF token, got %s\n", token_type_name(token1.type));
        lexer_destroy(lexer);
        return 1;
    }
    printf("✅ Found IF token\n\n");
    
    printf("Step 2: Check second token (should be LET)\n");
    Token token2 = lexer_next_token(lexer);
    printf("Token 2: %s\n", token_type_name(token2.type));
    
    if (token2.type != TOKEN_LET) {
        printf("❌ Expected LET token, got %s\n", token_type_name(token2.type));
        lexer_destroy(lexer);
        return 1;
    }
    printf("✅ Found LET token\n\n");
    
    printf("Step 3: Check third token (should be OPTION for 'Option')\n");
    Token token3 = lexer_next_token(lexer);
    printf("Token 3: %s", token_type_name(token3.type));
    if (token3.type == TOKEN_IDENTIFIER) {
        printf(" (%s)", token3.data.identifier.name);
    }
    printf("\n");
    
    if (token3.type != TOKEN_OPTION) {
        printf("❌ Expected OPTION token, got %s\n", token_type_name(token3.type));
        lexer_destroy(lexer);
        return 1;
    }
    printf("✅ Found OPTION token\n\n");
    
    printf("✅ If-let token sequence is correct!\n");
    printf("The lexer properly tokenizes: IF LET OPTION ...\n");
    printf("This confirms that the basic token sequence for if-let statements works.\n\n");
    
    printf("Our fix to parse_if_stmt should now work because:\n");
    printf("1. expect_token(parser, TOKEN_IF) will consume the IF token\n");
    printf("2. match_token(parser, TOKEN_LET) will detect the LET token\n");
    printf("3. advance_token(parser) will move past the LET token\n");
    printf("4. The pattern parsing can then begin with the OPTION token\n");
    
    // Cleanup
    lexer_destroy(lexer);
    
    return 0;
} 
