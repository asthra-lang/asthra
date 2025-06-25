/**
 * Asthra Programming Language Compiler
 * Simple Pipeline Orchestrator Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Basic validation of pipeline orchestrator functionality
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "pipeline_orchestrator.h"

// Simple test macros
#define TEST_SECTION(name) printf("\n--- %s ---\n", name)
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("❌ FAIL: %s\n", message);                                                      \
            return false;                                                                          \
        } else {                                                                                   \
            printf("✅ PASS: %s\n", message);                                                      \
        }                                                                                          \
    } while (0)

// =============================================================================
// TEST FIXTURES
// =============================================================================

static AsthraCompilerContext *test_context = NULL;

static bool setup_pipeline_tests(void) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.debug_info = true;
    options.verbose = false;
    options.opt_level = ASTHRA_OPT_BASIC;

    test_context = asthra_compiler_create(&options);
    return test_context != NULL;
}

static void teardown_pipeline_tests(void) {
    if (test_context) {
        asthra_compiler_destroy(test_context);
        test_context = NULL;
    }
}

// =============================================================================
// BASIC FUNCTIONALITY TESTS
// =============================================================================

static bool test_orchestrator_creation_and_destruction(void) {
    TEST_SECTION("Orchestrator Creation and Destruction");

    // Test successful creation
    PipelineOrchestrator *orchestrator = pipeline_orchestrator_create(test_context);
    TEST_ASSERT(orchestrator != NULL, "Pipeline orchestrator created successfully");

    if (orchestrator) {
        // Verify initial state
        TEST_ASSERT(orchestrator->compiler_context == test_context,
                    "Compiler context properly assigned");
        TEST_ASSERT(orchestrator->current_phase == ASTHRA_PHASE_LEXING, "Initial phase is lexing");
        TEST_ASSERT(!orchestrator->pipeline_active, "Pipeline starts in inactive state");
        TEST_ASSERT(!orchestrator->error_occurred, "No errors initially");
        TEST_ASSERT(orchestrator->source_file_count == 0, "No source files initially");

        // Test destruction
        pipeline_orchestrator_destroy(orchestrator);
        TEST_ASSERT(true, "Pipeline orchestrator destroyed without crashes");
    }

    // Test null parameter handling
    PipelineOrchestrator *null_orchestrator = pipeline_orchestrator_create(NULL);
    TEST_ASSERT(null_orchestrator == NULL, "Creation with NULL context returns NULL");

    // Test null destruction
    pipeline_orchestrator_destroy(NULL);
    TEST_ASSERT(true, "Destroying NULL orchestrator doesn't crash");

    return true;
}

static bool test_orchestrator_configuration(void) {
    TEST_SECTION("Orchestrator Configuration");

    PipelineOrchestrator *orchestrator = pipeline_orchestrator_create(test_context);
    TEST_ASSERT(orchestrator != NULL, "Orchestrator created for configuration test");

    if (orchestrator) {
        // Test default configuration
        TEST_ASSERT(orchestrator->config.generate_debug_info,
                    "Debug info generation enabled by default");
        TEST_ASSERT(orchestrator->config.optimize_pipeline,
                    "Pipeline optimization enabled by default");
        TEST_ASSERT(!orchestrator->config.parallel_compilation,
                    "Parallel compilation disabled by default");
        TEST_ASSERT(orchestrator->config.max_errors == 100, "Default max errors is 100");
        TEST_ASSERT(orchestrator->config.timeout_seconds == 300.0, "Default timeout is 5 minutes");

        // Test configuration modification
        orchestrator->config.verbose_output = true;
        orchestrator->config.parallel_compilation = true;
        orchestrator->config.max_errors = 50;

        TEST_ASSERT(orchestrator->config.verbose_output, "Configuration modification works");
        TEST_ASSERT(orchestrator->config.parallel_compilation,
                    "Parallel compilation can be enabled");
        TEST_ASSERT(orchestrator->config.max_errors == 50, "Max errors can be modified");

        pipeline_orchestrator_destroy(orchestrator);
    }

    return true;
}

static bool test_state_management(void) {
    TEST_SECTION("State Management");

    PipelineOrchestrator *orchestrator = pipeline_orchestrator_create(test_context);
    TEST_ASSERT(orchestrator != NULL, "Orchestrator created for state test");

    if (orchestrator) {
        // Test initial state
        TEST_ASSERT(pipeline_is_ready_for_execution(orchestrator),
                    "Orchestrator ready for execution initially");

        // Test state clearing
        orchestrator->error_occurred = true;
        orchestrator->pipeline_active = true;
        orchestrator->source_file_count = 5;

        pipeline_clear_state(orchestrator);

        TEST_ASSERT(!orchestrator->error_occurred, "Error state cleared");
        TEST_ASSERT(!orchestrator->pipeline_active, "Pipeline active state cleared");
        TEST_ASSERT(orchestrator->source_file_count == 0, "Source file count reset");
        TEST_ASSERT(orchestrator->current_phase == ASTHRA_PHASE_LEXING, "Phase reset to lexing");

        pipeline_orchestrator_destroy(orchestrator);
    }

    return true;
}

static bool test_utility_functions(void) {
    TEST_SECTION("Utility Functions");

    // Test phase name function
    TEST_ASSERT(strcmp(pipeline_get_phase_name(ASTHRA_PHASE_LEXING), "Lexing") == 0,
                "Lexing phase name correct");
    TEST_ASSERT(strcmp(pipeline_get_phase_name(ASTHRA_PHASE_PARSING), "Parsing") == 0,
                "Parsing phase name correct");
    TEST_ASSERT(
        strcmp(pipeline_get_phase_name(ASTHRA_PHASE_SEMANTIC_ANALYSIS), "Semantic Analysis") == 0,
        "Semantic analysis phase name correct");
    TEST_ASSERT(strcmp(pipeline_get_phase_name(ASTHRA_PHASE_CODE_GENERATION), "Code Generation") ==
                    0,
                "Code generation phase name correct");
    TEST_ASSERT(strcmp(pipeline_get_phase_name(ASTHRA_PHASE_LINKING), "Linking") == 0,
                "Linking phase name correct");

    // Test time function
    double time1 = pipeline_get_current_time_ms();
    TEST_ASSERT(time1 > 0.0, "Time function returns positive value");

    // Small delay to test time difference
    for (volatile int i = 0; i < 1000000; i++)
        ;

    double time2 = pipeline_get_current_time_ms();
    TEST_ASSERT(time2 > time1, "Time function shows progression");

    return true;
}

static bool test_error_handling(void) {
    TEST_SECTION("Error Handling");

    PipelineOrchestrator *orchestrator = pipeline_orchestrator_create(test_context);
    TEST_ASSERT(orchestrator != NULL, "Orchestrator created for error test");

    if (orchestrator) {
        // Test invalid file compilation
        bool result =
            pipeline_orchestrator_compile_file(orchestrator, "/nonexistent/file.asthra", "output");
        TEST_ASSERT(!result, "Compilation of nonexistent file fails");

        // Test null parameter handling
        result = pipeline_orchestrator_compile_file(NULL, "input", "output");
        TEST_ASSERT(!result, "Compilation with NULL orchestrator fails");

        result = pipeline_orchestrator_compile_file(orchestrator, NULL, "output");
        TEST_ASSERT(!result, "Compilation with NULL input fails");

        result = pipeline_orchestrator_compile_file(orchestrator, "input", NULL);
        TEST_ASSERT(!result, "Compilation with NULL output fails");

        pipeline_orchestrator_destroy(orchestrator);
    }

    return true;
}

static bool test_statistics_api(void) {
    TEST_SECTION("Statistics API");

    PipelineOrchestrator *orchestrator = pipeline_orchestrator_create(test_context);
    TEST_ASSERT(orchestrator != NULL, "Orchestrator created for statistics test");

    if (orchestrator) {
        // Test initial statistics
        size_t total_files, total_lines, total_errors;
        double total_time;

        pipeline_orchestrator_get_statistics(orchestrator, &total_files, &total_lines,
                                             &total_errors, &total_time);

        TEST_ASSERT(total_files == 0, "Initial file count is zero");
        TEST_ASSERT(total_lines == 0, "Initial line count is zero");
        TEST_ASSERT(total_errors == 0, "Initial error count is zero");
        TEST_ASSERT(total_time == 0.0, "Initial time is zero");

        // Test statistics with null parameters
        pipeline_orchestrator_get_statistics(orchestrator, NULL, NULL, NULL, NULL);
        TEST_ASSERT(true, "Statistics call with NULL parameters doesn't crash");

        pipeline_orchestrator_destroy(orchestrator);
    }

    return true;
}

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    printf("=== Simple Pipeline Orchestrator Tests ===\n");

    if (!setup_pipeline_tests()) {
        printf("❌ Test setup failed\n");
        return 1;
    }

    int passed = 0;
    int total = 0;

    // Run tests
    if (test_orchestrator_creation_and_destruction())
        passed++;
    total++;
    if (test_orchestrator_configuration())
        passed++;
    total++;
    if (test_state_management())
        passed++;
    total++;
    if (test_utility_functions())
        passed++;
    total++;
    if (test_error_handling())
        passed++;
    total++;
    if (test_statistics_api())
        passed++;
    total++;

    // Test summary
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    printf("Success rate: %.1f%%\n", (float)passed / (float)total * 100.0f);

    if (passed == total) {
        printf("\n✅ All pipeline orchestrator tests passed!\n");
        printf("✅ Pipeline orchestrator implementation is working correctly.\n");
        printf("✅ Ready for end-to-end compilation workflow.\n");
    } else {
        printf("\n❌ Some tests failed. Review the output above.\n");
    }

    teardown_pipeline_tests();

    return (passed == total) ? 0 : 1;
}
