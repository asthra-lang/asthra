/**
 * Parser stub functions for comprehensive tests
 */

#include <stdbool.h>
#include <stdlib.h>

// Forward declaration
struct ASTNode;
typedef struct ASTNode ASTNode;

// Parser functions
ASTNode *parse_annotation_list(void *parser) {
    (void)parser;
    return NULL;
}

ASTNode *parse_safe_ffi_annotation(void *parser) {
    (void)parser;
    return NULL;
}

ASTNode *parse_const_decl(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_enum_decl(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_extern_decl(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_for_stmt(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_function_decl(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_if_stmt(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_match_stmt(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_spawn_stmt(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_struct_decl(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_impl_block(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_method_decl(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_type(void *parser) {
    (void)parser;
    return malloc(1);
}

ASTNode *parse_unary(void *parser) {
    (void)parser;
    return malloc(1);
}

// Scanner function
int scan_token(void *lexer) {
    (void)lexer;
    return 0;
}

// Error recovery
void *error_recovery_strategy(void *parser) {
    (void)parser;
    return NULL;
}

bool recover_from_error(void *parser) {
    (void)parser;
    return true;
}