/**
 * LLVM Types Module Tests (Simplified)
 * Basic tests for the Asthra to LLVM type conversion functions
 */

#include "../../../src/analysis/type_info.h"
#include "../../../src/codegen/llvm_backend_internal.h"
#include "../../../src/codegen/llvm_types.h"
#include <llvm-c/Core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple test framework macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("FAILED: %s\n", message);                                                       \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_EQ(actual, expected, message)                                                  \
    do {                                                                                           \
        if ((actual) != (expected)) {                                                              \
            printf("FAILED: %s (expected %d, got %d)\n", message, (int)(expected), (int)(actual)); \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr, message)                                                         \
    do {                                                                                           \
        if ((ptr) == NULL) {                                                                       \
            printf("FAILED: %s (pointer is NULL)\n", message);                                     \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

#define RUN_TEST(test_name)                                                                        \
    do {                                                                                           \
        printf("Running %s... ", #test_name);                                                      \
        fflush(stdout);                                                                            \
        if (test_name() == 0) {                                                                    \
            printf("PASSED\n");                                                                    \
            passed++;                                                                              \
        } else {                                                                                   \
            printf("FAILED\n");                                                                    \
            failed++;                                                                              \
        }                                                                                          \
        total++;                                                                                   \
    } while (0)

// Global test fixture
static LLVMBackendData *g_backend_data = NULL;

// Setup function - simplified
static int setup_test_environment(void) {
    g_backend_data = calloc(1, sizeof(LLVMBackendData));
    TEST_ASSERT_NOT_NULL(g_backend_data, "Failed to allocate backend data");

    g_backend_data->context = LLVMContextCreate();
    TEST_ASSERT_NOT_NULL(g_backend_data->context, "Failed to create LLVM context");

    g_backend_data->module =
        LLVMModuleCreateWithNameInContext("test_module", g_backend_data->context);
    TEST_ASSERT_NOT_NULL(g_backend_data->module, "Failed to create LLVM module");

    g_backend_data->builder = LLVMCreateBuilderInContext(g_backend_data->context);
    TEST_ASSERT_NOT_NULL(g_backend_data->builder, "Failed to create LLVM builder");

    // Initialize type cache
    llvm_types_cache_init(g_backend_data);

    return 0;
}

// Teardown function
static void teardown_test_environment(void) {
    if (g_backend_data) {
        if (g_backend_data->builder) {
            LLVMDisposeBuilder(g_backend_data->builder);
        }
        if (g_backend_data->module) {
            LLVMDisposeModule(g_backend_data->module);
        }
        if (g_backend_data->context) {
            LLVMContextDispose(g_backend_data->context);
        }
        free(g_backend_data);
        g_backend_data = NULL;
    }
}

// Test basic type cache initialization
static int test_type_cache_init(void) {
    TEST_ASSERT_NOT_NULL(g_backend_data->i32_type, "i32 type should be cached");
    TEST_ASSERT_NOT_NULL(g_backend_data->i64_type, "i64 type should be cached");
    TEST_ASSERT_NOT_NULL(g_backend_data->f32_type, "f32 type should be cached");
    TEST_ASSERT_NOT_NULL(g_backend_data->f64_type, "f64 type should be cached");
    TEST_ASSERT_NOT_NULL(g_backend_data->bool_type, "bool type should be cached");
    TEST_ASSERT_NOT_NULL(g_backend_data->void_type, "void type should be cached");
    TEST_ASSERT_NOT_NULL(g_backend_data->ptr_type, "ptr type should be cached");
    TEST_ASSERT_NOT_NULL(g_backend_data->unit_type, "unit type should be cached");

    // Verify cached types are correct
    TEST_ASSERT_EQ(LLVMGetTypeKind(g_backend_data->i32_type), LLVMIntegerTypeKind,
                   "i32 cache should be integer");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(g_backend_data->i32_type), 32,
                   "i32 cache should be 32 bits");

    TEST_ASSERT_EQ(LLVMGetTypeKind(g_backend_data->bool_type), LLVMIntegerTypeKind,
                   "bool cache should be integer");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(g_backend_data->bool_type), 1, "bool cache should be 1 bit");

    TEST_ASSERT_EQ(LLVMGetTypeKind(g_backend_data->f32_type), LLVMFloatTypeKind,
                   "f32 cache should be float");
    TEST_ASSERT_EQ(LLVMGetTypeKind(g_backend_data->f64_type), LLVMDoubleTypeKind,
                   "f64 cache should be double");
    TEST_ASSERT_EQ(LLVMGetTypeKind(g_backend_data->void_type), LLVMVoidTypeKind,
                   "void cache should be void");
    TEST_ASSERT_EQ(LLVMGetTypeKind(g_backend_data->ptr_type), LLVMPointerTypeKind,
                   "ptr cache should be pointer");
    TEST_ASSERT_EQ(LLVMGetTypeKind(g_backend_data->unit_type), LLVMStructTypeKind,
                   "unit cache should be struct");

    return 0;
}

// Test primitive type conversion with predefined types
static int test_primitive_type_conversion(void) {
    // Create a simple i32 TypeInfo
    TypeInfo i32_type = {0};
    i32_type.category = TYPE_INFO_PRIMITIVE;
    i32_type.data.primitive.kind = PRIMITIVE_INFO_I32;

    LLVMTypeRef llvm_i32 = asthra_type_to_llvm(g_backend_data, &i32_type);
    TEST_ASSERT_NOT_NULL(llvm_i32, "i32 conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_i32), LLVMIntegerTypeKind,
                   "i32 should convert to integer type");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(llvm_i32), 32, "i32 should be 32 bits");

    // Test i64
    TypeInfo i64_type = {0};
    i64_type.category = TYPE_INFO_PRIMITIVE;
    i64_type.data.primitive.kind = PRIMITIVE_INFO_I64;

    LLVMTypeRef llvm_i64 = asthra_type_to_llvm(g_backend_data, &i64_type);
    TEST_ASSERT_NOT_NULL(llvm_i64, "i64 conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_i64), LLVMIntegerTypeKind,
                   "i64 should convert to integer type");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(llvm_i64), 64, "i64 should be 64 bits");

    // Test bool
    TypeInfo bool_type = {0};
    bool_type.category = TYPE_INFO_PRIMITIVE;
    bool_type.data.primitive.kind = PRIMITIVE_INFO_BOOL;

    LLVMTypeRef llvm_bool = asthra_type_to_llvm(g_backend_data, &bool_type);
    TEST_ASSERT_NOT_NULL(llvm_bool, "bool conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_bool), LLVMIntegerTypeKind,
                   "bool should convert to integer type");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(llvm_bool), 1, "bool should be 1 bit");

    // Test f32
    TypeInfo f32_type = {0};
    f32_type.category = TYPE_INFO_PRIMITIVE;
    f32_type.data.primitive.kind = PRIMITIVE_INFO_F32;

    LLVMTypeRef llvm_f32 = asthra_type_to_llvm(g_backend_data, &f32_type);
    TEST_ASSERT_NOT_NULL(llvm_f32, "f32 conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_f32), LLVMFloatTypeKind,
                   "f32 should convert to float type");

    // Test f64
    TypeInfo f64_type = {0};
    f64_type.category = TYPE_INFO_PRIMITIVE;
    f64_type.data.primitive.kind = PRIMITIVE_INFO_F64;

    LLVMTypeRef llvm_f64 = asthra_type_to_llvm(g_backend_data, &f64_type);
    TEST_ASSERT_NOT_NULL(llvm_f64, "f64 conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_f64), LLVMDoubleTypeKind,
                   "f64 should convert to double type");

    // Test void
    TypeInfo void_type = {0};
    void_type.category = TYPE_INFO_PRIMITIVE;
    void_type.data.primitive.kind = PRIMITIVE_INFO_VOID;

    LLVMTypeRef llvm_void = asthra_type_to_llvm(g_backend_data, &void_type);
    TEST_ASSERT_NOT_NULL(llvm_void, "void conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_void), LLVMVoidTypeKind,
                   "void should convert to void type");

    // Test never
    TypeInfo never_type = {0};
    never_type.category = TYPE_INFO_PRIMITIVE;
    never_type.data.primitive.kind = PRIMITIVE_INFO_NEVER;

    LLVMTypeRef llvm_never = asthra_type_to_llvm(g_backend_data, &never_type);
    TEST_ASSERT_NOT_NULL(llvm_never, "never conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_never), LLVMVoidTypeKind,
                   "never should convert to void type");

    return 0;
}

