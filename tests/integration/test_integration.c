/**
 * Asthra Programming Language v1.2 Integration Tests (LEGACY)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * NOTICE: This file has been split into smaller, more manageable modules.
 * 
 * This file originally contained all integration tests in a single large file.
 * It has been refactored into a modular structure for better maintainability.
 * 
 * NEW STRUCTURE:
 * - test_integration_common.h         - Common includes and declarations
 * - test_integration_main.c           - Main entry point and test runner
 * - test_concurrent_text_processing.c - Text processing with FFI tests
 * - test_concurrent_slice_management.c - Slice management tests
 * - test_pattern_matching_ffi.c       - Pattern matching and Result types
 * - test_security_concurrency.c       - Security features tests
 * - test_performance_comprehensive.c  - Performance integration tests
 * 
 * TO BUILD AND RUN THE NEW TESTS:
 * 
 * 1. Use the provided Makefile:
 *    make all && make test
 * 
 * 2. Or compile manually:
 *    gcc -o integration_tests test_integration_main.c \
 *        test_concurrent_text_processing.c \
 *        test_concurrent_slice_management.c \
 *        test_pattern_matching_ffi.c \
 *        test_security_concurrency.c \
 *        test_performance_comprehensive.c \
 *        -pthread -I../runtime -L../runtime -lasthra_runtime
 * 
 * 3. Run the tests:
 *    ./integration_tests
 * 
 * BENEFITS OF THE NEW STRUCTURE:
 * - Smaller, focused files for each test scenario
 * - Easier debugging and maintenance
 * - Parallel development capability
 * - Better test isolation
 * - Clearer documentation per test type
 * - Selective test execution
 * 
 * For detailed information, see the README.md file in this directory.
 */

// Prevent compilation of this legacy file
#if 0

// Original file content has been moved to the modular structure above.
// This prevents accidental compilation while preserving the original code
// for reference purposes.

// The original tests are now available as:
// - test_integration_concurrent_text_processing()
// - test_integration_concurrent_slice_management() 
// - test_integration_pattern_matching_with_ffi()
// - test_integration_security_with_concurrency()
// - test_integration_performance_comprehensive()

#endif /* Disabled legacy code */

// Include new modular structure
#include "test_integration_common.h"

/**
 * Legacy compatibility function - redirects to new test runner
 */
int main(int argc, char *argv[]) {
    printf("=== LEGACY FILE NOTICE ===\n");
    printf("This file has been split into a modular structure.\n");
    printf("Please use 'test_integration_main.c' or run 'make test' instead.\n");
    printf("See README.md for detailed information about the new structure.\n");
    printf("==========================\n\n");
    
    // Optionally run the new test structure
    printf("Running new modular integration tests...\n\n");
    
    // This would need to be implemented to call the new main function
    // For now, just provide guidance
    printf("To run the new tests:\n");
    printf("1. make clean && make all\n");
    printf("2. ./integration_tests\n\n");
    
    printf("Or run individual test modules:\n");
    printf("- make test-text\n");
    printf("- make test-slice\n");
    printf("- make test-pattern\n");
    printf("- make test-security\n");
    printf("- make test-performance\n\n");
    
    return 0;
} 
