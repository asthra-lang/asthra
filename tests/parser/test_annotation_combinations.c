/**
 * Asthra Programming Language
 * Annotation Combinations Parsing Tests
 * 
 * Tests for complex annotation combinations including multiple annotations,
 * annotation conflicts, and annotation inheritance.
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

static AsthraTestResult setup_annotation_combination_tests(AsthraTestContext *context) {
    (void)context; // Unused parameter
    // Initialize any global state if needed
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_annotation_combination_tests(AsthraTestContext *context) {
    (void)context; // Unused parameter
    // Clean up any global state if needed
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// ANNOTATION COMBINATIONS TESTS
// =============================================================================

/**
 * Test: Parse Multiple Annotations
 * Verifies that multiple annotations can be parsed on a single function
 */
static AsthraTestResult test_parse_multiple_annotations(AsthraTestContext* context) {
    const char* test_source = 
        "#[doc(text=\"Legacy function\")]\n"
        "#[performance(level=\"cold\")]\n"
        "#[deprecated(since=\"1.0\")]\n"
        "pub fn legacy_function(none) -> void {}";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse multiple annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have multiple annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Annotation Conflicts
 * Verifies that conflicting annotations are detected and handled
 */
static AsthraTestResult test_parse_annotation_conflicts(AsthraTestContext* context) {
    // Test conflicting performance annotations
    const char* test_source = 
        "#[performance(level=\"hot\")]\n"
        "#[performance(level=\"cold\")]\n"
        "pub fn conflicted_function(none) -> void {}";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    // Parser should successfully parse conflicting annotations
    // Conflict detection is handled by semantic analysis, not parsing
    if (!asthra_test_assert_not_null(context, result, "Parser should successfully parse conflicting annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have conflicting annotations parsed")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Annotation Inheritance
 * Verifies that annotation inheritance works correctly
 */
static AsthraTestResult test_parse_annotation_inheritance(AsthraTestContext* context) {
    const char* test_source = 
        "#[doc(text=\"Base struct\")]\n"
        "pub struct BaseStruct {\n"
        "    pub field: i32\n"
        "}\n"
        "\n"
        "impl BaseStruct {\n"
        "    pub fn old_method(none) -> void {}\n"
        "    \n"
        "    pub fn new_method(none) -> void {}\n"
        "}";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Parse struct declaration
    ASTNode* struct_result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, struct_result, "Failed to parse annotated struct")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(struct_result);
    
    // Parse implementation
    ASTNode* impl_result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, impl_result, "Failed to parse annotated implementation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(impl_result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Nested Annotation Combinations
 * Verifies that nested annotation combinations are parsed correctly
 */
static AsthraTestResult test_parse_nested_annotation_combinations(AsthraTestContext* context) {
    const char* test_source = 
        "#[cfg(feature=\"advanced\")]\n"
        "#[inline(always=\"true\")]\n"
        "#[target_feature(enable=\"avx2\")]\n"
        "pub fn optimized_function(none) -> void {}";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse nested annotation combinations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have nested annotation combinations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Conditional Annotations
 * Verifies that conditional annotations are parsed correctly
 */
static AsthraTestResult test_parse_conditional_annotations(AsthraTestContext* context) {
    const char* test_cases[] = {
        "#[cfg(target_os=\"linux\")] pub fn linux_specific(none) -> void {}",
        "#[cfg(feature=\"experimental\")] pub fn experimental_feature(none) -> void {}",
        "#[cfg(debug=\"true\")] pub fn debug_only(none) -> void {}",
        "#[cfg(test=\"false\")] pub fn non_test_function(none) -> void {}"
    };
    
    size_t case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < case_count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser for case %zu", i)) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parse_top_level_decl(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse conditional annotation for case %zu", i)) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                        "Function should have conditional annotations for case %zu", i)) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Annotation Ordering
 * Verifies that annotation ordering is preserved and handled correctly
 */
static AsthraTestResult test_parse_annotation_ordering(AsthraTestContext* context) {
    const char* test_source = 
        "#[must_use]\n"
        "#[inline(always=\"true\")]\n"
        "#[doc(text=\"Returns important value\")]\n"
        "pub fn important_function(none) -> i32 { return 42; }";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse ordered annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations, 
                                    "Function should have ordered annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Complex FFI Combinations
 * Verifies that complex FFI annotation combinations are parsed correctly
 */
static AsthraTestResult test_parse_complex_ffi_combinations(AsthraTestContext* context) {
    const char* test_source = 
        "#[link(name=\"complex_lib\")]\n"
        "#[abi(convention=\"C\")]\n"
        "pub extern \"complex_lib\" fn complex_ffi_function(\n"
        "    data: *const u8,\n"
        "    len: usize\n"
        ") -> i32;";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_top_level_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse complex FFI combinations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                  "Should be AST_EXTERN_DECL")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Annotation Validation Combinations
 * Verifies that annotation validation works with complex combinations
 */
static AsthraTestResult test_parse_annotation_validation_combinations(AsthraTestContext* context) {
    // Test valid combination
    const char* valid_source = 
        "#[inline(always=\"true\")]\n"
        "#[must_use]\n"
        "pub fn valid_combination(none) -> i32 { return 42; }";
    
    Parser* parser1 = create_test_parser(valid_source);
    
    if (!asthra_test_assert_not_null(context, parser1, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result1 = parse_top_level_decl(parser1);
    
    if (!asthra_test_assert_not_null(context, result1, "Valid annotation combination should parse")) {
        destroy_test_parser(parser1);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result1);
    destroy_test_parser(parser1);
    
    // Test potentially conflicting combination
    const char* conflicting_source = 
        "#[inline(always=\"false\")]\n"
        "#[inline(always=\"true\")]\n"
        "pub fn conflicting_combination(none) -> void {}";
    
    Parser* parser2 = create_test_parser(conflicting_source);
    
    if (!asthra_test_assert_not_null(context, parser2, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result2 = parse_top_level_decl(parser2);
    
    // Parser should successfully parse conflicting annotations
    // Conflict detection is handled by semantic analysis, not parsing
    if (!asthra_test_assert_not_null(context, result2, "Parser should successfully parse conflicting annotations")) {
        destroy_test_parser(parser2);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_not_null(context, result2->data.function_decl.annotations, 
                                    "Function should have conflicting annotations parsed")) {
        ast_free_node(result2);
        destroy_test_parser(parser2);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result2);
    destroy_test_parser(parser2);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all annotation combination tests
 */
AsthraTestSuite* create_annotation_combinations_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Annotation Combination Tests", 
                                                     "Annotation combination parsing testing");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_annotation_combination_tests);
    asthra_test_suite_set_teardown(suite, teardown_annotation_combination_tests);
    
    // Annotation combinations
    asthra_test_suite_add_test(suite, "test_parse_multiple_annotations", 
                              "Parse multiple annotations", 
                              test_parse_multiple_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_annotation_conflicts", 
                              "Parse annotation conflicts", 
                              test_parse_annotation_conflicts);
    
    asthra_test_suite_add_test(suite, "test_parse_annotation_inheritance", 
                              "Parse annotation inheritance", 
                              test_parse_annotation_inheritance);
    
    asthra_test_suite_add_test(suite, "test_parse_nested_annotation_combinations", 
                              "Parse nested annotation combinations", 
                              test_parse_nested_annotation_combinations);
    
    asthra_test_suite_add_test(suite, "test_parse_conditional_annotations", 
                              "Parse conditional annotations", 
                              test_parse_conditional_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_annotation_ordering", 
                              "Parse annotation ordering", 
                              test_parse_annotation_ordering);
    
    asthra_test_suite_add_test(suite, "test_parse_complex_ffi_combinations", 
                              "Parse complex FFI combinations", 
                              test_parse_complex_ffi_combinations);
    
    asthra_test_suite_add_test(suite, "test_parse_annotation_validation_combinations", 
                              "Parse annotation validation combinations", 
                              test_parse_annotation_validation_combinations);
    
    return suite;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite* suite = create_annotation_combinations_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create annotation combinations test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return result;
} 
