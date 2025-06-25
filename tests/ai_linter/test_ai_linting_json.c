#include "../framework/test_assertions.h"
#include "../framework/test_framework.h"
#include "ai_lint_core.h"
#include "guidance_output.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes
AsthraTestResult test_json_guidance_generation(AsthraTestContext *context);
AsthraTestResult test_json_rule_catalog(AsthraTestContext *context);
AsthraTestResult test_json_fix_suggestions(AsthraTestContext *context);

// Test JSON guidance generation
AsthraTestResult test_json_guidance_generation(AsthraTestContext *context) {
    // Create test results
    DiagnosticSpan location = {.start_line = 10,
                               .start_column = 5,
                               .end_line = 10,
                               .end_column = 15,
                               .file_path = strdup("test.asthra")};

    AILintResult *result = ai_lint_result_create(
        "AI_IDIOM_001", AI_LINT_IDIOM, AI_LINT_SUGGESTION, "Manual indexing detected",
        "Use iterator patterns instead of manual indexing",
        "Replace 'for i in range(0, vec.len()) { vec[i] }' with 'for item in vec { item }'",
        location, 0.8f, 0.7f, true);

    if (!asthra_test_assert_pointer(context, result, "Result creation failed")) {
        return ASTHRA_TEST_FAIL;
    }

    AILintResult *results[] = {result};
    char *json = ai_linter_generate_guidance_json(results, 1);

    if (!asthra_test_assert_pointer(context, json, "JSON generation failed")) {
        ai_lint_result_destroy(result);
        return ASTHRA_TEST_FAIL;
    }

    // Basic JSON validation - check for key fields
    if (!asthra_test_assert_bool(context, strstr(json, "\"schema_version\"") != NULL,
                                 "Missing schema_version")) {
        free(json);
        ai_lint_result_destroy(result);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, strstr(json, "\"AI_IDIOM_001\"") != NULL,
                                 "Missing rule_id")) {
        free(json);
        ai_lint_result_destroy(result);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, strstr(json, "\"statistics\"") != NULL,
                                 "Missing statistics")) {
        free(json);
        ai_lint_result_destroy(result);
        return ASTHRA_TEST_FAIL;
    }

    printf("Generated JSON: %s\n", json);

    free(json);
    ai_lint_result_destroy(result);
    return ASTHRA_TEST_PASS;
}

// Test rule catalog JSON generation
AsthraTestResult test_json_rule_catalog(AsthraTestContext *context) {
    AILinter *linter = ai_linter_create(NULL);
    if (!asthra_test_assert_pointer(context, linter, "Linter creation failed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Register a test rule
    AILintRule rule = {.rule_id = strdup("TEST_RULE"),
                       .category = AI_LINT_IDIOM,
                       .default_severity = AI_LINT_SUGGESTION,
                       .title = strdup("Test rule"),
                       .description = strdup("This is a test rule"),
                       .ai_guidance = strdup("Use better patterns"),
                       .check_function = NULL,
                       .before_examples = NULL,
                       .after_examples = NULL,
                       .example_count = 0,
                       .impact_score = 0.5f,
                       .auto_fixable = true};

    ai_linter_register_rule(linter, &rule);

    char *json = ai_linter_generate_rule_catalog_json(linter);

    if (!asthra_test_assert_pointer(context, json, "Rule catalog JSON generation failed")) {
        ai_linter_destroy(linter);
        return ASTHRA_TEST_FAIL;
    }

    // Basic validation
    if (!asthra_test_assert_bool(context, strstr(json, "\"TEST_RULE\"") != NULL,
                                 "Missing test rule")) {
        free(json);
        ai_linter_destroy(linter);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, strstr(json, "\"rule_count\"") != NULL,
                                 "Missing rule_count")) {
        free(json);
        ai_linter_destroy(linter);
        return ASTHRA_TEST_FAIL;
    }

    printf("Generated rule catalog JSON: %s\n", json);

    free(json);
    ai_linter_destroy(linter);
    return ASTHRA_TEST_PASS;
}

// Test fix suggestions JSON generation
AsthraTestResult test_json_fix_suggestions(AsthraTestContext *context) {
    DiagnosticSpan location = {.start_line = 5,
                               .start_column = 1,
                               .end_line = 5,
                               .end_column = 10,
                               .file_path = strdup("fix_test.asthra")};

    AILintResult *result = ai_lint_result_create("AI_IDIOM_002", AI_LINT_IDIOM, AI_LINT_SUGGESTION,
                                                 "Explicit return detected", "Use implicit returns",
                                                 "Remove 'return' keyword from the last expression",
                                                 location, 0.9f, 0.6f,
                                                 true // auto_fixable
    );

    if (!asthra_test_assert_pointer(context, result, "Result creation failed")) {
        return ASTHRA_TEST_FAIL;
    }

    AILintResult *results[] = {result};
    char *json = ai_linter_generate_fix_suggestions_json(results, 1);

    if (!asthra_test_assert_pointer(context, json, "Fix suggestions JSON generation failed")) {
        ai_lint_result_destroy(result);
        return ASTHRA_TEST_FAIL;
    }

    // Basic validation
    if (!asthra_test_assert_bool(context, strstr(json, "\"AI_IDIOM_002\"") != NULL,
                                 "Missing rule_id")) {
        free(json);
        ai_lint_result_destroy(result);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, strstr(json, "\"fix_suggestions\"") != NULL,
                                 "Missing fix_suggestions")) {
        free(json);
        ai_lint_result_destroy(result);
        return ASTHRA_TEST_FAIL;
    }

    printf("Generated fix suggestions JSON: %s\n", json);

    free(json);
    ai_lint_result_destroy(result);
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("Running AI Linter JSON Tests...\n");

    AsthraTestContext context = {0};
    int passed_tests = 0;
    int total_tests = 3;

    // Test 1: JSON guidance generation
    AsthraTestResult result1 = test_json_guidance_generation(&context);
    if (result1 == ASTHRA_TEST_PASS) {
        printf("✓ Test 1 passed\n");
        passed_tests++;
    } else {
        printf("✗ Test 1 failed\n");
    }

    // Test 2: Rule catalog JSON
    AsthraTestResult result2 = test_json_rule_catalog(&context);
    if (result2 == ASTHRA_TEST_PASS) {
        printf("✓ Test 2 passed\n");
        passed_tests++;
    } else {
        printf("✗ Test 2 failed\n");
    }

    // Test 3: Fix suggestions JSON
    AsthraTestResult result3 = test_json_fix_suggestions(&context);
    if (result3 == ASTHRA_TEST_PASS) {
        printf("✓ Test 3 passed\n");
        passed_tests++;
    } else {
        printf("✗ Test 3 failed\n");
    }

    printf("\nAI Linter JSON Tests: %d/%d passed\n", passed_tests, total_tests);
    return (passed_tests == total_tests) ? 0 : 1;
}
