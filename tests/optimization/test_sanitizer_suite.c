/**
 * Asthra Programming Language
 * Clang Sanitizer Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for Clang Sanitizers (ASan, TSan, UBSan, MSan) integration
 */

#include "test_optimization_common.h"

// Forward declarations
static AsthraTestResult test_sanitizer_makefile_targets(AsthraTestContext *context);
static AsthraTestResult test_sanitizer_binaries_created(AsthraTestContext *context);
static AsthraTestResult test_sanitizer_bug_detection(AsthraTestContext *context);

static AsthraTestResult test_sanitizer_makefile_targets(AsthraTestContext *context) {
    // Skip this test in CI environments or when make is not available
    CommandResult make_check = execute_command("which make", 5);
    if (make_check.exit_code != 0) {
        printf("Skipping sanitizer makefile tests - make not available\n");
        return ASTHRA_TEST_SKIP;
    }

    // Also skip if we're in a test-only environment (no source tree)
    if (!file_exists("Makefile")) {
        printf("Skipping sanitizer makefile tests - not in source tree\n");
        return ASTHRA_TEST_SKIP;
    }

    const char *sanitizer_targets[] = {"sanitizer-asan", "sanitizer-ubsan", "sanitizer-tsan"};

    for (size_t i = 0; i < sizeof(sanitizer_targets) / sizeof(sanitizer_targets[0]); i++) {
        char make_command[256];
        snprintf(make_command, sizeof(make_command), "make %s", sanitizer_targets[i]);
        CommandResult result = execute_command(make_command, TEST_TIMEOUT_SECONDS);

        if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                                   "Sanitizer target should succeed: %s - %s", sanitizer_targets[i],
                                   result.error)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test comprehensive sanitizer target
    CommandResult result = execute_command("make sanitizer-comprehensive", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Comprehensive sanitizer target should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_sanitizer_binaries_created(AsthraTestContext *context) {
    // Skip if we're not in a build environment
    if (!file_exists("bin/asthra")) {
        printf("Skipping sanitizer binary tests - not in build environment\n");
        return ASTHRA_TEST_SKIP;
    }

    const char *expected_binaries[] = {"bin/asthra-asan", "bin/asthra-ubsan", "bin/asthra-tsan",
                                       "bin/asthra-asan-ubsan"};

    for (size_t i = 0; i < sizeof(expected_binaries) / sizeof(expected_binaries[0]); i++) {
        if (!ASTHRA_TEST_ASSERT(context, file_exists(expected_binaries[i]),
                                "Sanitizer binary should exist: %s", expected_binaries[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(context, validate_binary_optimization(expected_binaries[i]),
                                "Sanitizer binary should be valid: %s", expected_binaries[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_sanitizer_bug_detection(AsthraTestContext *context) {
    // Skip if clang is not available
    CommandResult clang_check = execute_command("which clang", 5);
    if (clang_check.exit_code != 0) {
        printf("Skipping sanitizer bug detection tests - clang not available\n");
        return ASTHRA_TEST_SKIP;
    }

    // Create test files with intentional bugs for sanitizer detection

    // 1. Memory leak test for AddressSanitizer
    const char *memory_leak_test = "test_memory_leak.c";
    FILE *leak_file = fopen(memory_leak_test, "w");
    if (leak_file) {
        fprintf(leak_file, "#include <stdlib.h>\n");
        fprintf(leak_file, "int main(void) {\n");
        fprintf(leak_file, "    void *ptr = malloc(100);\n");
        fprintf(leak_file, "    // Intentional memory leak - don't free ptr\n");
        fprintf(leak_file, "    return 0;\n");
        fprintf(leak_file, "}\n");
        fclose(leak_file);
    }

    // 2. Buffer overflow test for AddressSanitizer
    const char *buffer_overflow_test = "test_buffer_overflow.c";
    FILE *overflow_file = fopen(buffer_overflow_test, "w");
    if (overflow_file) {
        fprintf(overflow_file, "#include <string.h>\n");
        fprintf(overflow_file, "int main(void) {\n");
        fprintf(overflow_file, "    char buffer[10];\n");
        fprintf(overflow_file, "    // Intentional buffer overflow for sanitizer testing\n");
        fprintf(overflow_file,
                "    memcpy(buffer, \"This string is too long for the buffer\", 40);\n");
        fprintf(overflow_file, "    return 0;\n");
        fprintf(overflow_file, "}\n");
        fclose(overflow_file);
    }

    // 3. Undefined behavior test for UBSan
    const char *undefined_behavior_test = "test_undefined_behavior.c";
    FILE *ub_file = fopen(undefined_behavior_test, "w");
    if (ub_file) {
        fprintf(ub_file, "#include <limits.h>\n");
        fprintf(ub_file, "int main(void) {\n");
        fprintf(ub_file, "    int x = INT_MAX;\n");
        fprintf(ub_file, "    x = x + 1; // Integer overflow\n");
        fprintf(ub_file, "    return 0;\n");
        fprintf(ub_file, "}\n");
        fclose(ub_file);
    }

    // Test AddressSanitizer detection
    char asan_command[MAX_COMMAND_LENGTH];
    snprintf(asan_command, sizeof(asan_command),
             "clang -fsanitize=address -g %s -o /tmp/test_asan && /tmp/test_asan",
             memory_leak_test);

    CommandResult asan_result = execute_command(asan_command, 30);
    // ASan should detect the leak (non-zero exit code expected)
    if (!ASTHRA_TEST_ASSERT(context,
                            asan_result.exit_code != 0 || strstr(asan_result.error, "leak") != NULL,
                            "AddressSanitizer should detect memory leak")) {
        printf("ASan output: %s\n", asan_result.error);
    }

    // Test UBSan detection
    char ubsan_command[MAX_COMMAND_LENGTH];
    snprintf(ubsan_command, sizeof(ubsan_command),
             "clang -fsanitize=undefined -g %s -o /tmp/test_ubsan && /tmp/test_ubsan",
             undefined_behavior_test);

    CommandResult ubsan_result = execute_command(ubsan_command, 30);
    // UBSan should detect undefined behavior
    if (!ASTHRA_TEST_ASSERT(
            context, ubsan_result.exit_code != 0 || strstr(ubsan_result.error, "overflow") != NULL,
            "UndefinedBehaviorSanitizer should detect integer overflow")) {
        printf("UBSan output: %s\n", ubsan_result.error);
    }

    // Cleanup test files
    unlink(memory_leak_test);
    unlink(buffer_overflow_test);
    unlink(undefined_behavior_test);
    unlink("/tmp/test_asan");
    unlink("/tmp/test_ubsan");

    return ASTHRA_TEST_PASS;
}

// Export test functions and metadata for the main test runner
AsthraTestFunction sanitizer_test_functions[] = {
    test_sanitizer_makefile_targets, test_sanitizer_binaries_created, test_sanitizer_bug_detection};

AsthraTestMetadata sanitizer_test_metadata[] = {
    {"Sanitizer Makefile Targets", __FILE__, __LINE__, "test_sanitizer_makefile_targets",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Sanitizer Binaries Created", __FILE__, __LINE__, "test_sanitizer_binaries_created",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Sanitizer Bug Detection", __FILE__, __LINE__, "test_sanitizer_bug_detection",
     ASTHRA_TEST_SEVERITY_CRITICAL, 0, false, NULL}};

const size_t sanitizer_test_count =
    sizeof(sanitizer_test_functions) / sizeof(sanitizer_test_functions[0]);

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Sanitizer Tests");

    for (size_t i = 0; i < sanitizer_test_count; i++) {
        asthra_test_suite_add_test(suite, sanitizer_test_metadata[i].name,
                                   sanitizer_test_metadata[i].name, sanitizer_test_functions[i]);
    }

    return asthra_test_suite_run_and_exit(suite);
}
