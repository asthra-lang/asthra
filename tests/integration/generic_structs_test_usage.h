/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Usage Validation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for generic type usage validation and complex nested types
 */

#ifndef GENERIC_STRUCTS_TEST_USAGE_H
#define GENERIC_STRUCTS_TEST_USAGE_H

#include "generic_structs_test_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// USAGE VALIDATION TESTS
// =============================================================================

/**
 * Test 3: Generic Type Usage Validation
 * Tests proper usage of generic types in various contexts
 * @return true if all tests pass, false otherwise
 */
bool test_generic_type_usage_validation(void);

/**
 * Test 4: Complex Nested Generic Types
 * Tests deeply nested and complex generic type combinations
 * @return true if all tests pass, false otherwise
 */
bool test_complex_nested_generic_types(void);

/**
 * Test 5: Generic Struct Pattern Matching Validation
 * Tests pattern matching with generic structs
 * @return true if all tests pass, false otherwise
 */
bool test_generic_struct_pattern_matching_validation(void);

#ifdef __cplusplus
}
#endif

#endif // GENERIC_STRUCTS_TEST_USAGE_H 
