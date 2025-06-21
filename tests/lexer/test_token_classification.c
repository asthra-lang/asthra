/**
 * Asthra Programming Language
 * Token Classification Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test token type classification functions (keyword, operator, literal, type checks)
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "lexer.h"
#include <string.h>

// =============================================================================
// TOKEN CLASSIFICATION TESTS
// =============================================================================

/**
 * Test token keyword classification
 */
AsthraTestResult test_token_is_keyword(AsthraTestContext *context) {
    const struct {
        TokenType type;
        bool should_be_keyword;
    } test_cases[] = {
        // Keywords
        {TOKEN_LET, true},
        {TOKEN_FN, true},
        {TOKEN_IF, true},
        {TOKEN_ELSE, true},
        {TOKEN_FOR, true},
        {TOKEN_RETURN, true},
        {TOKEN_STRUCT, true},
        {TOKEN_EXTERN, true},
        {TOKEN_MATCH, true},
        {TOKEN_SPAWN, true},
        {TOKEN_UNSAFE, true},
        {TOKEN_SIZEOF, true},
        {TOKEN_IMPL, true},
        {TOKEN_SELF, true},
        {TOKEN_BOOL_TRUE, true},
        {TOKEN_BOOL_FALSE, true},
        
        // Types (should be considered keywords)
        {TOKEN_INT, true},
        {TOKEN_FLOAT_TYPE, true},
        {TOKEN_BOOL, true},
        {TOKEN_STRING_TYPE, true},
        {TOKEN_VOID, true},
        {TOKEN_USIZE, true},
        {TOKEN_ISIZE, true},
        {TOKEN_U8, true},
        {TOKEN_I8, true},
        {TOKEN_U16, true},
        {TOKEN_I16, true},
        {TOKEN_U32, true},
        {TOKEN_I32, true},
        {TOKEN_U64, true},
        {TOKEN_I64, true},
        {TOKEN_F32, true},
        {TOKEN_F64, true},
        {TOKEN_RESULT, true},
        
        // Non-keywords
        {TOKEN_INTEGER, false},
        {TOKEN_FLOAT, false},
        {TOKEN_STRING, false},
        {TOKEN_IDENTIFIER, false},
        {TOKEN_CHAR, false},
        {TOKEN_PLUS, false},
        {TOKEN_MINUS, false},
        {TOKEN_MULTIPLY, false},
        {TOKEN_LEFT_PAREN, false},
        {TOKEN_RIGHT_PAREN, false},
        {TOKEN_SEMICOLON, false},
        {TOKEN_EOF, false},
        {TOKEN_ERROR, false}
    };

    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < test_count; i++) {
        bool is_keyword = token_is_keyword(test_cases[i].type);
        
        if (!asthra_test_assert_bool(context, is_keyword == test_cases[i].should_be_keyword,
                                    "token_is_keyword mismatch for %s: expected %s, got %s",
                                    token_type_name(test_cases[i].type),
                                    test_cases[i].should_be_keyword ? "true" : "false",
                                    is_keyword ? "true" : "false")) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test token operator classification
 */
AsthraTestResult test_token_is_operator(AsthraTestContext *context) {
    const struct {
        TokenType type;
        bool should_be_operator;
    } test_cases[] = {
        // Operators
        {TOKEN_PLUS, true},
        {TOKEN_MINUS, true},
        {TOKEN_MULTIPLY, true},
        {TOKEN_DIVIDE, true},
        {TOKEN_MODULO, true},
        {TOKEN_ASSIGN, true},
        {TOKEN_EQUAL, true},
        {TOKEN_NOT_EQUAL, true},
        {TOKEN_LESS_THAN, true},
        {TOKEN_LESS_EQUAL, true},
        {TOKEN_GREATER_THAN, true},
        {TOKEN_GREATER_EQUAL, true},
        {TOKEN_LOGICAL_AND, true},
        {TOKEN_LOGICAL_OR, true},
        {TOKEN_LOGICAL_NOT, true},
        {TOKEN_BITWISE_AND, true},
        {TOKEN_BITWISE_OR, true},
        {TOKEN_BITWISE_XOR, true},
        {TOKEN_BITWISE_NOT, true},
        {TOKEN_LEFT_SHIFT, true},
        {TOKEN_RIGHT_SHIFT, true},
        {TOKEN_ARROW, true},
        
        // Non-operators
        {TOKEN_INTEGER, false},
        {TOKEN_FLOAT, false},
        {TOKEN_STRING, false},
        {TOKEN_IDENTIFIER, false},
        {TOKEN_CHAR, false},
        {TOKEN_LET, false},
        {TOKEN_FN, false},
        {TOKEN_IF, false},
        {TOKEN_LEFT_PAREN, false},
        {TOKEN_RIGHT_PAREN, false},
        {TOKEN_SEMICOLON, false},
        {TOKEN_COMMA, false},
        {TOKEN_DOT, false},
        {TOKEN_COLON, false},
        {TOKEN_EOF, false},
        {TOKEN_ERROR, false}
    };

    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < test_count; i++) {
        bool is_operator = token_is_operator(test_cases[i].type);
        
        if (!asthra_test_assert_bool(context, is_operator == test_cases[i].should_be_operator,
                                    "token_is_operator mismatch for %s: expected %s, got %s",
                                    token_type_name(test_cases[i].type),
                                    test_cases[i].should_be_operator ? "true" : "false",
                                    is_operator ? "true" : "false")) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test token literal classification
 */
AsthraTestResult test_token_is_literal(AsthraTestContext *context) {
    const struct {
        TokenType type;
        bool should_be_literal;
    } test_cases[] = {
        // Literals
        {TOKEN_INTEGER, true},
        {TOKEN_FLOAT, true},
        {TOKEN_STRING, true},
        {TOKEN_CHAR, true},
        {TOKEN_BOOL_TRUE, true},
        {TOKEN_BOOL_FALSE, true},
        
        // Non-literals
        {TOKEN_IDENTIFIER, false},
        {TOKEN_LET, false},
        {TOKEN_FN, false},
        {TOKEN_IF, false},
        {TOKEN_PLUS, false},
        {TOKEN_MINUS, false},
        {TOKEN_LEFT_PAREN, false},
        {TOKEN_RIGHT_PAREN, false},
        {TOKEN_SEMICOLON, false},
        {TOKEN_INT, false},
        {TOKEN_FLOAT_TYPE, false},
        {TOKEN_STRING_TYPE, false},
        {TOKEN_EOF, false},
        {TOKEN_ERROR, false}
    };

    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < test_count; i++) {
        bool is_literal = token_is_literal(test_cases[i].type);
        
        if (!asthra_test_assert_bool(context, is_literal == test_cases[i].should_be_literal,
                                    "token_is_literal mismatch for %s: expected %s, got %s",
                                    token_type_name(test_cases[i].type),
                                    test_cases[i].should_be_literal ? "true" : "false",
                                    is_literal ? "true" : "false")) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test token type classification
 */
AsthraTestResult test_token_is_type(AsthraTestContext *context) {
    const struct {
        TokenType type;
        bool should_be_type;
    } test_cases[] = {
        // Type tokens
        {TOKEN_INT, true},
        {TOKEN_FLOAT_TYPE, true},
        {TOKEN_BOOL, true},
        {TOKEN_STRING_TYPE, true},
        {TOKEN_VOID, true},
        {TOKEN_USIZE, true},
        {TOKEN_ISIZE, true},
        {TOKEN_U8, true},
        {TOKEN_I8, true},
        {TOKEN_U16, true},
        {TOKEN_I16, true},
        {TOKEN_U32, true},
        {TOKEN_I32, true},
        {TOKEN_U64, true},
        {TOKEN_I64, true},
        {TOKEN_F32, true},
        {TOKEN_F64, true},
        {TOKEN_RESULT, true},
        
        // Non-type tokens
        {TOKEN_INTEGER, false},
        {TOKEN_FLOAT, false},
        {TOKEN_STRING, false},
        {TOKEN_IDENTIFIER, false},
        {TOKEN_CHAR, false},
        {TOKEN_LET, false},
        {TOKEN_FN, false},
        {TOKEN_IF, false},
        {TOKEN_PLUS, false},
        {TOKEN_MINUS, false},
        {TOKEN_LEFT_PAREN, false},
        {TOKEN_RIGHT_PAREN, false},
        {TOKEN_SEMICOLON, false},
        {TOKEN_EOF, false},
        {TOKEN_ERROR, false}
    };

    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < test_count; i++) {
        bool is_type = token_is_type(test_cases[i].type);
        
        if (!asthra_test_assert_bool(context, is_type == test_cases[i].should_be_type,
                                    "token_is_type mismatch for %s: expected %s, got %s",
                                    token_type_name(test_cases[i].type),
                                    test_cases[i].should_be_type ? "true" : "false",
                                    is_type ? "true" : "false")) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
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
        {test_token_is_keyword, "test_token_is_keyword"},
        {test_token_is_operator, "test_token_is_operator"},
        {test_token_is_literal, "test_token_is_literal"},
        {test_token_is_type, "test_token_is_type"},
        {NULL, NULL}
    };

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
