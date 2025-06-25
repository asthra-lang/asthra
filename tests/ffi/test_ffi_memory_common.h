/**
 * Common Test Infrastructure for Asthra Safe C Memory Interface
 * Shared definitions, macros, and utilities for FFI memory tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef TEST_FFI_MEMORY_COMMON_H
#define TEST_FFI_MEMORY_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../runtime/asthra_ffi_memory.h"
#include "../../runtime/asthra_runtime.h"

// Test result tracking (extern declarations for shared counters)
extern size_t tests_run;
extern size_t tests_passed;
extern size_t tests_failed;

// Test infrastructure macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("✓ %s\n", message);                                                             \
        } else {                                                                                   \
            tests_failed++;                                                                        \
            printf("✗ %s\n", message);                                                             \
        }                                                                                          \
    } while (0)

#define TEST_SECTION(name) printf("\n=== %s ===\n", name)

// Common test utilities
void test_runtime_init(void);
void test_runtime_cleanup(void);
void print_test_results(void);
void reset_test_counters(void);

// Test registration system
typedef void (*test_function_t)(void);

typedef struct {
    const char *name;
    test_function_t function;
} test_case_t;

// Test runner utilities
int run_test_suite(const char *suite_name, test_case_t *tests, size_t test_count);

#endif // TEST_FFI_MEMORY_COMMON_H
