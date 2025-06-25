/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Master Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Master test suite that runs all FFI assembly generator test suites:
 * - FFI calls
 * - Pattern matching
 * - String operations
 * - Slice operations
 * - Security operations
 * - Concurrency operations
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Define test categories
typedef enum {
    TEST_CALLS,
    TEST_PATTERNS,
    TEST_STRINGS_SLICES,
    TEST_SECURITY_CONCURRENCY,
    TEST_COUNT
} TestCategory;

// Test information structure
typedef struct {
    const char *name;
    const char *executable;
    bool run;
} TestInfo;

// Global test info
static TestInfo g_tests[TEST_COUNT] = {
    {.name = "FFI Calls", .executable = "../../bin/ffi_test_ffi_assembly_calls", .run = true},
    {
        .name = "Pattern Matching",
        .executable = "../../bin/test_ffi_assembly_patterns",
        .run = false // Binary doesn't exist yet
    },
    {
        .name = "Strings and Slices",
        .executable = "../../bin/test_ffi_assembly_strings_slices",
        .run = false // Binary doesn't exist yet
    },
    {
        .name = "Security and Concurrency",
        .executable = "../../bin/test_ffi_assembly_security_concurrency",
        .run = false // Binary doesn't exist yet
    }};

// Test statistics
static int g_total_tests = 0;
static int g_passed_tests = 0;

// Run a specific test suite in a child process
static bool run_test_suite(const TestInfo *test) {
    printf("\n========================================================\n");
    printf("Running Test Suite: %s\n", test->name);
    printf("========================================================\n");

    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        fprintf(stderr, "ERROR: Failed to fork process for test suite '%s'\n", test->name);
        return false;
    } else if (pid == 0) {
        // Child process
        execl(test->executable, test->executable, NULL);

        // If we reach here, exec failed
        fprintf(stderr, "ERROR: Failed to execute '%s'\n", test->executable);
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);

            if (exit_code == 0) {
                printf("\n✅ Test Suite '%s' PASSED\n", test->name);
                return true;
            } else {
                printf("\n❌ Test Suite '%s' FAILED (exit code: %d)\n", test->name, exit_code);
                return false;
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "ERROR: Test suite '%s' terminated by signal %d\n", test->name,
                    WTERMSIG(status));
            return false;
        } else {
            fprintf(stderr, "ERROR: Test suite '%s' terminated abnormally\n", test->name);
            return false;
        }
    }
}

// Run all test suites and report results
static void run_all_tests(void) {
    clock_t start_time = clock();

    printf("========================================================\n");
    printf("FFI Assembly Generator - Complete Test Suite\n");
    printf("========================================================\n");

    g_total_tests = 0;
    g_passed_tests = 0;

    // First, check which tests can actually be run
    for (int i = 0; i < TEST_COUNT; i++) {
        if (g_tests[i].run) {
            // Check if the executable exists
            if (access(g_tests[i].executable, X_OK) != 0) {
                printf("\n⚠️  Skipping Test Suite '%s' (executable not found: %s)\n",
                       g_tests[i].name, g_tests[i].executable);
                g_tests[i].run = false;
            }
        }
    }

    for (int i = 0; i < TEST_COUNT; i++) {
        if (g_tests[i].run) {
            g_total_tests++;
            if (run_test_suite(&g_tests[i])) {
                g_passed_tests++;
            }
        }
    }

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("\n========================================================\n");
    printf("Test Summary\n");
    printf("========================================================\n");
    printf("Test Suites: %d/%d passed (%.1f%%)\n", g_passed_tests, g_total_tests,
           (g_total_tests > 0) ? ((float)g_passed_tests / g_total_tests * 100.0f) : 0.0f);
    printf("Total Execution Time: %.2f seconds\n", elapsed_time);

    if (g_passed_tests == g_total_tests) {
        printf("\n🎉 ALL TEST SUITES PASSED!\n");
    } else {
        printf("\n❌ SOME TEST SUITES FAILED!\n");
    }
}

// Print usage information
static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Options:\n");
    printf("  --help           Display this help message\n");
    printf("  --only=CATEGORY  Run only the specified test category:\n");
    printf("                   calls, patterns, strings, security\n");
    printf("  --skip=CATEGORY  Skip the specified test category\n");
    printf("  --all            Run all test categories (default)\n");
}

// Parse command line arguments
static void parse_args(int argc, char *argv[]) {
    // Default is to run all tests
    for (int i = 0; i < TEST_COUNT; i++) {
        g_tests[i].run = true;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strncmp(argv[i], "--only=", 7) == 0) {
            // Set all tests to not run by default
            for (int j = 0; j < TEST_COUNT; j++) {
                g_tests[j].run = false;
            }

            // Enable only the specified test
            const char *category = argv[i] + 7;

            if (strcmp(category, "calls") == 0) {
                g_tests[TEST_CALLS].run = true;
            } else if (strcmp(category, "patterns") == 0) {
                g_tests[TEST_PATTERNS].run = true;
            } else if (strcmp(category, "strings") == 0) {
                g_tests[TEST_STRINGS_SLICES].run = true;
            } else if (strcmp(category, "security") == 0) {
                g_tests[TEST_SECURITY_CONCURRENCY].run = true;
            } else {
                fprintf(stderr, "ERROR: Unknown test category '%s'\n", category);
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(argv[i], "--skip=", 7) == 0) {
            // Disable the specified test
            const char *category = argv[i] + 7;

            if (strcmp(category, "calls") == 0) {
                g_tests[TEST_CALLS].run = false;
            } else if (strcmp(category, "patterns") == 0) {
                g_tests[TEST_PATTERNS].run = false;
            } else if (strcmp(category, "strings") == 0) {
                g_tests[TEST_STRINGS_SLICES].run = false;
            } else if (strcmp(category, "security") == 0) {
                g_tests[TEST_SECURITY_CONCURRENCY].run = false;
            } else {
                fprintf(stderr, "ERROR: Unknown test category '%s'\n", category);
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "--all") == 0) {
            // Enable all tests
            for (int j = 0; j < TEST_COUNT; j++) {
                g_tests[j].run = true;
            }
        } else {
            fprintf(stderr, "ERROR: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    // Parse command line arguments
    parse_args(argc, argv);

    // Run tests
    run_all_tests();

    // Return success if all tests passed
    return (g_passed_tests == g_total_tests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
