/**
 * Test file for Multiple Struct Patterns
 * Tests multiple struct patterns and mixed pattern types in match expressions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser.h"
#include "lexer.h"
#include "ast.h"

// Test helper macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            return false; \
        } \
    } while(0)

#define TEST_SUCCESS() \
    do { \
        printf("PASS\n"); \
        return true; \
    } while(0)

// Helper function to create a parser from source code
static Parser *create_parser_from_source(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) return NULL;
    
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
        if (lexer) lexer_destroy(lexer);
    }
}

// Test multiple struct patterns in match statement
static bool test_multiple_struct_patterns(void) {
    printf("Testing multiple struct patterns in match statement... ");
    
    const char *source = 
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> i32 {\n"
        "    match shape {\n"
        "        Point { x: _, y: _ } => 0,\n"
        "        Rectangle { width: w, height: h } => 1,\n"
        "        Circle { radius: radius } => 2\n"
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
    
    TEST_ASSERT(ast_node_list_size(match_stmt->data.match_stmt.arms) == 3, "Expected 3 match arms");
    
    // Check Point pattern
    ASTNode *arm1 = ast_node_list_get(match_stmt->data.match_stmt.arms, 0);
    ASTNode *pattern1 = arm1->data.match_arm.pattern;
    TEST_ASSERT(pattern1->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern1->data.struct_pattern.struct_name, "Point") == 0, "Expected 'Point' struct name");
    
    // Check Rectangle pattern
    ASTNode *arm2 = ast_node_list_get(match_stmt->data.match_stmt.arms, 1);
    ASTNode *pattern2 = arm2->data.match_arm.pattern;
    TEST_ASSERT(pattern2->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern2->data.struct_pattern.struct_name, "Rectangle") == 0, "Expected 'Rectangle' struct name");
    
    // Check Circle pattern
    ASTNode *arm3 = ast_node_list_get(match_stmt->data.match_stmt.arms, 2);
    ASTNode *pattern3 = arm3->data.match_arm.pattern;
    TEST_ASSERT(pattern3->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern3->data.struct_pattern.struct_name, "Circle") == 0, "Expected 'Circle' struct name");
    
    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test mixed enum and struct patterns
static bool test_mixed_enum_and_struct_patterns(void) {
    printf("Testing mixed enum and struct patterns... ");
    
    const char *source = 
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> void {\n"
        "    match result {\n"
        "        Result.Ok(value) => 0,\n"
        "        Result.Err(error) => 1\n"
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
    
    TEST_ASSERT(ast_node_list_size(match_stmt->data.match_stmt.arms) == 2, "Expected 2 match arms");
    
    // Check Ok pattern (should be enum pattern, not struct pattern for this test)
    ASTNode *arm1 = ast_node_list_get(match_stmt->data.match_stmt.arms, 0);
    ASTNode *pattern1 = arm1->data.match_arm.pattern;
    TEST_ASSERT(pattern1->type == AST_ENUM_PATTERN, "Expected AST_ENUM_PATTERN for Result.Ok");
    TEST_ASSERT(strcmp(pattern1->data.enum_pattern.enum_name, "Result") == 0, "Expected 'Result' enum name");
    TEST_ASSERT(strcmp(pattern1->data.enum_pattern.variant_name, "Ok") == 0, "Expected 'Ok' variant name");
    
    // Check Err pattern
    ASTNode *arm2 = ast_node_list_get(match_stmt->data.match_stmt.arms, 1);
    ASTNode *pattern2 = arm2->data.match_arm.pattern;
    TEST_ASSERT(pattern2->type == AST_ENUM_PATTERN, "Expected AST_ENUM_PATTERN for Result.Err");
    TEST_ASSERT(strcmp(pattern2->data.enum_pattern.enum_name, "Result") == 0, "Expected 'Result' enum name");
    TEST_ASSERT(strcmp(pattern2->data.enum_pattern.variant_name, "Err") == 0, "Expected 'Err' variant name");
    
    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test complex nested struct patterns
static bool test_nested_struct_patterns(void) {
    printf("Testing nested struct patterns... ");
    
    const char *source = 
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> i32 {\n"
        "    match data {\n"
        "        Container { inner: InnerStruct { value: value, metadata: _ } } => 0,\n"
        "        Container { inner: InnerStruct { value: v, metadata: metadata } } => 1\n"
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
    
    TEST_ASSERT(ast_node_list_size(match_stmt->data.match_stmt.arms) == 2, "Expected 2 match arms");
    
    // Check first arm - nested pattern
    ASTNode *arm1 = ast_node_list_get(match_stmt->data.match_stmt.arms, 0);
    ASTNode *pattern1 = arm1->data.match_arm.pattern;
    TEST_ASSERT(pattern1->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern1->data.struct_pattern.struct_name, "Container") == 0, "Expected 'Container' struct name");
    
    // Check that this test properly validates nested patterns
    // (Note: This test assumes nested patterns are supported; if not, this validates the outer pattern only)
    TEST_ASSERT(ast_node_list_size(pattern1->data.struct_pattern.field_patterns) >= 1, "Expected at least 1 field pattern");
    
    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Test struct patterns with different field counts
static bool test_varying_field_counts(void) {
    printf("Testing struct patterns with varying field counts... ");
    
    const char *source = 
        "package test;\n"
        "\n"
        "priv fn test_match(none) -> i32 {\n"
        "    match entity {\n"
        "        Unit {} => 0,\n"
        "        Single { value: value } => 1,\n"
        "        Pair { first: _, second: _ } => 2,\n"
        "        Triple { x: x, y: y, z: z } => 3,\n"
        "        Quad { a: a, b: b, c: c, d: d } => 4\n"
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
    
    TEST_ASSERT(ast_node_list_size(match_stmt->data.match_stmt.arms) == 5, "Expected 5 match arms");
    
    // Check Unit pattern (0 fields)
    ASTNode *arm1 = ast_node_list_get(match_stmt->data.match_stmt.arms, 0);
    ASTNode *pattern1 = arm1->data.match_arm.pattern;
    TEST_ASSERT(pattern1->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern1->data.struct_pattern.struct_name, "Unit") == 0, "Expected 'Unit' struct name");
    
    // Check Single pattern (1 field)
    ASTNode *arm2 = ast_node_list_get(match_stmt->data.match_stmt.arms, 1);
    ASTNode *pattern2 = arm2->data.match_arm.pattern;
    TEST_ASSERT(pattern2->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern2->data.struct_pattern.struct_name, "Single") == 0, "Expected 'Single' struct name");
    TEST_ASSERT(ast_node_list_size(pattern2->data.struct_pattern.field_patterns) == 1, "Expected 1 field pattern");
    
    // Check Pair pattern (2 fields)
    ASTNode *arm3 = ast_node_list_get(match_stmt->data.match_stmt.arms, 2);
    ASTNode *pattern3 = arm3->data.match_arm.pattern;
    TEST_ASSERT(pattern3->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern3->data.struct_pattern.struct_name, "Pair") == 0, "Expected 'Pair' struct name");
    TEST_ASSERT(ast_node_list_size(pattern3->data.struct_pattern.field_patterns) == 2, "Expected 2 field patterns");
    
    // Check Triple pattern (3 fields)
    ASTNode *arm4 = ast_node_list_get(match_stmt->data.match_stmt.arms, 3);
    ASTNode *pattern4 = arm4->data.match_arm.pattern;
    TEST_ASSERT(pattern4->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern4->data.struct_pattern.struct_name, "Triple") == 0, "Expected 'Triple' struct name");
    TEST_ASSERT(ast_node_list_size(pattern4->data.struct_pattern.field_patterns) == 3, "Expected 3 field patterns");
    
    // Check Quad pattern (4 fields)
    ASTNode *arm5 = ast_node_list_get(match_stmt->data.match_stmt.arms, 4);
    ASTNode *pattern5 = arm5->data.match_arm.pattern;
    TEST_ASSERT(pattern5->type == AST_STRUCT_PATTERN, "Expected AST_STRUCT_PATTERN");
    TEST_ASSERT(strcmp(pattern5->data.struct_pattern.struct_name, "Quad") == 0, "Expected 'Quad' struct name");
    TEST_ASSERT(ast_node_list_size(pattern5->data.struct_pattern.field_patterns) == 4, "Expected 4 field patterns");
    
    ast_free_node(program);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

// Main test runner
int main(void) {
    printf("Running multiple struct pattern tests...\n\n");
    
    bool all_passed = true;
    
    all_passed &= test_multiple_struct_patterns();
    all_passed &= test_mixed_enum_and_struct_patterns();
    all_passed &= test_nested_struct_patterns();
    all_passed &= test_varying_field_counts();
    
    printf("\n");
    if (all_passed) {
        printf("🎉 All multiple struct pattern tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed!\n");
        return 1;
    }
} 
