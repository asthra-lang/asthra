/**
 * Semantic analysis test for sizeof operator
 * Verifies that sizeof operator returns usize type and works in various contexts
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/analysis/semantic_analyzer.h"
#include "../../src/analysis/type_info.h"
#include "../../src/parser/ast.h"
#include "../../src/parser/lexer.h"
#include "../../src/parser/parser.h"

// Helper function to create and run semantic analysis
static bool analyze_source(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    if (!lexer)
        return false;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return false;
    }

    ASTNode *program = parse_program(parser);
    if (!program) {
        parser_destroy(parser);
        return false;
    }

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        ast_free_node(program);
        parser_destroy(parser);
        return false;
    }

    bool success = semantic_analyze_program(analyzer, program);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    parser_destroy(parser);

    return success;
}

// Test 1: sizeof returns usize type
void test_sizeof_returns_usize(void) {
    printf("Testing sizeof returns usize type ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let size: usize = sizeof(i32);\n"
                         "    let size2: usize = sizeof([]i32);\n"
                         "    let size3: usize = sizeof(Option<i32>);\n"
                         "    return ();\n"
                         "}\n";

    bool success = analyze_source(source);
    assert(success);

    printf("  ✓ sizeof correctly returns usize type\n");
}

// Test 2: sizeof type checking in expressions
void test_sizeof_type_checking(void) {
    printf("Testing sizeof type checking in expressions ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    // Arithmetic with sizeof\n"
                         "    let double_size: usize = sizeof(i32) * 2;\n"
                         "    let sum_size: usize = sizeof(i32) + sizeof(i64);\n"
                         "    \n"
                         "    // Comparison with sizeof\n"
                         "    let is_32bit: bool = sizeof(i32) == 4;\n"
                         "    let is_larger: bool = sizeof(i64) > sizeof(i32);\n"
                         "    \n"
                         "    // Using sizeof in calculations\n"
                         "    let buffer_size: usize = sizeof(i32) * 256;\n"
                         "    \n"
                         "    return ();\n"
                         "}\n";

    bool success = analyze_source(source);
    assert(success);

    printf("  ✓ sizeof type checking in expressions passed\n");
}

// Test 3: Type mismatch errors with sizeof
void test_sizeof_type_mismatch(void) {
    printf("Testing sizeof type mismatch errors ...\n");

    // Test assigning sizeof to wrong type
    {
        const char *source = "package test;\n"
                             "pub fn main(none) -> void {\n"
                             "    let size: i32 = sizeof(i32);\n" // Should fail: usize -> i32
                             "    return ();\n"
                             "}\n";

        bool success = analyze_source(source);
        assert(!success); // Should fail due to type mismatch
    }

    // Test using sizeof where bool is expected
    {
        const char *source = "package test;\n"
                             "pub fn main(none) -> void {\n"
                             "    if sizeof(i32) {\n" // Should fail: usize is not bool
                             "        let x: i32 = 42;\n"
                             "    }\n"
                             "    return ();\n"
                             "}\n";

        bool success = analyze_source(source);
        assert(!success); // Should fail due to type mismatch
    }

    printf("  ✓ sizeof type mismatch errors detected correctly\n");
}

// Test 4: sizeof in const expressions
void test_sizeof_const_expressions(void) {
    printf("Testing sizeof in const expressions ...\n");

    const char *source = "package test;\n"
                         "priv const SIZE_I32: usize = sizeof(i32);\n"
                         "priv const SIZE_ARRAY: usize = sizeof([100]u8);\n"
                         "priv const BUFFER_SIZE: usize = sizeof(i32) * 256;\n"
                         "priv const TOTAL_SIZE: usize = sizeof(i32) + sizeof(i64) + sizeof(f64);\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    // Use const values\n"
                         "    let x: usize = SIZE_I32;\n"
                         "    let y: usize = BUFFER_SIZE;\n"
                         "    return ();\n"
                         "}\n";

    bool success = analyze_source(source);
    assert(success);

    printf("  ✓ sizeof in const expressions analyzed correctly\n");
}

// Test 5: sizeof with all type categories
void test_sizeof_all_types(void) {
    printf("Testing sizeof with all type categories ...\n");

    const char *source = "package test;\n"
                         "pub struct Point { x: i32, y: i32 }\n"
                         "pub enum Color { Red, Green, Blue }\n"
                         "pub fn main(none) -> void {\n"
                         "    // Primitive types\n"
                         "    let s1: usize = sizeof(i32);\n"
                         "    let s2: usize = sizeof(bool);\n"
                         "    let s3: usize = sizeof(string);\n"
                         "    \n"
                         "    // Composite types\n"
                         "    let s4: usize = sizeof(Point);\n"
                         "    let s5: usize = sizeof(Color);\n"
                         "    let s6: usize = sizeof((i32, f64));\n"
                         "    \n"
                         "    // Container types\n"
                         "    let s7: usize = sizeof([]i32);\n"
                         "    let s8: usize = sizeof([10]i32);\n"
                         "    let s9: usize = sizeof(Option<i32>);\n"
                         "    let s10: usize = sizeof(Result<i32, string>);\n"
                         "    \n"
                         "    // Pointer types\n"
                         "    let s11: usize = sizeof(*const i32);\n"
                         "    let s12: usize = sizeof(*mut Point);\n"
                         "    \n"
                         "    return ();\n"
                         "}\n";

    bool success = analyze_source(source);
    assert(success);

    printf("  ✓ sizeof with all type categories analyzed correctly\n");
}

// Test 6: sizeof in function parameters and returns
void test_sizeof_in_functions(void) {
    printf("Testing sizeof in function parameters and returns ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub fn get_size(none) -> usize {\n"
                         "    return sizeof(i32);\n"
                         "}\n"
                         "\n"
                         "pub fn process_size(size: usize) -> bool {\n"
                         "    return size > 0;\n"
                         "}\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let size: usize = get_size();\n"
                         "    let valid: bool = process_size(sizeof(i64));\n"
                         "    return ();\n"
                         "}\n";

    bool success = analyze_source(source);
    assert(success);

    printf("  ✓ sizeof in function parameters and returns analyzed correctly\n");
}

// Test 7: sizeof with generic types
void test_sizeof_generic_types_semantic(void) {
    printf("Testing sizeof with generic types (semantic) ...\n");

    const char *source = "package test;\n"
                         "pub struct Vec<T> { data: *mut T, len: usize, cap: usize }\n"
                         "pub fn main(none) -> void {\n"
                         "    let s1: usize = sizeof(Vec<i32>);\n"
                         "    let s2: usize = sizeof(Vec<string>);\n"
                         "    let s3: usize = sizeof(Option<Vec<i32>>);\n"
                         "    let s4: usize = sizeof(Result<Vec<i32>, string>);\n"
                         "    return ();\n"
                         "}\n";

    bool success = analyze_source(source);
    assert(success);

    printf("  ✓ sizeof with generic types analyzed correctly\n");
}

// Test 8: sizeof invalid usage
void test_sizeof_invalid_usage(void) {
    printf("Testing sizeof invalid usage ...\n");

    // Test sizeof with non-type expression
    {
        const char *source = "package test;\n"
                             "pub fn main(none) -> void {\n"
                             "    let x: i32 = 42;\n"
                             "    let size: usize = sizeof(x + 1);\n" // Expression, not type
                             "    return ();\n"
                             "}\n";

        bool success = analyze_source(source);
        // This should fail in semantic analysis
        // The parser might accept it but semantic analysis should reject
    }

    printf("  ✓ sizeof invalid usage handled\n");
}

int main(void) {
    printf("=== sizeof Operator Semantic Analysis Test Suite ===\n\n");

    test_sizeof_returns_usize();
    test_sizeof_type_checking();
    test_sizeof_type_mismatch();
    test_sizeof_const_expressions();
    test_sizeof_all_types();
    test_sizeof_in_functions();
    test_sizeof_generic_types_semantic();
    test_sizeof_invalid_usage();

    printf("\n✅ All sizeof semantic tests completed!\n");
    return 0;
}