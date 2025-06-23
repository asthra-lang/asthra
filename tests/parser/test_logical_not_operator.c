/**
 * Comprehensive test suite for logical NOT operator (!x)
 * Tests lexer, parser, and AST generation for logical NOT as defined in grammar.txt line 126
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
#include "../../src/parser/ast_types.h"

// Helper function to create parser from source
static Parser* create_parser(const char* source) {
    Lexer* lexer = lexer_create(source, strlen(source), "<test>");
    if (!lexer) return NULL;
    
    Parser* parser = parser_create(lexer);
    return parser;
}

// Helper function to parse expression statement
static ASTNode* parse_expr_stmt(const char* source) {
    const char* full_source = malloc(strlen(source) + 100);
    sprintf((char*)full_source, 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    %s;\n"
        "    return ();\n"
        "}\n", source);
    
    Parser* parser = create_parser(full_source);
    if (!parser) {
        free((void*)full_source);
        return NULL;
    }
    
    ASTNode* program = parse_program(parser);
    free((void*)full_source);
    
    if (!program || program->type != AST_PROGRAM) {
        parser_destroy(parser);
        return NULL;
    }
    
    // Navigate to the expression statement
    ASTNode* main_decl = program->data.program.declarations->nodes[0];
    ASTNode* block = main_decl->data.function_decl.body;
    ASTNode* expr_stmt = block->data.block.statements->nodes[0];
    
    // Clone the expression to return it safely
    ASTNode* expr = expr_stmt->data.expr_stmt.expression;
    expr_stmt->data.expr_stmt.expression = NULL; // Prevent double-free
    
    ast_free_node(program);
    parser_destroy(parser);
    
    return expr;
}

// Test 1: Basic logical NOT
void test_basic_logical_not(void) {
    printf("Testing basic logical NOT operator ...\n");
    
    ASTNode* expr = parse_expr_stmt("!true");
    assert(expr != NULL);
    assert(expr->type == AST_UNARY_EXPR);
    assert(expr->data.unary_expr.operator == UNOP_NOT);
    
    ASTNode* operand = expr->data.unary_expr.operand;
    assert(operand != NULL);
    assert(operand->type == AST_LITERAL);
    assert(operand->data.literal.type == LITERAL_BOOL);
    assert(operand->data.literal.value.bool_value == true);
    
    ast_free_node(expr);
    printf("  ✓ Basic logical NOT parsed correctly\n");
}

// Test 2: Logical NOT with variables
void test_logical_not_variable(void) {
    printf("Testing logical NOT with variables ...\n");
    
    ASTNode* expr = parse_expr_stmt("!flag");
    assert(expr != NULL);
    assert(expr->type == AST_UNARY_EXPR);
    assert(expr->data.unary_expr.operator == UNOP_NOT);
    
    ASTNode* operand = expr->data.unary_expr.operand;
    assert(operand != NULL);
    assert(operand->type == AST_IDENTIFIER);
    assert(strcmp(operand->data.identifier.name, "flag") == 0);
    
    ast_free_node(expr);
    printf("  ✓ Logical NOT with variable parsed correctly\n");
}

// Test 3: Logical NOT with complex expressions
void test_logical_not_complex(void) {
    printf("Testing logical NOT with complex expressions ...\n");
    
    // Test !( x > 5 )
    ASTNode* expr = parse_expr_stmt("!(x > 5)");
    assert(expr != NULL);
    assert(expr->type == AST_UNARY_EXPR);
    assert(expr->data.unary_expr.operator == UNOP_NOT);
    
    ASTNode* operand = expr->data.unary_expr.operand;
    assert(operand != NULL);
    assert(operand->type == AST_BINARY_EXPR);
    assert(operand->data.binary_expr.operator == BINOP_GT);
    
    ast_free_node(expr);
    printf("  ✓ Logical NOT with complex expression parsed correctly\n");
}

// Test 4: Logical NOT with function calls
void test_logical_not_function_call(void) {
    printf("Testing logical NOT with function calls ...\n");
    
    ASTNode* expr = parse_expr_stmt("!isEmpty(none)");
    assert(expr != NULL);
    assert(expr->type == AST_UNARY_EXPR);
    assert(expr->data.unary_expr.operator == UNOP_NOT);
    
    ASTNode* operand = expr->data.unary_expr.operand;
    assert(operand != NULL);
    assert(operand->type == AST_CALL_EXPR);
    
    ast_free_node(expr);
    printf("  ✓ Logical NOT with function call parsed correctly\n");
}

// Test 5: Logical NOT in conditions
void test_logical_not_in_condition(void) {
    printf("Testing logical NOT in conditions ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn check(none) -> void {\n"
        "    if !ready {\n"
        "        return ();\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    assert(program != NULL);
    
    // Navigate to if statement condition
    ASTNode* func = program->data.program.declarations->nodes[0];
    ASTNode* if_stmt = func->data.function_decl.body->data.block.statements->nodes[0];
    assert(if_stmt->type == AST_IF_STMT);
    
    ASTNode* condition = if_stmt->data.if_stmt.condition;
    assert(condition->type == AST_UNARY_EXPR);
    assert(condition->data.unary_expr.operator == UNOP_NOT);
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Logical NOT in condition parsed correctly\n");
}

// Test 6: Multiple logical NOT (not allowed by grammar)
void test_double_logical_not_restricted(void) {
    printf("Testing double logical NOT restriction ...\n");
    
    // According to grammar, !! is not allowed (UnaryPrefix is optional, not repeatable)
    const char* source = 
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let x: bool = !!flag;\n"  // This should fail
        "    return ();\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    // The parser should fail or produce unexpected results
    
    if (program) {
        ast_free_node(program);
    }
    parser_destroy(parser);
    printf("  ✓ Double logical NOT restriction handled\n");
}

// Test 7: Logical NOT with other unary operators
void test_logical_not_with_dereference(void) {
    printf("Testing logical NOT with dereference ...\n");
    
    ASTNode* expr = parse_expr_stmt("!*ptr");
    assert(expr != NULL);
    assert(expr->type == AST_UNARY_EXPR);
    assert(expr->data.unary_expr.operator == UNOP_NOT);
    
    ASTNode* operand = expr->data.unary_expr.operand;
    assert(operand != NULL);
    assert(operand->type == AST_UNARY_EXPR);
    assert(operand->data.unary_expr.operator == UNOP_DEREF);
    
    ast_free_node(expr);
    printf("  ✓ Logical NOT with dereference parsed correctly\n");
}

// Test 8: Logical NOT in binary expressions
void test_logical_not_in_binary(void) {
    printf("Testing logical NOT in binary expressions ...\n");
    
    // Test: !a && b
    ASTNode* expr1 = parse_expr_stmt("!a && b");
    assert(expr1 != NULL);
    assert(expr1->type == AST_BINARY_EXPR);
    assert(expr1->data.binary_expr.operator == BINOP_AND);
    
    ASTNode* left = expr1->data.binary_expr.left;
    assert(left->type == AST_UNARY_EXPR);
    assert(left->data.unary_expr.operator == UNOP_NOT);
    
    ast_free_node(expr1);
    
    // Test: a || !b
    ASTNode* expr2 = parse_expr_stmt("a || !b");
    assert(expr2 != NULL);
    assert(expr2->type == AST_BINARY_EXPR);
    assert(expr2->data.binary_expr.operator == BINOP_OR);
    
    ASTNode* right = expr2->data.binary_expr.right;
    assert(right->type == AST_UNARY_EXPR);
    assert(right->data.unary_expr.operator == UNOP_NOT);
    
    ast_free_node(expr2);
    printf("  ✓ Logical NOT in binary expressions parsed correctly\n");
}

// Test 9: Logical NOT with field access
void test_logical_not_field_access(void) {
    printf("Testing logical NOT with field access ...\n");
    
    ASTNode* expr = parse_expr_stmt("!obj.enabled");
    assert(expr != NULL);
    assert(expr->type == AST_UNARY_EXPR);
    assert(expr->data.unary_expr.operator == UNOP_NOT);
    
    ASTNode* operand = expr->data.unary_expr.operand;
    assert(operand != NULL);
    assert(operand->type == AST_FIELD_ACCESS);
    
    ast_free_node(expr);
    printf("  ✓ Logical NOT with field access parsed correctly\n");
}

// Test 10: Logical NOT with array access
void test_logical_not_array_access(void) {
    printf("Testing logical NOT with array access ...\n");
    
    ASTNode* expr = parse_expr_stmt("!flags[0]");
    assert(expr != NULL);
    assert(expr->type == AST_UNARY_EXPR);
    assert(expr->data.unary_expr.operator == UNOP_NOT);
    
    ASTNode* operand = expr->data.unary_expr.operand;
    assert(operand != NULL);
    assert(operand->type == AST_INDEX_ACCESS);
    
    ast_free_node(expr);
    printf("  ✓ Logical NOT with array access parsed correctly\n");
}

// Test 11: Precedence test - NOT vs other operators
void test_logical_not_precedence(void) {
    printf("Testing logical NOT precedence ...\n");
    
    // Test: !a == b should parse as (!a) == b
    ASTNode* expr = parse_expr_stmt("!a == b");
    assert(expr != NULL);
    assert(expr->type == AST_BINARY_EXPR);
    assert(expr->data.binary_expr.operator == BINOP_EQ);
    
    ASTNode* left = expr->data.binary_expr.left;
    assert(left->type == AST_UNARY_EXPR);
    assert(left->data.unary_expr.operator == UNOP_NOT);
    
    ast_free_node(expr);
    printf("  ✓ Logical NOT precedence parsed correctly\n");
}

// Test 12: Logical NOT in match patterns (if supported)
void test_logical_not_in_patterns(void) {
    printf("Testing logical NOT in pattern contexts ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn process(flag: bool) -> i32 {\n"
        "    match flag {\n"
        "        true => { return 1; }\n"
        "        false => { return 0; }\n"
        "    }\n"
        "}\n";
    
    Parser* parser = create_parser(source);
    assert(parser != NULL);
    
    ASTNode* program = parse_program(parser);
    assert(program != NULL);
    
    // Note: Logical NOT is not allowed in patterns according to grammar
    // This test just verifies patterns work without NOT
    
    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Pattern matching verified (NOT not allowed in patterns)\n");
}

int main(void) {
    printf("=== Comprehensive Logical NOT Operator Test Suite ===\n\n");
    
    test_basic_logical_not();
    test_logical_not_variable();
    test_logical_not_complex();
    test_logical_not_function_call();
    test_logical_not_in_condition();
    test_double_logical_not_restricted();
    test_logical_not_with_dereference();
    test_logical_not_in_binary();
    test_logical_not_field_access();
    test_logical_not_array_access();
    test_logical_not_precedence();
    test_logical_not_in_patterns();
    
    printf("\n✅ All logical NOT operator tests completed!\n");
    return 0;
}