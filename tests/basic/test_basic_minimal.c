/**
 * Minimal Basic Test
 * Tests basic functionality without dependencies
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Test basic arithmetic
 */
static bool test_basic_arithmetic(void) {
    printf("Testing basic arithmetic...\n");

    int a = 5;
    int b = 3;
    int sum = a + b;

    if (sum != 8) {
        printf("❌ Basic arithmetic failed: %d + %d = %d (expected 8)\n", a, b, sum);
        return false;
    }

    printf("  ✓ Basic arithmetic test passed\n");
    return true;
}

/**
 * Test string operations
 */
static bool test_string_operations(void) {
    printf("Testing string operations...\n");

    const char *str1 = "Hello";
    const char *str2 = "World";
    char buffer[20];

    snprintf(buffer, sizeof(buffer), "%s %s", str1, str2);

    if (strcmp(buffer, "Hello World") != 0) {
        printf("❌ String operation failed: got '%s', expected 'Hello World'\n", buffer);
        return false;
    }

    printf("  ✓ String operations test passed\n");
    return true;
}

/**
 * Test memory allocation
 */
static bool test_memory_allocation(void) {
    printf("Testing memory allocation...\n");

    int *array = malloc(10 * sizeof(int));
    if (!array) {
        printf("❌ Memory allocation failed\n");
        return false;
    }

    // Initialize array
    for (int i = 0; i < 10; i++) {
        array[i] = i * i;
    }

    // Verify values
    bool success = true;
    for (int i = 0; i < 10; i++) {
        if (array[i] != i * i) {
            printf("❌ Memory test failed: array[%d] = %d (expected %d)\n", i, array[i], i * i);
            success = false;
            break;
        }
    }

    free(array);

    if (success) {
        printf("  ✓ Memory allocation test passed\n");
    }

    return success;
}

/**
 * Run all basic tests
 */
static bool run_basic_tests(void) {
    printf("\n=== Basic Minimal Tests ===\n");

    bool all_passed = true;

    all_passed &= test_basic_arithmetic();
    all_passed &= test_string_operations();
    all_passed &= test_memory_allocation();

    printf("\n=== Test Results ===\n");
    if (all_passed) {
        printf("✅ All basic minimal tests passed!\n");
    } else {
        printf("❌ Some basic minimal tests failed!\n");
    }

    return all_passed;
}

/**
 * Main entry point
 */
int main(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused

    bool success = run_basic_tests();
    return success ? 0 : 1;
}
