/**
 * Simple Backend Infrastructure Tests
 * Minimal test to validate backend selection and basic operations
 */

#include "../../../src/codegen/backend_interface.h"
#include "../../../src/compiler.h"
#include <stdbool.h>
#include <stdint.h>
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

#define TEST_ASSERT_STR_EQ(actual, expected, message)                                              \
    do {                                                                                           \
        if (strcmp((actual), (expected)) != 0) {                                                   \
            printf("FAILED: %s (expected '%s', got '%s')\n", message, expected, actual);           \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

// Test backend selection
int test_backend_selection(void) {
    printf("Testing backend selection...\n");

    // Test default backend (LLVM IR)
    {
        printf("Getting default options...\n");
        AsthraCompilerOptions options = asthra_compiler_default_options();
        printf("Creating backend...\n");
        AsthraBackend *backend = asthra_backend_create(&options);
        printf("Backend pointer: %p\n", (void *)backend);
        printf("Backend address as uintptr_t: %lu\n", (uintptr_t)backend);

        // Double-check the pointer validity
        volatile AsthraBackend *volatile_backend = backend;
        printf("Volatile backend pointer: %p\n", (void *)volatile_backend);

        if (backend == NULL) {
            printf("FAILED: Backend is NULL\n");
            return 1;
        }
        printf("Backend type: %d\n", backend->type);
        if (backend->type != ASTHRA_BACKEND_LLVM_IR) {
            printf("FAILED: Expected type %d, got %d\n", ASTHRA_BACKEND_LLVM_IR, backend->type);
            return 1;
        }
        asthra_backend_destroy(backend);
    }

    // Test LLVM backend selection (should succeed since LLVM is now enabled)
    {
        printf("Testing LLVM backend...\n");
        AsthraCompilerOptions options = asthra_compiler_default_options();
        options.backend_type = ASTHRA_BACKEND_LLVM_IR;
        AsthraBackend *backend = asthra_backend_create(&options);
        printf("LLVM Backend pointer: %p\n", (void *)backend);
        if (backend == NULL) {
            printf("FAILED: LLVM backend should not be NULL since it's now enabled\n");
            return 1;
        }
        if (backend->type != ASTHRA_BACKEND_LLVM_IR) {
            printf("FAILED: Expected LLVM backend type %d, got %d\n", ASTHRA_BACKEND_LLVM_IR,
                   backend->type);
            return 1;
        }
        printf("✓ LLVM backend created successfully\n");
        asthra_backend_destroy(backend);
    }

    printf("✓ Backend selection tests passed\n");
    return 0;
}

// Test backend type strings
int test_backend_type_strings(void) {
    printf("Testing backend type strings...\n");

    TEST_ASSERT_STR_EQ(asthra_get_backend_type_string(ASTHRA_BACKEND_LLVM_IR), "LLVM IR",
                       "Backend type string for LLVM IR should be 'LLVM IR'");

    printf("✓ Backend type string tests passed\n");
    return 0;
}

// Test output filename generation
int test_output_filename_generation(void) {
    printf("Testing output filename generation...\n");

    // Test LLVM backend
    {
        char *output =
            asthra_backend_get_output_filename(ASTHRA_BACKEND_LLVM_IR, "test.asthra", NULL);
        TEST_ASSERT_NOT_NULL(output, "Output filename should not be null");
        TEST_ASSERT_STR_EQ(output, "test.ll", "LLVM backend should generate .ll extension");
        free(output);
    }

    // Test with explicit output file
    {
        char *output =
            asthra_backend_get_output_filename(ASTHRA_BACKEND_LLVM_IR, "test.asthra", "output.txt");
        TEST_ASSERT_NOT_NULL(output, "Output filename should not be null");
        TEST_ASSERT_STR_EQ(output, "output.txt", "Should use explicit output filename");
        free(output);
    }

    printf("✓ Output filename generation tests passed\n");
    return 0;
}

// Test backend file extensions
int test_backend_file_extensions(void) {
    printf("Testing backend file extensions...\n");

    TEST_ASSERT_STR_EQ(asthra_backend_get_file_extension(ASTHRA_BACKEND_LLVM_IR), "ll",
                       "LLVM backend extension should be 'll'");

    printf("✓ Backend file extension tests passed\n");
    return 0;
}

// Test backend initialization
int test_backend_initialization(void) {
    printf("Testing backend initialization...\n");

    AsthraCompilerOptions options = asthra_compiler_default_options();
    AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_LLVM_IR);
    TEST_ASSERT_NOT_NULL(backend, "Backend should not be null");

    int result = asthra_backend_initialize(backend, &options);
    TEST_ASSERT_EQ(result, 0, "Backend initialization should succeed");

    asthra_backend_destroy(backend);

    printf("✓ Backend initialization tests passed\n");
    return 0;
}

// Main test runner
int main(void) {
    printf("=== Running Simple Backend Tests ===\n");

    int result = 0;

    result |= test_backend_selection();
    result |= test_backend_type_strings();
    result |= test_output_filename_generation();
    result |= test_backend_file_extensions();
    result |= test_backend_initialization();

    if (result == 0) {
        printf("\n✓ All tests passed!\n");
    } else {
        printf("\n✗ Some tests failed!\n");
    }

    return result;
}