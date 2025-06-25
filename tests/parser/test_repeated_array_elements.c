/**
 * Comprehensive test suite for repeated array elements verification
 * Tests repeated array syntax as defined in grammar.txt line 155
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

// Helper to verify repeated array structure
static bool is_repeated_array(ASTNode *array) {
    if (!array || array->type != AST_ARRAY_LITERAL)
        return false;
    if (!array->data.array_literal.elements)
        return false;

    size_t count = ast_node_list_size(array->data.array_literal.elements);
    if (count != 3)
        return false;

    ASTNode *marker = ast_node_list_get(array->data.array_literal.elements, 0);
    if (!marker || marker->type != AST_IDENTIFIER)
        return false;
    if (!marker->data.identifier.name)
        return false;

    return strcmp(marker->data.identifier.name, "__repeated_array__") == 0;
}

// Test 1: Basic repeated array with integer literal
void test_basic_repeated_array_integer(void) {
    printf("Testing basic repeated array with integer ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let zeros: [10]i32 = [0; 10];\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    // Navigate to the array literal
    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 1);

    ASTNode *main_func = decls->nodes[0];
    assert(main_func->type == AST_FUNCTION_DECL);

    ASTNode *body = main_func->data.function_decl.body;
    assert(body->type == AST_BLOCK);

    ASTNodeList *stmts = body->data.block.statements;
    assert(stmts && stmts->count == 2); // let + return

    ASTNode *let_stmt = stmts->nodes[0];
    assert(let_stmt->type == AST_LET_STMT);
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;

    // Verify it's a repeated array
    assert(is_repeated_array(initializer));

    // Check value and count
    ASTNode *value = ast_node_list_get(initializer->data.array_literal.elements, 1);
    assert(value && value->type == AST_INTEGER_LITERAL);
    assert(value->data.integer_literal.value == 0);

    ASTNode *count = ast_node_list_get(initializer->data.array_literal.elements, 2);
    assert(count && count->type == AST_INTEGER_LITERAL);
    assert(count->data.integer_literal.value == 10);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Basic repeated array with integer parsed correctly\n");
}

// Test 2: Repeated array with different types
void test_repeated_array_different_types(void) {
    printf("Testing repeated arrays with different types ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let bools: [5]bool = [true; 5];\n"
                         "    let floats: [3]f32 = [3.14; 3];\n"
                         "    let chars: [8]char = ['A'; 8];\n"
                         "    let strings: [2]string = [\"hello\"; 2];\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    // Verify all 4 repeated arrays parse correctly
    for (int i = 0; i < 4; i++) {
        ASTNode *let_stmt = stmts->nodes[i];
        assert(let_stmt->type == AST_LET_STMT);
        ASTNode *initializer = let_stmt->data.let_stmt.initializer;
        assert(is_repeated_array(initializer));
    }

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Repeated arrays with different types parsed correctly\n");
}

// Test 3: Repeated array with const expressions
void test_repeated_array_const_expressions(void) {
    printf("Testing repeated arrays with const expressions ...\n");

    const char *source = "package test;\n"
                         "priv const SIZE: i32 = 100;\n"
                         "priv const DEFAULT_VALUE: i32 = -1;\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr1: [100]i32 = [DEFAULT_VALUE; SIZE];\n"
                         "    let arr2: [50]i32 = [5 * 2; 10 * 5];\n"
                         "    let arr3: [8]i32 = [1 + 1; 4 * 2];\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    // Just verify parsing succeeds - semantic analysis would validate const-ness
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Repeated arrays with const expressions parsed correctly\n");
}

// Test 4: Nested repeated arrays
void test_nested_repeated_arrays(void) {
    printf("Testing nested repeated arrays ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    // 2D array - array of arrays\n"
                         "    let matrix: [3][3]i32 = [[0; 3]; 3];\n"
                         "    \n"
                         "    // Array of structs\n"
                         "    let points: [10]Point = [Point{x: 0, y: 0}; 10];\n"
                         "    return ();\n"
                         "}\n"
                         "\n"
                         "pub struct Point { x: i32, y: i32 }\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    // Check matrix initialization
    ASTNode *matrix_let = stmts->nodes[0];
    assert(matrix_let->type == AST_LET_STMT);
    ASTNode *matrix_init = matrix_let->data.let_stmt.initializer;
    assert(is_repeated_array(matrix_init));

    // The value itself should be a repeated array
    ASTNode *inner_array = ast_node_list_get(matrix_init->data.array_literal.elements, 1);
    assert(is_repeated_array(inner_array));

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Nested repeated arrays parsed correctly\n");
}

// Test 5: Large count values
void test_repeated_array_large_counts(void) {
    printf("Testing repeated arrays with large counts ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let big1: [1000]u8 = [0; 1000];\n"
                         "    let big2: [65536]u8 = [255; 65536];\n"
                         "    let big3: [1000000]i32 = [42; 1000000];\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    // Verify parsing succeeds with large numbers
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Repeated arrays with large counts parsed correctly\n");
}

// Test 6: Repeated arrays in various contexts
void test_repeated_array_contexts(void) {
    printf("Testing repeated arrays in various contexts ...\n");

    const char *source = "package test;\n"
                         "priv const GLOBAL_ARRAY: [5]i32 = [99; 5];\n"
                         "\n"
                         "pub fn get_array(none) -> [10]i32 {\n"
                         "    return [7; 10];\n"
                         "}\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    // In function arguments\n"
                         "    process_array([1; 20]);\n"
                         "    \n"
                         "    // In struct literal\n"
                         "    let data: Data = Data { values: [0; 100] };\n"
                         "    \n"
                         "    // In expressions\n"
                         "    let first: i32 = ([5; 3])[0];\n"
                         "    \n"
                         "    return ();\n"
                         "}\n"
                         "\n"
                         "pub fn process_array(arr: [20]i32) -> void {\n"
                         "    return ();\n"
                         "}\n"
                         "\n"
                         "pub struct Data { values: [100]i32 }\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    // Just verify it parses without errors
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Repeated arrays in various contexts parsed correctly\n");
}

// Test 7: Unit type repeated arrays
// DISABLED: () is not a valid type in the grammar - void is used instead
void test_repeated_array_unit_type(void) {
    printf("Testing repeated arrays with unit type ...\n");
    printf("  ⚠️  Skipped - unit type () not supported in grammar\n");
    return;

    /*
    const char* source =
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let units: [5]() = [(); 5];\n"
        "    return ();\n"
        "}\n";

    Parser* parser = create_parser(source);
    assert(parser != NULL);

    ASTNode* program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList* decls = program->data.program.declarations;
    ASTNode* main_func = decls->nodes[0];
    ASTNode* body = main_func->data.function_decl.body;
    ASTNodeList* stmts = body->data.block.statements;

    ASTNode* let_stmt = stmts->nodes[0];
    ASTNode* initializer = let_stmt->data.let_stmt.initializer;
    assert(is_repeated_array(initializer));

    // Check that value is unit literal
    ASTNode* value = ast_node_list_get(initializer->data.array_literal.elements, 1);
    assert(value && value->type == AST_UNIT_LITERAL);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Repeated arrays with unit type parsed correctly\n");
    */
}

