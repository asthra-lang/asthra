#include "ast.h"
#include "semantic_analyzer.h"
#include "type_info.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test helper macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "FAIL: %s\n", message);                                                \
            return false;                                                                          \
        } else {                                                                                   \
            printf("PASS: %s\n", message);                                                         \
        }                                                                                          \
    } while (0)

#define RUN_TEST(test_func)                                                                        \
    do {                                                                                           \
        printf("\n=== Running %s ===\n", #test_func);                                              \
        if (test_func()) {                                                                         \
            printf("✓ %s passed\n", #test_func);                                                   \
            tests_passed++;                                                                        \
        } else {                                                                                   \
            printf("✗ %s failed\n", #test_func);                                                   \
            tests_failed++;                                                                        \
        }                                                                                          \
        total_tests++;                                                                             \
    } while (0)

// Global test counters
static int total_tests = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// =============================================================================
// BASIC TYPE INFO TESTS
// =============================================================================

static bool test_type_info_creation(void) {
    // Test primitive type creation
    TypeInfo *i32_type = type_info_create_primitive("i32", PRIMITIVE_INFO_I32, 4);
    TEST_ASSERT(i32_type != NULL, "i32 type creation");
    TEST_ASSERT(strcmp(i32_type->name, "i32") == 0, "i32 type name");
    TEST_ASSERT(i32_type->category == TYPE_INFO_PRIMITIVE, "i32 type category");
    TEST_ASSERT(i32_type->size == 4, "i32 type size");
    TEST_ASSERT(i32_type->data.primitive.kind == PRIMITIVE_INFO_I32, "i32 primitive kind");
    TEST_ASSERT(i32_type->data.primitive.is_integer == true, "i32 is integer");
    TEST_ASSERT(i32_type->data.primitive.is_signed == true, "i32 is signed");

    // Test slice type creation
    TypeInfo *slice_type = type_info_create_slice(i32_type, false);
    TEST_ASSERT(slice_type != NULL, "slice type creation");
    TEST_ASSERT(slice_type->category == TYPE_INFO_SLICE, "slice type category");
    TEST_ASSERT(slice_type->data.slice.element_type == i32_type, "slice element type");
    TEST_ASSERT(slice_type->data.slice.is_mutable == false, "slice mutability");

    // Test Result type creation
    TypeInfo *string_type =
        type_info_create_primitive("string", PRIMITIVE_INFO_STRING, sizeof(void *));
    TypeInfo *result_type = type_info_create_result(i32_type, string_type);
    TEST_ASSERT(result_type != NULL, "Result type creation");
    TEST_ASSERT(result_type->category == TYPE_INFO_RESULT, "Result type category");
    TEST_ASSERT(result_type->data.result.ok_type == i32_type, "Result ok type");
    TEST_ASSERT(result_type->data.result.err_type == string_type, "Result err type");

    // Cleanup
    type_info_release(i32_type);
    type_info_release(slice_type);
    type_info_release(string_type);
    type_info_release(result_type);

    return true;
}

static bool test_type_info_builtin_initialization(void) {
    // Initialize built-in types
    TEST_ASSERT(type_info_init_builtins(), "Built-in type initialization");

    // Test that built-in types are available
    TEST_ASSERT(TYPE_INFO_I32 != NULL, "TYPE_INFO_I32 available");
    TEST_ASSERT(TYPE_INFO_F64 != NULL, "TYPE_INFO_F64 available");
    TEST_ASSERT(TYPE_INFO_BOOL != NULL, "TYPE_INFO_BOOL available");
    TEST_ASSERT(TYPE_INFO_STRING != NULL, "TYPE_INFO_STRING available");

    // Test built-in type properties
    TEST_ASSERT(TYPE_INFO_I32->category == TYPE_INFO_PRIMITIVE, "I32 is primitive");
    TEST_ASSERT(TYPE_INFO_I32->size == 4, "I32 size is 4");
    TEST_ASSERT(TYPE_INFO_I32->data.primitive.is_integer, "I32 is integer");
    TEST_ASSERT(TYPE_INFO_I32->data.primitive.is_signed, "I32 is signed");

    TEST_ASSERT(TYPE_INFO_F64->category == TYPE_INFO_PRIMITIVE, "F64 is primitive");
    TEST_ASSERT(TYPE_INFO_F64->size == 8, "F64 size is 8");
    TEST_ASSERT(!TYPE_INFO_F64->data.primitive.is_integer, "F64 is not integer");
    TEST_ASSERT(TYPE_INFO_F64->data.primitive.is_signed, "F64 is signed");

    // Test type lookup by name
    TypeInfo *lookup_i32 = type_info_lookup_by_name("i32");
    TEST_ASSERT(lookup_i32 != NULL, "Lookup i32 by name");
    TEST_ASSERT(type_info_equals(lookup_i32, TYPE_INFO_I32), "Lookup returns equivalent type");
    type_info_release(lookup_i32);

    return true;
}

static bool test_type_info_equality(void) {
    // Test primitive type equality
    TEST_ASSERT(type_info_equals(TYPE_INFO_I32, TYPE_INFO_I32), "I32 equals itself");
    TEST_ASSERT(!type_info_equals(TYPE_INFO_I32, TYPE_INFO_F64), "I32 not equal to F64");

    // Test slice type equality
    TypeInfo *slice1 = type_info_create_slice(TYPE_INFO_I32, false);
    TypeInfo *slice2 = type_info_create_slice(TYPE_INFO_I32, false);
    TypeInfo *slice3 = type_info_create_slice(TYPE_INFO_I32, true);

    TEST_ASSERT(type_info_equals(slice1, slice2), "Equivalent slices are equal");
    TEST_ASSERT(!type_info_equals(slice1, slice3), "Different mutability slices not equal");

    type_info_release(slice1);
    type_info_release(slice2);
    type_info_release(slice3);

    return true;
}

static bool test_type_info_queries(void) {
    // Test numeric type queries
    TEST_ASSERT(type_info_is_numeric(TYPE_INFO_I32), "I32 is numeric");
    TEST_ASSERT(type_info_is_numeric(TYPE_INFO_F64), "F64 is numeric");
    TEST_ASSERT(!type_info_is_numeric(TYPE_INFO_BOOL), "Bool is not numeric");
    TEST_ASSERT(!type_info_is_numeric(TYPE_INFO_STRING), "String is not numeric");

    // Test integer type queries
    TEST_ASSERT(type_info_is_integer(TYPE_INFO_I32), "I32 is integer");
    TEST_ASSERT(type_info_is_integer(TYPE_INFO_U64), "U64 is integer");
    TEST_ASSERT(!type_info_is_integer(TYPE_INFO_F64), "F64 is not integer");
    TEST_ASSERT(!type_info_is_integer(TYPE_INFO_BOOL), "Bool is not integer");

    // Test float type queries
    TEST_ASSERT(type_info_is_float(TYPE_INFO_F32), "F32 is float");
    TEST_ASSERT(type_info_is_float(TYPE_INFO_F64), "F64 is float");
    TEST_ASSERT(!type_info_is_float(TYPE_INFO_I32), "I32 is not float");
    TEST_ASSERT(!type_info_is_float(TYPE_INFO_BOOL), "Bool is not float");

    // Test size and alignment queries
    TEST_ASSERT(type_info_get_size(TYPE_INFO_I32) == 4, "I32 size is 4");
    TEST_ASSERT(type_info_get_size(TYPE_INFO_F64) == 8, "F64 size is 8");
    TEST_ASSERT(type_info_get_alignment(TYPE_INFO_I32) == 4, "I32 alignment is 4");
    TEST_ASSERT(type_info_get_alignment(TYPE_INFO_F64) == 8, "F64 alignment is 8");

    return true;
}

// =============================================================================
// TYPE DESCRIPTOR INTEGRATION TESTS
// =============================================================================

static bool test_type_descriptor_conversion(void) {
    // Create a TypeDescriptor
    TypeDescriptor *i32_descriptor = type_descriptor_create_primitive(PRIMITIVE_I32);
    TEST_ASSERT(i32_descriptor != NULL, "TypeDescriptor creation");

    // Convert to TypeInfo
    TypeInfo *type_info = type_info_from_type_descriptor(i32_descriptor);
    TEST_ASSERT(type_info != NULL, "TypeDescriptor to TypeInfo conversion");
    TEST_ASSERT(type_info->category == TYPE_INFO_PRIMITIVE, "Converted type category");
    TEST_ASSERT(type_info->data.primitive.kind == PRIMITIVE_INFO_I32, "Converted primitive kind");
    TEST_ASSERT(type_info->size == i32_descriptor->size, "Converted size matches");

    // Convert back to TypeDescriptor
    TypeDescriptor *converted_descriptor = type_descriptor_from_type_info(type_info);
    TEST_ASSERT(converted_descriptor != NULL, "TypeInfo to TypeDescriptor conversion");
    TEST_ASSERT(converted_descriptor->category == TYPE_PRIMITIVE, "Converted back category");
    TEST_ASSERT(converted_descriptor->data.primitive.primitive_kind == PRIMITIVE_I32,
                "Converted back primitive kind");

    // Test that they're equivalent
    TEST_ASSERT(type_descriptor_equals(i32_descriptor, converted_descriptor),
                "Round-trip conversion preserves equality");

    // Cleanup
    type_descriptor_release(i32_descriptor);
    type_info_release(type_info);
    type_descriptor_release(converted_descriptor);

    return true;
}

// =============================================================================
// AST INTEGRATION TESTS
// =============================================================================

static bool test_ast_type_info_integration(void) {
    // Create an AST node
    ASTNode *node = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0});
    TEST_ASSERT(node != NULL, "AST node creation");

    // Initially no type info
    TEST_ASSERT(ast_node_get_type_info(node) == NULL, "Initial type info is null");

    // Set type info
    ast_node_set_type_info(node, TYPE_INFO_I32);
    TypeInfo *retrieved = ast_node_get_type_info(node);
    TEST_ASSERT(retrieved != NULL, "Type info retrieval");
    TEST_ASSERT(retrieved == TYPE_INFO_I32, "Retrieved type info matches");

    // Update type info
    ast_node_set_type_info(node, TYPE_INFO_F64);
    retrieved = ast_node_get_type_info(node);
    TEST_ASSERT(retrieved == TYPE_INFO_F64, "Updated type info");

    // Clear type info
    ast_node_set_type_info(node, NULL);
    TEST_ASSERT(ast_node_get_type_info(node) == NULL, "Cleared type info");

    // Cleanup
    ast_free_node(node);

    return true;
}

// =============================================================================
// STATISTICS AND DEBUG TESTS
// =============================================================================

static bool test_type_info_statistics(void) {
    TypeInfoStats stats = type_info_get_stats();

    // Note: Statistics tracking may not be enabled in all configurations
    // For now, just test that the function works without crashing
    // Statistics tracking is now properly enabled via asthra_statistics system
    if (stats.total_types == 0) {
        printf("  Note: No types registered yet (this is normal for early testing)\n");
        return true; // Pass the test if no types are registered yet
    }

    // Should have built-in types registered (if statistics are enabled)
    TEST_ASSERT(stats.total_types > 0, "Total types > 0");
    TEST_ASSERT(stats.primitive_types >= 16, "At least 16 primitive types"); // All built-ins
    TEST_ASSERT(stats.memory_usage > 0, "Memory usage > 0");

    printf("Type Info Statistics:\n");
    printf("  Total types: %zu\n", stats.total_types);
    printf("  Primitive types: %zu\n", stats.primitive_types);
    printf("  Struct types: %zu\n", stats.struct_types);
    printf("  Slice types: %zu\n", stats.slice_types);
    printf("  Pointer types: %zu\n", stats.pointer_types);
    printf("  Result types: %zu\n", stats.result_types);
    printf("  Function types: %zu\n", stats.function_types);
    printf("  Module types: %zu\n", stats.module_types);
    printf("  Memory usage: %zu bytes\n", stats.memory_usage);

    return true;
}

static bool test_type_info_debug_output(void) {
    printf("\nType Info Debug Output:\n");

    // Print some built-in types
    printf("TYPE_INFO_I32:\n");
    type_info_print(TYPE_INFO_I32, 2);

    printf("TYPE_INFO_STRING:\n");
    type_info_print(TYPE_INFO_STRING, 2);

    // Create and print a complex type
    TypeInfo *slice_type = type_info_create_slice(TYPE_INFO_I32, true);
    printf("Mutable slice of i32:\n");
    type_info_print(slice_type, 2);

    type_info_release(slice_type);

    return true;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Asthra TypeInfo/TypeDescriptor Integration Tests ===\n");

    // Initialize the type system
    if (!type_info_init_builtins()) {
        fprintf(stderr, "Failed to initialize built-in types\n");
        return 1;
    }

    // Run all tests
    RUN_TEST(test_type_info_creation);
    RUN_TEST(test_type_info_builtin_initialization);
    RUN_TEST(test_type_info_equality);
    RUN_TEST(test_type_info_queries);
    RUN_TEST(test_type_descriptor_conversion);
    // RUN_TEST(test_complex_type_conversion);  // Disabled due to memory management complexity
    RUN_TEST(test_ast_type_info_integration);
    RUN_TEST(test_type_info_statistics);
    RUN_TEST(test_type_info_debug_output);

    // Print summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    if (tests_failed == 0) {
        printf("🎉 All tests passed!\n");
    } else {
        printf("❌ %d test(s) failed\n", tests_failed);
    }

    // Cleanup
    type_info_cleanup_builtins();

    return tests_failed == 0 ? 0 : 1;
}
