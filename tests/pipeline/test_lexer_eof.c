#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *source = "package test;\n\npub fn main(none) -> int {\n    return 0;\n}";

    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) {
        printf("Failed to create lexer\n");
        return 1;
    }

    Token token;
    do {
        token = lexer_next_token(lexer);
        printf("Token: type=%d, line=%d, col=%d\n", token.type, token.location.line,
               token.location.column);
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);

    // Try to get another token after EOF
    token = lexer_next_token(lexer);
    printf("After EOF: type=%d, line=%d, col=%d\n", token.type, token.location.line,
           token.location.column);

    lexer_destroy(lexer);
    return 0;
}
