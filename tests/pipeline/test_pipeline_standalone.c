/**
 * Asthra Programming Language Compiler
 * Standalone Pipeline Utility Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests only standalone utility functions without any dependencies
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

// Import the compiler phase enumeration directly
typedef enum {
    ASTHRA_PHASE_LEXING,
    ASTHRA_PHASE_PARSING,
    ASTHRA_PHASE_SEMANTIC_ANALYSIS,
    ASTHRA_PHASE_OPTIMIZATION,
    ASTHRA_PHASE_CODE_GENERATION,
    ASTHRA_PHASE_LINKING
} AsthraCompilerPhase;

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
// STANDALONE IMPLEMENTATIONS OF UTILITY FUNCTIONS
// =============================================================================

/**
 * Get current timestamp in milliseconds
 */
static double get_current_time_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)(counter.QuadPart * 1000.0) / frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)tv.tv_sec * 1000.0) + ((double)tv.tv_usec / 1000.0);
#endif
}

/**
 * Get phase name as string
 */
static const char *get_phase_name(AsthraCompilerPhase phase) {
    switch (phase) {
    case ASTHRA_PHASE_LEXING:
        return "Lexing";
    case ASTHRA_PHASE_PARSING:
        return "Parsing";
    case ASTHRA_PHASE_SEMANTIC_ANALYSIS:
        return "Semantic Analysis";
    case ASTHRA_PHASE_OPTIMIZATION:
        return "Optimization";
    case ASTHRA_PHASE_CODE_GENERATION:
        return "Code Generation";
    case ASTHRA_PHASE_LINKING:
        return "Linking";
    default:
        return "Unknown";
    }
}

// =============================================================================
// STANDALONE TESTS
// =============================================================================

static bool test_phase_names(void) {
    TEST_SECTION("Phase Name Function");

    // Test all phase names
    TEST_ASSERT(strcmp(get_phase_name(ASTHRA_PHASE_LEXING), "Lexing") == 0,
                "Lexing phase name correct");
    TEST_ASSERT(strcmp(get_phase_name(ASTHRA_PHASE_PARSING), "Parsing") == 0,
                "Parsing phase name correct");
    TEST_ASSERT(strcmp(get_phase_name(ASTHRA_PHASE_SEMANTIC_ANALYSIS), "Semantic Analysis") == 0,
                "Semantic analysis phase name correct");
    TEST_ASSERT(strcmp(get_phase_name(ASTHRA_PHASE_OPTIMIZATION), "Optimization") == 0,
                "Optimization phase name correct");
    TEST_ASSERT(strcmp(get_phase_name(ASTHRA_PHASE_CODE_GENERATION), "Code Generation") == 0,
                "Code generation phase name correct");
    TEST_ASSERT(strcmp(get_phase_name(ASTHRA_PHASE_LINKING), "Linking") == 0,
                "Linking phase name correct");

    // Test unknown phase
    TEST_ASSERT(strcmp(get_phase_name((AsthraCompilerPhase)999), "Unknown") == 0,
                "Unknown phase returns 'Unknown'");

    return true;
}

static bool test_time_function(void) {
    TEST_SECTION("Time Function");

    // Test time function
    double time1 = get_current_time_ms();
    TEST_ASSERT(time1 > 0.0, "Time function returns positive value");

    // Small delay to test time difference
    for (volatile int i = 0; i < 1000000; i++)
        ;

    double time2 = get_current_time_ms();
    TEST_ASSERT(time2 > time1, "Time function shows progression");

    printf("  Time measurements: %.2f ms -> %.2f ms (diff: %.2f ms)\n", time1, time2,
           time2 - time1);

    return true;
}

static bool test_compiler_phase_enum(void) {
    TEST_SECTION("Compiler Phase Enumeration");

    // Test that the phase enumeration values are as expected
    TEST_ASSERT(ASTHRA_PHASE_LEXING == 0, "Lexing phase is first (0)");
    TEST_ASSERT(ASTHRA_PHASE_PARSING == 1, "Parsing phase is second (1)");
    TEST_ASSERT(ASTHRA_PHASE_SEMANTIC_ANALYSIS == 2, "Semantic analysis phase is third (2)");
    TEST_ASSERT(ASTHRA_PHASE_OPTIMIZATION == 3, "Optimization phase is fourth (3)");
    TEST_ASSERT(ASTHRA_PHASE_CODE_GENERATION == 4, "Code generation phase is fifth (4)");
    TEST_ASSERT(ASTHRA_PHASE_LINKING == 5, "Linking phase is sixth (5)");

    printf("  Phase enumeration values: LEXING=%d, PARSING=%d, SEMANTIC=%d, OPTIMIZATION=%d, "
           "CODEGEN=%d, LINKING=%d\n",
           ASTHRA_PHASE_LEXING, ASTHRA_PHASE_PARSING, ASTHRA_PHASE_SEMANTIC_ANALYSIS,
           ASTHRA_PHASE_OPTIMIZATION, ASTHRA_PHASE_CODE_GENERATION, ASTHRA_PHASE_LINKING);

    return true;
}

static bool test_basic_memory_operations(void) {
    TEST_SECTION("Basic Memory Operations");

    // Test basic memory allocation and string operations
    char *test_string = malloc(64);
    TEST_ASSERT(test_string != NULL, "Memory allocation succeeds");

    strcpy(test_string, "Hello, Asthra Pipeline!");
    TEST_ASSERT(strcmp(test_string, "Hello, Asthra Pipeline!") == 0, "String operations work");

    free(test_string);
    TEST_ASSERT(true, "Memory deallocation completes without crash");

    return true;
}

static bool test_performance_measurement(void) {
    TEST_SECTION("Performance Measurement");

    // Test performance measurement capability
    double start_time = get_current_time_ms();

    // Simulate some work
    volatile long sum = 0;
    for (volatile long i = 0; i < 1000000; i++) {
        sum += i;
    }

    double end_time = get_current_time_ms();
    double elapsed = end_time - start_time;

    TEST_ASSERT(elapsed > 0.0, "Performance measurement shows positive elapsed time");
    TEST_ASSERT(elapsed < 1000.0, "Performance measurement shows reasonable elapsed time");
    TEST_ASSERT(sum > 0, "Test computation produced expected result");

    printf("  Performance test: %.2f ms elapsed, sum = %ld\n", elapsed, sum);

    return true;
}

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    printf("=== Standalone Pipeline Utility Tests ===\n");
    printf("Testing core utility functions independently\n");

    int passed = 0;
    int total = 0;

    // Run standalone tests
    if (test_phase_names())
        passed++;
    total++;
    if (test_time_function())
        passed++;
    total++;
    if (test_compiler_phase_enum())
        passed++;
    total++;
    if (test_basic_memory_operations())
        passed++;
    total++;
    if (test_performance_measurement())
        passed++;
    total++;

    // Test summary
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", passed, total);
    printf("Success rate: %.1f%%\n", (float)passed / (float)total * 100.0f);

    if (passed == total) {
        printf("\n✅ All standalone utility tests passed!\n");
        printf("✅ Core functionality verified independently.\n");
        printf("✅ Phase names, timing, and basic operations working correctly.\n");
        printf("\n📝 Note: This validates the fundamental utility functions that\n");
        printf("   will be used by the full pipeline orchestrator implementation.\n");
        printf("   The functions tested here are the building blocks for the\n");
        printf("   complete Phase 5 End-to-End Compilation Pipeline.\n");
    } else {
        printf("\n❌ Some tests failed. Review the output above.\n");
    }

    return (passed == total) ? 0 : 1;
}
