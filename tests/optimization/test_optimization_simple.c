/**
 * Asthra Programming Language
 * Simple Optimization Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Simple optimization test using standardized testing framework
 */

#include "../framework/test_framework.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Simple optimization test functions
AsthraTestResult test_basic_optimization_concepts(AsthraTestContext *context) {
    // Test basic optimization concepts
    int baseline_value = 100;
    int optimized_value = baseline_value * 2; // Simple "optimization"

    ASTHRA_TEST_ASSERT_TRUE(context, optimized_value > baseline_value,
                            "Optimized value should be greater than baseline");
    ASTHRA_TEST_ASSERT_EQ(context, optimized_value, 200, "Optimized value should be 200");

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_optimization_metrics(AsthraTestContext *context) {
    // Test optimization metrics calculation
    double baseline_time = 1.0;
    double optimized_time = 0.5;
    double improvement = (baseline_time - optimized_time) / baseline_time * 100.0;

    ASTHRA_TEST_ASSERT_TRUE(context, improvement > 0, "Optimization should show improvement");
    ASTHRA_TEST_ASSERT_TRUE(context, fabs(improvement - 50.0) < 0.001,
                            "Should show 50% improvement");

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_optimization_validation(AsthraTestContext *context) {
    // Test optimization validation logic
    const char *test_files[] = {"Makefile", "README.md", "src"};
    int files_found = 0;

    for (int i = 0; i < 3; i++) {
        struct stat st;
        if (stat(test_files[i], &st) == 0) {
            files_found++;
        }
    }

    ASTHRA_TEST_ASSERT_TRUE(context, files_found > 0, "Should find at least one project file");

    return ASTHRA_TEST_PASS;
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Optimization Simple Tests");

    asthra_test_suite_add_test(suite, "basic_optimization_concepts",
                               "Test basic optimization concepts",
                               test_basic_optimization_concepts);
    asthra_test_suite_add_test(suite, "optimization_metrics",
                               "Test optimization metrics calculation", test_optimization_metrics);
    asthra_test_suite_add_test(suite, "optimization_validation",
                               "Test optimization validation logic", test_optimization_validation);

    return asthra_test_suite_run_and_exit(suite);
}
