/**
 * Test file for Struct Pattern Bindings
 * Tests struct field bindings and ignored fields in match expressions
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

// Test struct pattern with explicit bindings
static bool test_struct_pattern_with_bindings(void) {
    printf("Testing struct pattern with explicit bindings... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match person {\n"
                         "        Person { name: person_name, age: person_age } => 0\n"
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
    TEST_ASSERT(strcmp(struct_pattern->data.struct_pattern.struct_name, "Person") == 0,
                "Expected 'Person' struct name");

    // Check field patterns with explicit bindings
    TEST_ASSERT(ast_node_list_size(struct_pattern->data.struct_pattern.field_patterns) == 2,
                "Expected 2 field patterns");

    // Check name: person_name binding
    ASTNode *field1 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 0);
    TEST_ASSERT(strcmp(field1->data.field_pattern.field_name, "name") == 0,
                "Expected 'name' field name");
    TEST_ASSERT(field1->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'name'");
    TEST_ASSERT(strcmp(field1->data.field_pattern.binding_name, "person_name") == 0,
                "Expected 'person_name' binding");
    TEST_ASSERT(!field1->data.field_pattern.is_ignored, "Field 'name' should not be ignored");

    // Check age: person_age binding
    ASTNode *field2 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 1);
    TEST_ASSERT(strcmp(field2->data.field_pattern.field_name, "age") == 0,
                "Expected 'age' field name");
    TEST_ASSERT(field2->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'age'");
    TEST_ASSERT(strcmp(field2->data.field_pattern.binding_name, "person_age") == 0,
                "Expected 'person_age' binding");
    TEST_ASSERT(!field2->data.field_pattern.is_ignored, "Field 'age' should not be ignored");

    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test struct pattern with ignored fields
static bool test_struct_pattern_with_ignored_fields(void) {
    printf("Testing struct pattern with ignored fields... ");

    const char *source = "package test;\n"
                         "\n"
                         "priv fn test_match(none) -> i32 {\n"
                         "    match data {\n"
                         "        DataStruct { important: important, useless: _ } => 0\n"
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

    // Check important field (explicit binding)
    ASTNode *field1 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 0);
    TEST_ASSERT(strcmp(field1->data.field_pattern.field_name, "important") == 0,
                "Expected 'important' field name");
    TEST_ASSERT(field1->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'important'");
    TEST_ASSERT(strcmp(field1->data.field_pattern.binding_name, "important") == 0,
                "Expected 'important' binding");
    TEST_ASSERT(!field1->data.field_pattern.is_ignored, "Field 'important' should not be ignored");

    // Check useless: _ (field with ignored binding)
    ASTNode *field2 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 1);
    TEST_ASSERT(strcmp(field2->data.field_pattern.field_name, "useless") == 0,
                "Expected 'useless' field name");
    TEST_ASSERT(field2->data.field_pattern.binding_name == NULL,
                "Expected no binding for ignored field");
    TEST_ASSERT(field2->data.field_pattern.is_ignored, "Field 'useless' should be ignored");

    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test mixed binding patterns
static bool test_mixed_binding_patterns(void) {
    printf("Testing mixed binding patterns... ");

    const char *source =
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> i32 {\n"
        "    match config {\n"
        "        Config { name: name, value: val, debug: _, timestamp: timestamp } => 0\n"
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

    TEST_ASSERT(ast_node_list_size(struct_pattern->data.struct_pattern.field_patterns) == 4,
                "Expected 4 field patterns");

    // Check name (explicit binding)
    ASTNode *field1 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 0);
    TEST_ASSERT(strcmp(field1->data.field_pattern.field_name, "name") == 0,
                "Expected 'name' field name");
    TEST_ASSERT(field1->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'name'");
    TEST_ASSERT(strcmp(field1->data.field_pattern.binding_name, "name") == 0,
                "Expected 'name' binding");
    TEST_ASSERT(!field1->data.field_pattern.is_ignored, "Field 'name' should not be ignored");

    // Check value: val (explicit binding)
    ASTNode *field2 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 1);
    TEST_ASSERT(strcmp(field2->data.field_pattern.field_name, "value") == 0,
                "Expected 'value' field name");
    TEST_ASSERT(field2->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'value'");
    TEST_ASSERT(strcmp(field2->data.field_pattern.binding_name, "val") == 0,
                "Expected 'val' binding");
    TEST_ASSERT(!field2->data.field_pattern.is_ignored, "Field 'value' should not be ignored");

    // Check debug: _ (ignored)
    ASTNode *field3 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 2);
    TEST_ASSERT(strcmp(field3->data.field_pattern.field_name, "debug") == 0,
                "Expected 'debug' field name");
    TEST_ASSERT(field3->data.field_pattern.binding_name == NULL,
                "Expected no binding for ignored field");
    TEST_ASSERT(field3->data.field_pattern.is_ignored, "Field 'debug' should be ignored");

    // Check timestamp (explicit binding)
    ASTNode *field4 = ast_node_list_get(struct_pattern->data.struct_pattern.field_patterns, 3);
    TEST_ASSERT(strcmp(field4->data.field_pattern.field_name, "timestamp") == 0,
                "Expected 'timestamp' field name");
    TEST_ASSERT(field4->data.field_pattern.binding_name != NULL,
                "Expected explicit binding for 'timestamp'");
    TEST_ASSERT(strcmp(field4->data.field_pattern.binding_name, "timestamp") == 0,
                "Expected 'timestamp' binding");
    TEST_ASSERT(!field4->data.field_pattern.is_ignored, "Field 'timestamp' should not be ignored");

    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Main test runner
int main(void) {
    printf("Running struct pattern binding tests...\n\n");

    bool all_passed = true;

    all_passed &= test_struct_pattern_with_bindings();
    all_passed &= test_struct_pattern_with_ignored_fields();
    all_passed &= test_mixed_binding_patterns();

    printf("\n");
    if (all_passed) {
        printf("🎉 All struct pattern binding tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
}
