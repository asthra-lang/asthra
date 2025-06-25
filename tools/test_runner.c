/**
 * Asthra Programming Language
 * Test runner tool
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <getopt.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../runtime/asthra_runtime.h"
#include "../src/platform.h"

#if ASTHRA_PLATFORM_UNIX
#include <sys/wait.h>
#endif
#include "../src/compiler.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for test runner");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void *),
                     "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe test statistics
typedef struct {
    _Atomic(uint64_t) tests_run;
    _Atomic(uint64_t) tests_passed;
    _Atomic(uint64_t) tests_failed;
    _Atomic(uint64_t) tests_skipped;
    _Atomic(uint64_t) total_time_ms;
} TestStatistics;

// Test result types
typedef enum { TEST_PASS, TEST_FAIL, TEST_SKIP, TEST_ERROR } TestResult;

// Test case structure
typedef struct {
    const char *name;
    const char *file_path;
    TestResult result;
    uint64_t duration_ms;
    const char *error_message;
} TestCase;

// C17 designated initializer for test runner options
typedef struct {
    const char *test_directory;
    const char *pattern;
    bool verbose;
    bool parallel;
    bool stop_on_failure;
    bool json_output;
    int timeout_seconds;
    TestStatistics *stats;
} TestRunnerOptions;

// C17 _Generic for polymorphic test operations
#define run_test(test)                                                                             \
    _Generic((test),                                                                               \
        char *: run_test_file,                                                                     \
        const char *: run_test_file,                                                               \
        TestCase *: run_test_case,                                                                 \
        default: run_test_generic)(test)

// Forward declarations for _Generic dispatch
static TestResult run_test_file(const char *file_path);
static TestResult run_test_case(TestCase *test_case);
static TestResult run_test_generic(void *test);

// C17 atomic operations for thread-safe statistics
static inline void increment_stat(_Atomic(uint64_t) *counter, uint64_t value) {
    atomic_fetch_add_explicit(counter, value, memory_order_relaxed);
}

static inline uint64_t get_stat(_Atomic(uint64_t) *counter) {
    return atomic_load_explicit(counter, memory_order_relaxed);
}

static void print_usage(const char *program_name) {
    printf("Asthra Test Runner\n");
    printf("Usage: %s [options] [test_directory]\n\n", program_name);
    printf("Options:\n");
    printf("  -p, --pattern <pattern> Test file pattern (default: *.asthra)\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -j, --parallel          Run tests in parallel\n");
    printf("  -s, --stop-on-failure   Stop on first failure\n");
    printf("  -J, --json              Output results in JSON format\n");
    printf("  -t, --timeout <seconds> Test timeout (default: 30)\n");
    printf("  -h, --help              Show this help message\n");
}

static TestStatistics *create_test_statistics(void) {
    TestStatistics *stats = malloc(sizeof(TestStatistics));
    if (!stats) {
        return NULL;
    }

    // C17 designated initializer with atomic initialization
    *stats = (TestStatistics){.tests_run = 0,
                              .tests_passed = 0,
                              .tests_failed = 0,
                              .tests_skipped = 0,
                              .total_time_ms = 0};

    return stats;
}

static void destroy_test_statistics(TestStatistics *stats) {
    free(stats);
}

static int parse_arguments(int argc, char *argv[], TestRunnerOptions *options) {
    // C17 designated initializer for long options
    static struct option long_options[] = {
        {.name = "pattern", .has_arg = required_argument, .flag = 0, .val = 'p'},
        {.name = "verbose", .has_arg = no_argument, .flag = 0, .val = 'v'},
        {.name = "parallel", .has_arg = no_argument, .flag = 0, .val = 'j'},
        {.name = "stop-on-failure", .has_arg = no_argument, .flag = 0, .val = 's'},
        {.name = "json", .has_arg = no_argument, .flag = 0, .val = 'J'},
        {.name = "timeout", .has_arg = required_argument, .flag = 0, .val = 't'},
        {.name = "help", .has_arg = no_argument, .flag = 0, .val = 'h'},
        {0, 0, 0, 0}};

    // C17 designated initializer for default options
    *options = (TestRunnerOptions){.test_directory = "tests",
                                   .pattern = "*.asthra",
                                   .verbose = false,
                                   .parallel = false,
                                   .stop_on_failure = false,
                                   .json_output = false,
                                   .timeout_seconds = 30,
                                   .stats = create_test_statistics()};

    if (!options->stats) {
        fprintf(stderr, "Error: Failed to create test statistics\n");
        return -1;
    }

    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "p:vjsJt:h", long_options, &option_index)) != -1) {
        switch (c) {
        case 'p':
            options->pattern = optarg;
            break;
        case 'v':
            options->verbose = true;
            break;
        case 'j':
            options->parallel = true;
            break;
        case 's':
            options->stop_on_failure = true;
            break;
        case 'J':
            options->json_output = true;
            break;
        case 't':
            options->timeout_seconds = atoi(optarg);
            if (options->timeout_seconds <= 0) {
                fprintf(stderr, "Error: Invalid timeout: %s\n", optarg);
                return -1;
            }
            break;
        case 'h':
            print_usage(argv[0]);
            return 1;
        case '?':
            return -1;
        default:
            abort();
        }
    }

    // Check for test directory argument
    if (optind < argc) {
        options->test_directory = argv[optind];
    }

    // Validate test directory exists
    if (access(options->test_directory, R_OK) != 0) {
        perror(options->test_directory);
        return -1;
    }

    return 0;
}

// C17 _Generic dispatch implementations
static TestResult run_test_file(const char *file_path) {
    if (!file_path)
        return TEST_ERROR;

    // Simple test execution - compile and run
    char compile_cmd[1024];
    snprintf(compile_cmd, sizeof(compile_cmd), "./bin/asthra %s -o /tmp/test_output", file_path);

    int compile_result = system(compile_cmd);
    if (compile_result == -1) {
        return TEST_ERROR;
    }
#if ASTHRA_PLATFORM_UNIX
    if (WIFEXITED(compile_result) && WEXITSTATUS(compile_result) != 0) {
        return TEST_FAIL;
    }
#else
    if (compile_result != 0) {
        return TEST_FAIL;
    }
#endif

    // Run the compiled test
    int run_result = system("/tmp/test_output");
    if (run_result == -1) {
        return TEST_ERROR;
    }
#if ASTHRA_PLATFORM_UNIX
    return (WIFEXITED(run_result) && WEXITSTATUS(run_result) == 0) ? TEST_PASS : TEST_FAIL;
#else
    return (run_result == 0) ? TEST_PASS : TEST_FAIL;
#endif
}

static TestResult run_test_case(TestCase *test_case) {
    if (!test_case)
        return TEST_ERROR;

    clock_t start = clock();
    TestResult result = run_test_file(test_case->file_path);
    clock_t end = clock();

    test_case->duration_ms = ((end - start) * 1000) / CLOCKS_PER_SEC;
    test_case->result = result;

    return result;
}

static TestResult run_test_generic(void *test) {
    (void)test;        // Suppress unused parameter warning
    return TEST_ERROR; // Generic fallback
}

// C17 compound literal for test result mapping
static const char *result_to_string(TestResult result) {
    static const struct {
        TestResult result;
        const char *string;
    } result_strings[] = {
        {TEST_PASS, "PASS"}, {TEST_FAIL, "FAIL"}, {TEST_SKIP, "SKIP"}, {TEST_ERROR, "ERROR"}};

    for (size_t i = 0; i < sizeof(result_strings) / sizeof(result_strings[0]); i++) {
        if (result_strings[i].result == result) {
            return result_strings[i].string;
        }
    }

    return "UNKNOWN";
}

static TestCase *discover_tests(const char *directory, const char *pattern, size_t *test_count) {
    // Simplified test discovery - in a real implementation, this would
    // scan the directory for files matching the pattern

    // For now, create some mock test cases
    static TestCase mock_tests[] = {{.name = "basic_syntax",
                                     .file_path = "tests/basic_syntax.asthra",
                                     .result = TEST_PASS,
                                     .duration_ms = 0,
                                     .error_message = NULL},
                                    {.name = "arithmetic",
                                     .file_path = "tests/arithmetic.asthra",
                                     .result = TEST_PASS,
                                     .duration_ms = 0,
                                     .error_message = NULL},
                                    {.name = "functions",
                                     .file_path = "tests/functions.asthra",
                                     .result = TEST_PASS,
                                     .duration_ms = 0,
                                     .error_message = NULL},
                                    {.name = "control_flow",
                                     .file_path = "tests/control_flow.asthra",
                                     .result = TEST_PASS,
                                     .duration_ms = 0,
                                     .error_message = NULL}};

    (void)directory;
    (void)pattern; // Suppress unused parameter warnings

    *test_count = sizeof(mock_tests) / sizeof(mock_tests[0]);

    // Allocate and copy test cases
    TestCase *tests = malloc(*test_count * sizeof(TestCase));
    if (tests) {
        memcpy(tests, mock_tests, *test_count * sizeof(TestCase));
    }

    return tests;
}

static void run_tests_sequential(TestCase *tests, size_t test_count, TestRunnerOptions *options) {
    for (size_t i = 0; i < test_count; i++) {
        TestCase *test = &tests[i];

        if (options->verbose) {
            printf("Running test: %s... ", test->name);
            fflush(stdout);
        }

        clock_t start = clock();
        TestResult result = run_test_case(test);
        clock_t end = clock();

        uint64_t duration_ms = ((end - start) * 1000) / CLOCKS_PER_SEC;
        test->duration_ms = duration_ms;

        // Update statistics atomically
        increment_stat(&options->stats->tests_run, 1);
        increment_stat(&options->stats->total_time_ms, duration_ms);

        switch (result) {
        case TEST_PASS:
            increment_stat(&options->stats->tests_passed, 1);
            if (options->verbose) {
                printf("%s (%llums)\n", result_to_string(result), (unsigned long long)duration_ms);
            }
            break;
        case TEST_FAIL:
            increment_stat(&options->stats->tests_failed, 1);
            if (options->verbose) {
                printf("%s (%llums)\n", result_to_string(result), (unsigned long long)duration_ms);
            }
            if (options->stop_on_failure) {
                return;
            }
            break;
        case TEST_SKIP:
            increment_stat(&options->stats->tests_skipped, 1);
            if (options->verbose) {
                printf("%s\n", result_to_string(result));
            }
            break;
        case TEST_ERROR:
            increment_stat(&options->stats->tests_failed, 1);
            if (options->verbose) {
                printf("%s\n", result_to_string(result));
            }
            if (options->stop_on_failure) {
                return;
            }
            break;
        }
    }
}

static void print_results_text(const TestCase *tests, size_t test_count,
                               const TestRunnerOptions *options) {
    printf("\nTest Results:\n");
    printf("=============\n");

    for (size_t i = 0; i < test_count; i++) {
        const TestCase *test = &tests[i];
        printf("%-20s %s", test->name, result_to_string(test->result));

        if (test->duration_ms > 0) {
            printf(" (%llums)", (unsigned long long)test->duration_ms);
        }

        if (test->error_message) {
            printf(" - %s", test->error_message);
        }

        printf("\n");
    }

    printf("\nSummary:\n");
    printf("  Tests run: %llu\n", (unsigned long long)get_stat(&options->stats->tests_run));
    printf("  Passed: %llu\n", (unsigned long long)get_stat(&options->stats->tests_passed));
    printf("  Failed: %llu\n", (unsigned long long)get_stat(&options->stats->tests_failed));
    printf("  Skipped: %llu\n", (unsigned long long)get_stat(&options->stats->tests_skipped));
    printf("  Total time: %llums\n", (unsigned long long)get_stat(&options->stats->total_time_ms));
}

static void print_results_json(const TestCase *tests, size_t test_count,
                               const TestRunnerOptions *options) {
    printf("{\n");
    printf("  \"summary\": {\n");
    printf("    \"tests_run\": %llu,\n", (unsigned long long)get_stat(&options->stats->tests_run));
    printf("    \"tests_passed\": %llu,\n",
           (unsigned long long)get_stat(&options->stats->tests_passed));
    printf("    \"tests_failed\": %llu,\n",
           (unsigned long long)get_stat(&options->stats->tests_failed));
    printf("    \"tests_skipped\": %llu,\n",
           (unsigned long long)get_stat(&options->stats->tests_skipped));
    printf("    \"total_time_ms\": %llu\n",
           (unsigned long long)get_stat(&options->stats->total_time_ms));
    printf("  },\n");
    printf("  \"tests\": [\n");

    for (size_t i = 0; i < test_count; i++) {
        const TestCase *test = &tests[i];

        if (i > 0) {
            printf(",\n");
        }

        printf("    {\n");
        printf("      \"name\": \"%s\",\n", test->name);
        printf("      \"file\": \"%s\",\n", test->file_path);
        printf("      \"result\": \"%s\",\n", result_to_string(test->result));
        printf("      \"duration_ms\": %llu", (unsigned long long)test->duration_ms);

        if (test->error_message) {
            printf(",\n      \"error\": \"%s\"", test->error_message);
        }

        printf("\n    }");
    }

    printf("\n  ]\n");
    printf("}\n");
}

int main(int argc, char *argv[]) {
    TestRunnerOptions options;

    // Parse command line arguments
    int parse_result = parse_arguments(argc, argv, &options);
    if (parse_result != 0) {
        if (options.stats) {
            destroy_test_statistics(options.stats);
        }
        return (parse_result > 0) ? 0 : 1; // 1 for help, -1 for error
    }

    if (options.verbose) {
        printf("Asthra Test Runner\n");
        printf("Test directory: %s\n", options.test_directory);
        printf("Pattern: %s\n", options.pattern);
        printf("Parallel: %s\n", options.parallel ? "yes" : "no");
        printf("\n");
    }

    // Discover tests
    size_t test_count;
    TestCase *tests = discover_tests(options.test_directory, options.pattern, &test_count);

    if (!tests) {
        fprintf(stderr, "Error: Failed to discover tests\n");
        destroy_test_statistics(options.stats);
        return 1;
    }

    if (test_count == 0) {
        if (options.verbose) {
            printf("No tests found in %s\n", options.test_directory);
        }
        free(tests);
        destroy_test_statistics(options.stats);
        return 0;
    }

    if (options.verbose) {
        printf("Found %zu test(s)\n\n", test_count);
    }

    // Run tests
    if (options.parallel) {
        // TODO: Implement parallel test execution
        if (options.verbose) {
            printf("Parallel execution not yet implemented, running sequentially\n");
        }
    }

    run_tests_sequential(tests, test_count, &options);

    // Print results
    if (options.json_output) {
        print_results_json(tests, test_count, &options);
    } else {
        print_results_text(tests, test_count, &options);
    }

    // Determine exit code
    uint64_t failed_count = get_stat(&options.stats->tests_failed);
    int exit_code = (failed_count > 0) ? 1 : 0;

    // Cleanup
    free(tests);
    destroy_test_statistics(options.stats);

    return exit_code;
}
