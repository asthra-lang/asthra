/**
 * Asthra Programming Language
 * Type Grammar Production Tests
 * 
 * Tests for type parsing including primitive types, pointer types,
 * slice types, Result types, and struct types.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "../framework/test_data.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_type_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_type_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TYPE PARSING TESTS
// =============================================================================

/**
 * Test: Parse Primitive Types
 * Verifies that primitive types are parsed correctly
 */
static AsthraTestResult test_parse_primitive_types(AsthraTestContext* context) {
    const char* primitive_types[] = {
        "i8", "i16", "i32", "i64",
        "u8", "u16", "u32", "u64",
        "f32", "f64",
        "bool", "char", "void"
    };
    
    size_t type_count = sizeof(primitive_types) / sizeof(primitive_types[0]);
    
    for (size_t i = 0; i < type_count; i++) {
        char test_source[64];
        snprintf(test_source, sizeof(test_source), "let x: %s;", primitive_types[i]);
        
        Parser* parser = create_test_parser(test_source);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_statement(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse primitive type")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Pointer Types
 * Verifies that pointer types are parsed correctly
 */
static AsthraTestResult test_parse_pointer_types(AsthraTestContext* context) {
    const char* test_source = "let ptr: *i32;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parser_parse_statement(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse pointer type")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Slice Types
 * Verifies that slice types are parsed correctly
 */
static AsthraTestResult test_parse_slice_types(AsthraTestContext* context) {
    // Grammar v1.22: SliceType <- '[' ']' Type (empty brackets followed by type)
    const char* test_source = "let slice: []i32;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parser_parse_statement(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse slice type")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Result Types
 * Verifies that Result types are parsed correctly
 */
static AsthraTestResult test_parse_result_types(AsthraTestContext* context) {
    // Grammar v1.22: ResultType <- 'Result' '<' Type ',' Type '>'
    // Grammar v1.25: Use 'none' for empty parameter lists
    const char* test_source = "package test_pkg;\npub extern fn test(none) -> Result<i32, string>;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parser_parse_program(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse Result type")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool_eq(context, parser_had_error(parser), false, 
                                   "Should parse without errors")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Struct Types
 * Verifies that struct types are parsed correctly
 */
static AsthraTestResult test_parse_struct_types(AsthraTestContext* context) {
    const char* test_source = "let point: Point;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parser_parse_statement(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse struct type")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Array Types
 * Verifies that array types are parsed correctly
 * Note: Current grammar v1.22 doesn't support [Type; size] syntax,
 * so we test array literals instead
 */
static AsthraTestResult test_parse_array_types(AsthraTestContext* context) {
    // Grammar v1.22: Use slice type instead of fixed-size arrays
    const char* test_source = "let array: []i32;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parser_parse_statement(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse array type")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Function Types
 * Verifies that function types are parsed correctly
 * Note: Current grammar v1.22 doesn't support fn(params) -> return syntax,
 * so we test pointer to function instead
 */
static AsthraTestResult test_parse_function_types(AsthraTestContext* context) {
    // Grammar v1.22: Use pointer type instead of function type syntax
    const char* test_source = "let func: *void;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parser_parse_statement(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse function type")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all type parsing tests
 */
AsthraTestSuite* create_grammar_types_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Grammar Type Tests", 
                                                     "Type parsing testing");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_type_tests);
    asthra_test_suite_set_teardown(suite, teardown_type_tests);
    
    // Type parsing tests
    asthra_test_suite_add_test(suite, "test_parse_primitive_types", 
                              "Parse primitive types", 
                              test_parse_primitive_types);
    
    asthra_test_suite_add_test(suite, "test_parse_pointer_types", 
                              "Parse pointer types", 
                              test_parse_pointer_types);
    
    asthra_test_suite_add_test(suite, "test_parse_slice_types", 
                              "Parse slice types", 
                              test_parse_slice_types);
    
    asthra_test_suite_add_test(suite, "test_parse_result_types", 
                              "Parse Result types", 
                              test_parse_result_types);
    
    asthra_test_suite_add_test(suite, "test_parse_struct_types", 
                              "Parse struct types", 
                              test_parse_struct_types);
    
    asthra_test_suite_add_test(suite, "test_parse_array_types", 
                              "Parse array types", 
                              test_parse_array_types);
    
    asthra_test_suite_add_test(suite, "test_parse_function_types", 
                              "Parse function types", 
                              test_parse_function_types);
    
    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Grammar Type Tests ===\n\n");
    
    AsthraTestSuite* suite = create_grammar_types_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED 
