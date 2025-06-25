/**
 * Integration test for args() function
 * Tests that args() works correctly in a compiled program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"

static bool test_args_basic_usage(void) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    // For now, just verify args() is recognized as a valid function\n"
                         "    // Full slice support in C code generation is not yet implemented\n"
                         "    args();\n"
                         "    log(\"args() function called successfully\");\n"
                         "    return ();\n"
                         "}\n";

    // Write source to temporary file
    const char *source_file = "test_args_basic.astra";
    FILE *f = fopen(source_file, "w");
    if (!f) {
        printf("Failed to create source file\n");
        return false;
    }
    fprintf(f, "%s", source);
    fclose(f);

    // Try to find the compiler - test executables run from build/bin
    const char *compiler_paths[] = {"../bin/asthra",    // If running from build/tests
                                    "../../bin/asthra", // If running from build/tests/integration
                                    "./asthra",         // If running from build/bin
                                    "asthra",           // If in PATH
                                    NULL};

    const char *compiler = NULL;
    for (int i = 0; compiler_paths[i] != NULL; i++) {
        if (access(compiler_paths[i], X_OK) == 0) {
            compiler = compiler_paths[i];
            break;
        }
    }

    if (!compiler) {
        printf("Could not find asthra compiler\n");
        unlink(source_file);
        return false;
    }

    // Compile the program
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd), "%s %s -o test_args_basic", compiler, source_file);

    int compile_result = system(compile_cmd);
    if (compile_result != 0) {
        printf("Compilation failed with code %d\n", compile_result);
        printf("Command was: %s\n", compile_cmd);
        unlink(source_file);
        return false;
    }

    // Run the compiled program
    int run_result = system("./test_args_basic");

    // Clean up
    unlink(source_file);
    unlink("test_args_basic");

    return run_result == 0;
}

static bool test_args_with_arguments(void) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    // Test that args() is recognized in different contexts\n"
                         "    args();\n"
                         "    log(\"Test with arguments passed\");\n"
                         "    // TODO: Once slice iteration is implemented, iterate over args\n"
                         "    return ();\n"
                         "}\n";

    // Write source to temporary file
    const char *source_file = "test_args_iterate.astra";
    FILE *f = fopen(source_file, "w");
    if (!f) {
        printf("Failed to create source file\n");
        return false;
    }
    fprintf(f, "%s", source);
    fclose(f);

    // Try to find the compiler - test executables run from build/bin
    const char *compiler_paths[] = {"../bin/asthra",    // If running from build/tests
                                    "../../bin/asthra", // If running from build/tests/integration
                                    "./asthra",         // If running from build/bin
                                    "asthra",           // If in PATH
                                    NULL};

    const char *compiler = NULL;
    for (int i = 0; compiler_paths[i] != NULL; i++) {
        if (access(compiler_paths[i], X_OK) == 0) {
            compiler = compiler_paths[i];
            break;
        }
    }

    if (!compiler) {
        printf("Could not find asthra compiler\n");
        unlink(source_file);
        return false;
    }

    // Compile the program
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd), "%s %s -o test_args_iterate", compiler, source_file);

    int compile_result = system(compile_cmd);
    if (compile_result != 0) {
        printf("Compilation failed with code %d\n", compile_result);
        printf("Command was: %s\n", compile_cmd);
        unlink(source_file);
        return false;
    }

    // Run the compiled program with arguments
    int run_result = system("./test_args_iterate arg1 arg2 arg3");

    // Clean up
    unlink(source_file);
    unlink("test_args_iterate");

    return run_result == 0;
}

// =============================================================================
// TEST FRAMEWORK INTEGRATION
// =============================================================================

ASTHRA_TEST_DEFINE(args_basic_usage, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_args_basic_usage() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_with_arguments, ASTHRA_TEST_SEVERITY_HIGH) {
    return test_args_with_arguments() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

int main(void) {
    AsthraTestFunction tests[] = {args_basic_usage, args_with_arguments};

    AsthraTestMetadata metadatas[] = {{.name = "args_basic_usage",
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = "args_basic_usage",
                                       .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                                       .timeout_ns = 0,
                                       .skip = false},
                                      {.name = "args_with_arguments",
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = "args_with_arguments",
                                       .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                       .timeout_ns = 0,
                                       .skip = false}};

    AsthraTestSuiteConfig config = asthra_test_suite_config_create(
        "Args Function Integration Tests", "Tests for args() function runtime behavior");

    AsthraTestResult result =
        asthra_test_run_suite(tests, metadatas, sizeof(tests) / sizeof(tests[0]), &config);

    return result == ASTHRA_TEST_PASS ? 0 : 1;
}