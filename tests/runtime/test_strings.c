/**
 * Asthra Programming Language Runtime v1.2
 * String Operations Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// STRING OPERATIONS TESTS
// =============================================================================

int test_string_operations(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");
    
    // Test string creation
    AsthraString str1 = asthra_string_new("Hello, ");
    AsthraString str2 = asthra_string_new("Asthra!");
    TEST_ASSERT(str1.data != NULL, "String creation failed");
    TEST_ASSERT(str2.data != NULL, "String creation failed");
    TEST_ASSERT(asthra_string_len(str1) == 7, "String length incorrect");
    
    // Test string concatenation
    AsthraString concat_result = asthra_string_concat(str1, str2);
    TEST_ASSERT(concat_result.data != NULL, "String concatenation failed");
    TEST_ASSERT(asthra_string_len(concat_result) == 14, "Concatenated string length incorrect");
    
    char *cstr = asthra_string_to_cstr(concat_result, ASTHRA_TRANSFER_NONE);
    TEST_ASSERT(strcmp(cstr, "Hello, Asthra!") == 0, "Concatenated string content incorrect");
    free(cstr);
    
    // Test string equality
    AsthraString str3 = asthra_string_new("Hello, Asthra!");
    TEST_ASSERT(asthra_string_equals(concat_result, str3), "String equality failed");
    
    // Test string cloning
    AsthraString cloned = asthra_string_clone(str3);
    TEST_ASSERT(asthra_string_equals(str3, cloned), "String cloning failed");
    
    // Cleanup
    asthra_string_free(str1);
    asthra_string_free(str2);
    asthra_string_free(concat_result);
    asthra_string_free(str3);
    asthra_string_free(cloned);
    
    asthra_runtime_cleanup();
    TEST_PASS("String operations");
}

// String interpolation test removed - feature deprecated for AI generation efficiency

// Test function declarations for external use
int test_string_operations(void); 
