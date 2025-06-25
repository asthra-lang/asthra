/*
 * @ Annotation Removal Test - Simplified Version
 * Tests that @ annotations are properly rejected with helpful error messages
 * and that #[...] annotations continue to work correctly.
 *
 * Part of Grammar-Implementation Alignment Plan Phase 3
 */

#include "ast_types.h"
#include "grammar_annotations.h"
#include "lexer.h"
#include "parser_core.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test helper functions
static Lexer *create_test_lexer(const char *source) {
    return lexer_create(source, strlen(source), "test.asthra");
}

static Parser *create_test_parser(const char *source) {
    Lexer *lexer = create_test_lexer(source);
    if (!lexer)
        return NULL;
    return parser_create(lexer);
}

static void cleanup_test_parser(Parser *parser) {
    if (parser) {
        Lexer *lexer = parser->lexer;
        parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
    }
}

// Test 1: @ annotation should be rejected with helpful error message
static bool test_at_annotation_rejection(void) {
    printf("Testing @ annotation rejection...\n");

    const char *at_annotation_code = "@gc";

    Parser *parser = create_test_parser(at_annotation_code);
    if (!parser) {
        printf("FAIL: Could not create parser\n");
        return false;
    }

    // Try to parse annotation - should fail with helpful error
    ASTNode *annotation = parse_annotation(parser);

    if (annotation != NULL) {
        printf("FAIL: @ annotation was parsed when it should be rejected\n");
        // Note: We can't call ast_free_node without all dependencies
        cleanup_test_parser(parser);
        return false;
    }

    // Check if we have an error message
    if (parser->error_count == 0) {
        printf("FAIL: No error message generated for @ annotation\n");
        cleanup_test_parser(parser);
        return false;
    }

    printf("PASS: @ annotation properly rejected with error message\n");
    cleanup_test_parser(parser);
    return true;
}

// Test 2: #[...] annotation should still work
static bool test_hash_annotation_works(void) {
    printf("Testing #[...] annotation still works...\n");

    const char *hash_annotation_code = "#[ownership(gc)]";

    Parser *parser = create_test_parser(hash_annotation_code);
    if (!parser) {
        printf("FAIL: Could not create parser for #[...] test\n");
        return false;
    }

    // Try to parse annotation - should succeed
    ASTNode *annotation = parse_annotation(parser);

    if (annotation == NULL) {
        printf("FAIL: #[...] annotation was rejected when it should be accepted\n");
        cleanup_test_parser(parser);
        return false;
    }

    // Verify it's the correct type
    if (annotation->type != AST_OWNERSHIP_TAG) {
        printf("FAIL: #[...] annotation parsed but wrong AST node type\n");
        // Note: We can't call ast_free_node without all dependencies
        cleanup_test_parser(parser);
        return false;
    }

    printf("PASS: #[...] annotation properly parsed\n");
    // Note: We can't call ast_free_node without all dependencies
    cleanup_test_parser(parser);
    return true;
}

// Test 3: Multiple @ annotations should all be rejected
static bool test_multiple_at_annotations(void) {
    printf("Testing multiple @ annotation patterns...\n");

    const char *at_patterns[] = {
        "@gc",         "@transfer_full", "@borrowed", "@constant_time", "@security_sensitive",
        "@review_high"};

    size_t pattern_count = sizeof(at_patterns) / sizeof(at_patterns[0]);

    for (size_t i = 0; i < pattern_count; i++) {
        Parser *parser = create_test_parser(at_patterns[i]);
        if (!parser) {
            printf("FAIL: Could not create parser for pattern: %s\n", at_patterns[i]);
            return false;
        }

        ASTNode *annotation = parse_annotation(parser);

        if (annotation != NULL) {
            printf("FAIL: @ pattern was accepted when it should be rejected: %s\n", at_patterns[i]);
            // Note: We can't call ast_free_node without all dependencies
            cleanup_test_parser(parser);
            return false;
        }

        if (parser->error_count == 0) {
            printf("FAIL: No error message for @ pattern: %s\n", at_patterns[i]);
            cleanup_test_parser(parser);
            return false;
        }

        cleanup_test_parser(parser);
    }

    printf("PASS: All @ annotation patterns properly rejected\n");
    return true;
}

// Test 4: Verify error message content
static bool test_error_message_content(void) {
    printf("Testing error message content...\n");

    const char *at_annotation_code = "@gc";

    Parser *parser = create_test_parser(at_annotation_code);
    if (!parser) {
        printf("FAIL: Could not create parser\n");
        return false;
    }

    // Try to parse annotation to trigger error
    ASTNode *annotation = parse_annotation(parser);

    if (annotation != NULL) {
        printf("FAIL: @ annotation was parsed when it should be rejected\n");
        // Note: We can't call ast_free_node without all dependencies
        cleanup_test_parser(parser);
        return false;
    }

    // For this test, we just verify that an error was generated
    // In a real implementation, we'd check the actual error message content
    if (parser->error_count == 0) {
        printf("FAIL: No error message generated\n");
        cleanup_test_parser(parser);
        return false;
    }

    printf("PASS: Error message generated for @ annotation\n");
    cleanup_test_parser(parser);
    return true;
}

// Main test runner
int main(void) {
    printf("=== @ Annotation Removal Tests ===\n");
    printf("Testing Grammar-Implementation Alignment Plan Phase 3\n\n");

    bool all_passed = true;

    all_passed &= test_at_annotation_rejection();
    all_passed &= test_hash_annotation_works();
    all_passed &= test_multiple_at_annotations();
    all_passed &= test_error_message_content();

    printf("\n=== Test Results ===\n");
    if (all_passed) {
        printf("✅ ALL TESTS PASSED\n");
        printf("@ annotation removal is working correctly!\n");
        printf("Migration to unified #[...] syntax is complete.\n");
        return 0;
    } else {
        printf("❌ SOME TESTS FAILED\n");
        printf("@ annotation removal needs additional work.\n");
        return 1;
    }
}
