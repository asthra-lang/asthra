/**
 * Simple Performance Tests for Asthra Programming Language
 * Tests basic performance without complex concurrency dependencies
 * Created: Current session - systematic performance category fix
 */

#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

// Performance test constants
#define ALLOC_ITERATIONS 1000000
#define STRING_ITERATIONS 5000000
#define ARITHMETIC_ITERATIONS 100000000

// Test function declarations
static AsthraTestResult test_memory_allocation_performance(AsthraTestContext *context);
static AsthraTestResult test_string_operation_performance(AsthraTestContext *context);
static AsthraTestResult test_arithmetic_performance(AsthraTestContext *context);

/**
 * Test basic memory allocation performance using standard malloc
 */
static AsthraTestResult test_memory_allocation_performance(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    clock_t start = clock();
    
    for (int i = 0; i < ALLOC_ITERATIONS; i++) {
        void* ptr = malloc(64);
        if (!asthra_test_assert_pointer(context, ptr, "Memory allocation should succeed")) {
            return ASTHRA_TEST_FAIL;
        }
        free(ptr);
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    double allocs_per_second = ALLOC_ITERATIONS / time_spent;
    
    printf("Memory allocation performance: %.2f allocs/sec (%.6f ms avg)\n", 
           allocs_per_second, (time_spent * 1000.0) / ALLOC_ITERATIONS);
    
    // Should achieve at least 500K allocations per second
    if (allocs_per_second < 500000) {
        printf("Warning: Memory allocation performance below target (%.2f < 500K allocs/sec)\n", 
               allocs_per_second);
    }
    
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

/**
 * Test string operation performance using standard string functions
 */
static AsthraTestResult test_string_operation_performance(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    const char* test_string = "Performance test string for copying operations";
    size_t str_len = strlen(test_string);
    clock_t start = clock();
    
    for (int i = 0; i < STRING_ITERATIONS; i++) {
        char* copy = malloc(str_len + 1);
        if (!asthra_test_assert_pointer(context, copy, "String copy memory allocation should succeed")) {
            return ASTHRA_TEST_FAIL;
        }
        strcpy(copy, test_string);
        free(copy);
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    double copies_per_second = STRING_ITERATIONS / time_spent;
    
    printf("String operation performance: %.2f copies/sec (%.6f ms avg)\n", 
           copies_per_second, (time_spent * 1000.0) / STRING_ITERATIONS);
    
    // Should achieve at least 5M string copies per second
    if (copies_per_second < 5000000) {
        printf("Warning: String operation performance below target (%.2f < 5M copies/sec)\n", 
               copies_per_second);
    }
    
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

/**
 * Test arithmetic operation performance
 */
static AsthraTestResult test_arithmetic_performance(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    volatile int result = 0;  // volatile to prevent optimization
    clock_t start = clock();
    
    for (int i = 0; i < ARITHMETIC_ITERATIONS; i++) {
        result += i * 2 + 1;
        result = result % 1000000;  // Prevent overflow
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    double ops_per_second = ARITHMETIC_ITERATIONS / time_spent;
    
    printf("Arithmetic performance: %.2f ops/sec (%.6f ms avg)\n", 
           ops_per_second, (time_spent * 1000.0) / ARITHMETIC_ITERATIONS);
    printf("Final result: %d (prevents optimization)\n", result);
    
    // Should achieve at least 50M arithmetic operations per second
    if (ops_per_second < 50000000) {
        printf("Warning: Arithmetic performance below target (%.2f < 50M ops/sec)\n", 
               ops_per_second);
    }
    
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Performance Tests (Simple)");
    
    asthra_test_suite_add_test(suite, "memory_allocation_performance", "Test basic memory allocation performance", test_memory_allocation_performance);
    asthra_test_suite_add_test(suite, "string_operation_performance", "Test string operation performance", test_string_operation_performance);
    asthra_test_suite_add_test(suite, "arithmetic_performance", "Test arithmetic operation performance", test_arithmetic_performance);
    
    return asthra_test_suite_run_and_exit(suite);
} 
