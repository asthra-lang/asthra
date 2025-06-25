/**
 * Step-by-step If-Let Parsing Test
 *
 * Manually steps through the if-let parsing process
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include essential headers
#include "lexer.h"
#include "parser_core.h"
#include "parser_token.h"

int main(void) {
    printf("=== Step-by-step If-Let Parsing Test ===\n\n");

    const char *source = "if let Option.Some(x) = opt { return x; }";
    printf("Source: %s\n\n", source);

    // Create lexer
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) {
        printf("❌ Failed to create lexer\n");
        return 1;
    }

    // Create parser
    Parser *parser = parser_create(lexer);
    if (!parser) {
        printf("❌ Failed to create parser\n");
        lexer_destroy(lexer);
        return 1;
    }

    printf("Step 1: Check initial token\n");
    printf("Current token: %s\n", token_type_name(parser->current_token.type));

    if (parser->current_token.type != TOKEN_IF) {
        printf("❌ Expected IF token, got %s\n", token_type_name(parser->current_token.type));
        parser_destroy(parser);
        return 1;
    }
    printf("✅ Found IF token\n\n");

    printf("Step 2: Advance past IF token\n");
    advance_token(parser);
    printf("Current token: %s\n", token_type_name(parser->current_token.type));

    if (parser->current_token.type != TOKEN_LET) {
        printf("❌ Expected LET token, got %s\n", token_type_name(parser->current_token.type));
        parser_destroy(parser);
        return 1;
    }
    printf("✅ Found LET token\n\n");

    printf("Step 3: Check if match_token detects LET\n");
    bool matches_let = match_token(parser, TOKEN_LET);
    printf("match_token(parser, TOKEN_LET) = %s\n", matches_let ? "true" : "false");

    if (!matches_let) {
        printf("❌ match_token failed to detect LET token\n");
        parser_destroy(parser);
        return 1;
    }
    printf("✅ match_token correctly detects LET token\n\n");

    printf("Step 4: Advance past LET token\n");
    advance_token(parser);
    printf("Current token: %s\n", token_type_name(parser->current_token.type));

    if (parser->current_token.type != TOKEN_OPTION) {
        printf("❌ Expected OPTION token, got %s\n", token_type_name(parser->current_token.type));
        parser_destroy(parser);
        return 1;
    }
    printf("✅ Found OPTION token (start of pattern)\n\n");

    printf("✅ All steps completed successfully!\n");
    printf("The if-let token sequence is being parsed correctly.\n");

    // Cleanup
    parser_destroy(parser);

    return 0;
}
