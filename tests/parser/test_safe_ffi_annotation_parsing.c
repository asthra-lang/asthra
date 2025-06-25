/**
 * Asthra Programming Language
 * SafeFFIAnnotation Parser - Core Parsing Tests
 *
 * Tests basic parsing of individual FFI annotations
 * (transfer_full, transfer_none, borrowed)
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

static AsthraTestResult setup_parser_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_parser_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Create a parser from source and attempt to parse SafeFFIAnnotation
 */
static ASTNode *parse_ffi_annotation_from_source(const char *source) {
    if (!source)
        return NULL;

    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    ASTNode *annotation = parse_safe_ffi_annotation(parser);

    parser_destroy(parser);
    lexer_destroy(lexer);

    return annotation;
}

/**
 * Validate FFI annotation AST node structure
 */
static bool validate_ffi_annotation_ast(ASTNode *node, FFITransferType expected_type) {
    if (!node)
        return false;

    if (node->type != AST_FFI_ANNOTATION)
        return false;

    return node->data.ffi_annotation.transfer_type == expected_type;
}

// =============================================================================
// CORE PARSING TESTS
// =============================================================================

/**
 * Test: Parse Transfer Full Annotation
 * Validates parsing of #[transfer_full] annotation
 */
static AsthraTestResult test_parse_transfer_full_annotation(AsthraTestContext *context) {
    const char *source = "#[transfer_full]";

    ASTNode *annotation = parse_ffi_annotation_from_source(source);

    if (!asthra_test_assert_not_null(context, annotation,
                                     "Should parse transfer_full annotation")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, validate_ffi_annotation_ast(annotation, FFI_TRANSFER_FULL),
                            "Should create correct FFI_TRANSFER_FULL AST node")) {
        ast_free_node(annotation);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(annotation);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Transfer None Annotation
 * Validates parsing of #[transfer_none] annotation
 */
static AsthraTestResult test_parse_transfer_none_annotation(AsthraTestContext *context) {
    const char *source = "#[transfer_none]";

    ASTNode *annotation = parse_ffi_annotation_from_source(source);

    if (!asthra_test_assert_not_null(context, annotation,
                                     "Should parse transfer_none annotation")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, validate_ffi_annotation_ast(annotation, FFI_TRANSFER_NONE),
                            "Should create correct FFI_TRANSFER_NONE AST node")) {
        ast_free_node(annotation);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(annotation);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Borrowed Annotation
 * Validates parsing of #[borrowed] annotation
 */
static AsthraTestResult test_parse_borrowed_annotation(AsthraTestContext *context) {
    const char *source = "#[borrowed]";

    ASTNode *annotation = parse_ffi_annotation_from_source(source);

    if (!asthra_test_assert_not_null(context, annotation, "Should parse borrowed annotation")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, validate_ffi_annotation_ast(annotation, FFI_BORROWED),
                            "Should create correct FFI_BORROWED AST node")) {
        ast_free_node(annotation);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(annotation);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Create SafeFFIAnnotation core parsing test suite
 */
AsthraTestSuite *create_safe_ffi_annotation_parsing_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("SafeFFIAnnotation Core Parsing", "Basic parsing of FFI annotations");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_parser_tests);
    asthra_test_suite_set_teardown(suite, teardown_parser_tests);

    // Core annotation parsing tests
    asthra_test_suite_add_test(suite, "test_parse_transfer_full_annotation",
                               "Parse Transfer Full Annotation", test_parse_transfer_full_annotation);
    asthra_test_suite_add_test(suite, "test_parse_transfer_none_annotation",
                               "Parse Transfer None Annotation", test_parse_transfer_none_annotation);
    asthra_test_suite_add_test(suite, "test_parse_borrowed_annotation",
                               "Parse Borrowed Annotation", test_parse_borrowed_annotation);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== SafeFFIAnnotation Core Parsing Tests ===\n\n");

    AsthraTestSuite *suite = create_safe_ffi_annotation_parsing_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED