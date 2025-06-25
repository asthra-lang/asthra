#include "concurrency_test_utils.h"
#include "parser_string_interface.h"
#include <stdio.h>
#include <string.h>

// Simple test to check if a minimal function with 'none' parameters parses correctly
static ConcurrencyTestResult test_simple_none_function(AsthraV12TestContext *context) {
    const char *simple_code = "package test;\n"
                              "pub fn simple(none) -> void {\n"
                              "    \n"
                              "}\n";

    printf("Testing simple none function:\n%s\n", simple_code);

    ParseResult result = parse_string(simple_code);

    printf("Parse result: success=%s\n", result.success ? "true" : "false");
    printf("Error count: %d\n", result.error_count);

    if (result.error_count > 0) {
        for (int i = 0; i < result.error_count; i++) {
            printf("Error %d: %s\n", i + 1, result.errors[i]);
        }
    }

    bool success = result.success;
    cleanup_parse_result(&result);

    if (success) {
        printf("✅ Simple none function parsed successfully\n");
        return CONCURRENCY_TEST_PASS;
    } else {
        printf("❌ Simple none function failed to parse\n");
        return CONCURRENCY_TEST_FAIL;
    }
}

int main(void) {
    AsthraV12TestContext context = {0};
    test_context_init(&context);

    printf("=== Testing Simple None Parameter Parsing ===\n");
    ConcurrencyTestResult result = test_simple_none_function(&context);

    test_context_cleanup(&context);

    if (result == CONCURRENCY_TEST_PASS) {
        printf("TEST PASSED\n");
        return 0;
    } else {
        printf("TEST FAILED\n");
        return 1;
    }
}
