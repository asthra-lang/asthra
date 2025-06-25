/**
 * Asthra Programming Language v1.18 Comprehensive Explicit Syntax Validation
 *
 * This test file validates that all v1.18 explicit syntax requirements are properly enforced:
 * 1. Explicit Visibility Modifiers (pub/priv required)
 * 2. Explicit Parameter Lists (void required for no parameters)
 * 3. Explicit Content (void required for empty structs/enums)
 * 4. Explicit Function Arguments (void required for no arguments)
 * 5. Explicit Pattern Arguments (void required for parameterless patterns)
 * 6. Explicit Array Elements (void required for empty arrays)
 * 7. No Import Aliases (simplified import syntax only)
 * 8. Explicit Annotation Parameters (void required for no parameters)
 *
 * Part of Phase 7: Test Suite Updates - Optional Elements Simplification Plan
 * Grammar Version: v1.18 (Comprehensive optional element removal)
 * Implementation Date: 2024-12-30
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_v1_18_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_v1_18_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// V1.18 EXPLICIT SYNTAX VALIDATION TESTS
// =============================================================================

/**
 * Test: Valid v1.18 Explicit Syntax
 * Verifies that new explicit syntax is properly accepted
 */
static AsthraTestResult test_v1_18_valid_explicit_syntax(AsthraTestContext *context) {
    const char *valid_syntax_tests[] = {
        // Explicit Visibility Modifiers
        "priv struct Point { x: i32, y: i32 }", "pub struct Point { x: i32, y: i32 }",
        "priv enum Status { Active, Inactive }", "pub enum Status { Active, Inactive }",
        "pub fn main(void) -> i32 { return 0; }", "priv fn helper(void) -> void { return void; }",

        // Explicit Parameter Lists
        "pub fn no_params(void) -> void { }",
        "pub fn with_params(x: i32, y: i32) -> i32 { return x + y; }",

        // Explicit Content for Empty Structures
        "priv struct Empty { void }", "priv enum EmptyEnum { void }",

        // Explicit Function Arguments
        "main(void);", "process(data, flags);",

        // Explicit Pattern Arguments
        "match status { Active(void) => \"active\", Inactive(void) => \"inactive\" }",
        "match option { Option.Some(value) => value, Option.None(void) => 0 }",

        // Explicit Array Elements
        "let empty: []i32 = [void];", "let numbers: []i32 = [1, 2, 3];",

        // Simplified Imports (No Aliases)
        "import \"stdlib/string\";", "import \"module/path\";",

        // Explicit Annotation Parameters
        "#[deprecated(void)] fn old_function(void) -> void { }",
        "#[cache_friendly(strategy=\"temporal\")] fn fast_function(void) -> void { }",

        NULL};

    for (int i = 0; valid_syntax_tests[i] != NULL; i++) {
        Parser *parser = create_test_parser(valid_syntax_tests[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_program(parser);

        if (!asthra_test_assert_not_null(context, result,
                                         "Valid v1.18 syntax should parse successfully: %s",
                                         valid_syntax_tests[i])) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Invalid Old Implicit Syntax
 * Verifies that old implicit syntax is properly rejected
 */
static AsthraTestResult test_v1_18_reject_old_implicit_syntax(AsthraTestContext *context) {
    const char *invalid_old_syntax_tests[] = {
        // Missing Visibility Modifiers (should be rejected)
        "struct Point { x: i32, y: i32 }", "enum Status { Active, Inactive }",
        "fn main() -> i32 { return 0; }",

        // Missing Explicit Parameters (should be rejected)
        "pub fn no_params() -> void { }",

        // Missing Explicit Content (should be rejected)
        "priv struct Empty { }", "priv enum EmptyEnum { }",

        // Missing Explicit Function Arguments (should be rejected)
        "main();",

        // Missing Explicit Pattern Arguments (should be rejected)
        "match status { Active => \"active\", Inactive => \"inactive\" }",
        "match option { Option.Some(value) => value, Option.None => 0 }",

        // Missing Explicit Array Elements (should be rejected)
        "let empty: []i32 = [];",

        // Import Aliases (should be rejected - not supported in v1.18)
        "import \"stdlib/string\" as str;", "import \"module/path\" as module;",

        // Missing Explicit Annotation Parameters (should be rejected)
        "#[doc] fn undocumented_function(void) -> void { }",
        "#[cache_friendly()] fn fast_function(void) -> void { }",

        NULL};

    for (int i = 0; invalid_old_syntax_tests[i] != NULL; i++) {
        Parser *parser = create_test_parser(invalid_old_syntax_tests[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_program(parser);

        // Old implicit syntax should be rejected (result should be NULL or contain errors)
        if (result != NULL) {
            // Check if parser had errors even with a result
            if (!parser_had_error(parser)) {
                asthra_test_fail(context,
                                 "Old implicit syntax should be rejected but was accepted: %s",
                                 invalid_old_syntax_tests[i]);
                ast_free_node(result);
                destroy_test_parser(parser);
                return ASTHRA_TEST_FAIL;
            }
            ast_free_node(result);
        }

        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Complex Mixed Explicit Syntax
 * Verifies that complex combinations of explicit syntax work correctly
 */
static AsthraTestResult test_v1_18_complex_mixed_syntax(AsthraTestContext *context) {
    const char *complex_syntax_test =
        "priv struct UserData {\n"
        "    id: i32,\n"
        "    name: String,\n"
        "    active: bool\n"
        "}\n"
        "\n"
        "pub enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "\n"
        "pub fn process_user(data: UserData) -> Result<String, String> {\n"
        "    match data {\n"
        "        UserData { active: true, name, id } => {\n"
        "            let result = format(\"User {} ({})\", name, id);\n"
        "            return Result.Ok(result);\n"
        "        },\n"
        "        UserData { active: false, name, id: _ } => {\n"
        "            return Result.Err(format(\"User {} is inactive\", name));\n"
        "        }\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(void) -> i32 {\n"
        "    let user = UserData { id: 1, name: \"Alice\", active: true };\n"
        "    let result = process_user(user);\n"
        "    \n"
        "    match result {\n"
        "        Result.Ok(message) => {\n"
        "            println(message);\n"
        "            return 0;\n"
        "        },\n"
        "        Result.Err(error) => {\n"
        "            eprintln(error);\n"
        "            return 1;\n"
        "        }\n"
        "    }\n"
        "}\n";

    Parser *parser = create_test_parser(complex_syntax_test);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parser_parse_program(parser);

    if (!asthra_test_assert_not_null(context, result,
                                     "Complex mixed v1.18 syntax should parse successfully")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify the AST structure
    if (!asthra_test_assert_equal_int(context, result->type, AST_PROGRAM, "Expected AST_PROGRAM")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Error Messages for Implicit Syntax
 * Verifies that helpful error messages are provided for old implicit syntax
 */
static AsthraTestResult test_v1_18_helpful_error_messages(AsthraTestContext *context) {
    struct {
        const char *code;
        const char *expected_error_keyword;
    } error_test_cases[] = {{"struct Point { x: i32 }", "visibility"},
                            {"fn main(void) { }", "void"},
                            {"struct Empty { }", "void"},
                            {"main();", "void"},
                            {"[]", "void"},
                            {NULL, NULL}};

    for (int i = 0; error_test_cases[i].code != NULL; i++) {
        Parser *parser = create_test_parser(error_test_cases[i].code);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_program(parser);

        // Should have error
        if (!asthra_test_assert_true(context, parser_had_error(parser),
                                     "Parser should have error for: %s",
                                     error_test_cases[i].code)) {
            if (result)
                ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Error message should contain helpful keyword
        const ParseError *errors = parser_get_errors(parser);
        bool found_helpful_error = false;
        while (errors && !found_helpful_error) {
            if (strstr(errors->message, error_test_cases[i].expected_error_keyword) != NULL) {
                found_helpful_error = true;
            }
            errors = errors->next;
        }

        if (!asthra_test_assert_true(
                context, found_helpful_error, "Error message should contain '%s' for: %s",
                error_test_cases[i].expected_error_keyword, error_test_cases[i].code)) {
            if (result)
                ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        if (result)
            ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("ASTHRA v1.18 COMPREHENSIVE EXPLICIT SYNTAX VALIDATION TESTS\n");
    printf("=============================================================================\n");
    printf("Grammar Version: v1.18 (Comprehensive optional element removal)\n");
    printf("Part of Phase 7: Test Suite Updates - Optional Elements Simplification Plan\n\n");

    AsthraTestSuite suite = {.name = "V1.18 Explicit Syntax Validation",
                             .setup = setup_v1_18_tests,
                             .teardown = teardown_v1_18_tests};

    AsthraTestFunction tests[] = {
        test_v1_18_valid_explicit_syntax, test_v1_18_reject_old_implicit_syntax,
        test_v1_18_complex_mixed_syntax, test_v1_18_helpful_error_messages, NULL};

    AsthraTestMetadata metadata[] = {
        {"test_v1_18_valid_explicit_syntax", __FILE__, __LINE__,
         "Valid v1.18 explicit syntax acceptance", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"test_v1_18_reject_old_implicit_syntax", __FILE__, __LINE__,
         "Old implicit syntax rejection", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"test_v1_18_complex_mixed_syntax", __FILE__, __LINE__,
         "Complex mixed explicit syntax parsing", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"test_v1_18_helpful_error_messages", __FILE__, __LINE__,
         "Helpful error messages for implicit syntax", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {NULL, NULL, 0, NULL, ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL}};

    bool all_passed = asthra_test_run_suite(&suite, tests, metadata);

    printf("\n=============================================================================\n");
    if (all_passed) {
        printf("✅ All v1.18 Explicit Syntax Validation Tests PASSED!\n");
        printf("📊 Optional Elements Simplification Plan: Phase 7 Comprehensive Validation "
               "Complete\n");
        printf("🎯 Grammar v1.18 explicit syntax requirements properly enforced\n");
        return 0;
    } else {
        printf("❌ SOME v1.18 EXPLICIT SYNTAX VALIDATION TESTS FAILED!\n");
        printf("🔧 Check parser implementation for v1.18 grammar compliance\n");
        return 1;
    }
}
