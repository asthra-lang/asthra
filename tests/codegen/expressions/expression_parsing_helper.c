/**
 * Asthra Programming Language Compiler
 * Expression Parsing Helper for Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Helper to parse expressions correctly for codegen tests
 */

#include "expression_parsing_helper.h"
#include "parser_string_interface.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "ast_node_creation.h"
#include "semantic_analyzer.h"
#include "semantic_builtins.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Parse an expression string for testing
 * Returns the expression AST node directly, not wrapped in a program
 */
ASTNode* parse_test_expression_string(const char* expr_str) {
    if (!expr_str) return NULL;
    
    ParseResult result = parse_expression_string(expr_str);
    
    if (result.success && result.ast) {
        // Extract the AST and prevent it from being freed with the result
        ASTNode* ast = result.ast;
        result.ast = NULL;
        
        // Clean up the result structure (but not the AST)
        cleanup_parse_result(&result);
        
        return ast;
    }
    
    // Clean up on failure
    cleanup_parse_result(&result);
    return NULL;
}

/**
 * Parse and analyze an expression with proper context
 * This creates a minimal program with variable declarations,
 * runs semantic analysis, and returns the analyzed expression.
 */
ASTNode* parse_and_analyze_expression(const char* expr_str, SemanticAnalyzer* analyzer) {
    if (!expr_str || !analyzer) return NULL;
    
    // Create a minimal program that declares all common test variables
    // and contains the expression in the main function
    char program_buffer[4096];
    snprintf(program_buffer, sizeof(program_buffer),
        "package test;\n"
        "\n"
        "// Define test functions\n"
        "pub fn foo(none) -> i32 { return 42; }\n"
        "pub fn bar(x: i32, y: i32, z: i32) -> i32 { return x + y + z; }\n"
        "pub fn add(a: i32, b: i32) -> i32 { return a + b; }\n"
        "pub fn calc(x: i32, y: i32, z: i32) -> i32 { return x * y + z; }\n"
        "pub fn func(none) -> i32 { return 100; }\n"
        "pub fn inner(val: i32) -> i32 { return val * 2; }\n"
        "pub fn outer(val: i32) -> i32 { return val + 10; }\n"
        "pub fn nested(val: i32) -> i32 { return val; }\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // Declare common test variables\n"
        "    let a: i32 = 1;\n"
        "    let b: i32 = 2;\n"
        "    let x: i32 = 3;\n"
        "    let y: i32 = 4;\n"
        "    let p: i32 = 5;\n"
        "    let q: i32 = 6;\n"
        "    let m: i32 = 7;\n"
        "    let n: i32 = 8;\n"
        "    let c: i32 = 9;\n"
        "    let d: i32 = 10;\n"
        "    let e: i32 = 11;\n"
        "    let f: i32 = 12;\n"
        "    let value: i32 = 13;\n"
        "    let mod: i32 = 14;\n"
        "    let z: i32 = 15;\n"
        "    let condition: bool = true;\n"
        "    let flag: bool = false;\n"
        "    let flag1: bool = true;\n"
        "    let flag2: bool = false;\n"
        "    let flag3: bool = true;\n"
        "    let obj: i32 = 42;  // Simple int instead of struct\n"
        "    let arg: i32 = 5;\n"
        "    let index: i32 = 0;\n"
        "    let offset: i32 = 2;\n"
        "    let array: i32 = 123;  // Arrays not fully supported yet\n"
        "    let val: i32 = 10;\n"
        "    let limit: i32 = 100;\n"
        "    let score: i32 = 95;\n"
        "    let threshold: i32 = 90;\n"
        "    let bits: i32 = 0b1010;\n"
        "    let data: i32 = 0b11110000;\n"
        "    let mask: i32 = 0xFF;\n"
        "    let true_val: i32 = 1;\n"
        "    let false_val: i32 = 0;\n"
        "    let pointer: *i32 = &a;\n"
        "    let variable: i32 = 123;\n"
        "    let number: i32 = -42;\n"
        "    let result: i32 = %s;\n"  // Insert the expression here
        "    return ();\n"
        "}\n",
        expr_str);
    
    // Parse the complete program
    Lexer* lexer = lexer_create(program_buffer, strlen(program_buffer), "test_expr.asthra");
    if (!lexer) return NULL;
    
    Parser* parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }
    
    ASTNode* program = parser_parse_program(parser);
    parser_destroy(parser);  // This also destroys the lexer
    
    if (!program) return NULL;
    
    // Initialize builtin types if not already done
    semantic_init_builtin_types(analyzer);
    
    // Run semantic analysis on the program
    bool analysis_result = semantic_analyze_program(analyzer, program);
    if (!analysis_result) {
        ast_free_node(program);
        return NULL;
    }
    
    // Extract the expression from the analyzed program
    // Navigate: program -> declarations -> main function -> body -> let statement -> init expression
    if (program->type != AST_PROGRAM || 
        !program->data.program.declarations ||
        program->data.program.declarations->count == 0) {
        ast_free_node(program);
        return NULL;
    }
    
    // Find the main function
    ASTNode* main_func = NULL;
    for (size_t i = 0; i < program->data.program.declarations->count; i++) {
        ASTNode* decl = program->data.program.declarations->nodes[i];
        if (decl->type == AST_FUNCTION_DECL && 
            strcmp(decl->data.function_decl.name, "main") == 0) {
            main_func = decl;
            break;
        }
    }
    
    if (!main_func || !main_func->data.function_decl.body ||
        !main_func->data.function_decl.body->data.block.statements ||
        main_func->data.function_decl.body->data.block.statements->count == 0) {
        ast_free_node(program);
        return NULL;
    }
    
    // Find the result variable declaration (should be the last let statement before return)
    ASTNode* result_expr = NULL;
    ASTNodeList* statements = main_func->data.function_decl.body->data.block.statements;
    for (int i = statements->count - 2; i >= 0; i--) {  // -2 to skip the return statement
        ASTNode* stmt = statements->nodes[i];
        if (stmt->type == AST_LET_STMT &&
            strcmp(stmt->data.let_stmt.name, "result") == 0 &&
            stmt->data.let_stmt.initializer) {
            // Clone the expression to return it separately
            result_expr = ast_clone_node(stmt->data.let_stmt.initializer);
            break;
        }
    }
    
    // Free the program AST but keep the cloned expression
    ast_free_node(program);
    
    return result_expr;
}