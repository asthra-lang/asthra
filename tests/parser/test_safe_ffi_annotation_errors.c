/**
 * Asthra Programming Language
 * SafeFFIAnnotation Parser - Error Handling Tests
 *
 * Tests error handling and recovery for invalid FFI annotations
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

static AsthraTestResult setup_error_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_error_tests(AsthraTestContext *context) {
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

// =============================================================================
// ERROR HANDLING TESTS
// =============================================================================

/**
 * Test: Reject Invalid Annotation Names
 * Ensures invalid FFI annotation names are rejected
 */
static AsthraTestResult test_reject_invalid_annotation_names(AsthraTestContext *context) {
    const char *invalid_annotations[] = {
        "#[invalid_annotation]",
        "#[transfer_invalid]",
        "#[borrow]",   // Should be "borrowed"
        "#[transfer]", // Incomplete
        "#[full]",     // Incomplete
        "#[none]"      // Incomplete
    };

    size_t count = sizeof(invalid_annotations) / sizeof(invalid_annotations[0]);

    for (size_t i = 0; i < count; i++) {
        ASTNode *annotation = parse_ffi_annotation_from_source(invalid_annotations[i]);

        if (!asthra_test_assert_null(context, annotation,
                                     "Invalid annotation should be rejected: %s",
                                     invalid_annotations[i])) {
            if (annotation)
                ast_free_node(annotation);
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Reject Malformed Annotation Syntax
 * Ensures malformed annotation syntax is rejected
 */
static AsthraTestResult test_reject_malformed_annotation_syntax(AsthraTestContext *context) {
    const char *malformed_annotations[] = {
        "[transfer_full]", // Missing #
        "#transfer_full]", // Missing [
        "#[transfer_full", // Missing ]
        "#[]"              // Empty annotation
    };

    size_t count = sizeof(malformed_annotations) / sizeof(malformed_annotations[0]);

    for (size_t i = 0; i < count; i++) {
        ASTNode *annotation = parse_ffi_annotation_from_source(malformed_annotations[i]);

        if (!asthra_test_assert_null(context, annotation,
                                     "Malformed annotation should be rejected: %s",
                                     malformed_annotations[i])) {
            if (annotation)
                ast_free_node(annotation);
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parser Error Recovery
 * Validates that parser handles errors gracefully and recovers
 */
static AsthraTestResult test_parser_error_recovery(AsthraTestContext *context) {
    const char *error_sources[] = {
        "#[invalid_ffi_annotation]", "#[transfer_invalid]", "#[borrowed extra_content]",
        "#[transfer_full", // Incomplete
        "[transfer_none]"  // Malformed
    };

    size_t count = sizeof(error_sources) / sizeof(error_sources[0]);

    for (size_t i = 0; i < count; i++) {
        Lexer *lexer = lexer_create(error_sources[i], strlen(error_sources[i]), "test");
        if (!asthra_test_assert_not_null(context, lexer, "Failed to create lexer for error test")) {
            return ASTHRA_TEST_FAIL;
        }

        Parser *parser = parser_create(lexer);
        if (!asthra_test_assert_not_null(context, parser,
                                         "Failed to create parser for error test")) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *annotation = parse_safe_ffi_annotation(parser);

        // Should return NULL for invalid input
        if (!asthra_test_assert_null(context, annotation, "Parser should reject invalid input: %s",
                                     error_sources[i])) {
            if (annotation)
                ast_free_node(annotation);
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        // Parser should still be in valid state for error recovery
        if (!asthra_test_assert_not_null(context, parser,
                                         "Parser should remain valid after error")) {
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        parser_destroy(parser);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Create SafeFFIAnnotation error handling test suite
 */
AsthraTestSuite *create_safe_ffi_annotation_error_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("SafeFFIAnnotation Error Handling",
                                 "Error handling and recovery for invalid FFI annotations");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_error_tests);
    asthra_test_suite_set_teardown(suite, teardown_error_tests);

    // Error handling tests
    asthra_test_suite_add_test(suite, "test_reject_invalid_annotation_names",
                               "Reject Invalid Annotation Names",
                               test_reject_invalid_annotation_names);
    asthra_test_suite_add_test(suite, "test_reject_malformed_annotation_syntax",
                               "Reject Malformed Annotation Syntax",
                               test_reject_malformed_annotation_syntax);
    asthra_test_suite_add_test(suite, "test_parser_error_recovery", "Parser Error Recovery",
                               test_parser_error_recovery);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== SafeFFIAnnotation Error Handling Tests ===\n\n");

    AsthraTestSuite *suite = create_safe_ffi_annotation_error_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED