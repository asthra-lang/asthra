/**
 * Asthra Programming Language v1.2 Integration Tests
 * Pattern Matching with Result Types and FFI
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_integration_common.h"

// =============================================================================
// PATTERN MATCHING WITH FFI INTEGRATION TEST
// =============================================================================

// Scenario 3: Complex pattern matching with Result types and FFI
typedef enum {
    DATA_TYPE_INTEGER,
    DATA_TYPE_STRING,
    DATA_TYPE_ARRAY,
    DATA_TYPE_INVALID
} DataType;

typedef struct {
    DataType type;
    union {
        int int_value;
        char *string_value;
        struct {
            int *array_data;
            size_t array_size;
        } array_value;
    } data;
} VariantData;

typedef struct {
    bool is_ok;
    union {
        VariantData ok_value;
        const char *error_message;
    } result;
} ProcessingResult;

static ProcessingResult create_ok_result(VariantData data) {
    ProcessingResult result;
    result.is_ok = true;
    result.result.ok_value = data;
    return result;
}

static ProcessingResult create_error_result(const char *error) {
    ProcessingResult result;
    result.is_ok = false;
    result.result.error_message = error;
    return result;
}

// Mock C function for data processing
static ProcessingResult c_process_variant_data(VariantData input) {
    switch (input.type) {
        case DATA_TYPE_INTEGER:
            if (input.data.int_value < 0) {
                return create_error_result("Negative integers not supported");
            }
            
            VariantData result_data;
            result_data.type = DATA_TYPE_INTEGER;
            result_data.data.int_value = input.data.int_value * 2;
            return create_ok_result(result_data);
            
        case DATA_TYPE_STRING:
            if (!input.data.string_value) {
                return create_error_result("NULL string not supported");
            }
            
            size_t len = strlen(input.data.string_value);
            char *processed_string = malloc(len + 20);
            if (!processed_string) {
                return create_error_result("Memory allocation failed");
            }
            
            snprintf(processed_string, len + 20, "Processed: %s", input.data.string_value);
            
            VariantData string_result;
            string_result.type = DATA_TYPE_STRING;
            string_result.data.string_value = processed_string;
            return create_ok_result(string_result);
            
        case DATA_TYPE_ARRAY:
            if (!input.data.array_value.array_data || input.data.array_value.array_size == 0) {
                return create_error_result("Invalid array data");
            }
            
            int *processed_array = malloc(input.data.array_value.array_size * sizeof(int));
            if (!processed_array) {
                return create_error_result("Array allocation failed");
            }
            
            // Double each element
            for (size_t i = 0; i < input.data.array_value.array_size; i++) {
                processed_array[i] = input.data.array_value.array_data[i] * 2;
            }
            
            VariantData array_result;
            array_result.type = DATA_TYPE_ARRAY;
            array_result.data.array_value.array_data = processed_array;
            array_result.data.array_value.array_size = input.data.array_value.array_size;
            return create_ok_result(array_result);
            
        default:
            return create_error_result("Unsupported data type");
    }
}

AsthraTestResult test_integration_pattern_matching_with_ffi(AsthraV12TestContext *ctx) {
    // Integration test: Pattern Matching + Result Types + FFI + Memory Management
    
    // Test case 1: Integer processing
    VariantData int_input;
    int_input.type = DATA_TYPE_INTEGER;
    int_input.data.int_value = 21;
    
    ProcessingResult int_result = c_process_variant_data(int_input);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, int_result.is_ok,
                           "Integer processing should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, int_result.result.ok_value.type == DATA_TYPE_INTEGER,
                           "Result should be integer type")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, int_result.result.ok_value.data.int_value == 42,
                           "Integer result should be 42, got %d", 
                           int_result.result.ok_value.data.int_value)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test case 2: String processing
    VariantData string_input;
    string_input.type = DATA_TYPE_STRING;
    string_input.data.string_value = "Hello World";
    
    ProcessingResult string_result = c_process_variant_data(string_input);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, string_result.is_ok,
                           "String processing should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, string_result.result.ok_value.type == DATA_TYPE_STRING,
                           "Result should be string type")) {
        if (string_result.result.ok_value.data.string_value) {
            free(string_result.result.ok_value.data.string_value);
        }
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, 
                           strstr(string_result.result.ok_value.data.string_value, "Processed: Hello World") != NULL,
                           "String result should contain processed text")) {
        free(string_result.result.ok_value.data.string_value);
        return ASTHRA_TEST_FAIL;
    }
    
    free(string_result.result.ok_value.data.string_value);
    
    // Test case 3: Array processing
    int array_data[] = {1, 2, 3, 4, 5};
    VariantData array_input;
    array_input.type = DATA_TYPE_ARRAY;
    array_input.data.array_value.array_data = array_data;
    array_input.data.array_value.array_size = 5;
    
    ProcessingResult array_result = c_process_variant_data(array_input);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, array_result.is_ok,
                           "Array processing should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, array_result.result.ok_value.type == DATA_TYPE_ARRAY,
                           "Result should be array type")) {
        if (array_result.result.ok_value.data.array_value.array_data) {
            free(array_result.result.ok_value.data.array_value.array_data);
        }
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, array_result.result.ok_value.data.array_value.array_size == 5,
                           "Result array should have 5 elements")) {
        free(array_result.result.ok_value.data.array_value.array_data);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify array contents
    for (size_t i = 0; i < 5; i++) {
        int expected = array_data[i] * 2;
        int actual = array_result.result.ok_value.data.array_value.array_data[i];
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, actual == expected,
                               "Array element %zu should be %d, got %d", i, expected, actual)) {
            free(array_result.result.ok_value.data.array_value.array_data);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    free(array_result.result.ok_value.data.array_value.array_data);
    
    // Test case 4: Error cases
    VariantData negative_int;
    negative_int.type = DATA_TYPE_INTEGER;
    negative_int.data.int_value = -10;
    
    ProcessingResult error_result = c_process_variant_data(negative_int);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !error_result.is_ok,
                           "Negative integer should result in error")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, 
                           strcmp(error_result.result.error_message, "Negative integers not supported") == 0,
                           "Error message should match expected")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test invalid type
    VariantData invalid_input;
    invalid_input.type = DATA_TYPE_INVALID;
    
    ProcessingResult invalid_result = c_process_variant_data(invalid_input);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !invalid_result.is_ok,
                           "Invalid type should result in error")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
} 
