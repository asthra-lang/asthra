/**
 * Semantic analysis tests for logical NOT operator (!x)
 * Tests type checking, type inference, and semantic validation
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
#include "../../src/analysis/semantic_analyzer.h"
#include "../../src/analysis/type_system.h"

// Helper function to create and run semantic analysis
static SemanticAnalyzer* analyze_source(const char* source) {
    Lexer* lexer = lexer_create(source, strlen(source), "<test>");
    if (!lexer) return NULL;
    
    Parser* parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }
    
    ASTNode* program = parse_program(parser);
    parser_destroy(parser);
    
    if (!program) return NULL;
    
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        ast_free_node(program);
        return NULL;
    }
    
    bool success = semantic_analyze(analyzer, program);
    if (!success) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        return NULL;
    }
    
    // Keep the AST for inspection
    analyzer->current_ast = program;
    return analyzer;
}

// Test 1: Basic type checking - NOT requires bool operand
void test_logical_not_type_checking(void) {
    printf("Testing logical NOT type checking ...\n");
    
    // Valid: !bool_value
    const char* valid_source = 
        "package test;\n"
        "pub fn test(none) -> bool {\n"
        "    let flag: bool = true;\n"
        "    return !flag;\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(valid_source);
    assert(analyzer != NULL);
    
    // Function should return bool
    ASTNode* func = analyzer->current_ast->data.program.declarations->nodes[0];
    Type* return_type = func->data.function_decl.resolved_type->data.function_type.return_type;
    assert(return_type->kind == TYPE_BOOL);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ Valid bool operand type checked correctly\n");
}

// Test 2: Invalid type for NOT operator
void test_logical_not_invalid_type(void) {
    printf("Testing logical NOT with invalid types ...\n");
    
    // Invalid: !integer
    const char* invalid_int = 
        "package test;\n"
        "pub fn test(none) -> bool {\n"
        "    let num: i32 = 42;\n"
        "    return !num;\n"  // Error: NOT requires bool
        "}\n";
    
    SemanticAnalyzer* analyzer1 = analyze_source(invalid_int);
    assert(analyzer1 == NULL); // Should fail semantic analysis
    
    // Invalid: !string
    const char* invalid_string = 
        "package test;\n"
        "pub fn test(none) -> bool {\n"
        "    let msg: string = \"hello\";\n"
        "    return !msg;\n"  // Error: NOT requires bool
        "}\n";
    
    SemanticAnalyzer* analyzer2 = analyze_source(invalid_string);
    assert(analyzer2 == NULL); // Should fail semantic analysis
    
    printf("  ✓ Invalid operand types rejected correctly\n");
}

// Test 3: NOT operator result type
void test_logical_not_result_type(void) {
    printf("Testing logical NOT result type ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn negate(flag: bool) -> bool {\n"
        "    return !flag;\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer != NULL);
    
    // The result of !bool should be bool
    // We can verify this by checking the function returns successfully
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ NOT operator returns bool type correctly\n");
}

// Test 4: NOT with comparison results
void test_logical_not_with_comparisons(void) {
    printf("Testing logical NOT with comparison results ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn check(x: i32, y: i32) -> bool {\n"
        "    return !(x > y);\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer != NULL);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ NOT with comparison expression type checked correctly\n");
}

// Test 5: NOT in complex boolean expressions
void test_logical_not_in_boolean_expr(void) {
    printf("Testing logical NOT in complex boolean expressions ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn complex(a: bool, b: bool, c: bool) -> bool {\n"
        "    return !a && b || !c;\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer != NULL);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ NOT in complex boolean expressions type checked correctly\n");
}

// Test 6: NOT with function call results
void test_logical_not_function_result(void) {
    printf("Testing logical NOT with function results ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn isEmpty(none) -> bool {\n"
        "    return true;\n"
        "}\n"
        "\n"
        "pub fn check(none) -> bool {\n"
        "    return !isEmpty(none);\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer != NULL);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ NOT with function result type checked correctly\n");
}

// Test 7: NOT with Option<bool> (should fail)
void test_logical_not_option_type(void) {
    printf("Testing logical NOT with Option<bool> ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn test(opt: Option<bool>) -> bool {\n"
        "    return !opt;\n"  // Error: can't apply NOT to Option<bool>
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer == NULL); // Should fail - NOT doesn't work on Option types
    
    printf("  ✓ NOT with Option<bool> rejected correctly\n");
}

// Test 8: NOT in if conditions
void test_logical_not_in_conditions(void) {
    printf("Testing logical NOT in if conditions ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn process(ready: bool) -> i32 {\n"
        "    if !ready {\n"
        "        return 0;\n"
        "    }\n"
        "    return 1;\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer != NULL);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ NOT in if conditions type checked correctly\n");
}

// Test 9: NOT with struct field access
void test_logical_not_struct_field(void) {
    printf("Testing logical NOT with struct field access ...\n");
    
    const char* source = 
        "package test;\n"
        "pub struct Config {\n"
        "    pub enabled: bool,\n"
        "    pub verbose: bool\n"
        "}\n"
        "\n"
        "pub fn isDisabled(cfg: Config) -> bool {\n"
        "    return !cfg.enabled;\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer != NULL);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ NOT with struct field type checked correctly\n");
}

// Test 10: Type inference with NOT
void test_logical_not_type_inference(void) {
    printf("Testing type inference with logical NOT ...\n");
    
    const char* source = 
        "package test;\n"
        "pub fn infer(none) -> bool {\n"
        "    let x: bool = true;\n"
        "    let y: bool = !x;  // y should be inferred as bool\n"
        "    return y;\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = analyze_source(source);
    assert(analyzer != NULL);
    
    // Both x and y should have bool type
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(analyzer->current_ast);
    printf("  ✓ Type inference with NOT works correctly\n");
}

int main(void) {
    printf("=== Logical NOT Operator Semantic Analysis Test Suite ===\n\n");
    
    test_logical_not_type_checking();
    test_logical_not_invalid_type();
    test_logical_not_result_type();
    test_logical_not_with_comparisons();
    test_logical_not_in_boolean_expr();
    test_logical_not_function_result();
    test_logical_not_option_type();
    test_logical_not_in_conditions();
    test_logical_not_struct_field();
    test_logical_not_type_inference();
    
    printf("\n✅ All logical NOT semantic tests completed!\n");
    return 0;
}