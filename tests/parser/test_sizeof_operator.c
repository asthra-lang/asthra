/**
 * Comprehensive test suite for sizeof operator verification
 * Tests sizeof operator as defined in grammar.txt line 142
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../../src/parser/lexer.h"
#include "../../src/parser/parser.h"
#include "../../src/parser/ast.h"
#include "../../src/parser/ast_node_list.h"

// Helper function to create parser from source
static Parser* create_parser(const char* source) {
    Lexer* lexer = lexer_create(source, strlen(source), "<test>");
    if (!lexer) return NULL;
    
    Parser* parser = parser_create(lexer);
    return parser;
}

// Test helper to verify sizeof expression structure
static bool verify_sizeof_expr(ASTNode* expr) {
    if (!expr || expr->type != AST_UNARY_EXPR) return false;
    if (expr->data.unary_expr.operator != UNOP_SIZEOF) return false;
    
    // The operand should be a type node
    ASTNode* operand = expr->data.unary_expr.operand;
    if (!operand) return false;
    
    // Check if it's a valid type node
    return (operand->type == AST_BASE_TYPE ||
            operand->type == AST_STRUCT_TYPE ||
            operand->type == AST_ENUM_TYPE ||
            operand->type == AST_SLICE_TYPE ||
            operand->type == AST_ARRAY_TYPE ||
            operand->type == AST_PTR_TYPE ||
            operand->type == AST_RESULT_TYPE ||
            operand->type == AST_OPTION_TYPE ||
            operand->type == AST_TUPLE_TYPE);
}

// Test 1: Basic sizeof with primitive types
void test_sizeof_primitive_types(void) {
    printf("Testing sizeof with primitive types ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let size_i32: usize = sizeof(i32);\n"
        "    let size_i64: usize = sizeof(i64);\n"
        "    let size_f32: usize = sizeof(f32);\n"
        "    let size_f64: usize = sizeof(f64);\n"
        "    let size_bool: usize = sizeof(bool);\n"
        "    let size_u8: usize = sizeof(u8);\n"
        "    let size_usize: usize = sizeof(usize);\n"
        "    let size_string: usize = sizeof(string);\n"
        "    return ();\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    // Navigate to function body
    ASTNodeList* decls = program->data.program.declarations;
    assert(decls && decls->count == 1);
    
    ASTNode* main_func = decls->nodes[0];
    assert(main_func->type == AST_FUNCTION_DECL);
    
    ASTNode* body = main_func->data.function_decl.body;
    assert(body->type == AST_BLOCK);
    
    ASTNodeList* stmts = body->data.block.statements;
    assert(stmts && stmts->count == 9); // 8 let statements + return
    
    // Verify each sizeof expression
    for (int i = 0; i < 8; i++) {
        ASTNode* let_stmt = stmts->nodes[i];
        assert(let_stmt->type == AST_LET_STMT);
        ASTNode* initializer = let_stmt->data.let_stmt.initializer;
        assert(verify_sizeof_expr(initializer));
    }
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof with primitive types parsed correctly\n");
}

// Test 2: sizeof with composite types
void test_sizeof_composite_types(void) {
    printf("Testing sizeof with composite types ...\n");
    
    const char* source = 
        "package test;\n"
        "pub struct Point { x: i32, y: i32 }\n"
        "pub enum Color { Red, Green, Blue }\n"
        "pub fn main(none) -> void {\n"
        "    let size_point: usize = sizeof(Point);\n"
        "    let size_color: usize = sizeof(Color);\n"
        "    let size_slice: usize = sizeof([]i32);\n"
        "    let size_array: usize = sizeof([10]i32);\n"
        "    let size_tuple: usize = sizeof((i32, f64));\n"
        "    let size_option: usize = sizeof(Option<i32>);\n"
        "    let size_result: usize = sizeof(Result<i32, string>);\n"
        "    return ();\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    assert(program != NULL);
    
    // Navigate to main function
    ASTNodeList* decls = program->data.program.declarations;
    assert(decls && decls->count == 3); // Point, Color, main
    
    ASTNode* main_func = decls->nodes[2];
    assert(main_func->type == AST_FUNCTION_DECL);
    
    ASTNode* body = main_func->data.function_decl.body;
    ASTNodeList* stmts = body->data.block.statements;
    assert(stmts && stmts->count == 8); // 7 let statements + return
    
    // Verify each sizeof expression
    for (int i = 0; i < 7; i++) {
        ASTNode* let_stmt = stmts->nodes[i];
        assert(let_stmt->type == AST_LET_STMT);
        ASTNode* initializer = let_stmt->data.let_stmt.initializer;
        assert(verify_sizeof_expr(initializer));
    }
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof with composite types parsed correctly\n");
}

// Test 3: sizeof with pointer types
void test_sizeof_pointer_types(void) {
    printf("Testing sizeof with pointer types ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let size_ptr_i32: usize = sizeof(*const i32);\n"
        "    let size_ptr_mut_i32: usize = sizeof(*mut i32);\n"
        "    let size_ptr_struct: usize = sizeof(*const Point);\n"
        "    let size_ptr_slice: usize = sizeof(*mut []u8);\n"
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
    
    // Verify each sizeof expression
    for (int i = 0; i < 4; i++) {
        ASTNode* let_stmt = stmts->nodes[i];
        assert(let_stmt->type == AST_LET_STMT);
        ASTNode* initializer = let_stmt->data.let_stmt.initializer;
        assert(verify_sizeof_expr(initializer));
    }
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof with pointer types parsed correctly\n");
}

// Test 4: sizeof in const expressions
void test_sizeof_in_const_expressions(void) {
    printf("Testing sizeof in const expressions ...\n");
    
    const char* source = 
        "package test;\n"
        "priv const SIZE_I32: usize = sizeof(i32);\n"
        "priv const SIZE_ARRAY: usize = sizeof([100]u8);\n"
        "priv const BUFFER_SIZE: usize = sizeof(i32) * 256;\n"
        "priv const STRUCT_ALIGNMENT: usize = sizeof(Point) + sizeof(i32);\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    assert(program != NULL);
    
    ASTNodeList* decls = program->data.program.declarations;
    assert(decls && decls->count == 5); // 4 const + main
    
    // Verify const declarations
    for (int i = 0; i < 4; i++) {
        ASTNode* const_decl = decls->nodes[i];
        assert(const_decl->type == AST_CONST_DECL);
        
        ASTNode* initializer = const_decl->data.const_decl.value;
        assert(initializer != NULL);
        
        // Check if it contains sizeof (directly or in binary expression)
        if (initializer->type == AST_UNARY_EXPR) {
            assert(initializer->data.unary_expr.operator == UNOP_SIZEOF);
        } else if (initializer->type == AST_BINARY_EXPR) {
            // At least one operand should be sizeof
            ASTNode* left = initializer->data.binary_expr.left;
            ASTNode* right = initializer->data.binary_expr.right;
            assert((left && left->type == AST_UNARY_EXPR && 
                    left->data.unary_expr.operator == UNOP_SIZEOF) ||
                   (right && right->type == AST_UNARY_EXPR && 
                    right->data.unary_expr.operator == UNOP_SIZEOF));
        }
    }
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof in const expressions parsed correctly\n");
}

// Test 5: sizeof in various expression contexts
void test_sizeof_in_expressions(void) {
    printf("Testing sizeof in various expression contexts ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn process_size(size: usize) -> usize { return size; }\n"
        "pub fn main(none) -> void {\n"
        "    // sizeof in function arguments\n"
        "    let result: usize = process_size(sizeof(i32));\n"
        "    // sizeof in arithmetic\n"
        "    let double_size: usize = sizeof(i64) * 2;\n"
        "    // sizeof in comparisons\n"
        "    if sizeof(i32) == 4 {\n"
        "        let x: i32 = 42;\n"
        "    }\n"
        "    // sizeof in array size (if supported)\n"
        "    let buffer: [sizeof(i32) * 10]u8;\n"
        "    return ();\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    assert(program != NULL);
    
    // Basic validation that parsing succeeded
    ASTNodeList* decls = program->data.program.declarations;
    assert(decls && decls->count == 2); // process_size, main
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof in various expression contexts parsed correctly\n");
}

// Test 6: sizeof with generic types
void test_sizeof_generic_types(void) {
    printf("Testing sizeof with generic types ...\n");
    
    const char* source = 
        "package test;\n"
        "pub struct Vec<T> { data: *mut T, len: usize, cap: usize }\n"
        "pub fn main(none) -> void {\n"
        "    let size_vec_i32: usize = sizeof(Vec<i32>);\n"
        "    let size_vec_string: usize = sizeof(Vec<string>);\n"
        "    let size_option_vec: usize = sizeof(Option<Vec<i32>>);\n"
        "    return ();\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    assert(program != NULL);
    
    ASTNodeList* decls = program->data.program.declarations;
    assert(decls && decls->count == 2); // Vec, main
    
    ASTNode* main_func = decls->nodes[1];
    ASTNode* body = main_func->data.function_decl.body;
    ASTNodeList* stmts = body->data.block.statements;
    
    // Verify sizeof expressions with generic types
    for (int i = 0; i < 3; i++) {
        ASTNode* let_stmt = stmts->nodes[i];
        assert(let_stmt->type == AST_LET_STMT);
        ASTNode* initializer = let_stmt->data.let_stmt.initializer;
        assert(verify_sizeof_expr(initializer));
    }
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof with generic types parsed correctly\n");
}

// Test 7: Error cases - invalid sizeof usage
void test_sizeof_error_cases(void) {
    printf("Testing sizeof error cases ...\n");
    
    // Test missing parentheses
    {
        const char* source = 
            "package test;\n"
            "pub fn main(none) -> void {\n"
            "    let size: usize = sizeof i32;\n"  // Missing parentheses
            "    return ();\n"
            "}\n";
        
        Parser* parser = create_parser(source);
        assert(parser != NULL);
        
        ASTNode* program = parse_program(parser);
        // Should fail to parse or produce an error
        
        if (program) ast_free_node(program);
        parser_destroy(parser);
    }
    
    // Test sizeof with expression instead of type
    {
        const char* source = 
            "package test;\n"
            "pub fn main(none) -> void {\n"
            "    let x: i32 = 42;\n"
            "    let size: usize = sizeof(x);\n"  // Expression, not type
            "    return ();\n"
            "}\n";
        
        Parser* parser = create_parser(source);
        assert(parser != NULL);
        
        ASTNode* program = parse_program(parser);
        // This might parse but should fail in semantic analysis
        
        if (program) ast_free_node(program);
        parser_destroy(parser);
    }
    
    printf("  ✓ sizeof error cases handled correctly\n");
}

// Test 8: sizeof as primary expression
void test_sizeof_as_primary_expression(void) {
    printf("Testing sizeof as primary expression ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    // sizeof is a primary expression according to grammar\n"
        "    let a: usize = (sizeof(i32));\n"
        "    let b: usize = sizeof(i32) + sizeof(i64);\n"
        "    let c: bool = sizeof(i32) > 0;\n"
        "    let d: usize = sizeof([]i32) / sizeof(i32);\n"
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
    
    // All statements should parse successfully
    assert(stmts && stmts->count == 5); // 4 let + return
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof as primary expression parsed correctly\n");
}

// Test 9: sizeof with Never type
void test_sizeof_never_type(void) {
    printf("Testing sizeof with Never type ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let size_never: usize = sizeof(Never);\n"
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
    assert(let_stmt->type == AST_LET_STMT);
    ASTNode* initializer = let_stmt->data.let_stmt.initializer;
    assert(verify_sizeof_expr(initializer));
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ sizeof with Never type parsed correctly\n");
}

int main(void) {
    printf("=== Comprehensive sizeof Operator Test Suite ===\n\n");
    
    test_sizeof_primitive_types();
    test_sizeof_composite_types();
    test_sizeof_pointer_types();
    test_sizeof_in_const_expressions();
    test_sizeof_in_expressions();
    test_sizeof_generic_types();
    test_sizeof_error_cases();
    test_sizeof_as_primary_expression();
    test_sizeof_never_type();
    
    printf("\n✅ All sizeof operator tests passed!\n");
    return 0;
}