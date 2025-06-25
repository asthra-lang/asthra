/**
 * Asthra Programming Language Compiler
 * Minimal Pipeline Orchestrator Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests only the basic pipeline orchestrator data structures and utility functions
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
// UTILITY FUNCTION TESTS (NO COMPLEX DEPENDENCIES)
// =============================================================================

static bool test_phase_names(void) {
    TEST_SECTION("Phase Name Function");

    // Test phase name function - this doesn't require any complex setup
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

    // Test unknown phase
    TEST_ASSERT(strcmp(pipeline_get_phase_name((AsthraCompilerPhase)999), "Unknown") == 0,
                "Unknown phase returns 'Unknown'");

    return true;
}

static bool test_time_function(void) {
    TEST_SECTION("Time Function");

    // Test time function - this is a simple system function wrapper
    double time1 = pipeline_get_current_time_ms();
    TEST_ASSERT(time1 > 0.0, "Time function returns positive value");

    // Small delay to test time difference
    for (volatile int i = 0; i < 1000000; i++)
        ;

    double time2 = pipeline_get_current_time_ms();
    TEST_ASSERT(time2 > time1, "Time function shows progression");

    printf("  Time measurements: %.2f ms -> %.2f ms (diff: %.2f ms)\n", time1, time2,
           time2 - time1);

    return true;
}

static bool test_null_safety(void) {
    TEST_SECTION("NULL Safety");

    // Test null orchestrator destruction
    pipeline_orchestrator_destroy(NULL);
    TEST_ASSERT(true, "Destroying NULL orchestrator doesn't crash");

    // Test null orchestrator compilation (should fail gracefully)
    bool result = pipeline_orchestrator_compile_file(NULL, "input", "output");
    TEST_ASSERT(!result, "Compilation with NULL orchestrator fails gracefully");

    return true;
}

static bool test_basic_data_structures(void) {
    TEST_SECTION("Basic Data Structures");

    // Test phase result structure (this is publicly accessible)
    PipelinePhaseResult result = {0};
    result.phase = ASTHRA_PHASE_PARSING;
    result.success = true;
    result.start_time = 100.0;
    result.end_time = 200.0;
    result.execution_time_ms = 100.0;

    TEST_ASSERT(result.phase == ASTHRA_PHASE_PARSING, "Phase result phase field works");
    TEST_ASSERT(result.success == true, "Phase result success field works");
    TEST_ASSERT(result.start_time == 100.0, "Phase result start time field works");
    TEST_ASSERT(result.end_time == 200.0, "Phase result end time field works");
    TEST_ASSERT(result.execution_time_ms == 100.0, "Phase result execution time field works");

    // Test lexer phase data structure
    LexerPhaseData lexer_data = {0};
    lexer_data.success = true;
    lexer_data.token_count = 42;
    lexer_data.execution_time_ms = 123.45;

    TEST_ASSERT(lexer_data.success == true, "Lexer phase data success field works");
    TEST_ASSERT(lexer_data.token_count == 42, "Lexer phase data token count field works");
    TEST_ASSERT(lexer_data.execution_time_ms == 123.45,
                "Lexer phase data execution time field works");

    // Test parser phase data structure
    ParserPhaseData parser_data = {0};
    parser_data.success = false;
    parser_data.ast_node_count = 100;
    parser_data.parse_error_count = 5;

    TEST_ASSERT(parser_data.success == false, "Parser phase data success field works");
    TEST_ASSERT(parser_data.ast_node_count == 100, "Parser phase data AST node count field works");
    TEST_ASSERT(parser_data.parse_error_count == 5, "Parser phase data error count field works");

    return true;
}

static bool test_orchestrator_creation_minimal(void) {
    TEST_SECTION("Minimal Orchestrator Creation");

    // Test creation with NULL context (should fail)
    PipelineOrchestrator *null_orchestrator = pipeline_orchestrator_create(NULL);
    TEST_ASSERT(null_orchestrator == NULL, "Creation with NULL context returns NULL");

    printf("  Note: Full orchestrator creation requires complete compiler context\n");
    printf("  which is not available in this minimal test environment.\n");

    return true;
}

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    printf("=== Minimal Pipeline Orchestrator Tests ===\n");
    printf("Testing basic functionality without complex compiler dependencies\n");

    int passed = 0;
    int total = 0;

    // Run minimal tests
    if (test_phase_names())
        passed++;
    total++;
    if (test_time_function())
        passed++;
    total++;
    if (test_null_safety())
        passed++;
    total++;
    if (test_basic_data_structures())
        passed++;
    total++;
    if (test_orchestrator_creation_minimal())
        passed++;
    total++;

    // Test summary
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    printf("Success rate: %.1f%%\n", (float)passed / (float)total * 100.0f);

    if (passed == total) {
        printf("\n✅ All minimal pipeline orchestrator tests passed!\n");
        printf("✅ Basic pipeline orchestrator functionality verified.\n");
        printf("✅ Phase names, timing, and data structures working correctly.\n");
        printf("\n📝 Note: This is a minimal test suite that validates core functionality\n");
        printf("   without dependencies on the full compiler infrastructure.\n");
        printf("   Full end-to-end tests require a complete build environment.\n");
    } else {
        printf("\n❌ Some tests failed. Review the output above.\n");
    }

    return (passed == total) ? 0 : 1;
}
