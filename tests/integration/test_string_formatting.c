/**
 * Asthra Programming Language Compiler
 * String Formatting Integration Test
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "static_analysis.h"
#include "../framework/test_framework.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Helper function for string formatting
static int format_string(char *buffer, size_t buffer_size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    // FIXED: Suppress format warning by using pragma
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    int result = vsnprintf(buffer, buffer_size, format, args);
#ifdef __clang__
    #pragma clang diagnostic pop
#endif
    
    va_end(args);
    return result;
}

// Helper function for string comparison
static bool strings_equal(const char *str1, const char *str2) {
    return strcmp(str1, str2) == 0;
}

AsthraTestResult test_string_formatting(AsthraTestContext *context) {
    char buffer[256];
    
    int result = format_string(buffer, sizeof(buffer), "Hello, %s! Number: %d", "Asthra", 42);
    
    if (!ASTHRA_TEST_ASSERT(context, result > 0, 
                           "String formatting should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, strings_equal(buffer, "Hello, Asthra! Number: 42"), 
                           "Formatted string should match expected output")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test with different format
    result = format_string(buffer, sizeof(buffer), "Value: %.2f", 3.14159);
    
    if (!ASTHRA_TEST_ASSERT(context, result > 0, 
                           "Float formatting should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, strings_equal(buffer, "Value: 3.14"), 
                           "Float formatted string should match expected output")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("=== String Formatting Integration Test ===\n");
    
    AsthraTestContext context = {0};
    
    AsthraTestResult result = test_string_formatting(&context);
    if (result == ASTHRA_TEST_PASS) {
        printf("String formatting test: PASS\n");
        return 0;
    } else {
        printf("String formatting test: FAIL\n");
        return 1;
    }
} 
