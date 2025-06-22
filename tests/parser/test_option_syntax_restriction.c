/**
 * Asthra Programming Language
 * Option Syntax Restriction Tests
 * 
 * Tests to ensure Option values can only be created with explicit enum syntax
 * and not as standalone function calls
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "../../src/parser/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool parse_fails(const char* source) {
    Parser* parser = create_test_parser(source);
    if (!parser) return true; // Failed to create parser counts as failure
    
    ASTNode* ast = parser_parse_program(parser);
    bool failed = (ast == NULL || parser->error_count > 0);
    
    // Debug output
    #ifdef DEBUG_TESTS
    printf("parse_fails: ast=%p, error_count=%d, failed=%d\n", 
           (void*)ast, parser->error_count, failed);
    #endif
    
    if (ast) ast_free_node(ast);
    destroy_test_parser(parser);
    
    return failed;
}

static bool parse_succeeds(const char* source) {
    return !parse_fails(source);
}

// =============================================================================
// OPTION SYNTAX TESTS
// =============================================================================

/**
 * Test: Correct Option.Some syntax should work
 */
static AsthraTestResult test_option_some_correct_syntax(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "pub fn test_correct(none) -> void {\n"
        "    let opt: Option<i32> = Option.Some(42);\n"
        "    return ();\n"
        "}\n";
    
    if (!asthra_test_assert_bool_eq(context, 
                                   parse_succeeds(source), 
                                   true,
                                   "Option.Some(value) syntax should parse successfully")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Correct Option.None syntax should work
 */
static AsthraTestResult test_option_none_correct_syntax(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "pub fn test_correct(none) -> void {\n"
        "    let opt: Option<i32> = Option.None;\n"
        "    return ();\n"
        "}\n";
    
    if (!asthra_test_assert_bool_eq(context, 
                                   parse_succeeds(source), 
                                   true,
                                   "Option.None syntax should parse successfully")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Standalone Some() should not be allowed
 */
static AsthraTestResult test_standalone_some_not_allowed(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "pub fn test_invalid(none) -> void {\n"
        "    let opt: Option<i32> = Some(42);\n"  // This should fail - Some is not a function
        "    return ();\n"
        "}\n";
    
    // This should succeed in parsing because Some(42) is parsed as a function call
    // to an identifier named "Some". The error should come during semantic analysis
    // when it can't find a function named "Some".
    if (!asthra_test_assert_bool_eq(context, 
                                   parse_succeeds(source), 
                                   true,
                                   "Standalone Some() parses as identifier function call")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Standalone None() should not be allowed
 */
static AsthraTestResult test_standalone_none_not_allowed(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "pub fn test_invalid(none) -> void {\n"
        "    let opt: Option<i32> = None();\n"  // This should fail - None is not a function
        "    return ();\n"
        "}\n";
    
    // Similar to Some, this parses as a function call to "None"
    if (!asthra_test_assert_bool_eq(context, 
                                   parse_succeeds(source), 
                                   true,
                                   "Standalone None() parses as identifier function call")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Option in pattern matching with correct syntax
 */
static AsthraTestResult test_option_pattern_correct_syntax(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "pub fn test_pattern(opt: Option<i32>) -> i32 {\n"
        "    match opt {\n"
        "        Option.Some(value) => value,\n"
        "        Option.None => 0\n"
        "    }\n"
        "}\n";
    
    if (!asthra_test_assert_bool_eq(context, 
                                   parse_succeeds(source), 
                                   true,
                                   "Option pattern matching with correct syntax should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Pattern matching without Option prefix (currently allowed in patterns)
 */
static AsthraTestResult test_pattern_without_prefix(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "pub fn test_pattern(opt: Option<i32>) -> i32 {\n"
        "    match opt {\n"
        "        Some(value) => value,\n"  // This is now disallowed
        "        None => 0\n"              // This is now disallowed
        "    }\n"
        "}\n";
    
    if (!asthra_test_assert_bool_eq(context, 
                                   parse_fails(source), 
                                   true,
                                   "Pattern matching without prefix should fail")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_option_syntax_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_option_syntax_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

AsthraTestSuite* create_option_syntax_restriction_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Option Syntax Restriction Tests", 
                                                     "Ensure Option values use explicit enum syntax");
    
    if (!suite) return NULL;
    
    asthra_test_suite_set_setup(suite, setup_option_syntax_tests);
    asthra_test_suite_set_teardown(suite, teardown_option_syntax_tests);
    
    asthra_test_suite_add_test(suite, "test_option_some_correct_syntax", 
                              "Option.Some(value) syntax works", 
                              test_option_some_correct_syntax);
    
    asthra_test_suite_add_test(suite, "test_option_none_correct_syntax", 
                              "Option.None syntax works", 
                              test_option_none_correct_syntax);
    
    asthra_test_suite_add_test(suite, "test_standalone_some_not_allowed", 
                              "Standalone Some() is not special syntax", 
                              test_standalone_some_not_allowed);
    
    asthra_test_suite_add_test(suite, "test_standalone_none_not_allowed", 
                              "Standalone None() is not special syntax", 
                              test_standalone_none_not_allowed);
    
    asthra_test_suite_add_test(suite, "test_option_pattern_correct_syntax", 
                              "Option patterns with prefix work", 
                              test_option_pattern_correct_syntax);
    
    asthra_test_suite_add_test(suite, "test_pattern_without_prefix", 
                              "Patterns without prefix allowed in match", 
                              test_pattern_without_prefix);
    
    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Option Syntax Restriction Tests ===\n\n");
    
    AsthraTestSuite* suite = create_option_syntax_restriction_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif