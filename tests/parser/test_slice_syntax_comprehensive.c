/**
 * Comprehensive test suite for slice syntax parsing
 * Tests all slice patterns defined in grammar.txt lines 134-137
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

// Test helper to verify slice expression structure
static bool verify_slice_expr(ASTNode *expr, bool has_start, bool has_end) {
    if (!expr || expr->type != AST_SLICE_EXPR)
        return false;

    // Check start index presence
    if (has_start && !expr->data.slice_expr.start)
        return false;
    if (!has_start && expr->data.slice_expr.start)
        return false;

    // Check end index presence
    if (has_end && !expr->data.slice_expr.end)
        return false;
    if (!has_end && expr->data.slice_expr.end)
        return false;

    return true;
}

// Test 1: Full slice pattern [:]
void test_full_slice_pattern(void) {
    printf("Testing full slice pattern [:] ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
                         "    let full_slice: []i32 = arr[:];\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    // Navigate to the slice expression
    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 1);

    ASTNode *main_func = decls->nodes[0];
    assert(main_func->type == AST_FUNCTION_DECL);

    ASTNode *body = main_func->data.function_decl.body;
    assert(body->type == AST_BLOCK);

    ASTNodeList *stmts = body->data.block.statements;
    assert(stmts && stmts->count == 3); // let arr, let full_slice, return

    // Check the slice expression
    ASTNode *slice_let = stmts->nodes[1];
    assert(slice_let->type == AST_LET_STMT);

    ASTNode *initializer = slice_let->data.let_stmt.initializer;
    assert(initializer != NULL);

    // Verify it's a slice with no start and no end
    assert(verify_slice_expr(initializer, false, false));

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Full slice pattern [:] parsed correctly\n");
}

// Test 2: Start-only slice pattern [start:]
void test_start_only_slice_pattern(void) {
    printf("Testing start-only slice pattern [start:] ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
                         "    let from_2: []i32 = arr[2:];\n"
                         "    let from_var: []i32 = arr[start_idx:];\n"
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

    // Check first slice [2:]
    ASTNode *slice1_let = stmts->nodes[1];
    assert(slice1_let->type == AST_LET_STMT);
    ASTNode *slice1 = slice1_let->data.let_stmt.initializer;
    assert(verify_slice_expr(slice1, true, false));

    // Check second slice [start_idx:]
    ASTNode *slice2_let = stmts->nodes[2];
    assert(slice2_let->type == AST_LET_STMT);
    ASTNode *slice2 = slice2_let->data.let_stmt.initializer;
    assert(verify_slice_expr(slice2, true, false));

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Start-only slice pattern [start:] parsed correctly\n");
}

// Test 3: End-only slice pattern [:end]
void test_end_only_slice_pattern(void) {
    printf("Testing end-only slice pattern [:end] ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
                         "    let up_to_3: []i32 = arr[:3];\n"
                         "    let up_to_var: []i32 = arr[:end_idx];\n"
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

    // Check first slice [:3]
    ASTNode *slice1_let = stmts->nodes[1];
    assert(slice1_let->type == AST_LET_STMT);
    ASTNode *slice1 = slice1_let->data.let_stmt.initializer;
    assert(verify_slice_expr(slice1, false, true));

    // Check second slice [:end_idx]
    ASTNode *slice2_let = stmts->nodes[2];
    assert(slice2_let->type == AST_LET_STMT);
    ASTNode *slice2 = slice2_let->data.let_stmt.initializer;
    assert(verify_slice_expr(slice2, false, true));

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ End-only slice pattern [:end] parsed correctly\n");
}

// Test 4: Range slice pattern [start:end]
void test_range_slice_pattern(void) {
    printf("Testing range slice pattern [start:end] ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
                         "    let middle: []i32 = arr[1:4];\n"
                         "    let dynamic: []i32 = arr[start_idx:end_idx];\n"
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

    // Check first slice [1:4]
    ASTNode *slice1_let = stmts->nodes[1];
    assert(slice1_let->type == AST_LET_STMT);
    ASTNode *slice1 = slice1_let->data.let_stmt.initializer;
    assert(verify_slice_expr(slice1, true, true));

    // Check second slice [start_idx:end_idx]
    ASTNode *slice2_let = stmts->nodes[2];
    assert(slice2_let->type == AST_LET_STMT);
    ASTNode *slice2 = slice2_let->data.let_stmt.initializer;
    assert(verify_slice_expr(slice2, true, true));

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Range slice pattern [start:end] parsed correctly\n");
}

// Test 5: Complex expressions in slice bounds
void test_complex_slice_expressions(void) {
    printf("Testing complex expressions in slice bounds ...\n");

    const char *source = "package test;\n"
                         "pub fn calculate_start(none) -> i32 { return 1; }\n"
                         "pub fn calculate_end(none) -> i32 { return 4; }\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
                         "    let slice1: []i32 = arr[calculate_start():];\n"
                         "    let slice2: []i32 = arr[:calculate_end()];\n"
                         "    let slice3: []i32 = arr[calculate_start():calculate_end()];\n"
                         "    let slice4: []i32 = arr[1 + 2:arr.len - 1];\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 3); // calculate_start, calculate_end, main

    ASTNode *main_func = decls->nodes[2];
    assert(main_func->type == AST_FUNCTION_DECL);

    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;
    assert(stmts && stmts->count == 6); // let arr + 4 slices + return

    // All slice expressions should parse correctly
    for (int i = 1; i <= 4; i++) {
        ASTNode *slice_let = stmts->nodes[i];
        assert(slice_let->type == AST_LET_STMT);
        ASTNode *slice = slice_let->data.let_stmt.initializer;
        assert(slice && slice->type == AST_SLICE_EXPR);
    }

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Complex expressions in slice bounds parsed correctly\n");
}

// Test 6: Nested slice operations
void test_nested_slice_operations(void) {
    printf("Testing nested slice operations ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let matrix: [][]i32 = [[1, 2, 3], [4, 5, 6], [7, 8, 9]];\n"
                         "    let row_slice: [][]i32 = matrix[1:];\n"
                         "    let elem_slice: []i32 = matrix[0][:2];\n"
                         "    let complex: []i32 = matrix[1:][0][1:];\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    // Basic parsing should succeed
    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 1);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Nested slice operations parsed correctly\n");
}

// Test 7: Edge cases and error conditions
void test_slice_edge_cases(void) {
    printf("Testing slice edge cases ...\n");

    // Test empty slice bounds are handled
    {
        const char *source = "package test;\n"
                             "pub fn main(none) -> void {\n"
                             "    let arr: []i32 = [none];\n"
                             "    let s: []i32 = arr[:];\n"
                             "    return ();\n"
                             "}\n";

        Parser *parser = create_parser(source);
        assert(parser != NULL);

        ASTNode *program = parse_program(parser);
        assert(program != NULL);

        ast_free_node(program);
        parser_destroy(parser);
    }

    // Test slice on string
    {
        const char *source = "package test;\n"
                             "pub fn main(none) -> void {\n"
                             "    let str: string = \"hello world\";\n"
                             "    let substr: string = str[6:];\n"
                             "    return ();\n"
                             "}\n";

        Parser *parser = create_parser(source);
        assert(parser != NULL);

        ASTNode *program = parse_program(parser);
        assert(program != NULL);

        ast_free_node(program);
        parser_destroy(parser);
    }

    printf("  ✓ Slice edge cases handled correctly\n");
}

// Test 8: Slice syntax vs array access disambiguation
void test_slice_vs_array_access(void) {
    printf("Testing slice vs array access disambiguation ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
                         "    let elem: i32 = arr[2];          // Array access\n"
                         "    let slice1: []i32 = arr[2:];     // Slice from index\n"
                         "    let slice2: []i32 = arr[:2];     // Slice to index\n"
                         "    let slice3: []i32 = arr[1:4];    // Slice range\n"
                         "    let slice4: []i32 = arr[:];      // Full slice\n"
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

    // Check array access
    ASTNode *elem_let = stmts->nodes[1];
    assert(elem_let->type == AST_LET_STMT);
    ASTNode *array_access = elem_let->data.let_stmt.initializer;
    assert(array_access && array_access->type == AST_INDEX_ACCESS);

    // Check all slices
    for (int i = 2; i <= 5; i++) {
        ASTNode *slice_let = stmts->nodes[i];
        assert(slice_let->type == AST_LET_STMT);
        ASTNode *slice = slice_let->data.let_stmt.initializer;
        assert(slice && slice->type == AST_SLICE_EXPR);
    }

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Slice vs array access correctly disambiguated\n");
}

// Test 9: Slice in various contexts
void test_slice_in_contexts(void) {
    printf("Testing slice in various contexts ...\n");

    const char *source = "package test;\n"
                         "pub fn process_slice(data: []i32) -> i32 { return data[0]; }\n"
                         "pub fn main(none) -> void {\n"
                         "    let arr: []i32 = [1, 2, 3, 4, 5];\n"
                         "    // Slice as function argument\n"
                         "    let result1: i32 = process_slice(arr[1:4]);\n"
                         "    // Slice in if condition\n"
                         "    if arr[:3].len > 0 {\n"
                         "        let temp: []i32 = arr[:];\n"
                         "    }\n"
                         "    // Slice in for loop\n"
                         "    for elem in arr[2:] {\n"
                         "        let x: i32 = elem;\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    // Basic structure validation
    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 2); // process_slice, main

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Slice in various contexts parsed correctly\n");
}

int main(void) {
    printf("=== Comprehensive Slice Syntax Test Suite ===\n\n");

    test_full_slice_pattern();
    test_start_only_slice_pattern();
    test_end_only_slice_pattern();
    test_range_slice_pattern();
    test_complex_slice_expressions();
    test_nested_slice_operations();
    test_slice_edge_cases();
    test_slice_vs_array_access();
    test_slice_in_contexts();

    printf("\n✅ All slice syntax tests passed!\n");
    return 0;
}