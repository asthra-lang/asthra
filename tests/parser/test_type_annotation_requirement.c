/**
 * Asthra Programming Language Compiler
 * Type Annotation Requirement Tests (v1.15+)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for the mandatory type annotation requirement introduced in v1.15.
 * All variable declarations must now include explicit type annotations.
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "grammar_statements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_type_annotation_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_type_annotation_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// POSITIVE TESTS - VALID TYPE ANNOTATIONS
// =============================================================================

/**
 * Test: Valid Type Annotations
 * Verifies that variable declarations with explicit type annotations parse correctly
 */
static AsthraTestResult test_valid_type_annotations(AsthraTestContext* context) {
    const char* valid_declarations[] = {
        "let x: i32 = 42;",
        "let name: string = \"hello\";",
        "let is_valid: bool = true;",
        "let pi: f64 = 3.14159;",
        "let count: usize = 100;",
        "let result: Result<i32, string> = Result.Ok(42);"
    };
    
    size_t decl_count = sizeof(valid_declarations) / sizeof(valid_declarations[0]);
    
    for (size_t i = 0; i < decl_count; i++) {
        Parser* parser = create_test_parser(valid_declarations[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parse_var_decl(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse valid type annotation")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        // Verify that type is not NULL
        if (!asthra_test_assert_not_null(context, result->data.let_stmt.type, "Type should not be NULL in v1.15+")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// NEGATIVE TESTS - MISSING TYPE ANNOTATIONS
// =============================================================================

/**
 * Test: Missing Type Annotations (Should Fail)
 * Verifies that variable declarations without type annotations are rejected in v1.15+
 */
static AsthraTestResult test_missing_type_annotations_rejected(AsthraTestContext* context) {
    const char* invalid_declarations[] = {
        "let x = 42;",           // Missing type annotation
        "let name = \"hello\";", // Missing type annotation
        "let is_valid = true;",  // Missing type annotation
        "let pi = 3.14159;",     // Missing type annotation
        "let items = [];",       // Missing type annotation
    };
    
    size_t decl_count = sizeof(invalid_declarations) / sizeof(invalid_declarations[0]);
    
    for (size_t i = 0; i < decl_count; i++) {
        Parser* parser = create_test_parser(invalid_declarations[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parse_var_decl(parser);
        
        // Should fail to parse due to missing type annotation
        if (!asthra_test_assert_null(context, result, "Should reject variable declaration without type annotation")) {
            if (result) ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        // Parser should have error state
        if (!asthra_test_assert_true(context, parser_has_error(parser), "Parser should have error state after rejecting invalid syntax")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Partial Type Annotation (Should Fail)
 * Verifies that incomplete type annotations are rejected
 */
static AsthraTestResult test_partial_type_annotations_rejected(AsthraTestContext* context) {
    const char* invalid_declarations[] = {
        "let x: = 42;",       // Missing type after colon
        "let name: ;",        // Missing type and value
        "let x: i32",         // Missing assignment and semicolon
        "let x:",             // Missing type, assignment, and semicolon
    };
    
    size_t decl_count = sizeof(invalid_declarations) / sizeof(invalid_declarations[0]);
    
    for (size_t i = 0; i < decl_count; i++) {
        Parser* parser = create_test_parser(invalid_declarations[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parse_var_decl(parser);
        
        // Should fail to parse due to incomplete type annotation
        if (!asthra_test_assert_null(context, result, "Should reject incomplete type annotation")) {
            if (result) ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MIGRATION COMPATIBILITY TESTS
// =============================================================================

/**
 * Test: v1.14 vs v1.15 Compatibility
 * Demonstrates the breaking change from optional to mandatory type annotations
 */
static AsthraTestResult test_version_compatibility(AsthraTestContext* context) {
    // Code that was valid in v1.14 but invalid in v1.15+
    const char* v1_14_code[] = {
        "let x = 42;",
        "let name = \"hello\";",
        "let items = [];"
    };
    
    // Equivalent valid code in v1.15+
    const char* v1_15_code[] = {
        "let x: i32 = 42;",
        "let name: string = \"hello\";",
        "let items: []i32 = [];"  // Array type annotation
    };
    
    size_t code_count = sizeof(v1_14_code) / sizeof(v1_14_code[0]);
    
    // Test that v1.14 code is rejected
    for (size_t i = 0; i < code_count; i++) {
        Parser* parser = create_test_parser(v1_14_code[i]);
        ASTNode* result = parse_var_decl(parser);
        
        if (!asthra_test_assert_null(context, result, "v1.14 syntax should be rejected in v1.15+")) {
            if (result) ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        destroy_test_parser(parser);
    }
    
    // Test that v1.15 code is accepted
    for (size_t i = 0; i < code_count; i++) {
        Parser* parser = create_test_parser(v1_15_code[i]);
        ASTNode* result = parse_var_decl(parser);
        
        if (!asthra_test_assert_not_null(context, result, "v1.15 syntax should be accepted")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Error Message Quality
 * Verifies that error messages provide helpful migration guidance
 */
static AsthraTestResult test_error_message_quality(AsthraTestContext* context) {
    const char* invalid_code = "let x = 42;";
    
    Parser* parser = create_test_parser(invalid_code);
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_var_decl(parser);
    
    // Should fail
    if (!asthra_test_assert_null(context, result, "Should reject missing type annotation")) {
        if (result) ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that helpful error message was generated
    // Note: This would require access to parser error messages
    // For now, just verify parser has error state
    if (!asthra_test_assert_true(context, parser_has_error(parser), "Parser should have error state with helpful message")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all type annotation requirement tests
 */
AsthraTestSuite* create_type_annotation_requirement_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Type Annotation Requirement Tests (v1.15+)", 
                                                     "Testing mandatory type annotation requirement");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_type_annotation_tests);
    asthra_test_suite_set_teardown(suite, teardown_type_annotation_tests);
    
    // Positive tests
    asthra_test_suite_add_test(suite, "test_valid_type_annotations", 
                              "Valid type annotations should parse correctly", 
                              test_valid_type_annotations);
    
    // Negative tests
    asthra_test_suite_add_test(suite, "test_missing_type_annotations_rejected", 
                              "Missing type annotations should be rejected", 
                              test_missing_type_annotations_rejected);
    
    asthra_test_suite_add_test(suite, "test_partial_type_annotations_rejected", 
                              "Partial type annotations should be rejected", 
                              test_partial_type_annotations_rejected);
    
    // Migration tests
    asthra_test_suite_add_test(suite, "test_version_compatibility", 
                              "v1.14 vs v1.15 compatibility differences", 
                              test_version_compatibility);
    
    asthra_test_suite_add_test(suite, "test_error_message_quality", 
                              "Error messages should provide migration guidance", 
                              test_error_message_quality);
    
    return suite;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite* suite = create_type_annotation_requirement_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
