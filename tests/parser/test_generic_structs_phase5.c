/**
 * Asthra Programming Language Compiler
 * Phase 5: Generic Structs Parser Testing Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Comprehensive testing of generic struct parser functionality
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include parser headers
#include "ast.h"
#include "ast_node.h"
#include "lexer.h"
#include "parser.h"

// =============================================================================
// TEST FRAMEWORK MACROS
// =============================================================================

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("  ✅ PASS: %s\n", message);                                                    \
        } else {                                                                                   \
            printf("  ❌ FAIL: %s\n", message);                                                    \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST_SUCCESS()                                                                             \
    do {                                                                                           \
        printf("  🎉 Test completed successfully!\n");                                             \
        return true;                                                                               \
    } while (0)

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Create a parser from source code string
 */
static Parser *create_parser_from_source(const char *source) {
    if (!source)
        return NULL;

    Lexer *lexer = lexer_create_from_string(source);
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    return parser;
}

/**
 * Cleanup parser and associated resources
 */
static void cleanup_parser(Parser *parser) {
    if (!parser)
        return;

    Lexer *lexer = parser_get_lexer(parser);
    parser_destroy(parser);
    if (lexer) {
        lexer_destroy(lexer);
    }
}

/**
 * Check if type parameters list matches expected names
 */
static bool check_type_params(ASTNodeList *type_params, const char **expected_names,
                              size_t expected_count) {
    if (!type_params && expected_count == 0)
        return true;
    if (!type_params || !expected_names)
        return false;

    size_t actual_count = ast_node_list_size(type_params);
    if (actual_count != expected_count)
        return false;

    for (size_t i = 0; i < expected_count; i++) {
        ASTNode *param = ast_node_list_get(type_params, i);
        if (!param || param->type != AST_IDENTIFIER)
            return false;
        if (strcmp(param->data.identifier.name, expected_names[i]) != 0)
            return false;
    }

    return true;
}

/**
 * Check if type arguments list matches expected types
 */
static bool check_type_args(ASTNodeList *type_args, const char **expected_types,
                            size_t expected_count) {
    if (!type_args && expected_count == 0)
        return true;
    if (!type_args || !expected_types)
        return false;

    size_t actual_count = ast_node_list_size(type_args);
    if (actual_count != expected_count)
        return false;

    for (size_t i = 0; i < expected_count; i++) {
        ASTNode *arg = ast_node_list_get(type_args, i);
        if (!arg)
            return false;

        // For simple types, check identifier name
        if (arg->type == AST_IDENTIFIER) {
            if (strcmp(arg->data.identifier.name, expected_types[i]) != 0)
                return false;
        } else if (arg->type == AST_STRUCT_TYPE) {
            if (strcmp(arg->data.struct_type.name, expected_types[i]) != 0)
                return false;
        } else {
            return false; // Unexpected type node
        }
    }

    return true;
}

// =============================================================================
// PHASE 5 PARSER TESTS
// =============================================================================

/**
 * Test 1: Basic Generic Struct Declaration Parsing
 */
