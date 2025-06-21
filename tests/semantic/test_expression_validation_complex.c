/**
 * Expression Validation Tests - Complex Expressions
 * 
 * Tests for complex expression trees, nested operations, and type inference.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// COMPLEX EXPRESSION TREE TESTS
// =============================================================================

AsthraTestResult test_complex_expression_trees(AsthraTestContext* context) {
    // Valid complex expressions
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let result: int = (5 + 3) * (10 - 2) / 4;\n"
        "    let complex: int = ((1 + 2) * 3 - 4) / (5 + 6);\n"
        "}\n",
        "valid_complex_arithmetic"
    ), "Valid complex arithmetic expressions");
    
    // Type inference through expressions
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let condition: bool = true;\n"
        "    let value: int = if condition { 42 } else { 84 };\n"
        "}\n",
        "valid_if_expression_type_inference"
    ), "Valid if expression with type inference");
    
    // Error: Mismatched branch types
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let condition: bool = true;\n"
        "    let bad: int = if condition { 42 } else { \"text\" };\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "mismatched_if_branches"
    ), "Mismatched if branch types should fail");
    
    // Complex expression with mixed operations
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: int = 5;\n"
        "    let b: int = 10;\n"
        "    let result: bool = (a < b) && (a + b > 12) || (b - a == 5);\n"
        "}\n",
        "valid_mixed_operations"
    ), "Valid mixed operation expressions");
    
    return ASTHRA_TEST_PASS;
}