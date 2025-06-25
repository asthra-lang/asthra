/**
 * Test file for runtime enum support functionality
 * Tests enum variant construction, Result/Option operations, and pattern matching
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include enum support
#include "../../runtime/asthra_enum_support.h"

// Test framework helpers
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("FAIL: %s (line %d)\n", message, __LINE__);                                     \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST_SUCCESS()                                                                             \
    do {                                                                                           \
        printf("PASS\n");                                                                          \
        return true;                                                                               \
    } while (0)

// =============================================================================
// BASIC ENUM OPERATIONS TESTS
// =============================================================================

static bool test_enum_variant_creation(void) {
    printf("Testing enum variant creation... ");

    // Test creating variant with small inline data
    int32_t value = 42;
    AsthraEnumVariant variant =
        Asthra_enum_create_variant(0, &value, sizeof(value), 1, ASTHRA_OWNERSHIP_GC);

    TEST_ASSERT(variant.tag == 0, "Tag should be 0");
    TEST_ASSERT(variant.value_size == sizeof(int32_t), "Value size should match");
    TEST_ASSERT(variant.value_type_id == 1, "Type ID should be 1");

    // Get data back
    void *data = Asthra_enum_get_data(&variant);
    TEST_ASSERT(data != NULL, "Data should not be NULL");
    TEST_ASSERT(*(int32_t *)data == 42, "Data value should be 42");

    TEST_SUCCESS();
}

static bool test_enum_variant_large_data(void) {
    printf("Testing enum variant with large data... ");

    // Test creating variant with large data that requires pointer storage
    char large_data[256];
    memset(large_data, 'A', sizeof(large_data));
    large_data[255] = '\0';

    AsthraEnumVariant variant =
        Asthra_enum_create_variant(1, large_data, sizeof(large_data), 2, ASTHRA_OWNERSHIP_GC);

    TEST_ASSERT(variant.tag == 1, "Tag should be 1");
    TEST_ASSERT(variant.value_size == sizeof(large_data), "Value size should match");
    TEST_ASSERT(variant.data.value_ptr != NULL, "Should use pointer storage for large data");

    // Get data back
    void *data = Asthra_enum_get_data(&variant);
    TEST_ASSERT(data != NULL, "Data should not be NULL");
    TEST_ASSERT(memcmp(data, large_data, sizeof(large_data)) == 0, "Data should match original");

    // Free the variant
    Asthra_enum_free_variant(&variant);

    TEST_SUCCESS();
}

static bool test_enum_tag_checking(void) {
    printf("Testing enum tag checking... ");

    int32_t value = 100;
    AsthraEnumVariant variant =
        Asthra_enum_create_variant(5, &value, sizeof(value), 1, ASTHRA_OWNERSHIP_GC);

    TEST_ASSERT(Asthra_enum_is_variant(&variant, 5), "Should match tag 5");
    TEST_ASSERT(!Asthra_enum_is_variant(&variant, 4), "Should not match tag 4");
    TEST_ASSERT(Asthra_enum_get_tag(&variant) == 5, "Get tag should return 5");

    TEST_SUCCESS();
}

// =============================================================================
// RESULT<T,E> OPERATIONS TESTS
// =============================================================================

static bool test_result_ok_operations(void) {
    printf("Testing Result.Ok operations... ");

    int32_t success_value = 42;
    AsthraEnumVariant result =
        Asthra_result_create_ok(&success_value, sizeof(success_value), 1, ASTHRA_OWNERSHIP_GC);

    TEST_ASSERT(Asthra_result_is_ok(&result), "Should be Ok variant");
    TEST_ASSERT(!Asthra_result_is_err(&result), "Should not be Err variant");
    TEST_ASSERT(result.tag == ASTHRA_RESULT_TAG_OK, "Tag should be RESULT_TAG_OK");

    // Unwrap Ok value
    void *ok_data = Asthra_result_unwrap_ok(&result);
    TEST_ASSERT(ok_data != NULL, "Ok data should not be NULL");
    TEST_ASSERT(*(int32_t *)ok_data == 42, "Ok value should be 42");

    TEST_SUCCESS();
}

static bool test_result_err_operations(void) {
    printf("Testing Result.Err operations... ");

    char error_msg[] = "File not found";
    AsthraEnumVariant err_variant =
        Asthra_result_create_err(error_msg, strlen(error_msg) + 1, 2, ASTHRA_OWNERSHIP_GC);

    TEST_ASSERT(!Asthra_result_is_ok(&err_variant), "Should not be Ok variant");
    TEST_ASSERT(Asthra_result_is_err(&err_variant), "Should be Err variant");
    TEST_ASSERT(err_variant.tag == ASTHRA_RESULT_TAG_ERR, "Tag should be RESULT_TAG_ERR");

    // Unwrap Err value
    void *err_data = Asthra_result_unwrap_err(&err_variant);
    TEST_ASSERT(err_data != NULL, "Err data should not be NULL");
    TEST_ASSERT(strcmp((char *)err_data, error_msg) == 0, "Err message should match");

    TEST_SUCCESS();
}

// =============================================================================
// OPTION<T> OPERATIONS TESTS
// =============================================================================

static bool test_option_some_operations(void) {
    printf("Testing Option.Some operations... ");

    double some_value = 3.14159;
    AsthraEnumVariant option =
        Asthra_option_create_some(&some_value, sizeof(some_value), 3, ASTHRA_OWNERSHIP_GC);

    TEST_ASSERT(Asthra_option_is_some(&option), "Should be Some variant");
    TEST_ASSERT(!Asthra_option_is_none(&option), "Should not be None variant");
    TEST_ASSERT(option.tag == ASTHRA_OPTION_TAG_SOME, "Tag should be OPTION_TAG_SOME");

    // Unwrap Some value
    void *some_data = Asthra_option_unwrap(&option);
    TEST_ASSERT(some_data != NULL, "Some data should not be NULL");
    TEST_ASSERT(*(double *)some_data == some_value, "Some value should match");

    TEST_SUCCESS();
}

static bool test_option_none_operations(void) {
    printf("Testing Option.None operations... ");

    AsthraEnumVariant option = Asthra_option_create_none();

    TEST_ASSERT(!Asthra_option_is_some(&option), "Should not be Some variant");
    TEST_ASSERT(Asthra_option_is_none(&option), "Should be None variant");
    TEST_ASSERT(option.tag == ASTHRA_OPTION_TAG_NONE, "Tag should be OPTION_TAG_NONE");
    TEST_ASSERT(option.value_size == 0, "None should have no data");

    TEST_SUCCESS();
}

// =============================================================================
// PATTERN MATCHING TESTS
// =============================================================================

static int ok_callback(AsthraEnumVariant *variant, void *user_data) {
    int *result = (int *)user_data;
    *result = 100; // Success indicator
    return 0;
}

static int err_callback(AsthraEnumVariant *variant, void *user_data) {
    int *result = (int *)user_data;
    *result = -1; // Error indicator
    return 0;
}

static int wildcard_callback(AsthraEnumVariant *variant, void *user_data) {
    int *result = (int *)user_data;
    *result = 999; // Wildcard indicator
    return 0;
}

static bool test_pattern_matching(void) {
    printf("Testing pattern matching... ");

    // Create a Result.Ok variant
    int32_t value = 42;
    AsthraEnumVariant result =
        Asthra_result_create_ok(&value, sizeof(value), 1, ASTHRA_OWNERSHIP_GC);

    // Set up pattern matching arms
    int match_result = 0;
    AsthraEnumMatchArm arms[] = {{ASTHRA_RESULT_TAG_OK, ok_callback, &match_result},
                                 {ASTHRA_RESULT_TAG_ERR, err_callback, &match_result}};

    // Execute pattern matching
    int pattern_result = Asthra_enum_pattern_match(&result, arms, 2);

    TEST_ASSERT(pattern_result == 0, "Pattern match should succeed");
    TEST_ASSERT(match_result == 100, "Ok callback should have been called");

    TEST_SUCCESS();
}

static bool test_pattern_matching_wildcard(void) {
    printf("Testing pattern matching with wildcard... ");

    // Create an enum variant with unknown tag
    int32_t value = 42;
    AsthraEnumVariant variant =
        Asthra_enum_create_variant(999, &value, sizeof(value), 1, ASTHRA_OWNERSHIP_GC);

    // Set up pattern matching arms with wildcard
    int match_result = 0;
    AsthraEnumMatchArm arms[] = {{ASTHRA_RESULT_TAG_OK, ok_callback, &match_result},
                                 {ASTHRA_RESULT_TAG_ERR, err_callback, &match_result},
                                 {ASTHRA_ENUM_TAG_WILDCARD, wildcard_callback, &match_result}};

    // Execute pattern matching
    int pattern_result = Asthra_enum_pattern_match(&variant, arms, 3);

    TEST_ASSERT(pattern_result == 0, "Pattern match should succeed");
    TEST_ASSERT(match_result == 999, "Wildcard callback should have been called");

    TEST_SUCCESS();
}

// =============================================================================
// ERROR HANDLING TESTS
// =============================================================================

static bool test_error_handling(void) {
    printf("Testing error handling... ");

    // Test NULL pointer handling
    TEST_ASSERT(!Asthra_enum_is_variant(NULL, 0), "NULL enum should return false");
    TEST_ASSERT(Asthra_enum_get_data(NULL) == NULL, "NULL enum should return NULL data");
    TEST_ASSERT(Asthra_enum_get_tag(NULL) == UINT32_MAX, "NULL enum should return invalid tag");

    // Test type safety
    int32_t value = 42;
    AsthraEnumVariant err_result =
        Asthra_result_create_err(&value, sizeof(value), 1, ASTHRA_OWNERSHIP_GC);

    // Trying to unwrap Ok from Err should fail gracefully
    void *data = Asthra_result_unwrap_ok(&err_result);
    TEST_ASSERT(data == NULL, "Should not be able to unwrap Ok from Err");

    TEST_SUCCESS();
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

static bool test_result_option_integration(void) {
    printf("Testing Result<Option<T>, E> integration... ");

    // Create Option.Some(42)
    int32_t some_value = 42;
    AsthraEnumVariant option_some =
        Asthra_option_create_some(&some_value, sizeof(some_value), 1, ASTHRA_OWNERSHIP_GC);

    // Create Result.Ok(Option.Some(42))
    AsthraEnumVariant result_ok =
        Asthra_result_create_ok(&option_some, sizeof(option_some), 4, ASTHRA_OWNERSHIP_GC);

    TEST_ASSERT(Asthra_result_is_ok(&result_ok), "Should be Ok variant");

    // Unwrap the Result to get the Option
    AsthraEnumVariant *inner_option = (AsthraEnumVariant *)Asthra_result_unwrap_ok(&result_ok);
    TEST_ASSERT(inner_option != NULL, "Inner option should not be NULL");
    TEST_ASSERT(Asthra_option_is_some(inner_option), "Inner option should be Some");

    // Unwrap the Option to get the final value
    int32_t *final_value = (int32_t *)Asthra_option_unwrap(inner_option);
    TEST_ASSERT(final_value != NULL, "Final value should not be NULL");
    TEST_ASSERT(*final_value == 42, "Final value should be 42");

    TEST_SUCCESS();
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Running Runtime Enum Support Tests...\n");
    printf("=====================================\n\n");

    bool all_tests_passed = true;

    // Basic enum operations tests
    if (!test_enum_variant_creation())
        all_tests_passed = false;
    if (!test_enum_variant_large_data())
        all_tests_passed = false;
    if (!test_enum_tag_checking())
        all_tests_passed = false;

    // Result operations tests
    if (!test_result_ok_operations())
        all_tests_passed = false;
    if (!test_result_err_operations())
        all_tests_passed = false;

    // Option operations tests
    if (!test_option_some_operations())
        all_tests_passed = false;
    if (!test_option_none_operations())
        all_tests_passed = false;

    // Pattern matching tests
    if (!test_pattern_matching())
        all_tests_passed = false;
    if (!test_pattern_matching_wildcard())
        all_tests_passed = false;

    // Error handling tests
    if (!test_error_handling())
        all_tests_passed = false;

    // Integration tests
    if (!test_result_option_integration())
        all_tests_passed = false;

    printf("\n=====================================\n");
    if (all_tests_passed) {
        printf("✅ All Runtime Enum Support Tests PASSED!\n");
        printf("\nPhase 3 Runtime Support Implementation: COMPLETE\n");
        printf("- Basic enum operations: ✅ Working\n");
        printf("- Result<T,E> operations: ✅ Working\n");
        printf("- Option<T> operations: ✅ Working\n");
        printf("- Pattern matching: ✅ Working\n");
        printf("- Error handling: ✅ Working\n");
        printf("- Integration scenarios: ✅ Working\n");
        return 0;
    } else {
        printf("❌ Some Runtime Enum Support Tests FAILED!\n");
        return 1;
    }
}
