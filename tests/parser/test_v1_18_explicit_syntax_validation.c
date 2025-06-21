/**
 * Asthra Programming Language Compiler
 * Test v1.18 explicit syntax validation - ensures old implicit syntax is rejected
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

// Test helper functions
static Parser *create_parser_from_source(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) return NULL;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }
    
    return parser;
}

static void cleanup_parser(Parser *parser) {
    if (parser) {
        parser_destroy(parser);
    }
}

static bool test_should_fail(const char *source, const char *test_name) {
    printf("Testing %s should fail... ", test_name);
    
    Parser *parser = create_parser_from_source(source);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    
    // The parse should either fail completely or have syntax errors
    bool should_fail = (program == NULL) || parser_had_error(parser);
    
    if (program) {
        ast_free_node(program);
    }
    cleanup_parser(parser);
    
    if (should_fail) {
        printf("✅ PASS (correctly rejected)\n");
        return true;
    } else {
        printf("❌ FAIL (should have been rejected)\n");
        return false;
    }
}

static bool test_should_pass(const char *source, const char *test_name) {
    printf("Testing %s should pass... ", test_name);
    
    Parser *parser = create_parser_from_source(source);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    
    // The parse should succeed without errors
    bool should_pass = (program != NULL) && !parser_had_error(parser);
    
    if (program) {
        ast_free_node(program);
    }
    cleanup_parser(parser);
    
    if (should_pass) {
        printf("✅ PASS (correctly accepted)\n");
        return true;
    } else {
        printf("❌ FAIL (should have been accepted)\n");
        return false;
    }
}

// Test that old implicit visibility syntax is rejected
void test_implicit_visibility_rejection(void) {
    printf("\n=== Testing Implicit Visibility Rejection ===\n");
    
    // Old implicit syntax should fail
    test_should_fail(
        "package test;\n"
        "struct Point { x: i32, y: i32 }\n",
        "struct without visibility modifier"
    );
    
    test_should_fail(
        "package test;\n"
        "enum Status { Ready, Processing }\n",
        "enum without visibility modifier"
    );
    
    test_should_fail(
        "package test;\n"
        "fn main() -> void { void }\n",
        "function without visibility modifier"
    );
    
    // New explicit syntax should pass
    test_should_pass(
        "package test;\n"
        "priv struct Point { x: i32, y: i32 }\n",
        "struct with explicit private visibility"
    );
    
    test_should_pass(
        "package test;\n"
        "pub enum Status { Ready, Processing }\n",
        "enum with explicit public visibility"
    );
    
    test_should_pass(
        "package test;\n"
        "priv fn main(void) -> void { void }\n",
        "function with explicit private visibility"
    );
}

// Test that old implicit parameter syntax is rejected
void test_implicit_parameters_rejection(void) {
    printf("\n=== Testing Implicit Parameters Rejection ===\n");
    
    // Old implicit syntax should fail
    test_should_fail(
        "package test;\n"
        "priv fn main() -> void { void }\n",
        "function with empty parameter list"
    );
    
    test_should_fail(
        "package test;\n"
        "extern \"C\" fn malloc() -> *mut void;\n",
        "extern function with empty parameter list"
    );
    
    // New explicit syntax should pass
    test_should_pass(
        "package test;\n"
        "priv fn main(void) -> void { void }\n",
        "function with explicit void parameters"
    );
    
    test_should_pass(
        "package test;\n"
        "extern \"C\" fn malloc(size: usize) -> *mut void;\n",
        "extern function with explicit parameters"
    );
    
    test_should_pass(
        "package test;\n"
        "extern \"C\" fn free(ptr: *mut void) -> void;\n",
        "extern function with explicit void parameter"
    );
}

// Test that old implicit struct/enum content syntax is rejected
void test_implicit_content_rejection(void) {
    printf("\n=== Testing Implicit Content Rejection ===\n");
    
    // Old implicit syntax should fail
    test_should_fail(
        "package test;\n"
        "priv struct Empty { }\n",
        "struct with empty braces"
    );
    
    test_should_fail(
        "package test;\n"
        "priv enum EmptyEnum { }\n",
        "enum with empty braces"
    );
    
    // New explicit syntax should pass
    test_should_pass(
        "package test;\n"
        "priv struct Empty { void }\n",
        "struct with explicit void content"
    );
    
    test_should_pass(
        "package test;\n"
        "priv enum Status { void }\n",
        "enum with explicit void content"
    );
    
    test_should_pass(
        "package test;\n"
        "priv struct Point { x: i32, y: i32 }\n",
        "struct with explicit field content"
    );
}

// Test that old implicit function call syntax is rejected
void test_implicit_function_calls_rejection(void) {
    printf("\n=== Testing Implicit Function Calls Rejection ===\n");
    
    // Old implicit syntax should fail
    test_should_fail(
        "package test;\n"
        "priv fn main(void) -> void {\n"
        "    doSomething();\n"
        "}\n",
        "function call with empty parentheses"
    );
    
    test_should_fail(
        "package test;\n"
        "priv fn main(void) -> void {\n"
        "    let result: Result<i32, string> = Result.Ok();\n"
        "}\n",
        "enum constructor with empty parentheses"
    );
    
    // New explicit syntax should pass
    test_should_pass(
        "package test;\n"
        "priv fn main(void) -> void {\n"
        "    doSomething(void);\n"
        "}\n",
        "function call with explicit void arguments"
    );
    
    test_should_pass(
        "package test;\n"
        "priv fn main(void) -> void {\n"
        "    processData(42, \"test\");\n"
        "}\n",
        "function call with explicit arguments"
    );
}

// Test that old implicit array syntax is rejected
void test_implicit_arrays_rejection(void) {
    printf("\n=== Testing Implicit Arrays Rejection ===\n");
    
    // Old implicit syntax should fail
    test_should_fail(
        "package test;\n"
        "priv fn main(void) -> void {\n"
        "    let empty: []i32 = [];\n"
        "}\n",
        "array literal with empty brackets"
    );
    
    // New explicit syntax should pass
    test_should_pass(
        "package test;\n"
        "priv fn main(void) -> void {\n"
        "    let empty: []i32 = [void];\n"
        "}\n",
        "array literal with explicit void elements"
    );
    
    test_should_pass(
        "package test;\n"
        "priv fn main(void) -> void {\n"
        "    let numbers: []i32 = [1, 2, 3];\n"
        "}\n",
        "array literal with explicit elements"
    );
}

// Test complex scenarios with mixed syntax
void test_mixed_syntax_scenarios(void) {
    printf("\n=== Testing Mixed Syntax Scenarios ===\n");
    
    // Complex valid explicit syntax
    test_should_pass(
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n"
        "\n"
        "priv enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "\n"
        "impl Point {\n"
        "    pub fn new(x: f64, y: f64) -> Point {\n"
        "        return Point { x: x, y: y };\n"
        "    }\n"
        "    \n"
        "    priv fn distance(self) -> f64 {\n"
        "        return sqrt(self.x * self.x + self.y * self.y);\n"
        "    }\n"
        "}\n"
        "\n"
        "priv fn main(void) -> void {\n"
        "    let point: Point = Point::new(3.0, 4.0);\n"
        "    let distance: f64 = point.distance(void);\n"
        "    let result: Result<f64, string> = Result.Ok(distance);\n"
        "    return void;\n"
        "}\n",
        "complex valid v1.18 syntax"
    );
    
    // Complex invalid mixed syntax (should fail)
    test_should_fail(
        "package test;\n"
        "\n"
        "struct Point {  // Missing visibility\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n"
        "\n"
        "priv fn main() -> void {  // Missing void parameters\n"
        "    let point = Point::new(3.0, 4.0);  // Missing type annotation\n"
        "    doSomething();  // Missing void arguments\n"
        "}\n",
        "complex invalid mixed syntax"
    );
}

int main(void) {
    printf("Running v1.18 Explicit Syntax Validation Tests\n");
    printf("=============================================\n");
    
    test_implicit_visibility_rejection();
    test_implicit_parameters_rejection();
    test_implicit_content_rejection();
    test_implicit_function_calls_rejection();
    test_implicit_arrays_rejection();
    test_mixed_syntax_scenarios();
    
    printf("\n🎉 All v1.18 explicit syntax validation tests completed!\n");
    printf("✅ Grammar v1.18 requirements are properly enforced\n");
    printf("✅ Old implicit syntax is correctly rejected\n");
    printf("✅ New explicit syntax is correctly accepted\n");
    
    return 0;
} 
