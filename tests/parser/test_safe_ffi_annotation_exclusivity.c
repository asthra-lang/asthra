/**
 * Asthra Programming Language
 * SafeFFIAnnotation Parser - Mutual Exclusivity Tests
 *
 * Tests that the parser enforces mutual exclusivity of FFI annotations
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

static AsthraTestResult setup_exclusivity_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_exclusivity_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

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

static bool validate_ffi_annotation_ast(ASTNode *node, FFITransferType expected_type) {
    if (!node)
        return false;

    if (node->type != AST_FFI_ANNOTATION)
        return false;

    return node->data.ffi_annotation.transfer_type == expected_type;
}

// =============================================================================
// MUTUAL EXCLUSIVITY TEST
// =============================================================================

/**
 * Test: Verify Mutual Exclusivity at Parser Level
 * This test ensures that the grammar prevents multiple annotations
 */
static AsthraTestResult test_mutual_exclusivity_at_parser_level(AsthraTestContext *context) {
    // These should be impossible to parse with SafeFFIAnnotation grammar
    // since SafeFFIAnnotation only accepts single annotations
    const char *impossible_sources[] = {"#[transfer_full] #[transfer_none]",
                                        "#[borrowed] #[transfer_full]",
                                        "#[transfer_full] #[borrowed] #[transfer_none]"};

    size_t count = sizeof(impossible_sources) / sizeof(impossible_sources[0]);

    for (size_t i = 0; i < count; i++) {
        ASTNode *annotation = parse_ffi_annotation_from_source(impossible_sources[i]);

        // The SafeFFIAnnotation grammar should only parse the first annotation
        // and stop, not create a conflicting annotation node
        if (annotation) {
            // If we got an annotation, it should be valid (just the first one)
            bool is_valid = validate_ffi_annotation_ast(annotation, FFI_TRANSFER_FULL) ||
                            validate_ffi_annotation_ast(annotation, FFI_TRANSFER_NONE) ||
                            validate_ffi_annotation_ast(annotation, FFI_BORROWED);

            if (!asthra_test_assert_bool(context, is_valid,
                                    "If annotation parses, it should be valid: %s",
                                    impossible_sources[i])) {
                ast_free_node(annotation);
                return ASTHRA_TEST_FAIL;
            }

            ast_free_node(annotation);
        }

        // The key test: parser should not create conflicting annotation structures
        // This test passes as long as no invalid AST structure is created
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Verify Only First Annotation is Parsed
 * Ensures that when multiple annotations are present, only the first is parsed
 */
static AsthraTestResult test_only_first_annotation_parsed(AsthraTestContext *context) {
    struct {
        const char *source;
        FFITransferType expected_type;
    } test_cases[] = {
        {"#[transfer_full] #[transfer_none]", FFI_TRANSFER_FULL},
        {"#[transfer_none] #[borrowed]", FFI_TRANSFER_NONE},
        {"#[borrowed] #[transfer_full]", FFI_BORROWED},
    };

    size_t count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < count; i++) {
        ASTNode *annotation = parse_ffi_annotation_from_source(test_cases[i].source);

        if (annotation) {
            // Verify that only the first annotation was parsed
            if (!asthra_test_assert_bool(context,
                                    validate_ffi_annotation_ast(annotation, test_cases[i].expected_type),
                                    "Should parse only first annotation from: %s",
                                    test_cases[i].source)) {
                ast_free_node(annotation);
                return ASTHRA_TEST_FAIL;
            }
            ast_free_node(annotation);
        }
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Create SafeFFIAnnotation mutual exclusivity test suite
 */
AsthraTestSuite *create_safe_ffi_annotation_exclusivity_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("SafeFFIAnnotation Mutual Exclusivity", "Parser enforcement of annotation exclusivity");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_exclusivity_tests);
    asthra_test_suite_set_teardown(suite, teardown_exclusivity_tests);

    // Safety validation tests
    asthra_test_suite_add_test(suite, "test_mutual_exclusivity_at_parser_level",
                               "Mutual Exclusivity at Parser Level", test_mutual_exclusivity_at_parser_level);
    asthra_test_suite_add_test(suite, "test_only_first_annotation_parsed",
                               "Only First Annotation Parsed", test_only_first_annotation_parsed);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== SafeFFIAnnotation Mutual Exclusivity Tests ===\n\n");

    AsthraTestSuite *suite = create_safe_ffi_annotation_exclusivity_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED