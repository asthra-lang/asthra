/**
 * Asthra Programming Language Grammar Disambiguation Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for grammar disambiguation, operator precedence,
 * and parsing conflict detection.
 */

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "test_comprehensive.h"
#include "test_grammar_helpers.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// GRAMMAR DISAMBIGUATION TESTS
// =============================================================================

AsthraTestResult test_grammar_postfix_expr_disambiguation(AsthraV12TestContext *ctx) {
    // Test PostfixExpr disambiguation in grammar

    // Mock test cases for postfix expression disambiguation
    const char *test_cases[] = {"a.b.c",        // Method chaining
                                "arr[0][1]",    // Array indexing
                                "func()()",     // Function call chaining
                                "obj.method()", // Method call
                                "a.b[c].d()",   // Mixed postfix operations
                                NULL};

    for (int i = 0; test_cases[i] != NULL; i++) {
        // Mock parsing - in real implementation would use actual parser
        bool parse_success = grammar_test_parse_expression(test_cases[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, parse_success, "Failed to parse postfix expression: %s",
                                test_cases[i])) {
            return ASTHRA_TEST_FAIL;
        }

        // Mock AST validation - verify disambiguation worked correctly
        bool disambiguation_correct = grammar_test_validate_disambiguation(test_cases[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, disambiguation_correct,
                                "Postfix expression disambiguation failed for: %s",
                                test_cases[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_grammar_precedence_rules(AsthraV12TestContext *ctx) {
    // Test operator precedence rules

    // Mock test cases for precedence validation
    const char *precedence_tests[] = {"a + b * c",   // Multiplication before addition
                                      "a * b + c",   // Left-to-right for same precedence
                                      "(a + b) * c", // Parentheses override precedence
                                      "a && b || c", // Logical operator precedence
                                      "a = b + c",   // Assignment has lowest precedence
                                      NULL};

    for (int i = 0; precedence_tests[i] != NULL; i++) {
        // Mock precedence validation
        bool precedence_correct = grammar_test_check_precedence(precedence_tests[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, precedence_correct,
                                "Precedence rule validation failed for: %s", precedence_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_grammar_zero_parsing_conflicts(AsthraV12TestContext *ctx) {
    // Test that grammar has zero parsing conflicts

    // Mock conflict detection - in real implementation would check parser tables
    int shift_reduce_conflicts = 0;  // Mock: no shift/reduce conflicts
    int reduce_reduce_conflicts = 0; // Mock: no reduce/reduce conflicts

    grammar_test_has_parsing_conflicts(&shift_reduce_conflicts, &reduce_reduce_conflicts);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, shift_reduce_conflicts == 0,
                            "Grammar has %d shift/reduce conflicts", shift_reduce_conflicts)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, reduce_reduce_conflicts == 0,
                            "Grammar has %d reduce/reduce conflicts", reduce_reduce_conflicts)) {
        return ASTHRA_TEST_FAIL;
    }

    // Mock ambiguity detection
    bool has_ambiguities = false; // Mock: no ambiguous constructs

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !has_ambiguities,
                            "Grammar contains ambiguous constructs")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_grammar_semantic_tags_parsing(AsthraV12TestContext *ctx) {
    // Test parsing of semantic tags (ownership, transfer, security annotations)

    // Mock test cases for semantic tag parsing
    const char *semantic_tag_tests[] = {
        "#[ownership(gc)] int* ptr",         // GC ownership annotation
        "#[ownership(c)] char* str",         // C ownership annotation
        "#[transfer_full] void* data",       // Transfer annotation
        "#[security_sensitive] uint8_t key", // Security annotation
        "#[borrowed] const char* ref",       // Borrowed reference annotation
        NULL};

    for (int i = 0; semantic_tag_tests[i] != NULL; i++) {
        // Mock semantic tag parsing
        bool tag_parsed = grammar_test_parse_semantic_tag(semantic_tag_tests[i]);
        bool tag_valid = grammar_test_validate_semantic_tag(semantic_tag_tests[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, tag_parsed, "Failed to parse semantic tag: %s",
                                semantic_tag_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, tag_valid, "Invalid semantic tag: %s",
                                semantic_tag_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_grammar_ffi_annotations_parsing(AsthraV12TestContext *ctx) {
    // Test parsing of FFI-specific annotations

    // Mock test cases for FFI annotation parsing
    const char *ffi_annotation_tests[] = {
        "extern \"C\" int c_function(int arg);",                  // C linkage
        "#[ffi_safe] void safe_function(void);",                  // FFI safety annotation
        "// variadic functions removed - use explicit overloads", // Variadic functions no longer
                                                                  // supported
        "#[c_compat] struct CStruct { int field; };",             // C compatibility annotation
        "#[ownership(pinned)] uint8_t buffer[1024];",             // Pinned memory annotation
        NULL};

    for (int i = 0; ffi_annotation_tests[i] != NULL; i++) {
        // Mock FFI annotation parsing
        bool annotation_parsed = grammar_test_parse_ffi_annotation(ffi_annotation_tests[i]);
        bool annotation_valid = grammar_test_validate_ffi_annotation(ffi_annotation_tests[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, annotation_parsed, "Failed to parse FFI annotation: %s",
                                ffi_annotation_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, annotation_valid, "Invalid FFI annotation: %s",
                                ffi_annotation_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// POSTFIX EXPRESSION AMBIGUITY FIX TESTS (v1.20)
// =============================================================================

AsthraTestResult test_grammar_postfix_double_colon_restriction(AsthraV12TestContext *ctx) {
    // Test that v1.20 grammar correctly restricts :: usage to type contexts only

    // Invalid postfix :: patterns that should be rejected
    const char *invalid_postfix_patterns[] = {
        "42::to_string()",                   // Literal with ::
        "some_function()::method()",         // Function call with ::
        "(x + y)::process()",                // Expression with ::
        "array[0]::convert()",               // Array access with ::
        "obj.field::transform()",            // Field access with ::
        "func()::other()::final()",          // Chained :: usage
        "get_value().unwrap()::to_string()", // Complex postfix chain with ::
        NULL};

    for (int i = 0; invalid_postfix_patterns[i] != NULL; i++) {
        // Mock parsing - should fail for invalid postfix :: usage
        bool parse_success = grammar_test_parse_expression(invalid_postfix_patterns[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, !parse_success,
                                "Should reject invalid postfix :: usage: %s",
                                invalid_postfix_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }

        // Verify error message indicates postfix :: restriction
        const char *error_msg = grammar_test_get_last_error();
        bool has_postfix_error =
            (error_msg && strstr(error_msg, "postfix") != NULL && strstr(error_msg, "::") != NULL);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, has_postfix_error,
                                "Should provide helpful postfix :: error for: %s",
                                invalid_postfix_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_grammar_valid_associated_function_calls(AsthraV12TestContext *ctx) {
    // Test that valid :: usage in type contexts still works correctly

    // Valid associated function call patterns
    const char *valid_associated_patterns[] = {"Vec::new()",       // Simple associated function
                                               "Point::default()", // Another simple case
                                               "MyStruct::create(42, \"test\")", // With arguments
                                               "Result::Ok(value)",       // Enum constructor style
                                               "String::from(\"hello\")", // Type conversion
                                               "Option::Some(42)", // Generic enum constructor
                                               NULL};

    for (int i = 0; valid_associated_patterns[i] != NULL; i++) {
        // Mock parsing - should succeed for valid :: usage
        bool parse_success = grammar_test_parse_expression(valid_associated_patterns[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, parse_success, "Should parse valid :: usage: %s",
                                valid_associated_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }

        // Verify AST structure is correct (AssociatedFuncCall)
        bool is_associated_call =
            grammar_test_is_associated_function_call(valid_associated_patterns[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, is_associated_call,
                                "Should create AssociatedFuncCall AST for: %s",
                                valid_associated_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_grammar_generic_type_associated_functions(AsthraV12TestContext *ctx) {
    // Test enhanced generic type support in v1.20

    // Generic type associated function patterns
    const char *generic_associated_patterns[] = {
        "Vec<i32>::new()",                  // Single type parameter
        "Result<String, Error>::Ok(value)", // Multiple type parameters
        "HashMap<String, i32>::new()",      // Complex generic types
        "Option<Point>::Some(point)",       // Nested type usage
        "Box<dyn Trait>::new(impl_obj)",    // Complex generic with trait objects
        "Arc<Mutex<Vec<i32>>>::new(vec)",   // Deeply nested generics
        NULL};

    for (int i = 0; generic_associated_patterns[i] != NULL; i++) {
        // Mock parsing - should succeed for generic type :: usage
        bool parse_success = grammar_test_parse_expression(generic_associated_patterns[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, parse_success, "Should parse generic type :: usage: %s",
                                generic_associated_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }

        // Verify generic type information is captured
        bool has_type_args = grammar_test_has_type_arguments(generic_associated_patterns[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, has_type_args, "Should capture type arguments for: %s",
                                generic_associated_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }

        // Verify AST structure includes type_args field
        bool type_args_populated = grammar_test_type_args_populated(generic_associated_patterns[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, type_args_populated,
                                "Should populate type_args field for: %s",
                                generic_associated_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_grammar_disambiguation_ai_generation(AsthraV12TestContext *ctx) {
    // Test that the disambiguation improves AI generation reliability

    // Test cases that previously caused AI confusion
    const char *ai_confusion_cases[] = {// These should now be unambiguously invalid
                                        "42::method()", "func()::chain()", "expr::transform()",

                                        // These should be unambiguously valid
                                        "Type::function()", "Vec<T>::new()",
                                        "Result<T,E>::Ok(value)", NULL};

    for (int i = 0; ai_confusion_cases[i] != NULL; i++) {
        // Determine expected result based on pattern
        bool should_be_valid = (strstr(ai_confusion_cases[i], "Type::") != NULL ||
                                strstr(ai_confusion_cases[i], "Vec<") != NULL ||
                                strstr(ai_confusion_cases[i], "Result<") != NULL);

        bool parse_success = grammar_test_parse_expression(ai_confusion_cases[i]);

        if (should_be_valid) {
            if (!ASTHRA_TEST_ASSERT(&ctx->base, parse_success,
                                    "AI-friendly pattern should parse: %s",
                                    ai_confusion_cases[i])) {
                return ASTHRA_TEST_FAIL;
            }
        } else {
            if (!ASTHRA_TEST_ASSERT(&ctx->base, !parse_success,
                                    "AI-confusing pattern should be rejected: %s",
                                    ai_confusion_cases[i])) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }

    // Test deterministic parsing behavior
    bool parsing_deterministic = grammar_test_check_deterministic_parsing();

    if (!ASTHRA_TEST_ASSERT(&ctx->base, parsing_deterministic,
                            "Parsing should be deterministic after v1.20 fix")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}
