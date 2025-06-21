/**
 * Asthra Programming Language Compiler
 * Phase 4: Generic Structs Code Generation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Comprehensive test suite for generic struct monomorphization and C code generation
 */

#include "code_generator_core.h"
#include "generic_instantiation.h"
#include "semantic_analyzer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// TEST FRAMEWORK
// =============================================================================

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(condition, message) do { \
    tests_run++; \
    if (condition) { \
        tests_passed++; \
        printf("✅ PASS: %s\n", message); \
    } else { \
        printf("❌ FAIL: %s\n", message); \
    } \
} while (0)

#define TEST_SUCCESS() return true
#define TEST_FAILURE() return false

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Create a simple type descriptor for testing
 */
static TypeDescriptor *create_test_type(const char *name, TypeCategory category) {
    TypeDescriptor *type = malloc(sizeof(TypeDescriptor));
    if (!type) return NULL;
    
    type->category = category;
    type->name = strdup(name);
    type->size = 4; // Simplified
    type->alignment = 4;
    type->flags.is_mutable = false;
    type->flags.is_owned = false;
    type->flags.is_borrowed = false;
    type->flags.is_constant = false;
    type->flags.is_volatile = false;
    type->flags.is_atomic = false;
    type->flags.is_ffi_compatible = true;
    type->flags.reserved = 0;
    atomic_init(&type->ref_count, 1);
    
    return type;
}

/**
 * Create a test AST node for a generic struct declaration
 */
static ASTNode *create_test_generic_struct_decl(const char *struct_name, const char **type_params, size_t param_count) {
    ASTNode *struct_decl = malloc(sizeof(ASTNode));
    if (!struct_decl) return NULL;
    
    // Zero out the entire structure to avoid garbage values
    memset(struct_decl, 0, sizeof(ASTNode));
    
    struct_decl->type = AST_STRUCT_DECL;
    struct_decl->data.struct_decl.name = strdup(struct_name);
    struct_decl->data.struct_decl.fields = NULL; // Simplified for testing
    
    // Create type parameters
    if (type_params && param_count > 0) {
        struct_decl->data.struct_decl.type_params = ast_node_list_create(param_count);
        if (struct_decl->data.struct_decl.type_params) {
            for (size_t i = 0; i < param_count; i++) {
                ASTNode *param = malloc(sizeof(ASTNode));
                // Zero out the parameter node too
                memset(param, 0, sizeof(ASTNode));
                param->type = AST_IDENTIFIER;
                param->data.identifier.name = strdup(type_params[i]);
                ast_node_list_add(&struct_decl->data.struct_decl.type_params, param);
            }
        }
    } else {
        struct_decl->data.struct_decl.type_params = NULL;
    }
    
    return struct_decl;
}

/**
 * Cleanup test AST node
 */
static void cleanup_test_ast_node(ASTNode *node) {
    if (!node) return;
    
    if (node->type == AST_STRUCT_DECL) {
        free((void*)node->data.struct_decl.name);
        
        if (node->data.struct_decl.type_params) {
            // Use the proper AST node list cleanup function
            ast_node_list_destroy(node->data.struct_decl.type_params);
        }
    }
    
    free(node);
}

// =============================================================================
// PHASE 4 TESTS
// =============================================================================

/**
 * Test 1: Generic Registry Creation and Destruction
 */
static bool test_generic_registry_creation(void) {
    printf("\n=== Test 1: Generic Registry Creation ===\n");
    
    GenericRegistry *registry = generic_registry_create();
    TEST_ASSERT(registry != NULL, "Generic registry creation");
    TEST_ASSERT(registry->struct_count == 0, "Initial struct count is zero");
    TEST_ASSERT(registry->struct_capacity >= 1, "Registry has initial capacity");
    
    generic_registry_destroy(registry);
    TEST_ASSERT(true, "Generic registry destruction (no crash)");
    
    TEST_SUCCESS();
}

/**
 * Test 2: Generic Struct Registration
 */
