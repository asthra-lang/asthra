/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Declaration Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for generic struct declaration edge cases and syntax validation
 */

#ifndef GENERIC_STRUCTS_TEST_DECLARATIONS_H
#define GENERIC_STRUCTS_TEST_DECLARATIONS_H

#include "generic_structs_test_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DECLARATION VALIDATION TESTS
// =============================================================================

/**
 * Test 1: Edge Cases in Generic Struct Declarations
 * Tests various edge cases in generic struct declaration syntax
 * @return true if all tests pass, false otherwise
 */
bool test_generic_struct_declaration_edge_cases(void);

/**
 * Test 2: Invalid Generic Struct Syntax
 * Tests that invalid syntax is properly rejected
 * @return true if all tests pass, false otherwise
 */
bool test_invalid_generic_struct_syntax(void);

#ifdef __cplusplus
}
#endif

#endif // GENERIC_STRUCTS_TEST_DECLARATIONS_H 
