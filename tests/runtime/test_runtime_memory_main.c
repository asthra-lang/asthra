/**
 * Asthra Runtime Memory Tests - Main Runner
 * Provides main() function to run runtime memory tests
 */

#include "test_common.h"

// External test function declarations
extern int test_memory_zones(void);
extern int test_gc_operations(void);

int main(void) {
    printf("Running Asthra runtime memory tests...\n\n");

    int tests_run = 0;
    int tests_passed = 0;

    // Test memory zones
    tests_run++;
    if (test_memory_zones()) {
        tests_passed++;
    }

    // Test GC operations
    tests_run++;
    if (test_gc_operations()) {
        tests_passed++;
    }

    printf("\nTest Results: %d/%d passed\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
}