static bool test_generic_struct_registration(void) {
    printf("\n=== Test 2: Generic Struct Registration ===\n");
    
    GenericRegistry *registry = generic_registry_create();
    TEST_ASSERT(registry != NULL, "Registry creation");
    
    // Create test struct
    const char *type_params[] = {"T"};
    ASTNode *struct_decl = create_test_generic_struct_decl("Vec", type_params, 1);
    TypeDescriptor *struct_type = create_test_type("Vec", TYPE_STRUCT);
    
    TEST_ASSERT(struct_decl != NULL, "Test struct AST creation");
    TEST_ASSERT(struct_type != NULL, "Test struct type creation");
    
    // Register the struct
    bool success = generic_registry_register_struct(registry, "Vec", struct_decl, struct_type);
    TEST_ASSERT(success, "Generic struct registration");
    TEST_ASSERT(registry->struct_count == 1, "Registry has one struct after registration");
    
    // Test lookup
    GenericStructInfo *info = generic_registry_lookup_struct(registry, "Vec");
    TEST_ASSERT(info != NULL, "Lookup registered generic struct");
    TEST_ASSERT(strcmp(info->generic_name, "Vec") == 0, "Registered struct has correct name");
    TEST_ASSERT(info->type_param_count == 1, "Registered struct has correct type parameter count");
    
    // Test duplicate registration
    bool duplicate = generic_registry_register_struct(registry, "Vec", struct_decl, struct_type);
    TEST_ASSERT(duplicate, "Duplicate registration returns true (no error)");
    TEST_ASSERT(registry->struct_count == 1, "Registry still has one struct after duplicate registration");
    
    // Cleanup
    type_descriptor_release(struct_type);
    cleanup_test_ast_node(struct_decl);
    generic_registry_destroy(registry);
    
    TEST_SUCCESS();
}

/**
 * Test 3: Generic Struct Instantiation
 */
static bool test_generic_struct_instantiation(void) {
    printf("\n=== Test 3: Generic Struct Instantiation ===\n");
    
    // Create code generator and registry
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    TEST_ASSERT(generator != NULL, "Code generator creation");
    TEST_ASSERT(generator->generic_registry != NULL, "Code generator has generic registry");
    
    // Create and register generic struct
    const char *type_params[] = {"T"};
    ASTNode *struct_decl = create_test_generic_struct_decl("Vec", type_params, 1);
    TypeDescriptor *struct_type = create_test_type("Vec", TYPE_STRUCT);
    
    bool registered = code_generator_register_generic_struct(generator, "Vec", struct_decl, struct_type);
    TEST_ASSERT(registered, "Generic struct registration via code generator");
    
    // Create type arguments for instantiation
    TypeDescriptor *i32_type = create_test_type("i32", TYPE_INTEGER);
    TypeDescriptor *type_args[] = {i32_type};
    
    // Test instantiation
    bool instantiated = code_generate_generic_struct_instantiation(generator, "Vec", type_args, 1);
    TEST_ASSERT(instantiated, "Generic struct instantiation");
    
    // Verify instantiation exists
    GenericStructInfo *info = generic_registry_lookup_struct(generator->generic_registry, "Vec");
    TEST_ASSERT(info != NULL, "Can lookup struct info after instantiation");
    TEST_ASSERT(info->instantiation_count == 1, "Struct has one instantiation");
    
    GenericInstantiation *inst = info->instantiations[0];
    TEST_ASSERT(inst != NULL, "Instantiation exists");
    TEST_ASSERT(strcmp(inst->concrete_name, "Vec_i32") == 0, "Concrete name is correct");
    TEST_ASSERT(inst->type_arg_count == 1, "Instantiation has correct type argument count");
    
    // Test duplicate instantiation
    bool duplicate_inst = code_generate_generic_struct_instantiation(generator, "Vec", type_args, 1);
    TEST_ASSERT(duplicate_inst, "Duplicate instantiation succeeds");
    TEST_ASSERT(info->instantiation_count == 1, "Still has only one instantiation (deduplication works)");
    
    // Cleanup
    type_descriptor_release(i32_type);
    type_descriptor_release(struct_type);
    cleanup_test_ast_node(struct_decl);
    code_generator_destroy(generator);
    
    TEST_SUCCESS();
}

/**
 * Test 4: Multiple Type Parameters
 */
