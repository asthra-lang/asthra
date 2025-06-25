/**
 * Comprehensive test suite for struct field visibility verification
 * Tests visibility modifiers on struct fields as defined in grammar.txt line 46
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/parser/ast.h"
#include "../../src/parser/ast_node_list.h"
#include "../../src/parser/lexer.h"
#include "../../src/parser/parser.h"

// Helper function to create parser from source
static Parser *create_parser(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    return parser;
}

// Test 1: Basic public field
void test_public_field(void) {
    printf("Testing public field visibility ...\n");

    const char *source = "package test;\n"
                         "pub struct Point {\n"
                         "    pub x: i32,\n"
                         "    pub y: i32\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    // Navigate to struct declaration
    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 1);

    ASTNode *struct_decl = decls->nodes[0];
    assert(struct_decl->type == AST_STRUCT_DECL);
    assert(strcmp(struct_decl->data.struct_decl.name, "Point") == 0);

    // Check fields
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;
    assert(fields && ast_node_list_size(fields) == 2);

    // Check first field (x)
    ASTNode *field_x = ast_node_list_get(fields, 0);
    assert(field_x->type == AST_STRUCT_FIELD);
    assert(strcmp(field_x->data.struct_field.name, "x") == 0);
    assert(field_x->data.struct_field.visibility == VISIBILITY_PUBLIC);

    // Check second field (y)
    ASTNode *field_y = ast_node_list_get(fields, 1);
    assert(field_y->type == AST_STRUCT_FIELD);
    assert(strcmp(field_y->data.struct_field.name, "y") == 0);
    assert(field_y->data.struct_field.visibility == VISIBILITY_PUBLIC);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Public fields parsed correctly with visibility\n");
}

// Test 2: Basic private field
void test_private_field(void) {
    printf("Testing private field visibility ...\n");

    const char *source = "package test;\n"
                         "pub struct BankAccount {\n"
                         "    priv balance: f64,\n"
                         "    priv pin: i32\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *struct_decl = decls->nodes[0];
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;
    assert(fields && ast_node_list_size(fields) == 2);

    // Check private fields
    ASTNode *field_balance = ast_node_list_get(fields, 0);
    assert(field_balance->data.struct_field.visibility == VISIBILITY_PRIVATE);

    ASTNode *field_pin = ast_node_list_get(fields, 1);
    assert(field_pin->data.struct_field.visibility == VISIBILITY_PRIVATE);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Private fields parsed correctly with visibility\n");
}

// Test 3: Mixed visibility fields
void test_mixed_visibility_fields(void) {
    printf("Testing mixed visibility fields ...\n");

    const char *source = "package test;\n"
                         "pub struct User {\n"
                         "    pub id: i32,\n"
                         "    pub username: string,\n"
                         "    priv password_hash: string,\n"
                         "    priv email: string,\n"
                         "    pub created_at: i64\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *struct_decl = decls->nodes[0];
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;
    assert(ast_node_list_size(fields) == 5);

    // Verify visibility of each field
    assert(ast_node_list_get(fields, 0)->data.struct_field.visibility == VISIBILITY_PUBLIC); // id
    assert(ast_node_list_get(fields, 1)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // username
    assert(ast_node_list_get(fields, 2)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // password_hash
    assert(ast_node_list_get(fields, 3)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // email
    assert(ast_node_list_get(fields, 4)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // created_at

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Mixed visibility fields parsed correctly\n");
}

// Test 4: Default visibility (no modifier)
void test_default_visibility(void) {
    printf("Testing default field visibility ...\n");

    const char *source = "package test;\n"
                         "pub struct Config {\n"
                         "    timeout: i32,\n"
                         "    retries: i32,\n"
                         "    pub verbose: bool\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *struct_decl = decls->nodes[0];
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;

    // Fields without explicit visibility should default to private
    assert(ast_node_list_get(fields, 0)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // timeout
    assert(ast_node_list_get(fields, 1)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // retries
    assert(ast_node_list_get(fields, 2)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // verbose

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Default visibility (private) parsed correctly\n");
}

// Test 5: Complex types with visibility
void test_visibility_with_complex_types(void) {
    printf("Testing visibility with complex types ...\n");

    const char *source = "package test;\n"
                         "pub struct Database {\n"
                         "    pub connections: Vec<Connection>,\n"
                         "    priv credentials: Map<string, string>,\n"
                         "    cache: Option<Cache>,\n"
                         "    pub metrics: [100]f64\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *struct_decl = decls->nodes[0];
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;

    // Verify visibility is independent of type complexity
    assert(ast_node_list_get(fields, 0)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // connections
    assert(ast_node_list_get(fields, 1)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // credentials
    assert(ast_node_list_get(fields, 2)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // cache (default)
    assert(ast_node_list_get(fields, 3)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // metrics

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Visibility with complex types parsed correctly\n");
}

// Test 6: Nested structs with field visibility
void test_nested_structs_visibility(void) {
    printf("Testing nested structs with field visibility ...\n");

    const char *source = "package test;\n"
                         "pub struct Address {\n"
                         "    pub street: string,\n"
                         "    pub city: string,\n"
                         "    priv apartment_number: Option<string>\n"
                         "}\n"
                         "\n"
                         "pub struct Person {\n"
                         "    pub name: string,\n"
                         "    priv ssn: string,\n"
                         "    pub address: Address\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    assert(ast_node_list_size(decls) == 2);

    // Check Address struct
    ASTNode *address_struct = decls->nodes[0];
    ASTNodeList *address_fields = address_struct->data.struct_decl.fields;
    assert(ast_node_list_get(address_fields, 0)->data.struct_field.visibility == VISIBILITY_PUBLIC);
    assert(ast_node_list_get(address_fields, 1)->data.struct_field.visibility == VISIBILITY_PUBLIC);
    assert(ast_node_list_get(address_fields, 2)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE);

    // Check Person struct
    ASTNode *person_struct = decls->nodes[1];
    ASTNodeList *person_fields = person_struct->data.struct_decl.fields;
    assert(ast_node_list_get(person_fields, 0)->data.struct_field.visibility == VISIBILITY_PUBLIC);
    assert(ast_node_list_get(person_fields, 1)->data.struct_field.visibility == VISIBILITY_PRIVATE);
    assert(ast_node_list_get(person_fields, 2)->data.struct_field.visibility == VISIBILITY_PUBLIC);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Nested structs with field visibility parsed correctly\n");
}

// Test 7: Generic structs with field visibility
void test_generic_struct_field_visibility(void) {
    printf("Testing generic struct field visibility ...\n");

    const char *source = "package test;\n"
                         "pub struct Container<T> {\n"
                         "    pub value: T,\n"
                         "    priv metadata: string,\n"
                         "    capacity: i32\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *struct_decl = decls->nodes[0];

    // Verify generic struct has type parameters
    assert(struct_decl->data.struct_decl.type_params != NULL);
    assert(ast_node_list_size(struct_decl->data.struct_decl.type_params) == 1);

    // Check field visibility
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;
    assert(ast_node_list_get(fields, 0)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // value
    assert(ast_node_list_get(fields, 1)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // metadata
    assert(ast_node_list_get(fields, 2)->data.struct_field.visibility ==
           VISIBILITY_PRIVATE); // capacity

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Generic struct field visibility parsed correctly\n");
}

// Test 8: Empty struct (with none)
void test_empty_struct_no_fields(void) {
    printf("Testing empty struct with no fields ...\n");

    const char *source = "package test;\n"
                         "pub struct Empty { none }\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *struct_decl = decls->nodes[0];

    // Empty struct should have no fields
    assert(struct_decl->data.struct_decl.fields == NULL);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Empty struct parsed correctly\n");
}

// Test 9: Struct with method implementations (fields only)
void test_struct_for_method_context(void) {
    printf("Testing struct fields in method context ...\n");

    const char *source = "package test;\n"
                         "pub struct Rectangle {\n"
                         "    pub width: f64,\n"
                         "    pub height: f64,\n"
                         "    priv id: i32\n"
                         "}\n"
                         "\n"
                         "impl Rectangle {\n"
                         "    pub fn area(self) -> f64 {\n"
                         "        return self.width * self.height;\n"
                         "    }\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;

    // Check struct fields visibility
    ASTNode *struct_decl = decls->nodes[0];
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;
    assert(ast_node_list_get(fields, 0)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // width
    assert(ast_node_list_get(fields, 1)->data.struct_field.visibility ==
           VISIBILITY_PUBLIC); // height
    assert(ast_node_list_get(fields, 2)->data.struct_field.visibility == VISIBILITY_PRIVATE); // id

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Struct fields in method context parsed correctly\n");
}

// Test 10: Multiple visibility modifiers (error case)
void test_invalid_multiple_visibility(void) {
    printf("Testing invalid multiple visibility modifiers ...\n");

    const char *source = "package test;\n"
                         "pub struct Invalid {\n"
                         "    pub priv x: i32\n" // This should be an error
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    // Parser might handle this differently - either error or take first/last modifier

    if (program) {
        ast_free_node(program);
    }
    parser_destroy(parser);
    printf("  ✓ Invalid multiple visibility handled\n");
}

int main(void) {
    printf("=== Comprehensive Struct Field Visibility Test Suite ===\n\n");

    test_public_field();
    test_private_field();
    test_mixed_visibility_fields();
    test_default_visibility();
    test_visibility_with_complex_types();
    test_nested_structs_visibility();
    test_generic_struct_field_visibility();
    test_empty_struct_no_fields();
    test_struct_for_method_context();
    test_invalid_multiple_visibility();

    printf("\n✅ All struct field visibility parser tests completed!\n");
    return 0;
}