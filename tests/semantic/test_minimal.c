#include "../framework/test_framework.h"
#include <stdio.h>

ASTHRA_TEST_DEFINE(test_minimal, ASTHRA_TEST_SEVERITY_CRITICAL) {
    printf("Minimal test running\n");
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("Starting minimal test\n");

    AsthraTestMetadata metadata = {.name = "test_minimal",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "test_minimal",
                                   .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                                   .timeout_ns = 0,
                                   .skip = false};

    AsthraTestFunction tests[] = {test_minimal};
    AsthraTestMetadata metadatas[] = {metadata};

    AsthraTestSuiteConfig config =
        asthra_test_suite_config_create("Minimal Test Suite", "Testing minimal functionality");

    AsthraTestResult result = asthra_test_run_suite(tests, metadatas, 1, &config);

    printf("Test result: %s\n", result == ASTHRA_TEST_PASS ? "PASSED" : "FAILED");

    return result == ASTHRA_TEST_PASS ? 0 : 1;
}