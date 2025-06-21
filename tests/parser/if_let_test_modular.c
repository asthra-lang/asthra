/**
 * Asthra Programming Language
 * If-Let Statement Testing - Modular Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main test runner providing 100% backward compatibility with the original
 * test_if_let_phase4_comprehensive.c while using modular architecture
 */

#include "if_let_test_modular.h"

// =============================================================================
// MAIN TEST ORCHESTRATION
// =============================================================================

int run_if_let_comprehensive_tests(void) {
    printf("=== Asthra If-Let Statement Phase 4: Comprehensive Testing ===\n");
    printf("Testing complete if-let implementation across all phases\n\n");
    
    // Initialize test framework
    init_if_let_test_framework();
    
    // Run all test phases
    run_parser_tests();
    run_semantic_tests();
    run_codegen_tests();
    run_integration_tests();
    run_performance_tests();
    
    // Print results and return exit code
    return print_if_let_test_summary();
}

int if_let_test_main(void) {
    return run_if_let_comprehensive_tests();
}

// =============================================================================
// MAIN FUNCTION (for standalone execution)
// =============================================================================

int main(void) {
    return if_let_test_main();
} 
