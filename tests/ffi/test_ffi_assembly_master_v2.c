/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Test Suite - Master Runner v2.0
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Master test runner that coordinates all FFI assembly generator test modules
 */

#include "test_ffi_assembly_common.h"
#include <time.h>

// External test suite runners
extern int run_ffi_core_tests(void);
extern int run_ffi_pattern_matching_tests(void);
extern int run_ffi_string_slice_tests(void);
extern int run_ffi_security_concurrency_tests(void);
extern int run_ffi_optimization_tests(void);

// Test suite information
typedef struct {
    const char *name;
    const char *description;
    int (*run_tests)(void);
} TestSuite;

static TestSuite test_suites[] = {
    {
        "Core FFI",
        "Core FFI call generation, parameter marshaling, and struct layouts",
        run_ffi_core_tests
    },
    {
        "Pattern Matching",
        "Pattern matching, Result<T,E> types, and destructuring",
        run_ffi_pattern_matching_tests
    },
    {
        "String & Slice",
        "String operations, slice operations, and conversions",
        run_ffi_string_slice_tests
    },
    {
        "Security & Concurrency",
        "Security features, concurrency operations, and unsafe blocks",
        run_ffi_security_concurrency_tests
    },
    {
        "Optimization & Validation",
        "Code optimization, validation, and assembly output",
        run_ffi_optimization_tests
    }
};

static const size_t num_test_suites = sizeof(test_suites) / sizeof(test_suites[0]);

// Test runner options
typedef struct {
    bool run_all;
    bool verbose;
    bool stop_on_failure;
    size_t suite_index;
} TestOptions;

static void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("\nOptions:\n");
    printf("  -a, --all              Run all test suites (default)\n");
    printf("  -s, --suite <number>   Run specific test suite (0-%zu)\n", num_test_suites - 1);
    printf("  -v, --verbose          Enable verbose output\n");
    printf("  -f, --stop-on-failure  Stop on first test suite failure\n");
    printf("  -l, --list             List available test suites\n");
    printf("  -h, --help             Show this help message\n");
    printf("\nTest Suites:\n");
    for (size_t i = 0; i < num_test_suites; i++) {
        printf("  %zu: %s - %s\n", i, test_suites[i].name, test_suites[i].description);
    }
}

static void print_test_suites(void) {
    printf("Available Test Suites:\n");
    printf("======================\n");
    for (size_t i = 0; i < num_test_suites; i++) {
        printf("%zu. %s\n", i, test_suites[i].name);
        printf("   %s\n", test_suites[i].description);
        printf("\n");
    }
}

static TestOptions parse_arguments(int argc, char *argv[]) {
    TestOptions options = {
        .run_all = true,
        .verbose = false,
        .stop_on_failure = false,
        .suite_index = 0
    };
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            options.run_all = true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            options.verbose = true;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--stop-on-failure") == 0) {
            options.stop_on_failure = true;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            print_test_suites();
            exit(0);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--suite") == 0) {
            if (i + 1 < argc) {
                char *endptr;
                long suite_num = strtol(argv[i + 1], &endptr, 10);
                if (*endptr == '\0' && suite_num >= 0 && (size_t)suite_num < num_test_suites) {
                    options.run_all = false;
                    options.suite_index = (size_t)suite_num;
                    i++; // Skip the next argument
                } else {
                    fprintf(stderr, "Error: Invalid suite number '%s'\n", argv[i + 1]);
                    exit(1);
                }
            } else {
                fprintf(stderr, "Error: --suite requires a number\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            print_usage(argv[0]);
            exit(1);
        }
    }
    
    return options;
}

static void print_banner(void) {
    printf("═══════════════════════════════════════════════════════════════════════════════\n");
    printf("                    Asthra FFI Assembly Generator Test Suite v2.0\n");
    printf("═══════════════════════════════════════════════════════════════════════════════\n");
    printf("\n");
}

static void print_suite_header(const TestSuite *suite, size_t index) {
    printf("┌─────────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│ Test Suite %zu: %-65s │\n", index, suite->name);
    printf("│ %s%-79s │\n", suite->description, "");
    printf("└─────────────────────────────────────────────────────────────────────────────────┘\n");
    printf("\n");
}

static void print_suite_result(const TestSuite *suite, size_t index, int result, double duration) {
    const char *status = (result == 0) ? "PASSED" : "FAILED";
    const char *icon = (result == 0) ? "✅" : "❌";
    
    printf("\n");
    printf("┌─────────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│ %s Suite %zu (%s): %-53s │\n", icon, index, suite->name, status);
    printf("│ Duration: %.3f seconds%-59s │\n", duration, "");
    printf("└─────────────────────────────────────────────────────────────────────────────────┘\n");
    printf("\n");
}

static int run_single_suite(const TestSuite *suite, size_t index, const TestOptions *options) {
    if (options->verbose) {
        print_suite_header(suite, index);
    } else {
        printf("Running Test Suite %zu: %s...\n", index, suite->name);
    }
    
    clock_t start_time = clock();
    int result = suite->run_tests();
    clock_t end_time = clock();
    
    double duration = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    if (options->verbose) {
        print_suite_result(suite, index, result, duration);
    } else {
        const char *status = (result == 0) ? "PASSED" : "FAILED";
        printf("Test Suite %zu (%s): %s (%.3fs)\n", index, suite->name, status, duration);
    }
    
    return result;
}

static void print_summary(size_t suites_run, size_t suites_passed, double total_duration) {
    printf("═══════════════════════════════════════════════════════════════════════════════\n");
    printf("                              Final Test Results\n");
    printf("═══════════════════════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("Test Suites Summary:\n");
    printf("  Suites run:    %zu\n", suites_run);
    printf("  Suites passed: %zu\n", suites_passed);
    printf("  Suites failed: %zu\n", suites_run - suites_passed);
    printf("  Success rate:  %.1f%%\n", (double)suites_passed / suites_run * 100.0);
    printf("  Total time:    %.3f seconds\n", total_duration);
    printf("\n");
    
    if (suites_passed == suites_run) {
        printf("🎉 All test suites passed successfully!\n");
    } else {
        printf("❌ Some test suites failed. Please check the output above.\n");
    }
    
    printf("\n");
}

int main(int argc, char *argv[]) {
    TestOptions options = parse_arguments(argc, argv);
    
    print_banner();
    
    size_t suites_run = 0;
    size_t suites_passed = 0;
    clock_t total_start_time = clock();
    
    if (options.run_all) {
        printf("Running all %zu test suites...\n\n", num_test_suites);
        
        for (size_t i = 0; i < num_test_suites; i++) {
            int result = run_single_suite(&test_suites[i], i, &options);
            suites_run++;
            
            if (result == 0) {
                suites_passed++;
            } else if (options.stop_on_failure) {
                printf("Stopping due to test suite failure.\n");
                break;
            }
        }
    } else {
        printf("Running single test suite %zu...\n\n", options.suite_index);
        
        int result = run_single_suite(&test_suites[options.suite_index], options.suite_index, &options);
        suites_run = 1;
        if (result == 0) {
            suites_passed = 1;
        }
    }
    
    clock_t total_end_time = clock();
    double total_duration = ((double)(total_end_time - total_start_time)) / CLOCKS_PER_SEC;
    
    print_summary(suites_run, suites_passed, total_duration);
    
    return (suites_passed == suites_run) ? 0 : 1;
} 
