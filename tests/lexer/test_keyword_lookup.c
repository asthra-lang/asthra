/**
 * Asthra Programming Language
 * Keyword Lookup Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test keyword lookup, case sensitivity, and reserved keyword validation
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "lexer.h"
#include <string.h>

// =============================================================================
// KEYWORD LOOKUP TESTS
// =============================================================================

/**
 * Test keyword lookup for all keywords
 */
AsthraTestResult test_keyword_lookup_all_keywords(AsthraTestContext *context) {
    const struct {
        const char* keyword;
        TokenType expected_type;
    } test_cases[] = {
        {"package", TOKEN_PACKAGE},
        {"import", TOKEN_IMPORT},
        {"as", TOKEN_AS},
        {"pub", TOKEN_PUB},
        {"fn", TOKEN_FN},
        {"struct", TOKEN_STRUCT},
        {"extern", TOKEN_EXTERN},
        {"let", TOKEN_LET},
        {"if", TOKEN_IF},
        {"else", TOKEN_ELSE},
        {"enum", TOKEN_ENUM},
        {"for", TOKEN_FOR},
        {"in", TOKEN_IN},
        {"return", TOKEN_RETURN},
        {"match", TOKEN_MATCH},
        {"spawn", TOKEN_SPAWN},
        {"unsafe", TOKEN_UNSAFE},
        {"sizeof", TOKEN_SIZEOF},
        {"impl", TOKEN_IMPL},
        {"self", TOKEN_SELF},
        {"true", TOKEN_BOOL_TRUE},
        {"false", TOKEN_BOOL_FALSE},
        
        // Type keywords
        {"int", TOKEN_INT},
        {"float", TOKEN_FLOAT_TYPE},
        {"bool", TOKEN_BOOL},
        {"string", TOKEN_STRING_TYPE},
        {"void", TOKEN_VOID},
        {"usize", TOKEN_USIZE},
        {"isize", TOKEN_ISIZE},
        {"u8", TOKEN_U8},
        {"i8", TOKEN_I8},
        {"u16", TOKEN_U16},
        {"i16", TOKEN_I16},
        {"u32", TOKEN_U32},
        {"i32", TOKEN_I32},
        {"u64", TOKEN_U64},
        {"i64", TOKEN_I64},
        {"u128", TOKEN_U128},
        {"i128", TOKEN_I128},
        {"f32", TOKEN_F32},
        {"f64", TOKEN_F64},
        {"Result", TOKEN_RESULT},
        
        // Tier 1 Concurrency keywords (Core & Simple)
        {"spawn_with_handle", TOKEN_SPAWN_WITH_HANDLE},
        {"await", TOKEN_AWAIT},
        // Note: Tier 3 concurrency keywords moved to stdlib:
        // {"channel", TOKEN_CHANNEL}, {"send", TOKEN_SEND}, {"recv", TOKEN_RECV},
        // {"select", TOKEN_SELECT}, {"default", TOKEN_DEFAULT}, 
        // {"worker_pool", TOKEN_WORKER_POOL}, {"timeout", TOKEN_TIMEOUT}, {"close", TOKEN_CLOSE},
        
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
 * Test keyword lookup case sensitivity
 */
AsthraTestResult test_keyword_lookup_case_sensitivity(AsthraTestContext *context) {
    const struct {
        const char* input;
        bool should_be_keyword;
    } test_cases[] = {
        // Correct case (should be keywords)
        {"let", true},
        {"fn", true},
        {"if", true},
        {"true", true},
        {"false", true},
        
        // Wrong case (should not be keywords)
        {"LET", false},
        {"FN", false},
        {"IF", false},
        {"TRUE", false},
        {"FALSE", false},
        {"Let", false},
        {"Fn", false},
        {"If", false},
        {"True", false},
        {"False", false},
        
        {NULL, false}
    };

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char* input = test_cases[i].input;
        bool should_be_keyword = test_cases[i].should_be_keyword;
        
        TokenType result = keyword_lookup(input, strlen(input));
        bool is_keyword = (result != TOKEN_IDENTIFIER && result != TOKEN_EOF);
        
        if (!asthra_test_assert_bool(context, is_keyword == should_be_keyword,
                                    "Case sensitivity test failed for '%s': expected %s, got %s",
                                    input,
                                    should_be_keyword ? "keyword" : "not keyword",
                                    is_keyword ? "keyword" : "not keyword")) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test reserved keyword validation
 */
AsthraTestResult test_reserved_keyword_validation(AsthraTestContext *context) {
    const struct {
        const char* identifier;
        bool should_be_reserved;
    } test_cases[] = {
        // Reserved keywords
        {"let", true},
        {"fn", true},
        {"if", true},
        {"else", true},
        {"enum", true},
        {"struct", true},
        {"extern", true},
        {"unsafe", true},
        {"return", true},
        {"match", true},
        {"spawn", true},
        
        // Not reserved (valid identifiers)
        {"variable", false},
        {"myFunction", false},
        {"let_var", false},
        {"fn_name", false},
        {"if_condition", false},
        {"user_defined", false},
        {"custom_type", false},
        
        // Edge cases
        {"", false},
        {"_", false},
        {"_let", false},
        {"let_", false},
        
        {NULL, false}
    };

    for (int i = 0; test_cases[i].identifier != NULL; i++) {
        const char* identifier = test_cases[i].identifier;
        bool should_be_reserved = test_cases[i].should_be_reserved;
        
        bool is_reserved = is_reserved_keyword(identifier, strlen(identifier));
        
        if (!asthra_test_assert_bool(context, is_reserved == should_be_reserved,
                                    "Reserved keyword validation failed for '%s': expected %s, got %s",
                                    identifier,
                                    should_be_reserved ? "reserved" : "not reserved",
                                    is_reserved ? "reserved" : "not reserved")) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test keyword lookup edge cases
 */
AsthraTestResult test_keyword_lookup_edge_cases(AsthraTestContext *context) {
    // Test empty string
    TokenType result = keyword_lookup("", 0);
    if (!asthra_test_assert_int_eq(context, (int)result, (int)TOKEN_IDENTIFIER,
                                  "Empty string should return TOKEN_IDENTIFIER")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test NULL pointer (should handle gracefully)
    result = keyword_lookup(NULL, 0);
    if (!asthra_test_assert_int_eq(context, (int)result, (int)TOKEN_IDENTIFIER,
                                  "NULL pointer should return TOKEN_IDENTIFIER")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test partial matches (should not be keywords)
    const char* partial_matches[] = {
        "le",     // partial "let"
        "f",      // partial "fn"
        "i",      // partial "if"
        "lett",   // extended "let"
        "fnn",    // extended "fn"
        "iff",    // extended "if"
        NULL
    };

    for (int i = 0; partial_matches[i] != NULL; i++) {
        const char* partial = partial_matches[i];
        result = keyword_lookup(partial, strlen(partial));
        
        if (!asthra_test_assert_int_eq(context, (int)result, (int)TOKEN_IDENTIFIER,
                                      "Partial match '%s' should return TOKEN_IDENTIFIER", partial)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test keywords with different lengths
    result = keyword_lookup("let", 2); // Only "le"
    if (!asthra_test_assert_int_eq(context, (int)result, (int)TOKEN_IDENTIFIER,
                                  "Truncated keyword should return TOKEN_IDENTIFIER")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test keyword lookup performance with many lookups
 */
AsthraTestResult test_keyword_lookup_performance(AsthraTestContext *context) {
    // Fixed: Removed "log" as it's a predeclared identifier, not a keyword
    const char* keywords[] = {
        "let", "fn", "if", "else", "for", "return", "struct", "extern",
        "match", "spawn", "unsafe", "sizeof", "impl", "self", "mut",
        "const", "true", "false", "int", "float", "bool", "string", "void",
        NULL
    };

    const char* non_keywords[] = {
        "variable", "function", "condition", "loop", "value", "object",
        "instance", "property", "method", "class", "interface", "module",
        "namespace", "library", "framework", "application", "system", "data",
        "log",  // log is predeclared identifier, not keyword
        NULL
    };

    // Perform many lookups to test performance
    for (int round = 0; round < 100; round++) {
        // Test keywords
        for (int i = 0; keywords[i] != NULL; i++) {
            TokenType result = keyword_lookup(keywords[i], strlen(keywords[i]));
            if (result == TOKEN_IDENTIFIER || result == TOKEN_EOF) {
                asthra_test_assert_bool(context, false,
                                       "Round %d: Keyword '%s' should be recognized", 
                                       round, keywords[i]);
                return ASTHRA_TEST_FAIL;
            }
        }

        // Test non-keywords
        for (int i = 0; non_keywords[i] != NULL; i++) {
            TokenType result = keyword_lookup(non_keywords[i], strlen(non_keywords[i]));
            if (result != TOKEN_IDENTIFIER) {
                asthra_test_assert_bool(context, false,
                                       "Round %d: Non-keyword '%s' should return TOKEN_IDENTIFIER",
                                       round, non_keywords[i]);
                return ASTHRA_TEST_FAIL;
            }
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
        {test_keyword_lookup_all_keywords, "test_keyword_lookup_all_keywords"},
        {test_keyword_lookup_case_sensitivity, "test_keyword_lookup_case_sensitivity"},
        {test_reserved_keyword_validation, "test_reserved_keyword_validation"},
        {test_keyword_lookup_edge_cases, "test_keyword_lookup_edge_cases"},
        {test_keyword_lookup_performance, "test_keyword_lookup_performance"},
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
