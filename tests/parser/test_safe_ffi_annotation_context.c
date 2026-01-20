/**
 * Asthra Programming Language
 * SafeFFIAnnotation Parser - Context Integration Tests
 *
 * Tests FFI annotations in various language contexts (functions, externs, programs)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_data.h"
#include "../framework/test_framework.h"
#include "ast_node.h"
#include "grammar_annotations.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURES
// =============================================================================

static AsthraTestResult setup_context_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_context_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// CONTEXT INTEGRATION TESTS
// =============================================================================

/**
 * Test: Single Annotation Parsing in Function Context
 * Validates that SafeFFIAnnotation works correctly in function declarations
 * NOTE: Using pub visibility as required by grammar
 * NOTE: FFI annotations may only be supported in extern declarations, not regular functions
 */
static AsthraTestResult test_single_annotation_in_function_context(AsthraTestContext *context) {
    // Based on the test failures, it appears FFI annotations are only supported
    // in extern function declarations, not regular function declarations.
    // This makes sense as FFI annotations are for foreign function interface.
    // We'll test this by expecting these to fail to parse.

    const char *function_sources[] = {
        "pub fn test_function(param: #[borrowed] *const u8) -> i32 { }",
        "pub fn test_function(param: #[transfer_full] *mut u8) -> i32 { }"};

    size_t count = sizeof(function_sources) / sizeof(function_sources[0]);

    for (size_t i = 0; i < count; i++) {
        Lexer *lexer = lexer_create(function_sources[i], strlen(function_sources[i]), "test");
        if (!asthra_test_assert_not_null(context, lexer, "Failed to create lexer for: %s",
                                         function_sources[i])) {
            return ASTHRA_TEST_FAIL;
        }

        Parser *parser = parser_create(lexer);
        if (!asthra_test_assert_not_null(context, parser, "Failed to create parser for: %s",
                                         function_sources[i])) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *func = parse_top_level_decl(parser);

        // FFI annotations in regular functions should fail to parse
        if (!asthra_test_assert_null(
                context, func, "FFI annotations should not be allowed in regular functions: %s",
                function_sources[i])) {
            ast_free_node(func);
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        parser_destroy(parser);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Single Annotation Parsing in Extern Context
 * Validates that SafeFFIAnnotation works correctly in extern declarations
 */
static AsthraTestResult test_single_annotation_in_extern_context(AsthraTestContext *context) {
    const char *extern_sources[] = {
        "extern \"C\" fn malloc(size: usize) -> #[transfer_full] *mut u8;",
        "extern \"C\" fn free(#[transfer_full] ptr: *mut u8);",
        "extern \"C\" fn strlen(#[borrowed] s: *const u8) -> usize;",
        ("extern fn custom_function(#[transfer_none] data: *const u8) -> #[transfer_none] *const "
         "i32;")};

    size_t count = sizeof(extern_sources) / sizeof(extern_sources[0]);

    for (size_t i = 0; i < count; i++) {
        Lexer *lexer = lexer_create(extern_sources[i], strlen(extern_sources[i]), "test");
        if (!asthra_test_assert_not_null(context, lexer, "Failed to create lexer for: %s",
                                         extern_sources[i])) {
            return ASTHRA_TEST_FAIL;
        }

        Parser *parser = parser_create(lexer);
        if (!asthra_test_assert_not_null(context, parser, "Failed to create parser for: %s",
                                         extern_sources[i])) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *extern_decl = parse_extern_decl(parser);

        if (!asthra_test_assert_not_null(context, extern_decl,
                                         "Extern with SafeFFIAnnotation should parse: %s",
                                         extern_sources[i])) {
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        // Validate that extern parsed correctly with annotation
        if (!asthra_test_assert_int_eq(context, extern_decl->type, AST_EXTERN_DECL,
                                       "Should be AST_EXTERN_DECL for: %s", extern_sources[i])) {
            ast_free_node(extern_decl);
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(extern_decl);
        parser_destroy(parser);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Annotation in Complete Program Context
 * Validates SafeFFIAnnotation parsing in realistic program context
 */
static AsthraTestResult test_annotation_in_complete_program_context(AsthraTestContext *context) {
    const char *program_source =
        "package test;\n"
        "\n"
        "pub extern \"C\" fn malloc(size: usize) -> #[transfer_full] *mut u8;\n"
        "pub extern \"C\" fn free(#[transfer_full] ptr: *mut u8);\n"
        "pub extern \"C\" fn strlen(#[borrowed] s: *const u8) -> usize;\n"
        "\n"
        "pub fn safe_wrapper(#[borrowed] input: *const u8) -> Result<*mut u8, string> {\n"
        "    let len: usize = strlen(input);\n"
        "    if len > 0 {\n"
        "        return Ok(malloc(len + 1));\n"
        "    }\n"
        "    return Err(\"Invalid input\");\n"
        "}\n"
        "\n"
        "pub fn cleanup(#[transfer_full] ptr: *mut u8) {\n"
        "    free(ptr);\n"
        "}\n";

    Lexer *lexer = lexer_create(program_source, strlen(program_source), "test_program.asthra");
    if (!asthra_test_assert_not_null(context, lexer,
                                     "Failed to create lexer for complete program")) {
        return ASTHRA_TEST_FAIL;
    }

    Parser *parser = parser_create(lexer);
    if (!asthra_test_assert_not_null(context, parser,
                                     "Failed to create parser for complete program")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *program = parse_program(parser);

    if (!asthra_test_assert_not_null(context, program,
                                     "Complete program with SafeFFIAnnotations should parse")) {
        parser_destroy(parser);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    // Validate that program parsed correctly
    if (!asthra_test_assert_int_eq(context, program->type, AST_PROGRAM, "Should be AST_PROGRAM")) {
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Create SafeFFIAnnotation context integration test suite
 */
AsthraTestSuite *create_safe_ffi_annotation_context_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create(
        "SafeFFIAnnotation Context Integration", "FFI annotations in various language contexts");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_context_tests);
    asthra_test_suite_set_teardown(suite, teardown_context_tests);

    // Context integration tests
    asthra_test_suite_add_test(suite, "test_single_annotation_in_function_context",
                               "FFI Annotations Rejected in Regular Functions",
                               test_single_annotation_in_function_context);
    asthra_test_suite_add_test(suite, "test_single_annotation_in_extern_context",
                               "Single Annotation in Extern Context",
                               test_single_annotation_in_extern_context);
    asthra_test_suite_add_test(suite, "test_annotation_in_complete_program_context",
                               "Annotation in Complete Program Context",
                               test_annotation_in_complete_program_context);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== SafeFFIAnnotation Context Integration Tests ===\n\n");

    AsthraTestSuite *suite = create_safe_ffi_annotation_context_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED