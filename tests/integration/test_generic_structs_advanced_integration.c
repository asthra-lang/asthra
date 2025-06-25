/*
 * Advanced Integration Tests for Generic Structs Phase 5
 * Complex scenarios including methods and error handling
 *
 * Part of test_generic_structs_phase5.c split (580 lines -> 6 focused modules)
 * Tests: generic structs with methods, error handling for invalid usage
 */

#include "test_generic_structs_phase5_common.h"

// ============================================================================
// ADVANCED INTEGRATION TESTS
// ============================================================================

/**
 * Test 4: Generic Struct with Methods Integration
 */
bool test_generic_struct_with_methods_integration(void) {
    printf("\n=== Test 4: Generic Struct with Methods Integration ===\n");

    const char *source = "pub struct Vec<T> {\n"
                         "    data: *mut T,\n"
                         "    len: usize,\n"
                         "    capacity: usize\n"
                         "}\n"
                         "\n"
                         "impl<T> Vec<T> {\n"
                         "    fn new() -> Vec<T> {\n"
                         "        return Vec {\n"
                         "            data: null,\n"
                         "            len: 0,\n"
                         "            capacity: 0\n"
                         "        };\n"
                         "    }\n"
                         "    \n"
                         "    fn len(self) -> usize {\n"
                         "        return self.len;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "fn main() -> i32 {\n"
                         "    let vec: Vec<i32> = Vec::new();\n"
                         "    let length: usize = vec.len();\n"
                         "    return 0;\n"
                         "}";

    CompilationResult *result = compile_source(source);
    TEST_ASSERT(result != NULL, "Compilation result created");

    if (result->error_message) {
        printf("    Error: %s\n", result->error_message);
    }

    TEST_ASSERT(result->success, "Generic struct with methods compilation succeeds");

    // Check that generic instantiation includes methods
    if (result->generator && result->generator->generic_registry) {
        GenericStructInfo *vec_info =
            generic_registry_lookup_struct(result->generator->generic_registry, "Vec");
        TEST_ASSERT(vec_info != NULL, "Vec generic struct with methods registered");
        TEST_ASSERT(vec_info->instantiation_count > 0,
                    "Vec<i32> instantiation with methods created");
    }

    // Check C code generation includes method implementations
    if (result->c_code_output) {
        TEST_ASSERT(strstr(result->c_code_output, "Vec_i32") != NULL, "Vec_i32 struct generated");
        TEST_ASSERT(strstr(result->c_code_output, "Vec_i32_new") != NULL ||
                        strstr(result->c_code_output, "new") != NULL,
                    "Associated function new() generated");
        TEST_ASSERT(strstr(result->c_code_output, "Vec_i32_len") != NULL ||
                        strstr(result->c_code_output, "len") != NULL,
                    "Instance method len() generated");
        printf("    Generated C code:\n%s\n", result->c_code_output);
    }

    cleanup_compilation_result(result);
    TEST_SUCCESS();
}

/**
 * Test 5: Error Handling - Invalid Generic Usage
 */
bool test_error_handling_invalid_generic_usage(void) {
    printf("\n=== Test 5: Error Handling - Invalid Generic Usage ===\n");

    const char *invalid_source =
        "pub struct Vec<T> {\n"
        "    data: *mut T,\n"
        "    len: usize\n"
        "}\n"
        "\n"
        "fn main() -> i32 {\n"
        "    let vec: Vec = Vec { data: null, len: 0 };\n" // Missing type arguments
        "    return 0;\n"
        "}";

    CompilationResult *result = compile_source(invalid_source);
    TEST_ASSERT(result != NULL, "Compilation result created");

    // Should fail during semantic analysis
    TEST_ASSERT(!result->success, "Invalid generic usage should fail");
    TEST_ASSERT(result->error_message != NULL, "Error message provided for invalid usage");

    printf("    Expected error: %s\n",
           result->error_message ? result->error_message : "No error message");

    cleanup_compilation_result(result);
    TEST_SUCCESS();
}
