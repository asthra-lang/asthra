/**
 * Shared Test Utilities for Struct Pattern Tests
 * Common macros and helper functions for struct pattern testing
 */

#ifndef TEST_STRUCT_PATTERN_UTILS_H
#define TEST_STRUCT_PATTERN_UTILS_H

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test helper macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("FAIL: %s\n", message);                                                         \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST_SUCCESS()                                                                             \
    do {                                                                                           \
        printf("PASS\n");                                                                          \
        return true;                                                                               \
    } while (0)

// Helper function to create a parser from source code
static inline Parser *create_parser_from_source(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    return parser;
}

// Helper function to cleanup parser and lexer
static inline void cleanup_parser(Parser *parser) {
    if (parser) {
        Lexer *lexer = parser->lexer;
        parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
    }
}

// Helper function to navigate to the first match statement in a program
static inline ASTNode *get_first_match_statement(ASTNode *program) {
    if (!program || program->type != AST_PROGRAM)
        return NULL;
    if (!program->data.program.declarations)
        return NULL;
    if (ast_node_list_size(program->data.program.declarations) == 0)
        return NULL;

    ASTNode *func_decl = ast_node_list_get(program->data.program.declarations, 0);
    if (!func_decl || func_decl->type != AST_FUNCTION_DECL)
        return NULL;

    ASTNode *body = func_decl->data.function_decl.body;
    if (!body || body->type != AST_BLOCK)
        return NULL;
    if (!body->data.block.statements)
        return NULL;
    if (ast_node_list_size(body->data.block.statements) == 0)
        return NULL;

    ASTNode *match_stmt = ast_node_list_get(body->data.block.statements, 0);
    if (!match_stmt || match_stmt->type != AST_MATCH_STMT)
        return NULL;

    return match_stmt;
}

// Helper function to get a specific match arm from a match statement
static inline ASTNode *get_match_arm(ASTNode *match_stmt, int index) {
    if (!match_stmt || match_stmt->type != AST_MATCH_STMT)
        return NULL;
    if (!match_stmt->data.match_stmt.arms)
        return NULL;
    if (index >= ast_node_list_size(match_stmt->data.match_stmt.arms))
        return NULL;

    return ast_node_list_get(match_stmt->data.match_stmt.arms, index);
}

// Helper function to get the pattern from a match arm
static inline ASTNode *get_arm_pattern(ASTNode *arm) {
    if (!arm || arm->type != AST_MATCH_ARM)
        return NULL;
    return arm->data.match_arm.pattern;
}

// Helper function to get a specific field pattern from a struct pattern
static inline ASTNode *get_field_pattern(ASTNode *struct_pattern, int index) {
    if (!struct_pattern || struct_pattern->type != AST_STRUCT_PATTERN)
        return NULL;
    if (!struct_pattern->data.struct_pattern.field_patterns)
        return NULL;
    if (index >= ast_node_list_size(struct_pattern->data.struct_pattern.field_patterns))
        return NULL;

    return ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, index);
}

// Helper function to check if field pattern has expected properties
static inline bool check_field_pattern(ASTNode *field_pattern, const char *expected_field_name,
                                       const char *expected_binding_name, bool expected_ignored) {
    if (!field_pattern || field_pattern->type != AST_FIELD_PATTERN)
        return false;

    // Check field name
    if (strcmp(field_pattern->data.field_pattern.field_name, expected_field_name) != 0)
        return false;

    // Check binding name
    if (expected_binding_name) {
        if (!field_pattern->data.field_pattern.binding_name)
            return false;
        if (strcmp(field_pattern->data.field_pattern.binding_name, expected_binding_name) != 0)
            return false;
    } else {
        if (field_pattern->data.field_pattern.binding_name != NULL)
            return false;
    }

    // Check ignored flag
    if (field_pattern->data.field_pattern.is_ignored != expected_ignored)
        return false;

    return true;
}

#endif // TEST_STRUCT_PATTERN_UTILS_H
