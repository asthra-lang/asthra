#include "../../runtime/asthra_ffi_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

// Simple memory management functions for testing
static void* test_alloc(size_t size) {
    return malloc(size);
}

static void test_free(void* ptr) {
    free(ptr);
}

// Override Asthra runtime functions for testing
void* asthra_alloc(size_t size, int zone) {
    (void)zone; // Unused in this test
    return test_alloc(size);
}

void asthra_free(void* ptr, int zone) {
    (void)zone; // Unused in this test
    test_free(ptr);
}

uint32_t asthra_register_type(const char* name, size_t size, void (*destructor)(void*)) {
    (void)name; // Unused in this test
    (void)size; // Unused in this test
    (void)destructor; // Unused in this test
    return 1; // Return a dummy type ID
}

#define TEST_PASSED printf("✅ %s passed\n", __func__)
#define TEST_FAILED(msg) do { printf("❌ %s failed: %s\n", __func__, msg); exit(1); } while(0)

// Forward declarations of test functions
static void test_ffi_error_creation(void);
static void test_ffi_error_from_errno(void);
static void test_ffi_error_with_location(void);
static void test_ffi_error_to_string(void);
static void test_ffi_error_classification(void);
static void test_ffi_error_null_safety(void);

static void test_ffi_error_creation(void) {
    // Test creating an error from a specific code
    AsthraCFFIError* error = asthra_ffi_error_create(ENOENT, "test");
    
    if (!error) {
        TEST_FAILED("Failed to create error");
    }
    
    if (error->code != ENOENT) {
        TEST_FAILED("Error code mismatch");
    }
    
    if (!error->subsystem || strcmp(error->subsystem->data, "test") != 0) {
        TEST_FAILED("Subsystem mismatch");
    }
    
    if (!error->message || strlen(error->message->data) == 0) {
        TEST_FAILED("Message is empty");
    }
    
    asthra_ffi_error_free(error);
    TEST_PASSED;
}

static void test_ffi_error_from_errno(void) {
    // Set errno to a known value
    errno = EACCES;
    
    // Create error from errno
    AsthraCFFIError* error = asthra_ffi_error_from_errno("libc_test");
    
    if (!error) {
        TEST_FAILED("Failed to create error from errno");
    }
    
    if (error->code != EACCES) {
        TEST_FAILED("Error code mismatch");
    }
    
    if (!error->subsystem || strcmp(error->subsystem->data, "libc_test") != 0) {
        TEST_FAILED("Subsystem mismatch");
    }
    
    asthra_ffi_error_free(error);
    TEST_PASSED;
}

static void test_ffi_error_with_location(void) {
    // Test setting source location
    AsthraCFFIError* error = asthra_ffi_error_create_with_location(
        EINVAL, "test", "test_file.c", 42);
    
    if (!error) {
        TEST_FAILED("Failed to create error with location");
    }
    
    if (!error->source_file || strcmp(error->source_file->data, "test_file.c") != 0) {
        TEST_FAILED("Source file mismatch");
    }
    
    if (error->line != 42) {
        TEST_FAILED("Line number mismatch");
    }
    
    // Test updating location
    asthra_ffi_error_set_location(error, "another_file.c", 100);
    
    if (!error->source_file || strcmp(error->source_file->data, "another_file.c") != 0) {
        TEST_FAILED("Updated source file mismatch");
    }
    
    if (error->line != 100) {
        TEST_FAILED("Updated line number mismatch");
    }
    
    asthra_ffi_error_free(error);
    TEST_PASSED;
}

static void test_ffi_error_to_string(void) {
    AsthraCFFIError* error = asthra_ffi_error_create_with_location(
        ENOENT, "fs", "main.c", 123);
    
    char buffer[256];
    size_t len = asthra_ffi_error_to_string(error, buffer, sizeof(buffer));
    
    if (len == 0) {
        TEST_FAILED("Failed to convert error to string");
    }
    
    // Check that the string contains expected components
    if (strstr(buffer, "fs") == NULL) {
        TEST_FAILED("String missing subsystem");
    }
    
    if (strstr(buffer, "ENOENT") == NULL && strstr(buffer, "No such file") == NULL) {
        TEST_FAILED("String missing error message");
    }
    
    if (strstr(buffer, "main.c") == NULL) {
        TEST_FAILED("String missing source file");
    }
    
    if (strstr(buffer, "123") == NULL) {
        TEST_FAILED("String missing line number");
    }
    
    asthra_ffi_error_free(error);
    TEST_PASSED;
}

static void test_ffi_error_classification(void) {
    // Test temporary error classification
    if (!asthra_ffi_error_is_temporary(EAGAIN)) {
        TEST_FAILED("EAGAIN should be classified as temporary");
    }
    
    if (!asthra_ffi_error_is_temporary(EWOULDBLOCK)) {
        TEST_FAILED("EWOULDBLOCK should be classified as temporary");
    }
    
    if (!asthra_ffi_error_is_temporary(EINTR)) {
        TEST_FAILED("EINTR should be classified as temporary");
    }
    
    if (asthra_ffi_error_is_temporary(ENOENT)) {
        TEST_FAILED("ENOENT should not be classified as temporary");
    }
    
    // Test permission denied error classification
    if (!asthra_ffi_error_is_permission_denied(EACCES)) {
        TEST_FAILED("EACCES should be classified as permission denied");
    }
    
    if (!asthra_ffi_error_is_permission_denied(EPERM)) {
        TEST_FAILED("EPERM should be classified as permission denied");
    }
    
    if (asthra_ffi_error_is_permission_denied(ENOENT)) {
        TEST_FAILED("ENOENT should not be classified as permission denied");
    }
    
    // Test not found error classification
    if (!asthra_ffi_error_is_not_found(ENOENT)) {
        TEST_FAILED("ENOENT should be classified as not found");
    }
    
    if (asthra_ffi_error_is_not_found(EINVAL)) {
        TEST_FAILED("EINVAL should not be classified as not found");
    }
    
    TEST_PASSED;
}

static void test_ffi_error_null_safety(void) {
    // Test handling of NULL parameters
    asthra_ffi_error_free(NULL); // Should not crash
    
    asthra_ffi_error_set_location(NULL, "file.c", 42); // Should not crash
    
    const char* message = asthra_ffi_error_get_message(NULL);
    if (strcmp(message, "Unknown error") != 0) {
        TEST_FAILED("Expected 'Unknown error' for NULL error");
    }
    
    int32_t code = asthra_ffi_error_get_code(NULL);
    if (code != 0) {
        TEST_FAILED("Expected 0 for NULL error code");
    }
    
    char buffer[256];
    size_t len = asthra_ffi_error_to_string(NULL, buffer, sizeof(buffer));
    if (len != 0) {
        TEST_FAILED("Expected 0 length for NULL error string");
    }
    
    len = asthra_ffi_error_to_string(NULL, NULL, 0);
    if (len != 0) {
        TEST_FAILED("Expected 0 length for NULL error and buffer");
    }
    
    TEST_PASSED;
}

int main(void) {
    printf("Running FFI Error tests...\n");
    
    test_ffi_error_creation();
    test_ffi_error_from_errno();
    test_ffi_error_with_location();
    test_ffi_error_to_string();
    test_ffi_error_classification();
    test_ffi_error_null_safety();
    
    printf("All FFI Error tests passed! ✅\n");
    
    return 0;
} 