// Test slice type conversion
static int test_slice_type_conversion(void) {
    // Create an element type (i32)
    TypeInfo elem_type = {0};
    elem_type.category = TYPE_INFO_PRIMITIVE;
    elem_type.data.primitive.kind = PRIMITIVE_INFO_I32;

    // Create a slice type
    TypeInfo slice_type = {0};
    slice_type.category = TYPE_INFO_SLICE;
    slice_type.data.slice.element_type = &elem_type;
    slice_type.data.slice.is_mutable = false;

    LLVMTypeRef llvm_slice = asthra_type_to_llvm(g_backend_data, &slice_type);
    TEST_ASSERT_NOT_NULL(llvm_slice, "slice conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_slice), LLVMStructTypeKind,
                   "slice should convert to struct type");

    // Slice should have 2 fields: pointer and length
    TEST_ASSERT_EQ(LLVMCountStructElementTypes(llvm_slice), 2, "slice should have 2 fields");

    // First field should be a pointer
    LLVMTypeRef field0 = LLVMStructGetTypeAtIndex(llvm_slice, 0);
    TEST_ASSERT_EQ(LLVMGetTypeKind(field0), LLVMPointerTypeKind,
                   "slice first field should be pointer");

    // Second field should be i64 (length)
    LLVMTypeRef field1 = LLVMStructGetTypeAtIndex(llvm_slice, 1);
    TEST_ASSERT_EQ(LLVMGetTypeKind(field1), LLVMIntegerTypeKind,
                   "slice second field should be integer");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(field1), 64, "slice length should be 64 bits");

    return 0;
}

