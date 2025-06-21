/**
 * Asthra Programming Language
 * Struct Mutability Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for struct field mutability semantics
 */

#include "test_mutability_common.h"
#include "semantic_errors.h"
#include "../framework/test_framework.h"

// =============================================================================
// STRUCT FIELD MUTABILITY TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_immutable_struct_field_access, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Data {\n"
        "    pub value: int\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let data: Data = Data { value: 42 };\n"
        "    let x: int = data.value;  // OK: Can read field\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "immutable_struct_field_read"),
        "Should allow reading fields of immutable struct");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_immutable_struct_field_modification_error, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Data {\n"
        "    pub value: int\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let data: Data = Data { value: 42 };\n"
        "    data.value = 100;  // ERROR: Container is immutable\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_error(source, SEMANTIC_ERROR_IMMUTABLE_MODIFICATION, "immutable_struct_field_modification"),
        "Should error on field modification of immutable struct");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_mutable_struct_field_modification, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Data {\n"
        "    pub value: int\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut data: Data = Data { value: 42 };\n"
        "    data.value = 100;  // OK: Container is mutable\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "mutable_struct_field_modification"),
        "Should allow field modification of mutable struct");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_nested_struct_mutability, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Inner {\n"
        "    pub value: int\n"
        "}\n"
        "\n"
        "pub struct Outer {\n"
        "    pub inner: Inner\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut outer: Outer = Outer { inner: Inner { value: 42 } };\n"
        "    outer.inner.value = 100;  // OK: Nested mutability follows container\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_mutability_success(source, "nested_struct_mutability"),
        "Should allow nested field modification when container is mutable");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_immutable_struct_field_access_metadata,
        test_immutable_struct_field_modification_error_metadata,
        test_mutable_struct_field_modification_metadata,
        test_nested_struct_mutability_metadata
    };
    
    AsthraTestFunction test_functions[] = {
        test_immutable_struct_field_access,
        test_immutable_struct_field_modification_error,
        test_mutable_struct_field_modification,
        test_nested_struct_mutability
    };
    
    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);
    
    AsthraTestSuiteConfig config = {
        .name = "Struct Mutability Tests",
        .description = "Tests for struct field mutability semantics",
        .parallel_execution = false,
        .default_timeout_ns = 30000000000ULL,
        .json_output = false,
        .verbose_output = true
    };
    
    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}