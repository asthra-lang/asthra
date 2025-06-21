/**
 * Expression Validation Tests - Array/Slice Operations
 * 
 * Tests for array and slice operation validation.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// ARRAY/SLICE OPERATION TESTS
// =============================================================================

AsthraTestResult test_array_slice_operations(AsthraTestContext* context) {
    // Valid array operations
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3];\n"
        "    let first: i32 = arr[0];\n"
        "    let second: i32 = arr[1];\n"
        "}\n",
        "valid_array_indexing"
    ), "Valid array indexing");
    
    // Invalid index type
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3];\n"
        "    let bad: i32 = arr[\"zero\"];\n"
        "}\n",
        SEMANTIC_ERROR_INVALID_OPERATION,
        "string_array_index"
    ), "String as array index should fail");
    
    // Array length operation
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let arr: []i32 = [1, 2, 3];\n"
        "    let len: usize = arr.len();\n"
        "}\n",
        "valid_array_length"
    ), "Valid array length operation");
    
    return ASTHRA_TEST_PASS;
}