// Test Option<T> type conversion
static int test_option_type_conversion(void) {
    // Create a value type (i32)
    TypeInfo value_type = {0};
    value_type.category = TYPE_INFO_PRIMITIVE;
    value_type.data.primitive.kind = PRIMITIVE_INFO_I32;

    // Create an Option type
    TypeInfo option_type = {0};
    option_type.category = TYPE_INFO_OPTION;
    option_type.data.option.value_type = &value_type;

    LLVMTypeRef llvm_option = asthra_type_to_llvm(g_backend_data, &option_type);
    TEST_ASSERT_NOT_NULL(llvm_option, "option conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_option), LLVMStructTypeKind,
                   "option should convert to struct type");

    // Option should have 2 fields: bool present, T value
    TEST_ASSERT_EQ(LLVMCountStructElementTypes(llvm_option), 2, "option should have 2 fields");

    // First field should be bool (i1)
    LLVMTypeRef field0 = LLVMStructGetTypeAtIndex(llvm_option, 0);
    TEST_ASSERT_EQ(LLVMGetTypeKind(field0), LLVMIntegerTypeKind,
                   "option first field should be integer");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(field0), 1, "option present flag should be 1 bit");

    // Second field should be i32
    LLVMTypeRef field1 = LLVMStructGetTypeAtIndex(llvm_option, 1);
    TEST_ASSERT_EQ(LLVMGetTypeKind(field1), LLVMIntegerTypeKind,
                   "option second field should be integer");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(field1), 32, "option value should be 32 bits");

    return 0;
}

