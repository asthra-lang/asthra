/**
 * Asthra Programming Language v1.2 Enhanced FFI Tests
 * Header file for Enhanced FFI Test Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Declarations for all enhanced FFI test functions split across multiple files.
 */

#ifndef TEST_FFI_ENHANCED_H
#define TEST_FFI_ENHANCED_H

#include "../core/test_comprehensive.h"

// =============================================================================
// ENHANCED FFI CAPABILITY TESTS
// =============================================================================

/**
 * Test enhanced FFI function call capabilities including simple functions,
 * string functions, and function pointer handling.
 */
AsthraTestResult test_ffi_enhanced_function_calls(AsthraV12TestContext *ctx);

/**
 * Test bidirectional FFI calls (C calling Asthra, Asthra calling C)
 * including callback mechanisms.
 */
AsthraTestResult test_ffi_bidirectional_calls(AsthraV12TestContext *ctx);

/**
 * Test FFI with complex data structures including structs and arrays.
 */
AsthraTestResult test_ffi_complex_data_structures(AsthraV12TestContext *ctx);

// =============================================================================
// OWNERSHIP TRANSFER TESTS
// =============================================================================

/**
 * Test borrow ownership mode where data is borrowed without transferring ownership.
 */
AsthraTestResult test_ffi_ownership_borrow(AsthraV12TestContext *ctx);

/**
 * Test move ownership mode where ownership is transferred to the callee.
 */
AsthraTestResult test_ffi_ownership_move(AsthraV12TestContext *ctx);

/**
 * Test copy ownership mode where data is copied for independent usage.
 */
AsthraTestResult test_ffi_ownership_copy(AsthraV12TestContext *ctx);

// =============================================================================
// MEMORY SAFETY TESTS
// =============================================================================

/**
 * Test memory safety with bounds checking including safe array access
 * and NULL pointer safety.
 */
AsthraTestResult test_ffi_memory_safety_bounds_checking(AsthraV12TestContext *ctx);

/**
 * Test memory leak prevention in FFI including allocation tracking
 * and automatic cleanup.
 */
AsthraTestResult test_ffi_memory_safety_leak_prevention(AsthraV12TestContext *ctx);

/**
 * Test double-free prevention using safe pointer management.
 */
AsthraTestResult test_ffi_memory_safety_double_free_prevention(AsthraV12TestContext *ctx);

// =============================================================================
// C17 COMPLIANCE TESTS
// =============================================================================

/**
 * Test C17 standard compliance features including _Static_assert,
 * _Alignas/_Alignof, _Generic, and _Thread_local.
 */
AsthraTestResult test_ffi_c17_compliance_features(AsthraV12TestContext *ctx);

/**
 * Test C17 standard type compliance including exact-width integer types
 * and pointer conversions.
 */
AsthraTestResult test_ffi_c17_compliance_types(AsthraV12TestContext *ctx);

/**
 * Test C17 compliant function declarations and calls including inline functions,
 * restrict keyword, VLAs, and compound literals.
 */
AsthraTestResult test_ffi_c17_compliance_functions(AsthraV12TestContext *ctx);

#endif // TEST_FFI_ENHANCED_H
