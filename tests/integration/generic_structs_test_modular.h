/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Modular Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Umbrella header for modular generic structs validation testing
 * Provides complete backward compatibility with original test_generic_structs_validation_phase5.c
 */

#ifndef GENERIC_STRUCTS_TEST_MODULAR_H
#define GENERIC_STRUCTS_TEST_MODULAR_H

// Include all modular test components
#include "generic_structs_test_common.h"
#include "generic_structs_test_declarations.h"
#include "generic_structs_test_system.h"
#include "generic_structs_test_usage.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// COMPLETE TEST SUITE INTERFACE
// =============================================================================

/**
 * Run all generic structs validation tests
 * Equivalent to the original main() function behavior
 * @return 0 if all tests pass, 1 otherwise
 */
int run_all_generic_structs_tests(void);

/**
 * Main test runner function
 * Provides exact compatibility with original main() function
 * @return 0 if all tests pass, 1 otherwise
 */
int main(void);

// =============================================================================
// BACKWARD COMPATIBILITY ALIASES
// =============================================================================

// All original test functions are available through their respective modules:
// - test_generic_struct_declaration_edge_cases() from generic_structs_test_declarations.h
// - test_invalid_generic_struct_syntax() from generic_structs_test_declarations.h
// - test_generic_type_usage_validation() from generic_structs_test_usage.h
// - test_complex_nested_generic_types() from generic_structs_test_usage.h
// - test_generic_struct_pattern_matching_validation() from generic_structs_test_usage.h
// - test_memory_management_validation() from generic_structs_test_system.h
// - test_boundary_conditions() from generic_structs_test_system.h
// - test_type_system_integration_edge_cases() from generic_structs_test_system.h
// - test_comprehensive_error_recovery() from generic_structs_test_system.h

// All original helper functions are available through generic_structs_test_common.h:
// - test_parse_success()
// - test_parse_failure()
// - test_semantic_success()
// - TEST_ASSERT() macro
// - TEST_SUCCESS() macro

#ifdef __cplusplus
}
#endif

#endif // GENERIC_STRUCTS_TEST_MODULAR_H
