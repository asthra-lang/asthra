/**
 * Parser stub implementations
 * Minimal implementations to satisfy the linker for testing
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations to avoid header conflicts
struct ASTNode;
typedef struct ASTNode ASTNode;

// Parser core stubs
bool parse_program(void *parser, void *program) {
    (void)parser;
    (void)program;
    return true;
}

bool parse_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

bool parse_type(void *parser, void *type) {
    (void)parser;
    (void)type;
    return true;
}

bool parse_expr(void *parser, void *expr) {
    (void)parser;
    (void)expr;
    return true;
}

bool parse_annotation_list(void *parser, void *list) {
    (void)parser;
    (void)list;
    return true;
}

bool parse_method_decl(void *parser, void *method) {
    (void)parser;
    (void)method;
    return true;
}

void report_error(void *parser, const char *msg) {
    (void)parser;
    printf("Error: %s\n", msg);
}

// Grammar parsing stubs
bool parse_function_body(void *parser, void *body) {
    (void)parser;
    (void)body;
    return true;
}

bool parse_struct_body(void *parser, void *body) {
    (void)parser;
    (void)body;
    return true;
}

bool parse_enum_body(void *parser, void *body) {
    (void)parser;
    (void)body;
    return true;
}

bool parse_import_item(void *parser, void *item) {
    (void)parser;
    (void)item;
    return true;
}

bool parse_const_value(void *parser, void *value) {
    (void)parser;
    (void)value;
    return true;
}

bool parse_visibility(void *parser, void *vis) {
    (void)parser;
    (void)vis;
    return true;
}

bool parse_parameter_list(void *parser, void *params) {
    (void)parser;
    (void)params;
    return true;
}

// Expression parsing stubs
bool parse_binary_expression(void *parser, void *expr, int prec) {
    (void)parser;
    (void)expr;
    (void)prec;
    return true;
}

bool parse_unary_expression(void *parser, void *expr) {
    (void)parser;
    (void)expr;
    return true;
}

bool parse_primary_expression(void *parser, void *expr) {
    (void)parser;
    (void)expr;
    return true;
}

bool parse_call_expression(void *parser, void *expr) {
    (void)parser;
    (void)expr;
    return true;
}

bool parse_field_access(void *parser, void *expr) {
    (void)parser;
    (void)expr;
    return true;
}

bool parse_index_expression(void *parser, void *expr) {
    (void)parser;
    (void)expr;
    return true;
}

// Statement parsing stubs
bool parse_let_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

bool parse_if_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

bool parse_while_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

bool parse_for_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

bool parse_match_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

bool parse_return_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

bool parse_block_statement(void *parser, void *stmt) {
    (void)parser;
    (void)stmt;
    return true;
}

// Top-level parsing stubs
bool parse_function_declaration(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_struct_declaration(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_enum_declaration(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_const_declaration(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_import_declaration(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

// Additional parser stubs
bool parse_const_decl(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_enum_decl(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_extern_decl(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_function_decl(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_struct_decl(void *parser, void *decl) {
    (void)parser;
    (void)decl;
    return true;
}

bool parse_param(void *parser, void *param) {
    (void)parser;
    (void)param;
    return true;
}

bool parse_pattern(void *parser, void *pattern) {
    (void)parser;
    (void)pattern;
    return true;
}

// Parser error handling
void parser_error(void *parser, const char *msg) {
    (void)parser;
    printf("Parser error: %s\n", msg);
}

// AST utilities
void ast_free_node(ASTNode *node) {
    free(node);
}

ASTNode *create_identifier(const char *name) {
    (void)name;
    return (ASTNode *)calloc(1, 1); // Minimal stub
}

void *ast_node_copy(void *node) {
    (void)node;
    return malloc(1);
}

void ast_node_destroy(void *node) {
    free(node);
}

bool ast_node_equals(void *node1, void *node2) {
    return node1 == node2;
}