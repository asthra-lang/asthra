/**
 * Tests for PIE (Position Independent Executable) flags
 * Tests the --pie and --no-pie command-line options
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/cli.h"
#include "../../src/compiler.h"

// Test that pie_mode defaults to ASTHRA_PIE_DEFAULT
static bool test_pie_default(void) {
    printf("Testing PIE default mode...\n");

    AsthraCompilerOptions options = asthra_compiler_default_options();

    if (options.pie_mode != ASTHRA_PIE_DEFAULT) {
        printf("FAIL: PIE mode should default to ASTHRA_PIE_DEFAULT\n");
        return false;
    }

    printf("PASS: PIE defaults to platform-specific behavior\n");
    return true;
}

// Test parsing --pie flag
static bool test_pie_enabled_flag(void) {
    printf("Testing --pie flag parsing...\n");

    // Reset getopt state
    extern int optind;
    optind = 1;

    // Simulate command line: asthra --pie test.asthra
    char *argv[] = {"asthra", "--pie", "test.asthra"};
    int argc = 3;

    CliOptions cli_options = cli_options_init();
    int result = cli_parse_arguments(argc, argv, &cli_options);

    if (result != 0) {
        printf("FAIL: Failed to parse --pie flag\n");
        return false;
    }

    if (cli_options.compiler_options.pie_mode != ASTHRA_PIE_FORCE_ENABLED) {
        printf("FAIL: --pie flag should set PIE mode to ASTHRA_PIE_FORCE_ENABLED\n");
        cli_options_cleanup(&cli_options);
        return false;
    }

    cli_options_cleanup(&cli_options);
    printf("PASS: --pie flag correctly enables PIE\n");
    return true;
}

// Test parsing --no-pie flag
static bool test_pie_disabled_flag(void) {
    printf("Testing --no-pie flag parsing...\n");

    // Reset getopt state
    extern int optind;
    optind = 1;

    // Simulate command line: asthra --no-pie test.asthra
    char *argv[] = {"asthra", "--no-pie", "test.asthra"};
    int argc = 3;

    CliOptions cli_options = cli_options_init();
    int result = cli_parse_arguments(argc, argv, &cli_options);

    if (result != 0) {
        printf("FAIL: Failed to parse --no-pie flag (result=%d)\n", result);
        return false;
    }

    printf("DEBUG: PIE mode after parsing: %d (expected %d)\n",
           cli_options.compiler_options.pie_mode, ASTHRA_PIE_FORCE_DISABLED);

    if (cli_options.compiler_options.pie_mode != ASTHRA_PIE_FORCE_DISABLED) {
        printf("FAIL: --no-pie flag should set PIE mode to ASTHRA_PIE_FORCE_DISABLED\n");
        cli_options_cleanup(&cli_options);
        return false;
    }

    cli_options_cleanup(&cli_options);
    printf("PASS: --no-pie flag correctly disables PIE\n");
    return true;
}

// Test that --pie and --no-pie are mutually exclusive
static bool test_pie_mutual_exclusion(void) {
    printf("Testing PIE flag mutual exclusion...\n");

    // Reset getopt state
    extern int optind;
    optind = 1;

    // First set --pie, then --no-pie should override
    char *argv[] = {"asthra", "--pie", "--no-pie", "test.asthra"};
    int argc = 4;

    CliOptions cli_options = cli_options_init();
    int result = cli_parse_arguments(argc, argv, &cli_options);

    if (result != 0) {
        printf("FAIL: Failed to parse PIE flags\n");
        return false;
    }

    // The last flag should win
    if (cli_options.compiler_options.pie_mode != ASTHRA_PIE_FORCE_DISABLED) {
        printf("FAIL: Last PIE flag should take precedence\n");
        cli_options_cleanup(&cli_options);
        return false;
    }

    cli_options_cleanup(&cli_options);
    printf("PASS: PIE flags are mutually exclusive (last wins)\n");
    return true;
}

// Test PIE mode with other flags
static bool test_pie_with_other_flags(void) {
    printf("Testing PIE with other compiler flags...\n");

    // Reset getopt state
    extern int optind;
    optind = 1;

    // Combine PIE with optimization and debug flags
    char *argv[] = {"asthra", "-O3", "--pie", "-g", "test.asthra"};
    int argc = 5;

    CliOptions cli_options = cli_options_init();
    int result = cli_parse_arguments(argc, argv, &cli_options);

    if (result != 0) {
        printf("FAIL: Failed to parse combined flags\n");
        return false;
    }

    if (cli_options.compiler_options.pie_mode != ASTHRA_PIE_FORCE_ENABLED) {
        printf("FAIL: PIE mode not set correctly with other flags\n");
        cli_options_cleanup(&cli_options);
        return false;
    }

    if (cli_options.compiler_options.opt_level != ASTHRA_OPT_AGGRESSIVE) {
        printf("FAIL: Optimization level not preserved\n");
        cli_options_cleanup(&cli_options);
        return false;
    }

    if (!cli_options.compiler_options.debug_info) {
        printf("FAIL: Debug info flag not preserved\n");
        cli_options_cleanup(&cli_options);
        return false;
    }

    cli_options_cleanup(&cli_options);
    printf("PASS: PIE works correctly with other flags\n");
    return true;
}

int main(void) {
    printf("=== PIE Flag Tests ===\n\n");

    int tests_passed = 0;
    int tests_failed = 0;

    // Run tests
    if (test_pie_default())
        tests_passed++;
    else
        tests_failed++;
    if (test_pie_enabled_flag())
        tests_passed++;
    else
        tests_failed++;
    if (test_pie_disabled_flag())
        tests_passed++;
    else
        tests_failed++;
    if (test_pie_mutual_exclusion())
        tests_passed++;
    else
        tests_failed++;
    if (test_pie_with_other_flags())
        tests_passed++;
    else
        tests_failed++;

    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Total tests: %d\n", tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}