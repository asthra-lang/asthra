/**
 * Implementation of Common Test Infrastructure for Asthra Safe C Memory Interface
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"

// Test counters and runtime functions are defined in test_runner_minimal.c, so we don't redefine them here

void reset_test_counters(void) {
    tests_run = 0;
    tests_passed = 0;
    tests_failed = 0;
} 
