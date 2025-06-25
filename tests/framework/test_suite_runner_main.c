/**
 * Test Suite Runner - Main Entry Point and Legacy Compatibility
 *
 * Main integration file that includes all modular components
 * and provides the legacy compatibility interface.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_suite_runner_common.h"

// Include all modular implementations
#include "test_suite_runner_config.c"
#include "test_suite_runner_execution.c"
#include "test_suite_runner_filtering.c"
#include "test_suite_runner_interface.c"
#include "test_suite_runner_signals.c"

// =============================================================================
// LEGACY COMPATIBILITY AND MAIN ENTRY POINT
// =============================================================================

int test_suite_runner_main(int argc, char **argv) {
    AsthraTestRunner *runner = asthra_test_runner_create();
    if (!runner) {
        printf("Failed to create test runner\n");
        return 1;
    }

    int parse_result = parse_command_line(argc, argv, &runner->config);
    if (parse_result != 0) {
        asthra_test_runner_destroy(runner);
        return parse_result > 0 ? 0 : 1;
    }

    // For now, return success since we don't have registered test suites
    // This would typically be populated by test discovery or registration
    if (runner->suite_count == 0) {
        if (!runner->config.quiet) {
            printf("No test suites registered. Use asthra_test_runner_add_suite() to add tests.\n");
        }
        asthra_test_runner_destroy(runner);
        return 0;
    }

    int result = asthra_test_runner_execute(runner);
    asthra_test_runner_destroy(runner);

    return result;
}