static bool test_basic_generic_struct_declaration(void) {
    printf("\n=== Test 1: Basic Generic Struct Declaration ===\n");

    const char *source = "struct Vec<T> {\n"
                         "    data: *mut T,\n"
                         "    len: usize,\n"
                         "    capacity: usize\n"
                         "}";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation from source");

    ASTNode *node = parse_struct_declaration(parser);
    TEST_ASSERT(node != NULL, "Parse generic struct declaration");
    TEST_ASSERT(node->type == AST_STRUCT_DECL, "Node type is struct declaration");

    // Check struct name
    TEST_ASSERT(strcmp(node->data.struct_decl.name, "Vec") == 0, "Struct name is 'Vec'");

    // Check type parameters
    ASTNodeList *type_params = node->data.struct_decl.type_params;
    TEST_ASSERT(type_params != NULL, "Type parameters exist");
    TEST_ASSERT(ast_node_list_size(type_params) == 1, "One type parameter");

    const char *expected_params[] = {"T"};
    TEST_ASSERT(check_type_params(type_params, expected_params, 1), "Type parameter is 'T'");

    // Check fields exist
    ASTNodeList *fields = node->data.struct_decl.fields;
    TEST_ASSERT(fields != NULL, "Struct has fields");
    TEST_ASSERT(ast_node_list_size(fields) >= 3, "Has at least 3 fields");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 2: Multiple Type Parameters Parsing
 */
static bool test_multiple_type_parameters(void) {
    printf("\n=== Test 2: Multiple Type Parameters ===\n");

    const char *source = "struct Pair<A, B> {\n"
                         "    first: A,\n"
                         "    second: B\n"
                         "}";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_struct_declaration(parser);
    TEST_ASSERT(node != NULL, "Parse Pair struct declaration");
    TEST_ASSERT(node->type == AST_STRUCT_DECL, "Node type is struct declaration");

    // Check struct name
    TEST_ASSERT(strcmp(node->data.struct_decl.name, "Pair") == 0, "Struct name is 'Pair'");

    // Check type parameters
    ASTNodeList *type_params = node->data.struct_decl.type_params;
    TEST_ASSERT(type_params != NULL, "Type parameters exist");
    TEST_ASSERT(ast_node_list_size(type_params) == 2, "Two type parameters");

    const char *expected_params[] = {"A", "B"};
    TEST_ASSERT(check_type_params(type_params, expected_params, 2),
                "Type parameters are 'A' and 'B'");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 3: Non-Generic Struct Compatibility
 */
static bool test_non_generic_struct_compatibility(void) {
    printf("\n=== Test 3: Non-Generic Struct Compatibility ===\n");

    const char *source = "struct Point {\n"
                         "    x: f64,\n"
                         "    y: f64\n"
                         "}";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_struct_declaration(parser);
    TEST_ASSERT(node != NULL, "Parse non-generic struct declaration");
    TEST_ASSERT(node->type == AST_STRUCT_DECL, "Node type is struct declaration");

    // Check struct name
    TEST_ASSERT(strcmp(node->data.struct_decl.name, "Point") == 0, "Struct name is 'Point'");

    // Check no type parameters
    ASTNodeList *type_params = node->data.struct_decl.type_params;
    TEST_ASSERT(type_params == NULL, "No type parameters for non-generic struct");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 4: Generic Struct Type Parsing
 */
static bool test_generic_struct_type_parsing(void) {
    printf("\n=== Test 4: Generic Struct Type Parsing ===\n");

    const char *source = "Vec<i32>";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_type(parser);
    TEST_ASSERT(node != NULL, "Parse generic struct type");
    TEST_ASSERT(node->type == AST_STRUCT_TYPE, "Node type is struct type");

    // Check struct name
    TEST_ASSERT(strcmp(node->data.struct_type.name, "Vec") == 0, "Struct name is 'Vec'");

    // Check type arguments
    ASTNodeList *type_args = node->data.struct_type.type_args;
    TEST_ASSERT(type_args != NULL, "Type arguments exist");
    TEST_ASSERT(ast_node_list_size(type_args) == 1, "One type argument");

    const char *expected_args[] = {"i32"};
    TEST_ASSERT(check_type_args(type_args, expected_args, 1), "Type argument is 'i32'");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 5: Complex Generic Type Arguments
 */
static bool test_complex_generic_type_arguments(void) {
    printf("\n=== Test 5: Complex Generic Type Arguments ===\n");

    const char *source = "HashMap<string, User>";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_type(parser);
    TEST_ASSERT(node != NULL, "Parse HashMap type");
    TEST_ASSERT(node->type == AST_STRUCT_TYPE, "Node type is struct type");

    // Check struct name
    TEST_ASSERT(strcmp(node->data.struct_type.name, "HashMap") == 0, "Struct name is 'HashMap'");

    // Check type arguments
    ASTNodeList *type_args = node->data.struct_type.type_args;
    TEST_ASSERT(type_args != NULL, "Type arguments exist");
    TEST_ASSERT(ast_node_list_size(type_args) == 2, "Two type arguments");

    const char *expected_args[] = {"string", "User"};
    TEST_ASSERT(check_type_args(type_args, expected_args, 2),
                "Type arguments are 'string' and 'User'");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 6: Generic Struct Literal Parsing
 */
static bool test_generic_struct_literal_parsing(void) {
    printf("\n=== Test 6: Generic Struct Literal Parsing ===\n");

    const char *source = "Vec<i32> { data: null, len: 0, capacity: 0 }";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_expression(parser);
    TEST_ASSERT(node != NULL, "Parse generic struct literal");
    TEST_ASSERT(node->type == AST_STRUCT_LITERAL, "Node type is struct literal");

    // Check struct name
    TEST_ASSERT(strcmp(node->data.struct_literal.struct_name, "Vec") == 0, "Struct name is 'Vec'");

    // Check type arguments
    ASTNodeList *type_args = node->data.struct_literal.type_args;
    TEST_ASSERT(type_args != NULL, "Type arguments exist");
    TEST_ASSERT(ast_node_list_size(type_args) == 1, "One type argument");

    const char *expected_args[] = {"i32"};
    TEST_ASSERT(check_type_args(type_args, expected_args, 1), "Type argument is 'i32'");

    // Check field initializations
    ASTNodeList *fields = node->data.struct_literal.fields;
    TEST_ASSERT(fields != NULL, "Field initializations exist");
    TEST_ASSERT(ast_node_list_size(fields) >= 3, "Has at least 3 field initializations");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 7: Generic Struct Pattern Parsing
 */
static bool test_generic_struct_pattern_parsing(void) {
    printf("\n=== Test 7: Generic Struct Pattern Parsing ===\n");

    const char *source = "Pair<i32, string> { first, second }";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_pattern(parser);
    TEST_ASSERT(node != NULL, "Parse generic struct pattern");
    TEST_ASSERT(node->type == AST_STRUCT_PATTERN, "Node type is struct pattern");

    // Check struct name
    TEST_ASSERT(strcmp(node->data.struct_pattern.name, "Pair") == 0, "Struct name is 'Pair'");

    // Check type arguments
    ASTNodeList *type_args = node->data.struct_pattern.type_args;
    TEST_ASSERT(type_args != NULL, "Type arguments exist");
    TEST_ASSERT(ast_node_list_size(type_args) == 2, "Two type arguments");

    const char *expected_args[] = {"i32", "string"};
    TEST_ASSERT(check_type_args(type_args, expected_args, 2),
                "Type arguments are 'i32' and 'string'");

    // Check field patterns
    ASTNodeList *fields = node->data.struct_pattern.fields;
    TEST_ASSERT(fields != NULL, "Field patterns exist");
    TEST_ASSERT(ast_node_list_size(fields) >= 2, "Has at least 2 field patterns");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 8: Nested Generic Types
 */
static bool test_nested_generic_types(void) {
    printf("\n=== Test 8: Nested Generic Types ===\n");

    const char *source = "Vec<Vec<i32>>";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_type(parser);
    TEST_ASSERT(node != NULL, "Parse nested generic type");
    TEST_ASSERT(node->type == AST_STRUCT_TYPE, "Node type is struct type");

    // Check outer struct name
    TEST_ASSERT(strcmp(node->data.struct_type.name, "Vec") == 0, "Outer struct name is 'Vec'");

    // Check outer type arguments
    ASTNodeList *outer_type_args = node->data.struct_type.type_args;
    TEST_ASSERT(outer_type_args != NULL, "Outer type arguments exist");
    TEST_ASSERT(ast_node_list_size(outer_type_args) == 1, "One outer type argument");

    // Check inner generic type
    ASTNode *inner_type = ast_node_list_get(outer_type_args, 0);
    TEST_ASSERT(inner_type != NULL, "Inner type exists");
    TEST_ASSERT(inner_type->type == AST_STRUCT_TYPE, "Inner type is struct type");
    TEST_ASSERT(strcmp(inner_type->data.struct_type.name, "Vec") == 0,
                "Inner struct name is 'Vec'");

    // Check inner type arguments
    ASTNodeList *inner_type_args = inner_type->data.struct_type.type_args;
    TEST_ASSERT(inner_type_args != NULL, "Inner type arguments exist");
    TEST_ASSERT(ast_node_list_size(inner_type_args) == 1, "One inner type argument");

    const char *expected_inner_args[] = {"i32"};
    TEST_ASSERT(check_type_args(inner_type_args, expected_inner_args, 1),
                "Inner type argument is 'i32'");

    ast_node_destroy(node);
    cleanup_parser(parser);
    TEST_SUCCESS();
}

/**
 * Test 9: Error Handling - Invalid Syntax
 */
static bool test_error_handling_invalid_syntax(void) {
    printf("\n=== Test 9: Error Handling - Invalid Syntax ===\n");

    // Test incomplete generic declaration
    const char *invalid_source1 = "struct Vec< { }";

    Parser *parser1 = create_parser_from_source(invalid_source1);
    TEST_ASSERT(parser1 != NULL, "Parser creation for invalid source 1");

    ASTNode *node1 = parse_struct_declaration(parser1);
    TEST_ASSERT(node1 == NULL, "Invalid syntax should fail to parse");

    cleanup_parser(parser1);

    // Test mismatched brackets
    const char *invalid_source2 = "struct Map<K, V> { key: K, value: V )";

    Parser *parser2 = create_parser_from_source(invalid_source2);
    TEST_ASSERT(parser2 != NULL, "Parser creation for invalid source 2");

    ASTNode *node2 = parse_struct_declaration(parser2);
    TEST_ASSERT(node2 == NULL, "Mismatched brackets should fail to parse");

    cleanup_parser(parser2);

    TEST_SUCCESS();
}

/**
 * Test 10: AST Memory Management
 */
static bool test_ast_memory_management(void) {
    printf("\n=== Test 10: AST Memory Management ===\n");

    const char *source = "struct Container<T, U> {\n"
                         "    first: T,\n"
                         "    second: U,\n"
                         "    count: usize\n"
                         "}";

    Parser *parser = create_parser_from_source(source);
    TEST_ASSERT(parser != NULL, "Parser creation");

    ASTNode *node = parse_struct_declaration(parser);
    TEST_ASSERT(node != NULL, "Parse struct declaration");

    // Test that all components are properly allocated
    TEST_ASSERT(node->data.struct_decl.name != NULL, "Struct name is allocated");
    TEST_ASSERT(node->data.struct_decl.type_params != NULL, "Type parameters are allocated");
    TEST_ASSERT(node->data.struct_decl.fields != NULL, "Fields are allocated");

    // Test destruction doesn't crash (proper cleanup)
    ast_node_destroy(node);
    TEST_ASSERT(true, "AST node destruction completed without crash");

    cleanup_parser(parser);
    TEST_SUCCESS();
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("Phase 5: Generic Structs Parser Testing Suite\n");
    printf("=============================================================================\n");

    // Run all parser tests
    test_basic_generic_struct_declaration();
    test_multiple_type_parameters();
    test_non_generic_struct_compatibility();
    test_generic_struct_type_parsing();
    test_complex_generic_type_arguments();
    test_generic_struct_literal_parsing();
    test_generic_struct_pattern_parsing();
    test_nested_generic_types();
    test_error_handling_invalid_syntax();
    test_ast_memory_management();

    // Print summary
    printf("\n=============================================================================\n");
    printf("Parser Test Summary: %d/%d tests passed (%.1f%%)\n", tests_passed, tests_run,
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("=============================================================================\n");

    if (tests_passed == tests_run) {
        printf("🎉 All parser tests passed! Generic struct parsing is working correctly.\n");
        return 0;
    } else {
        printf("❌ Some parser tests failed. Check the implementation.\n");
        return 1;
    }
}