// Test tuple type conversion
static int test_tuple_type_conversion(void) {
    // Create element types
    TypeInfo elem0 = {0};
    elem0.category = TYPE_INFO_PRIMITIVE;
    elem0.data.primitive.kind = PRIMITIVE_INFO_I32;

    TypeInfo elem1 = {0};
    elem1.category = TYPE_INFO_PRIMITIVE;
    elem1.data.primitive.kind = PRIMITIVE_INFO_F64;

    TypeInfo *elements[2] = {&elem0, &elem1};

    // Create a tuple type
    TypeInfo tuple_type = {0};
    tuple_type.category = TYPE_INFO_TUPLE;
    tuple_type.data.tuple.element_types = elements;
    tuple_type.data.tuple.element_count = 2;

    LLVMTypeRef llvm_tuple = asthra_type_to_llvm(g_backend_data, &tuple_type);
    TEST_ASSERT_NOT_NULL(llvm_tuple, "tuple conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_tuple), LLVMStructTypeKind,
                   "tuple should convert to struct type");

    // Tuple should have 2 elements
    TEST_ASSERT_EQ(LLVMCountStructElementTypes(llvm_tuple), 2, "tuple should have 2 elements");

    // First element should be i32
    LLVMTypeRef elem_type0 = LLVMStructGetTypeAtIndex(llvm_tuple, 0);
    TEST_ASSERT_EQ(LLVMGetTypeKind(elem_type0), LLVMIntegerTypeKind,
                   "tuple first element should be integer");
    TEST_ASSERT_EQ(LLVMGetIntTypeWidth(elem_type0), 32, "tuple first element should be 32 bits");

    // Second element should be f64
    LLVMTypeRef elem_type1 = LLVMStructGetTypeAtIndex(llvm_tuple, 1);
    TEST_ASSERT_EQ(LLVMGetTypeKind(elem_type1), LLVMDoubleTypeKind,
                   "tuple second element should be double");

    return 0;
}

// Test empty tuple (unit type)
static int test_unit_type_conversion(void) {
    // Create an empty tuple (unit type)
    TypeInfo unit_type = {0};
    unit_type.category = TYPE_INFO_TUPLE;
    unit_type.data.tuple.element_types = NULL;
    unit_type.data.tuple.element_count = 0;

    LLVMTypeRef llvm_unit = asthra_type_to_llvm(g_backend_data, &unit_type);
    TEST_ASSERT_NOT_NULL(llvm_unit, "unit conversion should not return NULL");
    TEST_ASSERT_EQ(LLVMGetTypeKind(llvm_unit), LLVMStructTypeKind,
                   "unit should convert to struct type");

    // Should have 0 elements
    TEST_ASSERT_EQ(LLVMCountStructElementTypes(llvm_unit), 0, "unit should have 0 elements");

    // Should be the same as the cached unit type
    TEST_ASSERT(llvm_unit == g_backend_data->unit_type, "unit type should match cached type");

    return 0;
}

// Main test runner
int main(int argc, char **argv) {
    int total = 0, passed = 0, failed = 0;

    printf("Running LLVM Types Module Tests (Simplified)...\n\n");

    // Setup test environment
    if (setup_test_environment() != 0) {
        printf("FAILED: Could not setup test environment\n");
        return 1;
    }

    // Run tests
    RUN_TEST(test_type_cache_init);
    RUN_TEST(test_primitive_type_conversion);
    RUN_TEST(test_slice_type_conversion);
    RUN_TEST(test_option_type_conversion);
    RUN_TEST(test_tuple_type_conversion);
    RUN_TEST(test_unit_type_conversion);

    // Cleanup
    teardown_test_environment();

    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total: %d\n", total);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed > 0) {
        printf("Overall result: FAILED\n");
        return 1;
    } else {
        printf("Overall result: PASSED\n");
        return 0;
    }
}