static bool test_multiple_type_parameters(void) {
    printf("\n=== Test 4: Multiple Type Parameters ===\n");
    
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    TEST_ASSERT(generator != NULL, "Code generator creation");
    
    // Create Pair<A, B> struct
    const char *type_params[] = {"A", "B"};
    ASTNode *struct_decl = create_test_generic_struct_decl("Pair", type_params, 2);
    TypeDescriptor *struct_type = create_test_type("Pair", TYPE_STRUCT);
    
    bool registered = code_generator_register_generic_struct(generator, "Pair", struct_decl, struct_type);
    TEST_ASSERT(registered, "Pair struct registration");
    
    // Create type arguments for Pair<i32, string>
    TypeDescriptor *i32_type = create_test_type("i32", TYPE_INTEGER);
    TypeDescriptor *string_type = create_test_type("string", TYPE_PRIMITIVE);
    TypeDescriptor *type_args[] = {i32_type, string_type};
    
    // Test instantiation
    bool instantiated = code_generate_generic_struct_instantiation(generator, "Pair", type_args, 2);
    TEST_ASSERT(instantiated, "Pair<i32, string> instantiation");
    
    // Verify instantiation
    GenericStructInfo *info = generic_registry_lookup_struct(generator->generic_registry, "Pair");
    TEST_ASSERT(info != NULL, "Can lookup Pair struct info");
    TEST_ASSERT(info->instantiation_count == 1, "Pair has one instantiation");
    
    GenericInstantiation *inst = info->instantiations[0];
    TEST_ASSERT(inst != NULL, "Pair instantiation exists");
    TEST_ASSERT(strcmp(inst->concrete_name, "Pair_i32_string") == 0, "Concrete name is correct for multiple params");
    TEST_ASSERT(inst->type_arg_count == 2, "Instantiation has correct type argument count");
    
    // Create second instantiation Pair<string, bool>
    TypeDescriptor *bool_type = create_test_type("bool", TYPE_BOOL);
    TypeDescriptor *type_args2[] = {string_type, bool_type};
    
    bool instantiated2 = code_generate_generic_struct_instantiation(generator, "Pair", type_args2, 2);
    TEST_ASSERT(instantiated2, "Pair<string, bool> instantiation");
    TEST_ASSERT(info->instantiation_count == 2, "Pair has two instantiations");
    
    // Cleanup
    type_descriptor_release(i32_type);
    type_descriptor_release(string_type);
    type_descriptor_release(bool_type);
    type_descriptor_release(struct_type);
    cleanup_test_ast_node(struct_decl);
    code_generator_destroy(generator);
    
    TEST_SUCCESS();
}

/**
 * Test 5: C Code Generation
 */
static bool test_c_code_generation(void) {
    printf("\n=== Test 5: C Code Generation ===\n");
    
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    TEST_ASSERT(generator != NULL, "Code generator creation");
    
    // Register Vec<T> struct
    const char *type_params[] = {"T"};
    ASTNode *struct_decl = create_test_generic_struct_decl("Vec", type_params, 1);
    TypeDescriptor *struct_type = create_test_type("Vec", TYPE_STRUCT);
    
    code_generator_register_generic_struct(generator, "Vec", struct_decl, struct_type);
    
    // Instantiate Vec<i32>
    TypeDescriptor *i32_type = create_test_type("i32", TYPE_INTEGER);
    TypeDescriptor *type_args[] = {i32_type};
    
    code_generate_generic_struct_instantiation(generator, "Vec", type_args, 1);
    
    // Generate C code
    char output_buffer[4096];
    bool generated = code_generate_all_generic_instantiations(generator, output_buffer, sizeof(output_buffer));
    TEST_ASSERT(generated, "C code generation succeeds");
    
    // Verify generated code contains expected elements
    TEST_ASSERT(strstr(output_buffer, "Vec_i32") != NULL, "Generated code contains concrete struct name");
    TEST_ASSERT(strstr(output_buffer, "typedef struct") != NULL, "Generated code contains struct definition");
    TEST_ASSERT(strstr(output_buffer, "// Generated from generic struct Vec") != NULL, "Generated code contains comment");
    
    printf("Generated C code:\n%s\n", output_buffer);
    
    // Cleanup
    type_descriptor_release(i32_type);
    type_descriptor_release(struct_type);
    cleanup_test_ast_node(struct_decl);
    code_generator_destroy(generator);
    
    TEST_SUCCESS();
}

/**
 * Test 6: Error Handling
 */
