/**
 * Asthra Programming Language
 * Test Framework - Modular Assertions Umbrella Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Umbrella header providing 100% backward compatibility with original test_assertions.h
 * while using the new modular architecture
 */

#ifndef ASTHRA_TEST_ASSERTIONS_MODULAR_H
#define ASTHRA_TEST_ASSERTIONS_MODULAR_H

// Include all modular assertion components
#include "test_assertions_common.h"
#include "test_assertions_basic.h"
#include "test_assertions_equality.h"
#include "test_assertions_range.h"
#include "test_assertions_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MODULAR ASSERTION SYSTEM
// =============================================================================

/*
 * This header provides complete backward compatibility with the original
 * test_assertions.h interface while using the new modular architecture.
 * 
 * The modular system provides:
 * 
 * 1. test_assertions_common.{h,c} - Core assertion function and utilities
 * 2. test_assertions_basic.{h,c} - Basic type assertions (int, string, pointer)
 * 3. test_assertions_equality.{h,c} - Equality assertions with detailed error messages
 * 4. test_assertions_range.{h,c} - Range and comparison assertions
 * 5. test_assertions_compiler.{h,c} - Compiler-specific assertions (tokens, AST)
 * 
 * Benefits:
 * - Enhanced maintainability through focused modules
 * - Superior debugging with isolated functionality
 * - Development scalability with parallel workflows
 * - Zero breaking changes - all existing code works unchanged
 * - Selective inclusion for performance optimization
 */

// =============================================================================
// BACKWARD COMPATIBILITY VERIFICATION
// =============================================================================

/*
 * All functions from the original test_assertions.h are available:
 * 
 * Basic Assertions:
 * - asthra_test_assert_bool()
 * - asthra_test_assert_int()
 * - asthra_test_assert_long()
 * - asthra_test_assert_size()
 * - asthra_test_assert_string()
 * - asthra_test_assert_pointer()
 * - asthra_test_assert_not_null()
 * - asthra_test_assert_generic()
 * 
 * Equality Assertions:
 * - asthra_test_assert_bool_eq()
 * - asthra_test_assert_int_eq()
 * - asthra_test_assert_long_eq()
 * - asthra_test_assert_size_eq()
 * - asthra_test_assert_string_eq()
 * - asthra_test_assert_pointer_eq()
 * - asthra_test_assert_generic_eq()
 * - asthra_test_assert_str_eq()
 * - asthra_test_assert_ptr_eq()
 * - asthra_test_assert_ptr_ne()
 * 
 * Range Assertions:
 * - asthra_test_assert_int_range()
 * - asthra_test_assert_long_range()
 * - asthra_test_assert_size_range()
 * - asthra_test_assert_double_range()
 * - asthra_test_assert_generic_range()
 * - asthra_test_assert_int_gt()
 * 
 * Compiler Assertions:
 * - assert_token_equals()
 * - assert_ast_structure()
 * - tokens_equal()
 * - ast_has_structure()
 * 
 * Utilities:
 * - asthra_test_log()
 * 
 * All macros from the original header are also available:
 * - ASSERT_TOKEN_EQUALS()
 * - ASSERT_TOKEN_TYPE()
 * - ASSERT_AST_STRUCTURE()
 * - ASSERT_AST_NODE_TYPE()
 * - ASSERT_SYMBOL_EXISTS()
 * - ASSERT_TYPE_COMPATIBLE()
 * - ASSERT_ERROR_COUNT()
 * - ASSERT_NO_SEMANTIC_ERRORS()
 */

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_ASSERTIONS_MODULAR_H 
