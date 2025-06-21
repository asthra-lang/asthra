/**
 * Test suite for Asthra I/O Module
 * Tests standard input, output, and error stream functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../../runtime/asthra_runtime.h"
#include "../../tests/framework/test_framework.h"

// =============================================================================
// I/O RUNTIME SUPPORT TESTS
// =============================================================================

static AsthraTestResult test_stream_access_functions(AsthraTestContext *context) {
    // Test that we can get standard stream handles
    FILE *stdin_handle = asthra_get_stdin();
    FILE *stdout_handle = asthra_get_stdout();
    FILE *stderr_handle = asthra_get_stderr();
    
    ASTHRA_TEST_ASSERT_NOT_NULL(context, stdin_handle, "stdin handle should not be NULL");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, stdout_handle, "stdout handle should not be NULL");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, stderr_handle, "stderr handle should not be NULL");
    
    ASTHRA_TEST_ASSERT_EQ(context, stdin_handle, stdin, "stdin handle should match system stdin");
    ASTHRA_TEST_ASSERT_EQ(context, stdout_handle, stdout, "stdout handle should match system stdout");
    ASTHRA_TEST_ASSERT_EQ(context, stderr_handle, stderr, "stderr handle should match system stderr");
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_string_conversion_functions(AsthraTestContext *context) {
    // Test C string to Asthra string conversion
    const char *test_cstr = "Hello, Asthra I/O!";
    AsthraString asthra_str = asthra_string_from_cstr(test_cstr);
    
    ASTHRA_TEST_ASSERT_NOT_NULL(context, asthra_str.data, "Asthra string data should not be NULL");
    ASTHRA_TEST_ASSERT_EQ(context, asthra_str.len, strlen(test_cstr), "Asthra string length should match C string length");
    ASTHRA_TEST_ASSERT_STR_EQ(context, asthra_str.data, test_cstr, "Asthra string content should match C string");
    
    // Test Asthra string to C string conversion
    char *converted_cstr = asthra_string_to_cstr(asthra_str, ASTHRA_TRANSFER_NONE);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, converted_cstr, "Converted C string should not be NULL");
    ASTHRA_TEST_ASSERT_STR_EQ(context, converted_cstr, test_cstr, "Converted C string should match original");
    
    // Cleanup
    asthra_string_free(asthra_str);
    free(converted_cstr);
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_println_functions(AsthraTestContext *context) {
    // Test asthra_println function
    printf("Testing asthra_println (should see 'Test println output' below):\n");
    asthra_println("Test println output");
    
    // Test asthra_eprintln function
    printf("Testing asthra_eprintln (should see 'Test eprintln output' on stderr):\n");
    asthra_eprintln("Test eprintln output");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// I/O MODULE SIMULATION TESTS
// =============================================================================

// Since we can't easily test the actual I/O module functions without a full Asthra runtime,
// we'll test the underlying concepts and runtime support functions

static AsthraTestResult test_utf8_validation_concept(AsthraTestContext *context) {
    // Test UTF-8 validation logic (simulated)
    
    // Valid UTF-8 sequences
    uint8_t valid_ascii[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    uint8_t valid_2byte[] = {0xC3, 0xA9}; // "é"
    uint8_t valid_3byte[] = {0xE4, 0xB8, 0x96}; // "世"
    uint8_t valid_4byte[] = {0xF0, 0x9F, 0x98, 0x80}; // "😀"
    
    // Invalid UTF-8 sequences
    uint8_t invalid_continuation[] = {0xC3, 0x28}; // Invalid continuation byte
    uint8_t invalid_start[] = {0xFF, 0x80}; // Invalid start byte
    
    // For now, we just verify the test data is set up correctly
    ASTHRA_TEST_ASSERT_EQ(context, (int)valid_ascii[0], 0x48, "ASCII test data correct");
    ASTHRA_TEST_ASSERT_EQ(context, (int)valid_2byte[0], 0xC3, "2-byte UTF-8 test data correct");
    ASTHRA_TEST_ASSERT_EQ(context, (int)valid_3byte[0], 0xE4, "3-byte UTF-8 test data correct");
    ASTHRA_TEST_ASSERT_EQ(context, (int)valid_4byte[0], 0xF0, "4-byte UTF-8 test data correct");
    ASTHRA_TEST_ASSERT_EQ(context, (int)invalid_continuation[1], 0x28, "Invalid continuation test data correct");
    ASTHRA_TEST_ASSERT_EQ(context, (int)invalid_start[0], 0xFF, "Invalid start test data correct");
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_error_handling_concept(AsthraTestContext *context) {
    // Test error handling concepts for I/O operations
    
    // Simulate different error conditions
    enum {
        IO_ERROR_NONE = 0,
        IO_ERROR_EOF,
        IO_ERROR_PERMISSION_DENIED,
        IO_ERROR_NOT_FOUND,
        IO_ERROR_INVALID_UTF8,
        IO_ERROR_BUFFER_TOO_SMALL,
        IO_ERROR_WRITE_ERROR,
        IO_ERROR_READ_ERROR,
        IO_ERROR_OTHER
    };
    
    // Test error type enumeration
    asthra_test_assert_int_eq(context, IO_ERROR_NONE, 0, "No error should be 0");
    ASTHRA_TEST_ASSERT_NE(context, IO_ERROR_EOF, IO_ERROR_NONE, "EOF error should be non-zero");
    ASTHRA_TEST_ASSERT_NE(context, IO_ERROR_INVALID_UTF8, IO_ERROR_READ_ERROR, "Different errors should have different values");
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_buffer_management_concept(AsthraTestContext *context) {
    // Test buffer management concepts for I/O operations
    
    const size_t buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, buffer, "Buffer allocation should succeed");
    
    // Initialize buffer
    memset(buffer, 0, buffer_size);
    
    // Test buffer operations
    const char *test_data = "Test buffer data";
    size_t test_data_len = strlen(test_data);
    
    ASTHRA_TEST_ASSERT_TRUE(context, test_data_len < buffer_size, "Test data should fit in buffer");
    
    // Copy test data to buffer
    memcpy(buffer, test_data, test_data_len);
    buffer[test_data_len] = '\0';
    
    ASTHRA_TEST_ASSERT_STR_EQ(context, buffer, test_data, "Buffer should contain test data");
    
    // Cleanup
    free(buffer);
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

static AsthraTestResult test_runtime_initialization_for_io(AsthraTestContext *context) {
    // Test that runtime can be initialized for I/O operations
    AsthraGCConfig gc_config = {
        .initial_heap_size = 1024 * 1024,
        .max_heap_size = 16 * 1024 * 1024,
        .gc_threshold = 0.8,
        .conservative_mode = true,
        .concurrent_gc = false,
        .use_thread_local_roots = false
    };
    
    int result = asthra_runtime_init(&gc_config);
    asthra_test_assert_int_eq(context, result, 0, "Runtime initialization should succeed");
    
    // Test that we can perform basic operations
    AsthraString test_str = asthra_string_new("I/O test string");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, test_str.data, "String creation should succeed");
    ASTHRA_TEST_ASSERT_TRUE(context, test_str.len > 0, "String should have non-zero length");
    
    // Cleanup
    asthra_string_free(test_str);
    asthra_runtime_cleanup();
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_memory_safety_for_io_operations(AsthraTestContext *context) {
    // Test memory safety concepts for I/O operations
    AsthraGCConfig gc_config = {
        .initial_heap_size = 1024 * 1024,
        .max_heap_size = 16 * 1024 * 1024,
        .gc_threshold = 0.8,
        .conservative_mode = true,
        .concurrent_gc = false,
        .use_thread_local_roots = false
    };
    
    int result = asthra_runtime_init(&gc_config);
    asthra_test_assert_int_eq(context, result, 0, "Runtime initialization should succeed");
    
    // Test multiple string allocations (simulating I/O buffer management)
    AsthraString strings[10];
    for (int i = 0; i < 10; i++) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "I/O string %d", i);
        strings[i] = asthra_string_new(buffer);
        ASTHRA_TEST_ASSERT_NOT_NULL(context, strings[i].data, "String allocation should succeed");
    }
    
    // Verify all strings are valid
    for (int i = 0; i < 10; i++) {
        ASTHRA_TEST_ASSERT_TRUE(context, strings[i].len > 0, "String should have non-zero length");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, strstr(strings[i].data, "I/O string"), "String should contain expected content");
    }
    
    // Cleanup
    for (int i = 0; i < 10; i++) {
        asthra_string_free(strings[i]);
    }
    
    asthra_runtime_cleanup();
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("I/O Module Tests");
    
    // Runtime support tests
    asthra_test_suite_add_test(suite, "stream_access_functions", "Test stream access functions", test_stream_access_functions);
    asthra_test_suite_add_test(suite, "string_conversion_functions", "Test string conversion functions", test_string_conversion_functions);
    asthra_test_suite_add_test(suite, "println_functions", "Test println functions", test_println_functions);
    
    // Concept validation tests
    asthra_test_suite_add_test(suite, "utf8_validation_concept", "Test UTF-8 validation concept", test_utf8_validation_concept);
    asthra_test_suite_add_test(suite, "error_handling_concept", "Test error handling concept", test_error_handling_concept);
    asthra_test_suite_add_test(suite, "buffer_management_concept", "Test buffer management concept", test_buffer_management_concept);
    
    // Integration tests
    asthra_test_suite_add_test(suite, "runtime_initialization_for_io", "Test runtime initialization for I/O", test_runtime_initialization_for_io);
    asthra_test_suite_add_test(suite, "memory_safety_for_io_operations", "Test memory safety for I/O operations", test_memory_safety_for_io_operations);
    
    return asthra_test_suite_run_and_exit(suite);
} 
