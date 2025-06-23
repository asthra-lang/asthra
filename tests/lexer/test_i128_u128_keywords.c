/**
 * Asthra Programming Language
 * 128-bit Integer Keywords Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test i128 and u128 keyword recognition in the lexer
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "lexer.h"
#include <string.h>

// =============================================================================
// 128-BIT INTEGER KEYWORD TESTS
// =============================================================================

/**
 * Test i128 and u128 keyword recognition
 */
AsthraTestResult test_i128_u128_keywords(AsthraTestContext *context) {
    const struct {
        const char* keyword;
        TokenType expected_type;
    } test_cases[] = {
        {"i128", TOKEN_I128},
        {"u128", TOKEN_U128},
        {NULL, TOKEN_EOF}
    };

    for (int i = 0; test_cases[i].keyword != NULL; i++) {
        const char* keyword = test_cases[i].keyword;
        TokenType expected = test_cases[i].expected_type;
        
        TokenType result = keyword_lookup(keyword, strlen(keyword));
        
        if (!asthra_test_assert_int_eq(context, (int)result, (int)expected,
                                      "keyword_lookup mismatch for '%s': expected %s, got %s",
                                      keyword, token_type_name(expected), token_type_name(result))) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test i128 and u128 keywords in context
 */
AsthraTestResult test_i128_u128_in_context(AsthraTestContext *context) {
    const char *source = "let x: i128 = 42; let y: u128 = 100;";
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    
    if (!asthra_test_assert_not_null(context, lexer, "Failed to create lexer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    Token token;
    
    // let
    token = lexer_next_token(lexer);
    if (!asthra_test_assert_int_eq(context, (int)token.type, (int)TOKEN_LET,
                                   "Expected TOKEN_LET")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    // x
    token = lexer_next_token(lexer);
    if (!asthra_test_assert_int_eq(context, (int)token.type, (int)TOKEN_IDENTIFIER,
                                   "Expected TOKEN_IDENTIFIER for 'x'")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    // :
    token = lexer_next_token(lexer);
    if (!asthra_test_assert_int_eq(context, (int)token.type, (int)TOKEN_COLON,
                                   "Expected TOKEN_COLON")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    // i128
    token = lexer_next_token(lexer);
    if (!asthra_test_assert_int_eq(context, (int)token.type, (int)TOKEN_I128,
                                   "Expected TOKEN_I128")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

/**
 * Test that is_primitive_type recognizes 128-bit types
 */
AsthraTestResult test_is_primitive_type_128bit(AsthraTestContext *context) {
    if (!asthra_test_assert_bool(context, is_primitive_type(TOKEN_I128),
                                 "is_primitive_type should return true for TOKEN_I128")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, is_primitive_type(TOKEN_U128),
                                 "is_primitive_type should return true for TOKEN_U128")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test token display names for 128-bit types
 */
AsthraTestResult test_128bit_token_display_names(AsthraTestContext *context) {
    const char* i128_display = token_type_display_name(TOKEN_I128);
    if (!asthra_test_assert_string_eq(context, i128_display, "i128",
                                     "TOKEN_I128 display name should be 'i128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    const char* u128_display = token_type_display_name(TOKEN_U128);
    if (!asthra_test_assert_string_eq(context, u128_display, "u128",
                                     "TOKEN_U128 display name should be 'u128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    const char* i128_name = token_type_name(TOKEN_I128);
    if (!asthra_test_assert_string_eq(context, i128_name, "I128",
                                     "TOKEN_I128 type name should be 'I128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    const char* u128_name = token_type_name(TOKEN_U128);
    if (!asthra_test_assert_string_eq(context, u128_name, "U128",
                                     "TOKEN_U128 type name should be 'U128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

/**
 * Main test function
 */
int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Define test cases
    const struct {
        AsthraTestFunction test_func;
        const char* test_name;
    } test_cases[] = {
        {test_i128_u128_keywords, "128-bit keyword recognition"},
        {test_i128_u128_in_context, "128-bit keywords in context"},
        {test_is_primitive_type_128bit, "is_primitive_type for 128-bit types"},
        {test_128bit_token_display_names, "128-bit token display names"},
        {NULL, NULL}
    };

    printf("=== 128-bit Integer Keywords Tests ===\n\n");

    // Run tests
    for (int i = 0; test_cases[i].test_func != NULL; i++) {
        AsthraTestMetadata metadata = {
            .name = test_cases[i].test_name,
            .file = __FILE__,
            .line = __LINE__,
            .function = test_cases[i].test_name,
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
            .timeout_ns = 30000000000ULL,
            .skip = false,
            .skip_reason = NULL
        };

        AsthraTestResult result = asthra_test_run_single(test_cases[i].test_func, &metadata, stats);
        printf("[%s] %s\n", 
               result == ASTHRA_TEST_PASS ? "PASS" : "FAIL", 
               test_cases[i].test_name);
    }

    // Print results
    printf("\nTest Results: %llu/%llu passed\n", 
           (unsigned long long)stats->tests_passed, (unsigned long long)stats->tests_run);

    bool success = (stats->tests_failed == 0);
    asthra_test_statistics_destroy(stats);
    return success ? 0 : 1;
}