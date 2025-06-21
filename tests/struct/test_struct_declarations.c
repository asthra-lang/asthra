/**
 * Test struct declaration parsing
 * Migrated to standardized testing framework
 */

#include "../framework/test_framework.h"
#include "../framework/test_assertions.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AsthraTestResult test_basic_struct_declaration(AsthraTestContext *context) {
    const char *source = 
        "package test;\n"
        "priv struct Point {\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n";
    
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Lexer should be created");
    
    Parser *parser = parser_create(lexer);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Parser should be created");
    
    ASTNode *program = parser_parse_program(parser);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program, "Program should be parsed");
    asthra_test_assert_int_eq(context, (int)program->type, (int)AST_PROGRAM, "Should be program node");
    
    // Check that we have declarations
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program->data.program.declarations, "Declarations should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(program->data.program.declarations), 1, "Should have one declaration");
    
    // Get the struct declaration
    ASTNode *struct_decl = ast_node_list_get(program->data.program.declarations, 0);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl, "Struct declaration should exist");
    asthra_test_assert_int_eq(context, (int)struct_decl->type, (int)AST_STRUCT_DECL, "Should be struct declaration");
    
    // Check struct name
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl->data.struct_decl.name, "Struct name should exist");
    ASTHRA_TEST_ASSERT_STR_EQ(context, struct_decl->data.struct_decl.name, "Point", "Struct name should be Point");
    
    // Check fields
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl->data.struct_decl.fields, "Fields should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(struct_decl->data.struct_decl.fields), 2, "Should have two fields");
    
    // Check first field (x: f64)
    ASTNode *field1 = ast_node_list_get(struct_decl->data.struct_decl.fields, 0);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field1, "First field should exist");
    asthra_test_assert_int_eq(context, (int)field1->type, (int)AST_STRUCT_FIELD, "First field should be struct field");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field1->data.struct_field.name, "x", "First field name should be x");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field1->data.struct_field.type, "First field type should exist");
    asthra_test_assert_int_eq(context, (int)field1->data.struct_field.type->type, (int)AST_BASE_TYPE, "First field should be base type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field1->data.struct_field.type->data.base_type.name, "f64", "First field type should be f64");
    
    // Check second field (y: f64)
    ASTNode *field2 = ast_node_list_get(struct_decl->data.struct_decl.fields, 1);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field2, "Second field should exist");
    asthra_test_assert_int_eq(context, (int)field2->type, (int)AST_STRUCT_FIELD, "Second field should be struct field");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field2->data.struct_field.name, "y", "Second field name should be y");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field2->data.struct_field.type, "Second field type should exist");
    asthra_test_assert_int_eq(context, (int)field2->data.struct_field.type->type, (int)AST_BASE_TYPE, "Second field should be base type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field2->data.struct_field.type->data.base_type.name, "f64", "Second field type should be f64");
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_empty_struct_declaration(AsthraTestContext *context) {
    const char *source = 
        "package test;\n"
        "priv struct Empty {\n"
        "    none\n"
        "}\n";
    
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Lexer should be created");
    
    Parser *parser = parser_create(lexer);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Parser should be created");
    
    ASTNode *program = parser_parse_program(parser);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program, "Program should be parsed");
    asthra_test_assert_int_eq(context, (int)program->type, (int)AST_PROGRAM, "Should be program node");
    
    // Check that we have declarations
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program->data.program.declarations, "Declarations should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(program->data.program.declarations), 1, "Should have one declaration");
    
    // Get the struct declaration
    ASTNode *struct_decl = ast_node_list_get(program->data.program.declarations, 0);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl, "Struct declaration should exist");
    asthra_test_assert_int_eq(context, (int)struct_decl->type, (int)AST_STRUCT_DECL, "Should be struct declaration");
    
    // Check struct name
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl->data.struct_decl.name, "Struct name should exist");
    ASTHRA_TEST_ASSERT_STR_EQ(context, struct_decl->data.struct_decl.name, "Empty", "Struct name should be Empty");
    
    // Check that fields is empty
    if (struct_decl->data.struct_decl.fields != NULL) {
        asthra_test_assert_int_eq(context, ast_node_list_size(struct_decl->data.struct_decl.fields), 0, "Empty struct should have no fields");
    }
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_struct_with_complex_types(AsthraTestContext *context) {
    const char *source = 
        "package test;\n"
        "priv struct Complex {\n"
        "    id: i32,\n"
        "    name: string,\n"
        "    data: []u8,\n"
        "    ptr: *mut i32\n"
        "}\n";
    
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Lexer should be created");
    
    Parser *parser = parser_create(lexer);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Parser should be created");
    
    ASTNode *program = parser_parse_program(parser);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program, "Program should be parsed");
    asthra_test_assert_int_eq(context, (int)program->type, (int)AST_PROGRAM, "Should be program node");
    
    // Check that we have declarations
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program->data.program.declarations, "Declarations should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(program->data.program.declarations), 1, "Should have one declaration");
    
    // Get the struct declaration
    ASTNode *struct_decl = ast_node_list_get(program->data.program.declarations, 0);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl, "Struct declaration should exist");
    asthra_test_assert_int_eq(context, (int)struct_decl->type, (int)AST_STRUCT_DECL, "Should be struct declaration");
    
    // Check struct name
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl->data.struct_decl.name, "Struct name should exist");
    ASTHRA_TEST_ASSERT_STR_EQ(context, struct_decl->data.struct_decl.name, "Complex", "Struct name should be Complex");
    
    // Check fields
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_decl->data.struct_decl.fields, "Fields should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(struct_decl->data.struct_decl.fields), 4, "Should have four fields");
    
    // Check id: i32
    ASTNode *field1 = ast_node_list_get(struct_decl->data.struct_decl.fields, 0);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field1, "First field should exist");
    asthra_test_assert_int_eq(context, (int)field1->type, (int)AST_STRUCT_FIELD, "First field should be struct field");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field1->data.struct_field.name, "id", "First field name should be id");
    asthra_test_assert_int_eq(context, (int)field1->data.struct_field.type->type, (int)AST_BASE_TYPE, "First field should be base type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field1->data.struct_field.type->data.base_type.name, "i32", "First field type should be i32");
    
    // Check name: string
    ASTNode *field2 = ast_node_list_get(struct_decl->data.struct_decl.fields, 1);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field2, "Second field should exist");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field2->data.struct_field.name, "name", "Second field name should be name");
    asthra_test_assert_int_eq(context, (int)field2->data.struct_field.type->type, (int)AST_BASE_TYPE, "Second field should be base type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field2->data.struct_field.type->data.base_type.name, "string", "Second field type should be string");
    
    // Check data: []u8
    ASTNode *field3 = ast_node_list_get(struct_decl->data.struct_decl.fields, 2);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field3, "Third field should exist");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field3->data.struct_field.name, "data", "Third field name should be data");
    asthra_test_assert_int_eq(context, (int)field3->data.struct_field.type->type, (int)AST_SLICE_TYPE, "Third field should be slice type");
    asthra_test_assert_int_eq(context, (int)field3->data.struct_field.type->data.slice_type.element_type->type, (int)AST_BASE_TYPE, "Slice element should be base type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field3->data.struct_field.type->data.slice_type.element_type->data.base_type.name, "u8", "Slice element should be u8");
    
    // Check ptr: *mut i32
    ASTNode *field4 = ast_node_list_get(struct_decl->data.struct_decl.fields, 3);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field4, "Fourth field should exist");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field4->data.struct_field.name, "ptr", "Fourth field name should be ptr");
    asthra_test_assert_int_eq(context, (int)field4->data.struct_field.type->type, (int)AST_PTR_TYPE, "Fourth field should be pointer type");
    ASTHRA_TEST_ASSERT_TRUE(context, field4->data.struct_field.type->data.ptr_type.is_mutable, "Pointer should be mutable");
    asthra_test_assert_int_eq(context, (int)field4->data.struct_field.type->data.ptr_type.pointee_type->type, (int)AST_BASE_TYPE, "Pointee should be base type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field4->data.struct_field.type->data.ptr_type.pointee_type->data.base_type.name, "i32", "Pointee should be i32");
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_struct_instantiation(AsthraTestContext *context) {
    const char *source = 
        "package test;\n"
        "priv struct Point {\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n"
        "priv fn main(none) -> i32 {\n"
        "    let point: Point = Point { x: 3.14, y: 2.71 };\n"
        "    return 0;\n"
        "}\n";
    
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Lexer should be created");
    
    Parser *parser = parser_create(lexer);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Parser should be created");
    
    ASTNode *program = parser_parse_program(parser);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program, "Program should be parsed");
    asthra_test_assert_int_eq(context, (int)program->type, (int)AST_PROGRAM, "Should be program node");
    
    // Check that we have declarations (struct + function)
    ASTHRA_TEST_ASSERT_NOT_NULL(context, program->data.program.declarations, "Declarations should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(program->data.program.declarations), 2, "Should have two declarations");
    
    // Get the function declaration
    ASTNode *func_decl = ast_node_list_get(program->data.program.declarations, 1);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, func_decl, "Function declaration should exist");
    asthra_test_assert_int_eq(context, (int)func_decl->type, (int)AST_FUNCTION_DECL, "Should be function declaration");
    
    // Check function body
    ASTHRA_TEST_ASSERT_NOT_NULL(context, func_decl->data.function_decl.body, "Function body should exist");
    asthra_test_assert_int_eq(context, (int)func_decl->data.function_decl.body->type, (int)AST_BLOCK, "Body should be block");
    
    // Check statements in function body
    ASTHRA_TEST_ASSERT_NOT_NULL(context, func_decl->data.function_decl.body->data.block.statements, "Statements should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(func_decl->data.function_decl.body->data.block.statements), 2, "Should have two statements");
    
    // Get the let statement
    ASTNode *let_stmt = ast_node_list_get(func_decl->data.function_decl.body->data.block.statements, 0);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, let_stmt, "Let statement should exist");
    asthra_test_assert_int_eq(context, (int)let_stmt->type, (int)AST_LET_STMT, "Should be let statement");
    
    // Check variable initialization
    ASTHRA_TEST_ASSERT_NOT_NULL(context, let_stmt->data.let_stmt.initializer, "Initializer should exist");
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;
    ASTHRA_TEST_ASSERT_NOT_NULL(context, initializer, "Initializer should exist");
    asthra_test_assert_int_eq(context, (int)initializer->type, (int)AST_STRUCT_LITERAL, "Initializer should be struct literal");
    
    // Check struct literal fields
    ASTHRA_TEST_ASSERT_NOT_NULL(context, initializer->data.struct_literal.field_inits, "Struct literal fields should exist");
    asthra_test_assert_int_eq(context, ast_node_list_size(initializer->data.struct_literal.field_inits), 2, "Should have two field assignments");
    
    // Check first field assignment (x: 3.14)
    ASTNode *field1 = ast_node_list_get(initializer->data.struct_literal.field_inits, 0);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field1, "First field assignment should exist");
    asthra_test_assert_int_eq(context, (int)field1->type, (int)AST_ASSIGNMENT, "First field should be assignment");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field1->data.assignment.target, "Assignment target should exist");
    asthra_test_assert_int_eq(context, (int)field1->data.assignment.target->type, (int)AST_IDENTIFIER, "Target should be identifier");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field1->data.assignment.target->data.identifier.name, "x", "Target should be x");
    asthra_test_assert_int_eq(context, (int)field1->data.assignment.value->type, (int)AST_FLOAT_LITERAL, "Value should be float literal");
    
    // Check second field assignment (y: 2.71)
    ASTNode *field2 = ast_node_list_get(initializer->data.struct_literal.field_inits, 1);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field2, "Second field assignment should exist");
    asthra_test_assert_int_eq(context, (int)field2->type, (int)AST_ASSIGNMENT, "Second field should be assignment");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, field2->data.assignment.target, "Assignment target should exist");
    asthra_test_assert_int_eq(context, (int)field2->data.assignment.target->type, (int)AST_IDENTIFIER, "Target should be identifier");
    ASTHRA_TEST_ASSERT_STR_EQ(context, field2->data.assignment.target->data.identifier.name, "y", "Target should be y");
    asthra_test_assert_int_eq(context, (int)field2->data.assignment.value->type, (int)AST_FLOAT_LITERAL, "Value should be float literal");
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return ASTHRA_TEST_PASS;
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Struct Declaration Tests");
    
    asthra_test_suite_add_test(suite, "basic_struct_declaration", 
        "Test basic struct declaration parsing", test_basic_struct_declaration);
    asthra_test_suite_add_test(suite, "empty_struct_declaration", 
        "Test empty struct declaration parsing", test_empty_struct_declaration);
    asthra_test_suite_add_test(suite, "struct_with_complex_types", 
        "Test struct with complex field types", test_struct_with_complex_types);
    asthra_test_suite_add_test(suite, "struct_instantiation", 
        "Test struct instantiation parsing", test_struct_instantiation);
    
    return asthra_test_suite_run_and_exit(suite);
} 
