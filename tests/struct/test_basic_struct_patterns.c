/**
 * Test file for Basic Struct Pattern Matching
 * Tests basic struct destructuring in match expressions
 */

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
static Parser *create_parser_from_source(const char *source) {
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
static void cleanup_parser(Parser *parser) {
    if (parser) {
        Lexer *lexer = parser->lexer;
        parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
    }
}

// Test basic struct pattern parsing
static bool test_basic_struct_pattern(void) {
    printf("Testing basic struct pattern parsing... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point { x: x, y: y } => 0\n"
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program != NULL, "Failed to parse program");
    TEST_ASSERT(program->type == AST_PROGRAM, "Expected AST_PROGRAM");

    // Get the function declaration
    TEST_ASSERT(program->data.program.declarations != NULL, "Program declarations are NULL");
    TEST_ASSERT(ast_node_list_size(program->data.program.declarations) == 1,
                "Expected 1 declaration");

    ASTNode *func_decl = ast_node_list_get(program->data.program.declarations, 0);
    TEST_ASSERT(func_decl != NULL, "Function declaration is NULL");
    TEST_ASSERT(func_decl->type == AST_FUNCTION_DECL, "Expected AST_FUNCTION_DECL");

    // Get the function body
    ASTNode *body = func_decl->data.function_decl.body;
    TEST_ASSERT(body != NULL, "Function body is NULL");
    TEST_ASSERT(body->type == AST_BLOCK, "Expected AST_BLOCK");

    // Get the match statement
    TEST_ASSERT(body->data.block.statements != NULL, "Block statements are NULL");
    TEST_ASSERT(ast_node_list_size(body->data.block.statements) == 1,
                "Expected 1 statement (match)");

    ASTNode *match_stmt = ast_node_list_get(body->data.block.statements, 0);
    TEST_ASSERT(match_stmt != NULL, "Match statement is NULL");
    TEST_ASSERT(match_stmt->type == AST_MATCH_STMT, "Expected AST_MATCH_STMT");

    // Check expression
    TEST_ASSERT(match_stmt->data.match_stmt.expression != NULL, "Match expression is NULL");
    TEST_ASSERT(match_stmt->data.match_stmt.expression->type == AST_IDENTIFIER,
                "Expected identifier expression");
    TEST_ASSERT(strcmp(match_stmt->data.match_stmt.expression->data.identifier.name, "point") == 0,
                "Expected 'point' identifier");

    // Check arms
    TEST_ASSERT(match_stmt->data.match_stmt.arms != NULL, "Match arms are NULL");
    TEST_ASSERT(ast_node_list_size(match_stmt->data.match_stmt.arms) == 1, "Expected 1 match arm");

    // Check struct pattern arm
    ASTNode *arm = ast_node_list_get(match_stmt->data.match_stmt.arms, 0);
    TEST_ASSERT(arm != NULL, "Arm is NULL");
    TEST_ASSERT(arm->type == AST_MATCH_ARM, "Expected AST_MATCH_ARM");
    TEST_ASSERT(arm->data.match_arm.pattern != NULL, "Arm pattern is NULL");
    TEST_ASSERT(arm->data.match_arm.pattern->type == AST_STRUCT_PATTERN,
                "Expected AST_STRUCT_PATTERN");

    ASTNode *struct_pattern = arm->data.match_arm.pattern;
    TEST_ASSERT(strcmp(struct_pattern->data.struct_pattern.struct_name, "Point") == 0,
                "Expected 'Point' struct name");
    TEST_ASSERT(struct_pattern->data.struct_pattern.field_patterns != NULL,
                "Field patterns are NULL");
    TEST_ASSERT(ast_node_list_size(struct_pattern->data.struct_pattern.field_patterns) == 2,
                "Expected 2 field patterns");

    // Check first field pattern (x)
    ASTNode *field1 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 0);
    TEST_ASSERT(field1 != NULL, "First field pattern is NULL");
    TEST_ASSERT(field1->type == AST_FIELD_PATTERN, "Expected AST_FIELD_PATTERN");
    TEST_ASSERT(strcmp(field1->data.field_pattern.field_name, "x") == 0, "Expected 'x' field name");
    TEST_ASSERT(field1->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'x'");
    TEST_ASSERT(strcmp(field1->data.field_pattern.binding_name, "x") == 0, "Expected 'x' binding");
    TEST_ASSERT(!field1->data.field_pattern.is_ignored, "Field 'x' should not be ignored");

    // Check second field pattern (y)
    ASTNode *field2 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 1);
    TEST_ASSERT(field2 != NULL, "Second field pattern is NULL");
    TEST_ASSERT(field2->type == AST_FIELD_PATTERN, "Expected AST_FIELD_PATTERN");
    TEST_ASSERT(strcmp(field2->data.field_pattern.field_name, "y") == 0, "Expected 'y' field name");
    TEST_ASSERT(field2->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'y'");
    TEST_ASSERT(strcmp(field2->data.field_pattern.binding_name, "y") == 0, "Expected 'y' binding");
    TEST_ASSERT(!field2->data.field_pattern.is_ignored, "Field 'y' should not be ignored");

    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test empty struct pattern
static bool test_empty_struct_pattern(void) {
    printf("Testing empty struct pattern... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match unit {\n"
                         "        Unit {} => 0\n"
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program != NULL, "Failed to parse program");

    // Navigate to match statement
    ASTNode *func_decl = ast_node_list_get(program->data.program.declarations, 0);
    ASTNode *body = func_decl->data.function_decl.body;
    ASTNode *match_stmt = ast_node_list_get(body->data.block.statements, 0);

    ASTNode *arm = ast_node_list_get(match_stmt->data.match_stmt.arms, 0);
    ASTNode *struct_pattern = arm->data.match_arm.pattern;
    TEST_ASSERT(struct_pattern->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(struct_pattern->data.struct_pattern.struct_name, "Unit") == 0,
                "Expected 'Unit' struct name");

    // Check that field patterns is empty or NULL
    if (struct_pattern->data.struct_pattern.field_patterns != NULL) {
        TEST_ASSERT(ast_node_list_size(struct_pattern->data.struct_pattern.field_patterns) == 0,
                    "Expected 0 field patterns");
    }

    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test struct pattern with trailing comma
static bool test_struct_pattern_trailing_comma(void) {
    printf("Testing struct pattern with trailing comma... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match point {\n"
                         "        Point { x: x, y: y } => 0\n"
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Failed to create parser");

    ASTNode *program = parser_parse_program(parser);
    TEST_ASSERT(program != NULL, "Failed to parse program");

    // Navigate to match statement
    ASTNode *func_decl = ast_node_list_get(program->data.program.declarations, 0);
    ASTNode *body = func_decl->data.function_decl.body;
    ASTNode *match_stmt = ast_node_list_get(body->data.block.statements, 0);

    ASTNode *arm = ast_node_list_get(match_stmt->data.match_stmt.arms, 0);
    ASTNode *struct_pattern = arm->data.match_arm.pattern;
    TEST_ASSERT(struct_pattern->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(ast_node_list_size(struct_pattern->data.struct_pattern.field_patterns) == 2,
                "Expected 2 field patterns");

    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Main test runner
int main(void) {
    printf("Running basic struct pattern tests...\n\n");

    bool all_passed = true;

    all_passed &= test_basic_struct_pattern();
    all_passed &= test_empty_struct_pattern();
    all_passed &= test_struct_pattern_trailing_comma();

    printf("\n");
    if (all_passed) {
        printf("🎉 All basic struct pattern tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
}
