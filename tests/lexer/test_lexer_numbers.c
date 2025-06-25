/**
 * Asthra Programming Language
 * Lexer Numeric Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test numeric literal parsing including integers, floats, and edge cases
 */

#include "../framework/test_framework.h"
#include "lexer.h"
#include <string.h>

// Simple token assertion macro for lexer tests only
#define ASSERT_TOKEN_TYPE_SIMPLE(context, token, expected_type)                                    \
    do {                                                                                           \
        if (!asthra_test_assert_int_eq(context, (int)(token)->type, (int)expected_type,            \
                                       "Token type mismatch: expected %d, got %d",                 \
                                       (int)expected_type, (int)(token)->type)) {                  \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

// =============================================================================
// NUMERIC LITERAL TESTS
// =============================================================================

/**
 * Test integer literal parsing with different bases
 */
AsthraTestResult test_lexer_integer_literals(AsthraTestContext *context) {
    const struct {
        const char *input;
        int64_t expected_value;
    } test_cases[] = {{"0", 0},
                      {"42", 42},
                      {"123456", 123456},
                      {"9999", 9999},
                      {"0x10", 16},
                      {"0xFF", 255},
                      {"0xDEADBEEF", 0xDEADBEEF},
                      {"0b1010", 10},
                      {"0b11111111", 255},
                      {"0b10101010", 170},
                      {"0o17", 15},
                      {"0o777", 511},
                      {NULL, 0}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        int64_t expected = test_cases[i].expected_value;

        Lexer *lexer = lexer_create(input, strlen(input), "test_int_literals.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for integer: %s",
                                       input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_INTEGER);

        // Verify integer value
        if (!asthra_test_assert_long_eq(context, token.data.integer.value, expected,
                                        "Integer value mismatch for input '%s'", input)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test float literal parsing
 */
AsthraTestResult test_lexer_float_literals(AsthraTestContext *context) {
    const struct {
        const char *input;
        double expected_value;
    } test_cases[] = {{"0.0", 0.0},       {"3.14", 3.14},       {"123.456", 123.456},
                      {"1e6", 1000000.0}, {"2.5e-3", 0.0025},   {"1.23e+4", 12300.0},
                      {"0.001", 0.001},   {"999.999", 999.999}, {NULL, 0.0}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        double expected = test_cases[i].expected_value;

        Lexer *lexer = lexer_create(input, strlen(input), "test_float_literals.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for float: %s",
                                       input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_FLOAT);

        // Verify float value (with tolerance for floating point comparison)
        double diff = token.data.float_val.value - expected;
        if (diff < 0)
            diff = -diff;
        if (!asthra_test_assert_bool(context, diff < 0.000001,
                                     "Float value mismatch for input '%s': expected %f, got %f",
                                     input, expected, token.data.float_val.value)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test numeric edge cases and error conditions
 */
AsthraTestResult test_lexer_numeric_edge_cases(AsthraTestContext *context) {
    // Test large integers
    const char *large_int = "9223372036854775807"; // Max int64
    Lexer *lexer = lexer_create(large_int, strlen(large_int), "test_large_int.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for large integer")) {
        return ASTHRA_TEST_FAIL;
    }

    Token token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_INTEGER);

    token_free(&token);
    lexer_destroy(lexer);

    // Test very small float
    const char *small_float = "1.23e-100";
    lexer = lexer_create(small_float, strlen(small_float), "test_small_float.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for small float")) {
        return ASTHRA_TEST_FAIL;
    }

    token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_FLOAT);

    token_free(&token);
    lexer_destroy(lexer);

    // Test hexadecimal float (if supported)
    const char *hex_float = "0x1.5p4"; // 1.3125 * 16 = 21
    lexer = lexer_create(hex_float, strlen(hex_float), "test_hex_float.ast");
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for hex float")) {
        return ASTHRA_TEST_FAIL;
    }

    token = lexer_next_token(lexer);
    // This may be TOKEN_FLOAT, TOKEN_INTEGER, or TOKEN_ERROR depending on implementation
    // Hex floats are not required to be supported, so TOKEN_ERROR is acceptable
    if (token.type != TOKEN_FLOAT && token.type != TOKEN_INTEGER && token.type != TOKEN_ERROR) {
        asthra_test_assert_bool(context, false,
                                "Hex float should parse as numeric literal or error");
        token_free(&token);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    token_free(&token);
    lexer_destroy(lexer);

    return ASTHRA_TEST_PASS;
}

/**
 * Test numeric literals with different formats
 */
AsthraTestResult test_lexer_numeric_formats(AsthraTestContext *context) {
    const struct {
        const char *input;
        const char *description;
        TokenType expected_type;
    } test_cases[] = {{"0", "Zero", TOKEN_INTEGER},
                      {"00", "Zero with leading zero", TOKEN_INTEGER},
                      {"007", "Octal-like but decimal", TOKEN_INTEGER},
                      {"0.0", "Zero float", TOKEN_FLOAT},
                      {"0.", "Zero with trailing dot", TOKEN_FLOAT},
                      {".0", "Zero with leading dot", TOKEN_FLOAT},
                      {"1E10", "Scientific notation uppercase", TOKEN_FLOAT},
                      {"1e10", "Scientific notation lowercase", TOKEN_FLOAT},
                      {"1E+10", "Scientific notation positive exponent", TOKEN_FLOAT},
                      {"1E-10", "Scientific notation negative exponent", TOKEN_FLOAT},
                      {"0xABCD", "Uppercase hex", TOKEN_INTEGER},
                      {"0xabcd", "Lowercase hex", TOKEN_INTEGER},
                      {"0XABCD", "Uppercase X hex", TOKEN_INTEGER},
                      {"0b1100", "Binary literal", TOKEN_INTEGER},
                      {"0B1100", "Uppercase B binary", TOKEN_INTEGER},
                      {"0o1234", "Octal literal", TOKEN_INTEGER},
                      {"0O1234", "Uppercase O octal", TOKEN_INTEGER},
                      {NULL, NULL, TOKEN_EOF}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        const char *description = test_cases[i].description;
        TokenType expected_type = test_cases[i].expected_type;

        Lexer *lexer = lexer_create(input, strlen(input), "test_numeric_formats.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for %s: %s",
                                        description, input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, expected_type);

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test numeric literal bounds and overflow handling
 */
AsthraTestResult test_lexer_numeric_bounds(AsthraTestContext *context) {
    const struct {
        const char *input;
        const char *description;
        bool should_succeed;
    } test_cases[] = {{"2147483647", "Max 32-bit int", true},
                      {"2147483648", "Over 32-bit int", true}, // Should still parse as int64
                      {"9223372036854775807", "Max 64-bit int", true},
                      {"18446744073709551615", "Max uint64", true}, // May overflow but should parse
                      {"1.7976931348623157e+308", "Near max double", true},
                      {"2.2250738585072014e-308", "Near min positive double", true},
                      {"1e400", "Very large exponent", true},  // May be infinity but should parse
                      {"1e-400", "Very small exponent", true}, // May be zero but should parse
                      {NULL, NULL, false}};

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char *input = test_cases[i].input;
        const char *description = test_cases[i].description;
        bool should_succeed = test_cases[i].should_succeed;

        Lexer *lexer = lexer_create(input, strlen(input), "test_numeric_bounds.ast");
        if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for %s: %s",
                                        description, input)) {
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);

        if (should_succeed) {
            bool is_numeric = (token.type == TOKEN_INTEGER || token.type == TOKEN_FLOAT);
            if (!asthra_test_assert_bool(context, is_numeric, "Expected numeric token for %s: %s",
                                         description, input)) {
                token_free(&token);
                lexer_destroy(lexer);
                return ASTHRA_TEST_FAIL;
            }
        }

        token_free(&token);
        lexer_destroy(lexer);
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
        const char *test_name;
    } test_cases[] = {{test_lexer_integer_literals, "test_lexer_integer_literals"},
                      {test_lexer_float_literals, "test_lexer_float_literals"},
                      {test_lexer_numeric_edge_cases, "test_lexer_numeric_edge_cases"},
                      {test_lexer_numeric_formats, "test_lexer_numeric_formats"},
                      {test_lexer_numeric_bounds, "test_lexer_numeric_bounds"},
                      {NULL, NULL}};

    // Run tests
    for (int i = 0; test_cases[i].test_func != NULL; i++) {
        AsthraTestMetadata metadata = {.name = test_cases[i].test_name,
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = test_cases[i].test_name,
                                       .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                       .timeout_ns = 30000000000ULL,
                                       .skip = false,
                                       .skip_reason = NULL};

        AsthraTestResult result = asthra_test_run_single(test_cases[i].test_func, &metadata, stats);
        printf("[%s] %s\n", result == ASTHRA_TEST_PASS ? "PASS" : "FAIL", test_cases[i].test_name);
    }

    // Print results
    printf("\nTest Results: %llu/%llu passed\n", (unsigned long long)stats->tests_passed,
           (unsigned long long)stats->tests_run);

    bool success = (stats->tests_failed == 0);
    asthra_test_statistics_destroy(stats);
    return success ? 0 : 1;
}
