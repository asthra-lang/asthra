/**
 * Asthra Programming Language Runtime v1.2
 * Result Type and Pattern Matching Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// RESULT TYPE AND PATTERN MATCHING TESTS
// =============================================================================

static void ok_handler(void *data, void *context) {
    int *result = (int *)context;
    int *value = (int *)data;
    *result = *value;
}

static void err_handler(void *data, void *context) {
    int *result = (int *)context;
    *result = -1;
    (void)data; // Unused
}

int test_result_pattern_matching(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");
    
    // Test Ok result
    int ok_value = 42;
    AsthraResult ok_result = asthra_result_ok(&ok_value, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_C);
    TEST_ASSERT(asthra_result_is_ok(ok_result), "Ok result should be Ok");
    TEST_ASSERT(!asthra_result_is_err(ok_result), "Ok result should not be Err");
    
    int *unwrapped_ok = (int *)asthra_result_unwrap_ok(ok_result);
    TEST_ASSERT(unwrapped_ok != NULL, "Unwrapping Ok result failed");
    TEST_ASSERT(*unwrapped_ok == 42, "Unwrapped Ok value incorrect");
    
    // Test Err result
    char *err_msg = "Test error";
    AsthraResult err_result = asthra_result_err(err_msg, strlen(err_msg) + 1, ASTHRA_TYPE_STRING, ASTHRA_OWNERSHIP_C);
    TEST_ASSERT(asthra_result_is_err(err_result), "Err result should be Err");
    TEST_ASSERT(!asthra_result_is_ok(err_result), "Err result should not be Ok");
    
    char *unwrapped_err = (char *)asthra_result_unwrap_err(err_result);
    TEST_ASSERT(unwrapped_err != NULL, "Unwrapping Err result failed");
    TEST_ASSERT(strcmp(unwrapped_err, "Test error") == 0, "Unwrapped Err value incorrect");
    
    // Test pattern matching
    int match_result = 0;
    AsthraMatchArm arms[] = {
        {.pattern = ASTHRA_MATCH_OK, .expected_type_id = ASTHRA_TYPE_I32, .handler = ok_handler, .context = &match_result},
        {.pattern = ASTHRA_MATCH_ERR, .expected_type_id = 0, .handler = err_handler, .context = &match_result}
    };
    
    int matched_arm = asthra_result_match(ok_result, arms, 2);
    TEST_ASSERT(matched_arm == 0, "Pattern matching should match first arm");
    TEST_ASSERT(match_result == 42, "Pattern matching handler should set correct value");
    
    asthra_runtime_cleanup();
    TEST_PASS("Result type and pattern matching");
}

// Test function declarations for external use
int test_result_pattern_matching(void); 
