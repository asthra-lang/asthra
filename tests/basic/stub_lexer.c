/**
 * Lexer stub implementations
 * Minimal implementations to satisfy the linker for testing
 */

#include <stdio.h>

// Lexer state stubs
void lexer_push_state(void* lexer, int state) {
    (void)lexer;
    (void)state;
}

void lexer_pop_state(void* lexer) {
    (void)lexer;
}

int lexer_get_state(void* lexer) {
    (void)lexer;
    return 0;
}

// Lexer scanning stubs
int scan_identifier(void* lexer) {
    (void)lexer;
    return 0;
}

int scan_number(void* lexer) {
    (void)lexer;
    return 0;
}

int scan_string(void* lexer) {
    (void)lexer;
    return 0;
}

int scan_char(void* lexer) {
    (void)lexer;
    return 0;
}

int scan_token(void* lexer) {
    (void)lexer;
    return 0;
}

// Error handling
void lexer_error(void* lexer, const char* msg) {
    (void)lexer;
    printf("Lexer error: %s\n", msg);
}

// Token stubs
const char* get_token_string(void* token) {
    (void)token;
    return "token";
}

int get_token_type(void* token) {
    (void)token;
    return 0;
}