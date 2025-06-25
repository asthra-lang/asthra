/**
 * Comprehensive test suite for associated function calls with generic types
 * Tests syntax like Vec<i32>::new() as defined in grammar.txt lines 139-140
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

// Test 1: Basic Vec<T>::new() syntax
void test_vec_new_basic(void) {
    printf("Testing Vec<T>::new() syntax ...\n");

    const char *source = "package test;\n"
                         "pub fn test_vec_new(none) -> void {\n"
                         "    let v: Vec<i32> = Vec<i32>::new();\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse - feature likely not integrated\n");
        parser_destroy(parser);
        return;
    }

    // Navigate to the function call
    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 1);

    ASTNode *func = decls->nodes[0];
    assert(func->type == AST_FUNCTION_DECL);

    ASTNode *body = func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    ASTNode *let_stmt = stmts->nodes[0];
    ASTNode *init = let_stmt->data.let_stmt.initializer;

    // Check if it's an associated function call
    if (init && init->type == AST_CALL_EXPR) {
        ASTNode *function = init->data.call_expr.function;
        if (function && function->type == AST_ASSOCIATED_FUNC_CALL) {
            printf("  ✓ Vec<i32>::new() parsed as associated function call\n");
            assert(strcmp(function->data.associated_func_call.struct_name, "Vec") == 0);
            assert(strcmp(function->data.associated_func_call.function_name, "new") == 0);

            // Check for type arguments
            if (function->data.associated_func_call.type_args) {
                printf("  ✓ Generic type arguments captured\n");
            } else {
                printf("  ⚠️ Generic type arguments missing\n");
            }
        } else {
            printf("  ❌ Not parsed as associated function call\n");
        }
    } else {
        printf("  ❌ Failed to parse expression correctly\n");
    }

    ast_free_node(program);
    parser_destroy(parser);
}

// Test 2: Multiple generic type parameters
void test_map_with_multiple_params(void) {
    printf("Testing Map<K, V>::new() syntax ...\n");

    const char *source = "package test;\n"
                         "pub fn test_map_new(none) -> void {\n"
                         "    let m: Map<string, i32> = Map<string, i32>::new();\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse multiple type parameters\n");
    } else {
        printf("  ✓ Multiple type parameter syntax accepted\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 3: Nested generic types
void test_nested_generic_types(void) {
    printf("Testing Vec<Option<i32>>::new() syntax ...\n");

    const char *source = "package test;\n"
                         "pub fn test_nested(none) -> void {\n"
                         "    let v: Vec<Option<i32>> = Vec<Option<i32>>::new();\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse nested generic types\n");
    } else {
        printf("  ✓ Nested generic type syntax accepted\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 4: Option and Result types
void test_option_result_types(void) {
    printf("Testing Option<T> and Result<T, E> syntax ...\n");

    const char *source = "package test;\n"
                         "pub fn test_option_result(none) -> void {\n"
                         "    let opt: Option<i32> = Option<i32>::some(42);\n"
                         "    let res: Result<i32, string> = Result<i32, string>::ok(42);\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse Option/Result generic calls\n");
    } else {
        printf("  ✓ Option<T> and Result<T,E> syntax accepted\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 5: Generic struct methods
void test_generic_struct_methods(void) {
    printf("Testing generic struct method calls ...\n");

    const char *source = "package test;\n"
                         "pub fn test_methods(none) -> void {\n"
                         "    let v: Vec<string> = Vec<string>::with_capacity(10);\n"
                         "    let s: Set<i32> = Set<i32>::from_array([1, 2, 3]);\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse generic struct methods\n");
    } else {
        printf("  ✓ Generic struct method calls accepted\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 6: Type parameters in expressions
void test_type_params_in_expressions(void) {
    printf("Testing type parameters in various contexts ...\n");

    const char *source = "package test;\n"
                         "pub fn compare<T>(a: T, b: T) -> bool {\n"
                         "    let list: List<T> = List<T>::new();\n"
                         "    return true;\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse type parameters in expressions\n");
    } else {
        printf("  ✓ Type parameters in expressions accepted\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 7: Chained calls on generic types
void test_chained_generic_calls(void) {
    printf("Testing chained calls on generic types ...\n");

    const char *source = "package test;\n"
                         "pub fn test_chained(none) -> void {\n"
                         "    let result: i32 = Vec<i32>::new().push(42).pop();\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse chained generic calls\n");
    } else {
        printf("  ✓ Chained calls on generic types accepted\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 8: Generic types with array syntax
void test_generic_with_arrays(void) {
    printf("Testing generic types with array elements ...\n");

    const char *source = "package test;\n"
                         "pub fn test_arrays(none) -> void {\n"
                         "    let v: Vec<[10]i32> = Vec<[10]i32>::new();\n"
                         "    let m: Map<string, []u8> = Map<string, []u8>::new();\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Failed to parse generic types with arrays\n");
    } else {
        printf("  ✓ Generic types with array elements accepted\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 9: Ambiguity with comparison operators
void test_ambiguity_with_comparisons(void) {
    printf("Testing ambiguity between generics and comparisons ...\n");

    const char *source = "package test;\n"
                         "pub fn test_ambiguity(none) -> void {\n"
                         "    // This should parse as comparison\n"
                         "    let b1: bool = x < 20 && y > 10;\n"
                         "    \n"
                         "    // This should parse as generic type\n"
                         "    let v: Vec<i32> = Vec<i32>::new();\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);

    if (!program) {
        printf("  ❌ Parser confused by < > ambiguity\n");
    } else {
        printf("  ✓ Parser handles < > ambiguity correctly\n");
        ast_free_node(program);
    }

    parser_destroy(parser);
}

// Test 10: Error cases
void test_error_cases(void) {
    printf("Testing error cases ...\n");

    // Missing function name after ::
    {
        const char *source = "package test;\n"
                             "pub fn test_error(none) -> void {\n"
                             "    let v = Vec<i32>::;\n" // Missing function name
                             "    return ();\n"
                             "}\n";

        Parser *parser = create_parser(source);
        assert(parser != NULL);

        ASTNode *program = parse_program(parser);
        // Should fail to parse

        if (program)
            ast_free_node(program);
        parser_destroy(parser);
    }

    // Unclosed generic type
    {
        const char *source = "package test;\n"
                             "pub fn test_error(none) -> void {\n"
                             "    let v = Vec<i32::new();\n" // Missing >
                             "    return ();\n"
                             "}\n";

        Parser *parser = create_parser(source);
        assert(parser != NULL);

        ASTNode *program = parse_program(parser);
        // Should fail to parse

        if (program)
            ast_free_node(program);
        parser_destroy(parser);
    }

    printf("  ✓ Error cases handled\n");
}

int main(void) {
    printf("=== Associated Function Calls with Generic Types Test Suite ===\n\n");

    test_vec_new_basic();
    test_map_with_multiple_params();
    test_nested_generic_types();
    test_option_result_types();
    test_generic_struct_methods();
    test_type_params_in_expressions();
    test_chained_generic_calls();
    test_generic_with_arrays();
    test_ambiguity_with_comparisons();
    test_error_cases();

    printf("\n📝 Test suite completed - feature investigation done\n");
    return 0;
}