/**
 * Minimal FFI Assembly Generator Test - Test Runner
 * 
 * This file contains the main test runner and global test state
 * for the minimal FFI assembly generator test suite.
 */

#include "test_ffi_minimal_common.h"

// =============================================================================
// GLOBAL TEST STATE
// =============================================================================

size_t tests_run = 0;
size_t tests_passed = 0;

// =============================================================================
// TEST RUNNER
// =============================================================================

void run_test(test_function_t test_func) {
    tests_run++;
    if (test_func()) {
        tests_passed++;
    }
}

int main(void) {
    printf("Minimal FFI Assembly Generator Test Suite\n");
    printf("=========================================\n\n");
    
    test_function_t test_functions[] = {
        test_ffi_generator_creation,
        test_ffi_generator_configuration,
        test_string_operations,
        test_slice_operations,
        test_security_operations,
        test_concurrency_operations,
        test_assembly_validation,
        test_nasm_output,
        test_generation_statistics,
    };
    
    size_t num_tests = sizeof(test_functions) / sizeof(test_functions[0]);
    for (size_t i = 0; i < num_tests; i++) {
        run_test(test_functions[i]);
    }
    
    printf("\n=========================================\n");
    printf("Test Results:\n");
    printf("  Tests run: %zu\n", tests_run);
    printf("  Tests passed: %zu\n", tests_passed);
    printf("  Tests failed: %zu\n", tests_run - tests_passed);
    printf("  Success rate: %.1f%%\n", (double)tests_passed / (double)tests_run * 100.0);
    
    if (tests_passed == tests_run) {
        printf("\n🎉 All tests passed! The FFI Assembly Generator concept is working.\n");
        printf("\nThis demonstrates:\n");
        printf("  ✓ FFI generator creation and configuration\n");
        printf("  ✓ String operation code generation\n");
        printf("  ✓ Slice operation code generation with bounds checking\n");
        printf("  ✓ Security operation code generation (volatile memory, secure zero)\n");
        printf("  ✓ Concurrency operation code generation (task creation)\n");
        printf("  ✓ Assembly validation and NASM output\n");
        printf("  ✓ Statistics tracking with atomic operations\n");
        printf("\nThe full implementation in ffi_assembly_generator.c provides\n");
        printf("comprehensive support for all Asthra v1.2 language features.\n");
    }
    
    return (tests_passed == tests_run) ? 0 : 1;
} 
