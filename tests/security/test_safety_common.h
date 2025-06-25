/**
 * Asthra Programming Language Runtime Safety System Tests - Common Definitions
 * Shared header for all safety system test modules
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TEST_SAFETY_COMMON_H
#define ASTHRA_TEST_SAFETY_COMMON_H

#include "../runtime/asthra_runtime.h"
#include "../runtime/asthra_safety.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Test result tracking (shared across all test modules)
extern size_t tests_passed;
extern size_t tests_failed;

// Common test assertion macro
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (condition) {                                                                           \
            printf("✓ PASS: %s\n", message);                                                       \
            tests_passed++;                                                                        \
        } else {                                                                                   \
            printf("✗ FAIL: %s\n", message);                                                       \
            tests_failed++;                                                                        \
        }                                                                                          \
    } while (0)

// Function declarations for test modules
void test_grammar_validation(void);
void test_pattern_matching_completeness(void);
void test_type_safety_validation(void);

void test_ffi_annotation_verification(void);
void test_boundary_checks(void);
void test_ffi_pointer_tracking(void);

void test_string_operation_validation(void);
void test_slice_bounds_checking(void);
void test_memory_layout_validation(void);

void test_task_lifecycle_logging(void);
void test_scheduler_event_logging(void);
void test_result_tracking(void);

void test_stack_canary_management(void);
void test_ffi_call_logging(void);
void test_constant_time_verification(void);
void test_secure_memory_validation(void);

void test_fault_injection(void);
void test_performance_monitoring(void);
void test_safety_configuration(void);
void test_variadic_validation(void);

#endif // ASTHRA_TEST_SAFETY_COMMON_H
