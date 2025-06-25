/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - System Integration Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for memory management, boundary conditions, and type system integration
 */

#ifndef GENERIC_STRUCTS_TEST_SYSTEM_H
#define GENERIC_STRUCTS_TEST_SYSTEM_H

#include "generic_structs_test_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SYSTEM INTEGRATION TESTS
// =============================================================================

/**
 * Test 6: Memory Management and Resource Cleanup
 * Tests proper memory management with complex AST structures
 * @return true if all tests pass, false otherwise
 */
bool test_memory_management_validation(void);

/**
 * Test 7: Boundary Conditions and Limits
 * Tests various boundary conditions and parser limits
 * @return true if all tests pass, false otherwise
 */
bool test_boundary_conditions(void);

/**
 * Test 8: Type System Integration Edge Cases
 * Tests integration with various type system features
 * @return true if all tests pass, false otherwise
 */
bool test_type_system_integration_edge_cases(void);

/**
 * Test 9: Comprehensive Error Recovery
 * Tests parser error recovery with various invalid syntax cases
 * @return true if all tests pass, false otherwise
 */
bool test_comprehensive_error_recovery(void);

#ifdef __cplusplus
}
#endif

#endif // GENERIC_STRUCTS_TEST_SYSTEM_H
