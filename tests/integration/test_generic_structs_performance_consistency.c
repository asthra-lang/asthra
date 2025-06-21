/*
 * Performance and Consistency Tests for Generic Structs Phase 5
 * Deduplication, performance, and type system consistency testing
 * 
 * Part of test_generic_structs_phase5.c split (580 lines -> 6 focused modules)
 * Tests: deduplication, performance with many instantiations, type system consistency
 */

#include "test_generic_structs_phase5_common.h"

// ============================================================================
// PERFORMANCE AND CONSISTENCY TESTS
// ============================================================================

/**
 * Test 6: Deduplication Integration
 */
bool test_deduplication_integration(void) {
    printf("\n=== Test 6: Deduplication Integration ===\n");
    
    const char *source = 
        "pub struct Vec<T> {\n"
        "    data: *mut T,\n"
        "    len: usize\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    let vec1: Vec<i32> = Vec { data: null, len: 0 };\n"
        "    let vec2: Vec<i32> = Vec { data: null, len: 0 };\n"
        "    let vec3: Vec<i32> = Vec { data: null, len: 0 };\n"
        "    return 0;\n"
        "}";
    
    CompilationResult *result = compile_source(source);
    TEST_ASSERT(result != NULL, "Compilation result created");
    
    if (result->error_message) {
        printf("    Error: %s\n", result->error_message);
    }
    
    TEST_ASSERT(result->success, "Deduplication test compilation succeeds");
    
    // Check that only one instantiation was created despite multiple uses
    if (result->generator && result->generator->generic_registry) {
        GenericStructInfo *vec_info = generic_registry_lookup_struct(result->generator->generic_registry, "Vec");
        TEST_ASSERT(vec_info != NULL, "Vec generic struct registered");
        TEST_ASSERT(vec_info->instantiation_count == 1, "Only one Vec<i32> instantiation despite multiple uses");
    }
    
    // Check C code only contains one struct definition
    if (result->c_code_output) {
        const char *first_vec_i32 = strstr(result->c_code_output, "Vec_i32");
        TEST_ASSERT(first_vec_i32 != NULL, "First Vec_i32 definition found");
        
        const char *second_vec_i32 = strstr(first_vec_i32 + 1, "Vec_i32");
        // Should not find another complete struct definition
        TEST_ASSERT(second_vec_i32 == NULL || 
                   strstr(second_vec_i32, "typedef struct") == NULL,
                   "No duplicate struct definitions");
        
        printf("    Generated C code:\n%s\n", result->c_code_output);
    }
    
    cleanup_compilation_result(result);
    TEST_SUCCESS();
}

/**
 * Test 7: Performance - Large Number of Instantiations
 */
bool test_performance_many_instantiations(void) {
    printf("\n=== Test 7: Performance - Many Instantiations ===\n");
    
    const char *source = 
        "pub struct Pair<A, B> {\n"
        "    first: A,\n"
        "    second: B\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    let p1: Pair<i32, i32> = Pair { first: 1, second: 2 };\n"
        "    let p2: Pair<i32, string> = Pair { first: 1, second: \"a\" };\n"
        "    let p3: Pair<string, i32> = Pair { first: \"a\", second: 1 };\n"
        "    let p4: Pair<string, string> = Pair { first: \"a\", second: \"b\" };\n"
        "    let p5: Pair<f64, f64> = Pair { first: 1.0, second: 2.0 };\n"
        "    let p6: Pair<bool, i32> = Pair { first: true, second: 1 };\n"
        "    return 0;\n"
        "}";
    
    CompilationResult *result = compile_source(source);
    TEST_ASSERT(result != NULL, "Compilation result created");
    
    if (result->error_message) {
        printf("    Error: %s\n", result->error_message);
    }
    
    TEST_ASSERT(result->success, "Many instantiations compilation succeeds");
    
    // Check that all different instantiations were created
    if (result->generator && result->generator->generic_registry) {
        GenericStructInfo *pair_info = generic_registry_lookup_struct(result->generator->generic_registry, "Pair");
        TEST_ASSERT(pair_info != NULL, "Pair generic struct registered");
        TEST_ASSERT(pair_info->instantiation_count >= 6, "Multiple different Pair instantiations created");
        
        // Print statistics
        printf("    Total instantiations: %zu\n", pair_info->instantiation_count);
    }
    
    cleanup_compilation_result(result);
    TEST_SUCCESS();
}

/**
 * Test 8: Type System Consistency - Generic Enums vs Structs
 */
bool test_type_system_consistency(void) {
    printf("\n=== Test 8: Type System Consistency - Generic Enums vs Structs ===\n");
    
    const char *source = 
        "// Generic enum (was already supported)\n"
        "enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "\n"
        "// Generic struct (new in this implementation)\n"
        "pub struct Container<T> {\n"
        "    value: T,\n"
        "    has_value: bool\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    let opt: Option<i32> = Option.Some(42);\n"
        "    let cont: Container<i32> = Container { value: 42, has_value: true };\n"
        "    return 0;\n"
        "}";
    
    CompilationResult *result = compile_source(source);
    TEST_ASSERT(result != NULL, "Compilation result created");
    
    if (result->error_message) {
        printf("    Error: %s\n", result->error_message);
    }
    
    TEST_ASSERT(result->success, "Generic enum and struct consistency test succeeds");
    
    // Both generic enums and generic structs should work consistently
    if (result->generator && result->generator->generic_registry) {
        GenericStructInfo *container_info = generic_registry_lookup_struct(result->generator->generic_registry, "Container");
        TEST_ASSERT(container_info != NULL, "Container generic struct works like generic enums");
    }
    
    cleanup_compilation_result(result);
    TEST_SUCCESS();
} 
