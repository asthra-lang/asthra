/**
 * Asthra Programming Language Compiler
 * Semantic Test: Fixed-Size Array Types
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for fixed-size array type creation and operations
 */

#include "../framework/test_framework.h"
#include "test_type_system_common.h"
#include "../../src/analysis/semantic_type_creation.h"
#include "../../src/analysis/semantic_type_descriptors.h" 
#include "../../src/analysis/semantic_type_helpers.h"
#include "../../src/analysis/semantic_builtins.h"
#include <stdio.h>
#include <string.h>

ASTHRA_TEST_DEFINE(test_array_type_creation, ASTHRA_TEST_SEVERITY_CRITICAL) {
    // Get a basic element type
    TypeDescriptor *i32_type = get_builtin_type("i32");
    if (!i32_type) {
        printf("Failed to get i32 builtin type\n");
        return ASTHRA_TEST_FAIL;
    }
    
    // Create an array type [5]i32
    TypeDescriptor *array_type = type_descriptor_create_array(i32_type, 5);
    if (!array_type) {
        printf("Failed to create array type\n");
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify array properties
    if (array_type->category != TYPE_ARRAY) {
        printf("Array type category mismatch\n");
        type_descriptor_release(array_type);
        return ASTHRA_TEST_FAIL;
    }
    
    if (array_type->data.array.size != 5) {
        printf("Array size mismatch: expected 5, got %zu\n", array_type->data.array.size);
        type_descriptor_release(array_type);
        return ASTHRA_TEST_FAIL;
    }
    
    if (array_type->data.array.element_type != i32_type) {
        printf("Array element type mismatch\n");
        type_descriptor_release(array_type);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify size calculation (5 elements * 4 bytes each)
    if (array_type->size != 20) {
        printf("Array size calculation mismatch: expected 20, got %zu\n", array_type->size);
        type_descriptor_release(array_type);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify name generation
    if (!array_type->name) {
        printf("Array type name is null\n");
        type_descriptor_release(array_type);
        return ASTHRA_TEST_FAIL;
    }
    
    if (strcmp(array_type->name, "[5]i32") != 0) {
        printf("Array name mismatch: expected '[5]i32', got '%s'\n", array_type->name);
        type_descriptor_release(array_type);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify FFI compatibility
    if (!array_type->flags.is_ffi_compatible) {
        printf("Array type should be FFI compatible\n");
        type_descriptor_release(array_type);
        return ASTHRA_TEST_FAIL;
    }
    
    // Cleanup
    type_descriptor_release(array_type);
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("Starting Array Types Test\n");
    
    AsthraTestMetadata metadata = {
        .name = "test_array_type_creation",
        .file = __FILE__,
        .line = __LINE__,
        .function = "test_array_type_creation",
        .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
        .timeout_ns = 0,
        .skip = false
    };
    
    AsthraTestFunction tests[] = { test_array_type_creation };
    AsthraTestMetadata metadatas[] = { metadata };
    
    AsthraTestSuiteConfig config = asthra_test_suite_config_create(
        "Array Types Test Suite",
        "Testing array type creation and operations"
    );
    
    AsthraTestResult result = asthra_test_run_suite(
        tests,
        metadatas,
        1,
        &config
    );
    
    printf("Test result: %s\n", result == ASTHRA_TEST_PASS ? "PASSED" : "FAILED");
    
    return result == ASTHRA_TEST_PASS ? 0 : 1;
}