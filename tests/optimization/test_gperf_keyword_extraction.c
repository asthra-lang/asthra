/**
 * Asthra Programming Language
 * Gperf Keyword Extraction Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_gperf_common.h"

static AsthraTestResult test_keyword_extraction_script(AsthraTestContext *context) {
    const char *extraction_script = "optimization/extract_keywords_main.sh";

    // Check if extraction script exists
    if (!ASTHRA_TEST_ASSERT(context, gperf_file_exists(extraction_script),
                            "Keyword extraction script should exist")) {
        return ASTHRA_TEST_SKIP;
    }

    // Test script execution
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "bash %s --dry-run", extraction_script);

    int exit_code = run_command_with_timeout(command, 30.0);
    if (!ASTHRA_TEST_ASSERT_EQ(context, exit_code, 0,
                               "Keyword extraction script should run successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_extracted_keywords_completeness(AsthraTestContext *context) {
    const char *gperf_file = "optimization/asthra_keywords.gperf";
    FILE *file = fopen(gperf_file, "r");

    if (!ASTHRA_TEST_ASSERT(context, file != NULL, "Should be able to open gperf file")) {
        return ASTHRA_TEST_FAIL;
    }

    char line[256];
    bool *found_keywords = calloc(test_keywords_count, sizeof(bool));
    if (!found_keywords) {
        fclose(file);
        return ASTHRA_TEST_FAIL;
    }
    size_t found_count = 0;

    while (fgets(line, sizeof(line), file)) {
        // Skip comments and directives
        if (line[0] == '#' || line[0] == '%') {
            continue;
        }

        // Check if this line contains any of our test keywords
        for (size_t i = 0; i < test_keywords_count; i++) {
            if (!found_keywords[i] && strstr(line, test_keywords[i].keyword)) {
                found_keywords[i] = true;
                found_count++;
            }
        }
    }

    fclose(file);

    // We should find at least 80% of the expected keywords
    size_t required_count = (test_keywords_count * 80) / 100;

    if (!ASTHRA_TEST_ASSERT(context, found_count >= required_count,
                            "Should find at least %zu keywords, found %zu", required_count,
                            found_count)) {
        // Print missing keywords for debugging
        printf("Missing keywords:\n");
        for (size_t i = 0; i < test_keywords_count; i++) {
            if (!found_keywords[i]) {
                printf("  %s\n", test_keywords[i].keyword);
            }
        }
        free(found_keywords);
        return ASTHRA_TEST_FAIL;
    }

    printf("Keyword extraction completeness: %zu/%zu (%.1f%%)\n", found_count, test_keywords_count,
           (double)found_count / test_keywords_count * 100.0);

    free(found_keywords);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_keyword_extraction_accuracy(AsthraTestContext *context) {
    const char *gperf_file = "optimization/asthra_keywords.gperf";
    FILE *file = fopen(gperf_file, "r");

    if (!ASTHRA_TEST_ASSERT(context, file != NULL, "Should be able to open gperf file")) {
        return ASTHRA_TEST_FAIL;
    }

    char line[256];
    int valid_keyword_lines = 0;
    int total_keyword_lines = 0;
    bool in_keywords_section = false;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "%%")) {
            in_keywords_section = !in_keywords_section;
            continue;
        }

        if (!in_keywords_section || line[0] == '#' || line[0] == '%') {
            continue;
        }

        // Count as keyword line if it has content
        if (strlen(line) > 2) {
            total_keyword_lines++;

            // Check if it looks like a valid keyword line (has comma, no obvious syntax errors)
            if (strstr(line, ",") && !strstr(line, "//") && !strstr(line, "/*")) {
                valid_keyword_lines++;
            }
        }
    }

    fclose(file);

    // At least 90% of keyword lines should be valid
    double validity_ratio =
        total_keyword_lines > 0 ? (double)valid_keyword_lines / total_keyword_lines : 0.0;

    if (!ASTHRA_TEST_ASSERT(context, validity_ratio >= 0.9,
                            "At least 90%% of keyword lines should be valid (%.1f%%)",
                            validity_ratio * 100.0)) {
        return ASTHRA_TEST_FAIL;
    }

    printf("Keyword extraction accuracy: %d/%d valid lines (%.1f%%)\n", valid_keyword_lines,
           total_keyword_lines, validity_ratio * 100.0);

    return ASTHRA_TEST_PASS;
}

// Public test suite interface
AsthraTestFunction gperf_keyword_extraction_tests[] = {test_keyword_extraction_script,
                                                       test_extracted_keywords_completeness,
                                                       test_keyword_extraction_accuracy};

AsthraTestMetadata gperf_keyword_extraction_metadata[] = {
    {"Keyword Extraction Script", __FILE__, __LINE__, "test_keyword_extraction_script",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
    {"Extracted Keywords Completeness", __FILE__, __LINE__, "test_extracted_keywords_completeness",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Keyword Extraction Accuracy", __FILE__, __LINE__, "test_keyword_extraction_accuracy",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}};

const size_t gperf_keyword_extraction_test_count =
    sizeof(gperf_keyword_extraction_tests) / sizeof(gperf_keyword_extraction_tests[0]);
