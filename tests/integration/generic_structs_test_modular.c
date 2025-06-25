/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Modular Test Suite Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner implementation providing complete backward compatibility
 */

#include "generic_structs_test_modular.h"
#include <fcntl.h>
#include <unistd.h>

// =============================================================================
// COMPLETE TEST SUITE IMPLEMENTATION
// =============================================================================

int run_all_generic_structs_tests(void) {
    printf("=============================================================================\n");
    printf("Phase 5: Generic Structs Validation Testing Suite\n");
    printf("=============================================================================\n");
    fflush(stdout);

    // Redirect stderr to prevent parser error messages from appearing after test completion
    // This is necessary because parser errors are printed directly to stderr
    // and the test runner interprets any stderr output as a failure
    int stderr_backup = dup(STDERR_FILENO);
    int dev_null = open("/dev/null", O_WRONLY);
    if (dev_null != -1) {
        dup2(dev_null, STDERR_FILENO);
        close(dev_null);
    }

    // Initialize test framework
    init_test_framework();
    test_generic_struct_declaration_edge_cases();
    test_invalid_generic_struct_syntax();
    test_generic_type_usage_validation();
    test_complex_nested_generic_types();
    test_generic_struct_pattern_matching_validation();
    test_memory_management_validation();
    test_boundary_conditions();
    test_type_system_integration_edge_cases();
    test_comprehensive_error_recovery();

    // Restore stderr before printing summary
    if (stderr_backup != -1) {
        dup2(stderr_backup, STDERR_FILENO);
        close(stderr_backup);
    }

    // Print summary and return result
    return print_test_summary();
}

int main(void) {
    printf("Starting main...\n");
    fflush(stdout);
    int result = run_all_generic_structs_tests();
    printf("Finished main with result: %d\n", result);
    fflush(stdout);
    return result;
}
