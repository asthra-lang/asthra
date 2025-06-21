/**
 * Asthra Programming Language
 * Lexer Multi-line String Tests (Phase 2)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test multi-line string literals: raw (r"""content""") and processed ("""content""")
 */

#include "../framework/test_framework.h"
#include "lexer.h"
#include <string.h>

// Simple token assertion macro for lexer tests only
#define ASSERT_TOKEN_TYPE_SIMPLE(context, token, expected_type) \
    do { \
        if (!asthra_test_assert_int_eq(context, (int)(token)->type, (int)expected_type, \
                                      "Token type mismatch: expected %d, got %d", \
                                      (int)expected_type, (int)(token)->type)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

// =============================================================================
// RAW MULTI-LINE STRING TESTS
// =============================================================================

/**
 * Test raw multi-line string literal parsing
 */
static AsthraTestResult test_lexer_raw_multiline_strings(AsthraTestContext *context) {
    const struct {
        const char* input;
        const char* expected_value;
    } test_cases[] = {
        // Basic raw multi-line strings
        {
            "r\"\"\"hello\"\"\"",
            "hello"
        },
        {
            "r\"\"\"hello\nworld\"\"\"",
            "hello\nworld"
        },
        {
            "r\"\"\"\nSELECT * FROM users\nWHERE active = true\n\"\"\"",
            "\nSELECT * FROM users\nWHERE active = true\n"
        },
        // Raw strings with special characters (no escape processing)
        {
            "r\"\"\"String with \\n literal backslash\"\"\"",
            "String with \\n literal backslash"
        },
        {
            "r\"\"\"Path: C:\\Users\\Name\\Documents\"\"\"",
            "Path: C:\\Users\\Name\\Documents"
        },
        {
            "r\"\"\"Regex: \\d+\\.\\d+\"\"\"",
            "Regex: \\d+\\.\\d+"
        },
        // Empty raw multi-line string
        {
            "r\"\"\"\"\"\"",
            ""
        },
        // Raw string with quotes inside
        {
            "r\"\"\"He said \"Hello\" to me\"\"\"",
            "He said \"Hello\" to me"
        },
        {NULL, NULL}
    };

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char* input = test_cases[i].input;
        const char* expected = test_cases[i].expected_value;
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_raw_multiline.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for raw string: %s", input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_STRING);

        // Verify string value
        if (!asthra_test_assert_string_eq(context, token.data.string.value, expected,
                                         "Raw string value mismatch for input '%s'", input)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// PROCESSED MULTI-LINE STRING TESTS
// =============================================================================

/**
 * Test processed multi-line string literal parsing
 */
static AsthraTestResult test_lexer_processed_multiline_strings(AsthraTestContext *context) {
    const struct {
        const char* input;
        const char* expected_value;
    } test_cases[] = {
        // Basic processed multi-line strings
        {
            "\"\"\"hello\"\"\"",
            "hello"
        },
        {
            "\"\"\"hello\nworld\"\"\"",
            "hello\nworld"
        },
        {
            "\"\"\"\nSELECT * FROM users\nWHERE active = true\n\"\"\"",
            "\nSELECT * FROM users\nWHERE active = true\n"
        },
        // Processed strings with escape sequences
        {
            "\"\"\"String with \\n newline\"\"\"",
            "String with \n newline"
        },
        {
            "\"\"\"Tab\\tSeparated\\tValues\"\"\"",
            "Tab\tSeparated\tValues"
        },
        {
            "\"\"\"Quote: \\\"Hello\\\" World\"\"\"",
            "Quote: \"Hello\" World"
        },
        {
            "\"\"\"Backslash: \\\\\"\"\"",
            "Backslash: \\"
        },
        {
            "\"\"\"Null\\0Terminated\"\"\"",
            "Null\0Terminated"
        },
        {
            "\"\"\"Carriage\\rReturn\"\"\"",
            "Carriage\rReturn"
        },
        {
            "\"\"\"Single quote: \\'\"\"\"",
            "Single quote: '"
        },
        // Empty processed multi-line string
        {
            "\"\"\"\"\"\"",
            ""
        },
        // Mixed escape sequences
        {
            "\"\"\"Mixed\\tEscape\\nSequences\\\\Here\"\"\"",
            "Mixed\tEscape\nSequences\\Here"
        },
        {NULL, NULL}
    };

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char* input = test_cases[i].input;
        const char* expected = test_cases[i].expected_value;
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_processed_multiline.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for processed string: %s", input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_STRING);

        // Verify string value
        if (!asthra_test_assert_string_eq(context, token.data.string.value, expected,
                                         "Processed string value mismatch for input '%s'", input)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// COMPLEX MULTI-LINE STRING TESTS
// =============================================================================

/**
 * Test complex multi-line string scenarios
 */
static AsthraTestResult test_lexer_complex_multiline_strings(AsthraTestContext *context) {
    // Test SQL query
    const char* sql_input = "r\"\"\"\n"
                           "    SELECT users.name, users.email, profiles.bio\n"
                           "    FROM users \n"
                           "    JOIN profiles ON users.id = profiles.user_id \n"
                           "    WHERE users.active = true\n"
                           "    ORDER BY users.created_at DESC\n"
                           "\"\"\"";
    
    const char* sql_expected = "\n"
                              "    SELECT users.name, users.email, profiles.bio\n"
                              "    FROM users \n"
                              "    JOIN profiles ON users.id = profiles.user_id \n"
                              "    WHERE users.active = true\n"
                              "    ORDER BY users.created_at DESC\n";
    
    Lexer* lexer = lexer_create(sql_input, strlen(sql_input), "test_sql.ast");
    if (!lexer) {
        asthra_test_assert_pointer(context, lexer, "Failed to create lexer for SQL test");
        return ASTHRA_TEST_FAIL;
    }

    Token token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_STRING);

    if (!asthra_test_assert_string_eq(context, token.data.string.value, sql_expected,
                                     "SQL raw string value mismatch")) {
        token_free(&token);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    token_free(&token);
    lexer_destroy(lexer);

    // Test configuration template with escape sequences
    const char* config_input = "\"\"\"\n"
                              "server {\n"
                              "    listen 80;\n"
                              "    server_name \\{server_name};\n"
                              "    location / {\n"
                              "        proxy_pass http://\\{backend_host}:\\{backend_port};\n"
                              "    }\n"
                              "}\n"
                              "\"\"\"";
    
    const char* config_expected = "\n"
                                 "server {\n"
                                 "    listen 80;\n"
                                 "    server_name {server_name};\n"
                                 "    location / {\n"
                                 "        proxy_pass http://{backend_host}:{backend_port};\n"
                                 "    }\n"
                                 "}\n";
    
    lexer = lexer_create(config_input, strlen(config_input), "test_config.ast");
    if (!lexer) {
        asthra_test_assert_pointer(context, lexer, "Failed to create lexer for config test");
        return ASTHRA_TEST_FAIL;
    }

    token = lexer_next_token(lexer);
    ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_STRING);

    if (!asthra_test_assert_string_eq(context, token.data.string.value, config_expected,
                                     "Config processed string value mismatch")) {
        token_free(&token);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }

    token_free(&token);
    lexer_destroy(lexer);

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// BACKWARD COMPATIBILITY TESTS
// =============================================================================

/**
 * Test backward compatibility with regular single-line strings
 */
static AsthraTestResult test_lexer_multiline_backward_compatibility(AsthraTestContext *context) {
    const struct {
        const char* input;
        const char* expected_value;
    } test_cases[] = {
        // Regular single-line strings should still work
        {"\"hello\"", "hello"},
        {"\"world\"", "world"},
        {"\"\"", ""},
        {"\"Hello, World!\"", "Hello, World!"},
        {"\"String with spaces\"", "String with spaces"},
        {"\"Special chars: @#$%\"", "Special chars: @#$%"},
        // Regular strings with escape sequences
        {"\"Hello\\nWorld\"", "Hello\nWorld"},
        {"\"Tab\\tSeparated\"", "Tab\tSeparated"},
        {"\"Quote: \\\"Hello\\\"\"", "Quote: \"Hello\""},
        {"\"Backslash: \\\\\"", "Backslash: \\"},
        {NULL, NULL}
    };

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char* input = test_cases[i].input;
        const char* expected = test_cases[i].expected_value;
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_backward_compat.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for regular string: %s", input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        ASSERT_TOKEN_TYPE_SIMPLE(context, &token, TOKEN_STRING);

        // Verify string value
        if (!asthra_test_assert_string_eq(context, token.data.string.value, expected,
                                         "Regular string value mismatch for input '%s'", input)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }

        token_free(&token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// ERROR HANDLING TESTS
// =============================================================================

/**
 * Test error handling for malformed multi-line strings
 */
static AsthraTestResult test_lexer_multiline_string_errors(AsthraTestContext *context) {
    const char* error_cases[] = {
        // Unterminated raw multi-line strings
        "r\"\"\"hello",
        "r\"\"\"hello world",
        "r\"\"\"multiline\ncontent",
        // Unterminated processed multi-line strings
        "\"\"\"hello",
        "\"\"\"hello world",
        "\"\"\"multiline\ncontent",
        NULL
    };

    for (int i = 0; error_cases[i] != NULL; i++) {
        const char* input = error_cases[i];
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_errors.ast");
        if (!lexer) {
            asthra_test_assert_pointer(context, lexer, "Failed to create lexer for error case: %s", input);
            return ASTHRA_TEST_FAIL;
        }

        Token token = lexer_next_token(lexer);
        
        // Should get an error token
        if (!asthra_test_assert_int_eq(context, (int)token.type, (int)TOKEN_ERROR,
                                      "Expected error token for malformed input: %s", input)) {
            token_free(&token);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
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
 * Main test function for lexer multi-line string literals
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
        {test_lexer_raw_multiline_strings, "test_lexer_raw_multiline_strings"},
        {test_lexer_processed_multiline_strings, "test_lexer_processed_multiline_strings"},
        {test_lexer_complex_multiline_strings, "test_lexer_complex_multiline_strings"},
        {test_lexer_multiline_backward_compatibility, "test_lexer_multiline_backward_compatibility"},
        {test_lexer_multiline_string_errors, "test_lexer_multiline_string_errors"},
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
