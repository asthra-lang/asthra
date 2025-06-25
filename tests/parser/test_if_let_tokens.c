/**
 * Simple If-Let Token Test
 *
 * Tests if the lexer can tokenize if-let statements correctly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include only lexer
#include "lexer.h"
#include "parser_token.h"

int main(void) {
    printf("=== If-Let Token Test ===\n\n");

    const char *source = "if let Option.Some(x) = opt { return x; }";

    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) {
        printf("❌ Failed to create lexer\n");
        return 1;
    }

    printf("Tokenizing: %s\n\n", source);

    Token token;
    int token_count = 0;

    do {
        token = lexer_next_token(lexer);
        printf("Token %d: %s", token_count++, token_type_name(token.type));

        if (token.type == TOKEN_IDENTIFIER) {
            printf(" (%s)", token.data.identifier.name);
        } else if (token.type == TOKEN_STRING) {
            printf(" (\"%s\")", token.data.string.value);
        }

        printf("\n");

    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    lexer_destroy(lexer);

    if (token.type == TOKEN_ERROR) {
        printf("❌ Lexer error encountered\n");
        return 1;
    }

    printf("\n✅ Tokenization completed successfully\n");
    return 0;
}