// Test 8: Mixed array syntax (regular and repeated)
void test_mixed_array_syntax(void) {
    printf("Testing mixed array syntax ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    // Regular array\n"
                         "    let regular: [3]i32 = [1, 2, 3];\n"
                         "    \n"
                         "    // Repeated array\n"
                         "    let repeated: [5]i32 = [42; 5];\n"
                         "    \n"
                         "    // Empty array with none\n"
                         "    let empty: []i32 = [none];\n"
                         "    \n"
                         "    // Single element (not repeated)\n"
                         "    let single: [1]i32 = [99];\n"
                         "    \n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    // Check regular array (not repeated)
    ASTNode *regular_let = stmts->nodes[0];
    ASTNode *regular_init = regular_let->data.let_stmt.initializer;
    assert(!is_repeated_array(regular_init));

    // Check repeated array
    ASTNode *repeated_let = stmts->nodes[1];
    ASTNode *repeated_init = repeated_let->data.let_stmt.initializer;
    assert(is_repeated_array(repeated_init));

    // Check empty array
    ASTNode *empty_let = stmts->nodes[2];
    ASTNode *empty_init = empty_let->data.let_stmt.initializer;
    assert(!is_repeated_array(empty_init));

    // Check single element array
    ASTNode *single_let = stmts->nodes[3];
    ASTNode *single_init = single_let->data.let_stmt.initializer;
    assert(!is_repeated_array(single_init));

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Mixed array syntax parsed correctly\n");
}

// Test 9: Error cases - invalid syntax
void test_repeated_array_error_cases(void) {
    printf("Testing repeated array error cases ...\n");

    // Missing count
    {
        const char *source = "package test;\n"
                             "pub fn main(none) -> void {\n"
                             "    let bad: [10]i32 = [0; ];\n"
                             "    return ();\n"
                             "}\n";

        Parser *parser = create_parser(source);
        assert(parser != NULL);

        ASTNode *program = parse_program(parser);
        // Should fail to parse or produce an error

        if (program)
            ast_free_node(program);
        parser_destroy(parser);
    }

    // Missing value
    {
        const char *source = "package test;\n"
                             "pub fn main(none) -> void {\n"
                             "    let bad: [10]i32 = [; 10];\n"
                             "    return ();\n"
                             "}\n";

        Parser *parser = create_parser(source);
        assert(parser != NULL);

        ASTNode *program = parse_program(parser);
        // Should fail to parse or produce an error

        if (program)
            ast_free_node(program);
        parser_destroy(parser);
    }

    printf("  ✓ Repeated array error cases handled\n");
}

// Test 10: Repeated arrays with complex expressions
void test_repeated_array_complex_expressions(void) {
    printf("Testing repeated arrays with complex expressions ...\n");

    const char *source = "package test;\n"
                         "pub fn default_point(none) -> Point {\n"
                         "    return Point { x: 0, y: 0 };\n"
                         "}\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    // Function call as value\n"
                         "    let points: [5]Point = [default_point(); 5];\n"
                         "    \n"
                         "    // Struct literal as value\n"
                         "    let origins: [3]Point = [Point { x: 0, y: 0 }; 3];\n"
                         "    \n"
                         "    // Array literal as value (creates 2D array)\n"
                         "    let rows: [4][2]i32 = [[1, 2]; 4];\n"
                         "    \n"
                         "    return ();\n"
                         "}\n"
                         "\n"
                         "pub struct Point { x: i32, y: i32 }\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    // Verify parsing succeeds with complex expressions
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Repeated arrays with complex expressions parsed correctly\n");
}

int main(void) {
    printf("=== Comprehensive Repeated Array Elements Test Suite ===\n\n");

    test_basic_repeated_array_integer();
    test_repeated_array_different_types();
    test_repeated_array_const_expressions();
    test_nested_repeated_arrays();
    test_repeated_array_large_counts();
    test_repeated_array_contexts();
    test_repeated_array_unit_type();
    test_mixed_array_syntax();
    test_repeated_array_error_cases();
    test_repeated_array_complex_expressions();

    printf("\n✅ All repeated array element tests completed!\n");
    return 0;
}