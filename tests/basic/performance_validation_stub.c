/**
 * Simple stub for performance validation
 * Just to satisfy the linker for basic tests
 */

#include <stdbool.h>
#include <stdio.h>

bool performance_validation_run_complete(const char *test_name) {
    printf("Performance validation for '%s' (stub)\n", test_name);
    // Always return true for now
    return true;
}