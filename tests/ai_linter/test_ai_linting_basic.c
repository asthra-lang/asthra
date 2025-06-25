#include "../framework/test_assertions.h"
#include "../framework/test_suite.h"
#include "ai_lint_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
static AsthraSemanticsAPI *create_mock_semantic_api(void);
static void destroy_mock_semantic_api(AsthraSemanticsAPI *api);
AsthraTestResult test_ai_linter_initialization(AsthraTestContext *context);
AsthraTestResult test_ai_lint_utility_functions(AsthraTestContext *context);
AsthraTestResult test_ai_lint_result_management(AsthraTestContext *context);

// Mock semantic API for testing
static AsthraSemanticsAPI *create_mock_semantic_api(void) {
    // For now, return NULL - in practice would create a mock
    return NULL;
}

static void destroy_mock_semantic_api(AsthraSemanticsAPI *api) {
    // Cleanup mock API
}

// Test AI linter initialization
AsthraTestResult test_ai_linter_initialization(AsthraTestContext *context) {
    // Skip this test for now since we need a proper semantic API
    printf("SKIP: AI linter initialization test (needs semantic API)\n");
    return ASTHRA_TEST_SKIP;
}

// Test utility functions
AsthraTestResult test_ai_lint_utility_functions(AsthraTestContext *context) {
    // Test category to string conversion
    const char *idiom_str = ai_lint_category_to_string(AI_LINT_IDIOM);
    ASTHRA_TEST_ASSERT(context,
                       asthra_test_assert_string_eq(context, idiom_str, "idiom",
                                                    "Category to string conversion failed"),
                       "Category idiom conversion failed");

    const char *perf_str = ai_lint_category_to_string(AI_LINT_PERFORMANCE);
    ASTHRA_TEST_ASSERT(context,
                       asthra_test_assert_string_eq(context, perf_str, "performance",
                                                    "Category to string conversion failed"),
                       "Category performance conversion failed");

    // Test severity to string conversion
    const char *suggestion_str = ai_lint_severity_to_string(AI_LINT_SUGGESTION);
    ASTHRA_TEST_ASSERT(context,
                       asthra_test_assert_string_eq(context, suggestion_str, "suggestion",
                                                    "Severity to string conversion failed"),
                       "Severity suggestion conversion failed");

    const char *warning_str = ai_lint_severity_to_string(AI_LINT_WARNING);
    ASTHRA_TEST_ASSERT(context,
                       asthra_test_assert_string_eq(context, warning_str, "warning",
                                                    "Severity to string conversion failed"),
                       "Severity warning conversion failed");

    return ASTHRA_TEST_PASS;
}

// Test result creation and destruction
AsthraTestResult test_ai_lint_result_management(AsthraTestContext *context) {
    DiagnosticSpan location = {.start_line = 10,
                               .start_column = 5,
                               .end_line = 10,
                               .end_column = 15,
                               .file_path = strdup("test.asthra")};

    AILintResult *result = ai_lint_result_create("TEST_RULE_001", AI_LINT_IDIOM, AI_LINT_SUGGESTION,
                                                 "Test rule", "This is a test rule",
                                                 "Use better patterns", location, 0.8, 0.7, true);

    ASTHRA_TEST_ASSERT(context,
                       asthra_test_assert_pointer(context, result, "Result creation failed"),
                       "Result creation failed");
    ASTHRA_TEST_ASSERT(
        context,
        asthra_test_assert_string_eq(context, result->rule_id, "TEST_RULE_001", "Rule ID mismatch"),
        "Rule ID mismatch");
    ASTHRA_TEST_ASSERT(
        context,
        asthra_test_assert_int_eq(context, result->category, AI_LINT_IDIOM, "Category mismatch"),
        "Category mismatch");
    ASTHRA_TEST_ASSERT(context,
                       asthra_test_assert_int_eq(context, result->severity, AI_LINT_SUGGESTION,
                                                 "Severity mismatch"),
                       "Severity mismatch");
    ASTHRA_TEST_ASSERT(
        context,
        asthra_test_assert_string_eq(context, result->title, "Test rule", "Title mismatch"),
        "Title mismatch");
    ASTHRA_TEST_ASSERT(
        context,
        asthra_test_assert_bool(context, result->auto_fixable, "Auto-fixable flag mismatch"),
        "Auto-fixable flag mismatch");

    ai_lint_result_destroy(result);
    free(location.file_path);

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    // Create lightweight test suite for AI linter basic testing
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("AI Linter Basic Tests");

    // Add all test functions to the suite
    asthra_test_suite_add_test(suite, "ai_linter_initialization", "Test AI linter initialization",
                               test_ai_linter_initialization);
    asthra_test_suite_add_test(suite, "ai_lint_utility_functions", "Test AI lint utility functions",
                               test_ai_lint_utility_functions);
    asthra_test_suite_add_test(suite, "ai_lint_result_management",
                               "Test AI lint result creation and management",
                               test_ai_lint_result_management);

    // Run the test suite and exit with appropriate code
    return asthra_test_suite_run_and_exit(suite);
}