static bool test_error_handling(void) {
    printf("\n=== Test 6: Error Handling ===\n");
    
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    TEST_ASSERT(generator != NULL, "Code generator creation");
    
    // Test instantiation of non-existent struct
    TypeDescriptor *i32_type = create_test_type("i32", TYPE_INTEGER);
    TypeDescriptor *type_args[] = {i32_type};
    
    bool failed_inst = code_generate_generic_struct_instantiation(generator, "NonExistent", type_args, 1);
    TEST_ASSERT(!failed_inst, "Instantiation of non-existent struct fails");
    
    // Register a struct for further testing
    const char *type_params[] = {"T"};
    ASTNode *struct_decl = create_test_generic_struct_decl("TestStruct", type_params, 1);
    TypeDescriptor *struct_type = create_test_type("TestStruct", TYPE_STRUCT);
    
    code_generator_register_generic_struct(generator, "TestStruct", struct_decl, struct_type);
    
    // Test wrong type argument count
    TypeDescriptor *type_args_too_many[] = {i32_type, i32_type};
    bool failed_count = code_generate_generic_struct_instantiation(generator, "TestStruct", type_args_too_many, 2);
    TEST_ASSERT(!failed_count, "Instantiation with wrong type argument count fails");
    
    // Test NULL inputs
    bool failed_null = code_generate_generic_struct_instantiation(generator, NULL, type_args, 1);
    TEST_ASSERT(!failed_null, "Instantiation with NULL struct name fails");
    
    bool failed_null_args = code_generate_generic_struct_instantiation(generator, "TestStruct", NULL, 1);
    TEST_ASSERT(!failed_null_args, "Instantiation with NULL type args fails");
    
    // Cleanup
    type_descriptor_release(i32_type);
    type_descriptor_release(struct_type);
    cleanup_test_ast_node(struct_decl);
    code_generator_destroy(generator);
    
    TEST_SUCCESS();
}

/**
 * Test 7: Statistics and Debugging
 */
static bool test_statistics_and_debugging(void) {
    printf("\n=== Test 7: Statistics and Debugging ===\n");
    
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    TEST_ASSERT(generator != NULL, "Code generator creation");
    
    // Register and instantiate multiple structs
    const char *type_params[] = {"T"};
    ASTNode *vec_decl = create_test_generic_struct_decl("Vec", type_params, 1);
    ASTNode *list_decl = create_test_generic_struct_decl("List", type_params, 1);
    TypeDescriptor *vec_type = create_test_type("Vec", TYPE_STRUCT);
    TypeDescriptor *list_type = create_test_type("List", TYPE_STRUCT);
    
    code_generator_register_generic_struct(generator, "Vec", vec_decl, vec_type);
    code_generator_register_generic_struct(generator, "List", list_decl, list_type);
    
    // Create instantiations
    TypeDescriptor *i32_type = create_test_type("i32", TYPE_INTEGER);
    TypeDescriptor *string_type = create_test_type("string", TYPE_PRIMITIVE);
    TypeDescriptor *type_args_i32[] = {i32_type};
    TypeDescriptor *type_args_string[] = {string_type};
    
    code_generate_generic_struct_instantiation(generator, "Vec", type_args_i32, 1);
    code_generate_generic_struct_instantiation(generator, "Vec", type_args_string, 1);
    code_generate_generic_struct_instantiation(generator, "List", type_args_i32, 1);
    
    // Print statistics
    printf("\nGeneric Registry Statistics:\n");
    generic_registry_print_stats(generator->generic_registry);
    
    // Test debug strings
    GenericStructInfo *vec_info = generic_registry_lookup_struct(generator->generic_registry, "Vec");
    if (vec_info && vec_info->instantiation_count > 0) {
        char *debug_str = generic_instantiation_debug_string(vec_info->instantiations[0]);
        TEST_ASSERT(debug_str != NULL, "Debug string generation");
        printf("Debug info: %s\n", debug_str);
        free(debug_str);
    }
    
    // Verify statistics
    TEST_ASSERT(atomic_load(&generator->generic_registry->total_instantiations) == 3, "Total instantiations statistic");
    TEST_ASSERT(atomic_load(&generator->generic_registry->concrete_structs_generated) == 3, "Concrete structs generated statistic");
    TEST_ASSERT(generator->generic_registry->struct_count == 2, "Total generic structs registered");
    
    // Cleanup
    type_descriptor_release(i32_type);
    type_descriptor_release(string_type);
    type_descriptor_release(vec_type);
    type_descriptor_release(list_type);
    cleanup_test_ast_node(vec_decl);
    cleanup_test_ast_node(list_decl);
    code_generator_destroy(generator);
    
    TEST_SUCCESS();
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("Phase 4: Generic Structs Code Generation Test Suite\n");
    printf("=============================================================================\n");
    
    // Run all tests
    test_generic_registry_creation();
    test_generic_struct_registration();
    test_generic_struct_instantiation();
    test_multiple_type_parameters();
    test_c_code_generation();
    test_error_handling();
    test_statistics_and_debugging();
    
    // Print summary
    printf("\n=============================================================================\n");
    printf("Test Summary: %d/%d tests passed (%.1f%%)\n", 
           tests_passed, tests_run, 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("=============================================================================\n");
    
    if (tests_passed == tests_run) {
        printf("🎉 All tests passed! Phase 4 implementation is working correctly.\n");
        return 0;
    } else {
        printf("❌ Some tests failed. Check the implementation.\n");
        return 1;
    }
} 
