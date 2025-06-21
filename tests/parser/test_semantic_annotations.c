/**
 * Asthra Programming Language
 * Semantic Annotations Parsing Tests
 * 
 * Tests for semantic annotations including human review, ownership, transfer,
 * security, and lifetime annotations.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "grammar_annotations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_semantic_annotation_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_semantic_annotation_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// SEMANTIC ANNOTATIONS TESTS
// =============================================================================

/**
 * Test: Parse Human Review Annotation
 * Verifies that #[human_review] annotations are parsed correctly
 */
static AsthraTestResult test_parse_human_review_annotation(AsthraTestContext* context) {
    const char* test_source = "#[validate(min=0, max=100)] pub fn critical_function(value: i32) -> void {}";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse annotated function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify function has annotations
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Ownership Annotation
 * Verifies that #[ownership] annotations are parsed correctly
 */
static AsthraTestResult test_parse_ownership_annotation(AsthraTestContext* context) {
    const char* test_source = "#[doc(description=\"Function\", version=\"1.0\")] pub fn allocate_memory(none) -> *mut void {}";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse ownership annotated function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have ownership annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Transfer Annotation
 * Verifies that #[transfer] annotations are parsed correctly
 */
static AsthraTestResult test_parse_transfer_annotation(AsthraTestContext* context) {
    const char* test_source = "#[deprecated(since=\"1.0\")] pub fn borrow_data(data: *const u8) -> void {}";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse transfer annotated function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have transfer annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Security Annotation
 * Verifies that #[security] annotations are parsed correctly
 */
static AsthraTestResult test_parse_security_annotation(AsthraTestContext* context) {
    const char* test_source = "#[range(min=0, max=1)] pub fn crypto_compare(a: *mut u8, b: *mut u8) -> bool {}";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse security annotated function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have security annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Lifetime Annotation
 * Verifies that #[lifetime] annotations are parsed correctly
 */
static AsthraTestResult test_parse_lifetime_annotation(AsthraTestContext* context) {
    const char* test_source = "#[config(enabled=true, debug=false)] pub fn get_reference(data: *const i32) -> *const i32 {}";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse lifetime annotated function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have lifetime annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// Note: Deprecated annotation test removed - deprecated annotations are no longer supported

/**
 * Test: Parse Documentation Annotation
 * Verifies that #[doc] annotations are parsed correctly
 */
static AsthraTestResult test_parse_documentation_annotation(AsthraTestContext* context) {
    const char* test_source = "#[timeout(seconds=30, retries=3)] pub fn calculate(none) -> void {}";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse documentation annotated function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have documentation annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Performance Annotation
 * Verifies that #[performance] annotations are parsed correctly
 */
static AsthraTestResult test_parse_performance_annotation(AsthraTestContext* context) {
    const char* test_source = "#[cache(type=\"LRU\", size=1000, ttl=3600)] pub fn critical_loop(none) -> void {}";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse performance annotated function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have performance annotations")) {
        ast_free_node(result);
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
 * Register all semantic annotation tests
 */
AsthraTestSuite* create_semantic_annotations_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Semantic Annotation Tests", 
                                                     "Semantic annotation parsing testing");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_semantic_annotation_tests);
    asthra_test_suite_set_teardown(suite, teardown_semantic_annotation_tests);
    
    // Semantic annotations
    asthra_test_suite_add_test(suite, "test_parse_human_review_annotation", 
                              "Parse human review annotations", 
                              test_parse_human_review_annotation);
    
    asthra_test_suite_add_test(suite, "test_parse_ownership_annotation", 
                              "Parse ownership annotations", 
                              test_parse_ownership_annotation);
    
    asthra_test_suite_add_test(suite, "test_parse_transfer_annotation", 
                              "Parse transfer annotations", 
                              test_parse_transfer_annotation);
    
    asthra_test_suite_add_test(suite, "test_parse_security_annotation", 
                              "Parse security annotations", 
                              test_parse_security_annotation);
    
    asthra_test_suite_add_test(suite, "test_parse_lifetime_annotation", 
                              "Parse lifetime annotations", 
                              test_parse_lifetime_annotation);
    
    // Note: Deprecated annotation test removed - deprecated annotations are no longer supported
    
    asthra_test_suite_add_test(suite, "test_parse_documentation_annotation", 
                              "Parse documentation annotations", 
                              test_parse_documentation_annotation);
    
    asthra_test_suite_add_test(suite, "test_parse_performance_annotation", 
                              "Parse performance annotations", 
                              test_parse_performance_annotation);
    
    return suite;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite* suite = create_semantic_annotations_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create semantic annotations test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return result;
} 
