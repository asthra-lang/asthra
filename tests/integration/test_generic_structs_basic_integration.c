/*
 * Basic Integration Tests for Generic Structs Phase 5
 * Fundamental end-to-end testing of generic struct functionality
 * 
 * Part of test_generic_structs_phase5.c split (580 lines -> 6 focused modules)
 * Tests: basic generic structs, multiple type parameters, nested generics
 */

#include "test_generic_structs_phase5_common.h"

// ============================================================================
// BASIC INTEGRATION TESTS
// ============================================================================

/**
 * Test 1: Basic Generic Struct End-to-End
 */
bool test_basic_generic_struct_e2e(void) {
    printf("\n=== Test 1: Basic Generic Struct End-to-End ===\n");
    
    const char *source = 
        "pub struct Vec<T> {\n"
        "    data: *mut T,\n"
        "    len: usize,\n"
        "    capacity: usize\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    let numbers: Vec<i32> = Vec {\n"
        "        data: null,\n"
        "        len: 0,\n"
        "        capacity: 0\n"
        "    };\n"
        "    return 0;\n"
        "}";
    
    CompilationResult *result = compile_source(source);
    TEST_ASSERT(result != NULL, "Compilation result created");
    
    if (result->error_message) {
        printf("    Error: %s\n", result->error_message);
    }
    
    TEST_ASSERT(result->success, "End-to-end compilation succeeds");
    TEST_ASSERT(result->ast != NULL, "AST created successfully");
    TEST_ASSERT(result->analyzer != NULL, "Semantic analyzer created");
    TEST_ASSERT(result->generator != NULL, "Code generator created");
    
    // Check that generic instantiation was created
    if (result->generator && result->generator->generic_registry) {
        GenericStructInfo *vec_info = generic_registry_lookup_struct(result->generator->generic_registry, "Vec");
        TEST_ASSERT(vec_info != NULL, "Vec generic struct registered");
        TEST_ASSERT(vec_info->instantiation_count > 0, "Vec<i32> instantiation created");
    }
    
    // Check C code generation
    if (result->c_code_output) {
        TEST_ASSERT(strstr(result->c_code_output, "Vec_i32") != NULL, "Vec_i32 struct generated in C code");
        printf("    Generated C code:\n%s\n", result->c_code_output);
    }
    
    cleanup_compilation_result(result);
    TEST_SUCCESS();
}

/**
 * Test 2: Multiple Type Parameters Integration
 */
bool test_multiple_type_parameters_integration(void) {
    printf("\n=== Test 2: Multiple Type Parameters Integration ===\n");
    
    const char *source = 
        "pub struct Pair<A, B> {\n"
        "    first: A,\n"
        "    second: B\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    let pair: Pair<i32, string> = Pair {\n"
        "        first: 42,\n"
        "        second: \"hello\"\n"
        "    };\n"
        "    return 0;\n"
        "}";
    
    CompilationResult *result = compile_source(source);
    TEST_ASSERT(result != NULL, "Compilation result created");
    
    if (result->error_message) {
        printf("    Error: %s\n", result->error_message);
    }
    
    TEST_ASSERT(result->success, "Multiple type parameters compilation succeeds");
    
    // Check that generic instantiation was created with correct types
    if (result->generator && result->generator->generic_registry) {
        GenericStructInfo *pair_info = generic_registry_lookup_struct(result->generator->generic_registry, "Pair");
        TEST_ASSERT(pair_info != NULL, "Pair generic struct registered");
        TEST_ASSERT(pair_info->instantiation_count > 0, "Pair<i32, string> instantiation created");
    }
    
    // Check C code generation for multiple type parameters
    if (result->c_code_output) {
        TEST_ASSERT(strstr(result->c_code_output, "Pair_i32_string") != NULL, 
                   "Pair_i32_string struct generated with correct naming");
        printf("    Generated C code:\n%s\n", result->c_code_output);
    }
    
    cleanup_compilation_result(result);
    TEST_SUCCESS();
}

/**
 * Test 3: Nested Generic Types Integration
 */
bool test_nested_generic_types_integration(void) {
    printf("\n=== Test 3: Nested Generic Types Integration ===\n");
    
    const char *source = 
        "pub struct Vec<T> {\n"
        "    data: *mut T,\n"
        "    len: usize\n"
        "}\n"
        "\n"
        "pub struct Container<T, M> {\n"
        "    items: Vec<T>,\n"
        "    metadata: M\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    let container: Container<i32, string> = Container {\n"
        "        items: Vec { data: null, len: 0 },\n"
        "        metadata: \"test\"\n"
        "    };\n"
        "    return 0;\n"
        "}";
    
    CompilationResult *result = compile_source(source);
    TEST_ASSERT(result != NULL, "Compilation result created");
    
    if (result->error_message) {
        printf("    Error: %s\n", result->error_message);
    }
    
    TEST_ASSERT(result->success, "Nested generic types compilation succeeds");
    
    // Check that both generic instantiations were created
    if (result->generator && result->generator->generic_registry) {
        GenericStructInfo *vec_info = generic_registry_lookup_struct(result->generator->generic_registry, "Vec");
        GenericStructInfo *container_info = generic_registry_lookup_struct(result->generator->generic_registry, "Container");
        
        TEST_ASSERT(vec_info != NULL, "Vec generic struct registered");
        TEST_ASSERT(container_info != NULL, "Container generic struct registered");
        TEST_ASSERT(vec_info->instantiation_count > 0, "Vec<i32> instantiation created");
        TEST_ASSERT(container_info->instantiation_count > 0, "Container<i32, string> instantiation created");
    }
    
    // Check C code generation for nested types
    if (result->c_code_output) {
        TEST_ASSERT(strstr(result->c_code_output, "Vec_i32") != NULL, "Vec_i32 struct generated");
        TEST_ASSERT(strstr(result->c_code_output, "Container_i32_string") != NULL, 
                   "Container_i32_string struct generated");
        printf("    Generated C code:\n%s\n", result->c_code_output);
    }
    
    cleanup_compilation_result(result);
    TEST_SUCCESS();
} 
