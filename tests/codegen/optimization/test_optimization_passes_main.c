/**
 * Asthra Programming Language Compiler
 * Optimization Passes Testing - Main Test Orchestrator
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_optimization_passes_common.h"

// Include individual test implementations
extern bool test_constant_folding(void);
extern bool test_dead_code_elimination(void);
extern bool test_common_subexpression_elimination(void);
extern bool test_strength_reduction(void);
extern bool test_peephole_optimization(void);

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Optimization Passes Minimal Test Suite\n");
    printf("=======================================\n");
    
    bool all_passed = true;
    
    // Run test suite and capture result
    printf("Running OptimizationPasses test suite...\n");
    typedef bool (*test_func_t)(void);
    test_func_t tests[] = {
        test_constant_folding,
        test_dead_code_elimination,
        test_common_subexpression_elimination,
        test_strength_reduction,
        test_peephole_optimization
    };
    int total = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    for (int i = 0; i < total; i++) {
        if (tests[i]()) {
            passed++;
            printf("  ✓ Test %d passed\n", i + 1);
        } else {
            printf("  ✗ Test %d failed\n", i + 1);
            all_passed = false;
        }
    }
    printf("OptimizationPasses: %d/%d tests passed\n", passed, total);
    
    printf("\nTest suite completed!\n");
    return all_passed ? 0 : 1;
}
