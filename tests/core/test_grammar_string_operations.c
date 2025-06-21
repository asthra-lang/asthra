/**
 * Asthra Programming Language String Operations Syntax Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for string operations syntax parsing, including concatenation
 * and edge case parsing.
 */

#include "../framework/test_framework.h"
#include "test_grammar_helpers.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// STRING OPERATIONS SYNTAX TESTS
// =============================================================================

AsthraTestResult test_string_concatenation_syntax(AsthraTestContext *ctx) {
    // Test string concatenation syntax parsing

    // Mock test cases for string concatenation
    const char *concat_tests[] = {
        "let combined = \"Hello\" + \" \" + \"World\";",  // Basic string concatenation
        "let message = name + \": \" + greeting;",        // Variable concatenation
        "let report = prefix + *dynamic_str + suffix;",   // Dereference in concatenation
        "let path = dir + \"/\" + file + \".\" + ext;",   // Multiple concatenation
        "let multiline = \"line1\" +\n\"line2\";",        // Multiline concatenation
        NULL
    };

    for (int i = 0; concat_tests[i] != NULL; i++) {
        // Mock concatenation parsing
        bool syntax_valid = grammar_test_parse_expression(concat_tests[i]);
        bool type_valid = grammar_test_validate_disambiguation(concat_tests[i]);

        if (!ASTHRA_TEST_ASSERT(ctx, syntax_valid,
                               "Invalid string concatenation syntax: %s", concat_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(ctx, type_valid,
                               "Invalid types in string concatenation: %s", concat_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

// String interpolation tests removed - feature deprecated for AI generation efficiency

AsthraTestResult test_string_edge_case_parsing(AsthraTestContext *ctx) {
    // Test string edge case parsing

    // Mock test cases for string edge cases
    const char *edge_case_tests[] = {
        "let escape = \"\\{not_interpolated}\";",           // Escaped braces (now just literal)
        "let escaped_quote = \"\\\"quoted\\\"\";",          // Escaped quotes
        "let mixed = \"literal content\";",                 // Simple literal content
        "let raw = r\"raw string \\n {no_interp}\";",       // Raw string
        NULL
    };

    for (int i = 0; edge_case_tests[i] != NULL; i++) {
        // Mock edge case parsing
        bool syntax_valid = grammar_test_parse_expression(edge_case_tests[i]);
        bool handled_correctly = grammar_test_validate_disambiguation(edge_case_tests[i]);

        if (!ASTHRA_TEST_ASSERT(ctx, syntax_valid,
                               "Invalid string edge case syntax: %s", edge_case_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(ctx, handled_correctly,
                               "Edge case not handled correctly: %s", edge_case_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
} 
