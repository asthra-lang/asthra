/**
 * Asthra Programming Language v1.2 Integration Tests - Common Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common includes, types, and utilities for integration tests.
 */

#ifndef ASTHRA_TEST_INTEGRATION_COMMON_H
#define ASTHRA_TEST_INTEGRATION_COMMON_H

#include "../../runtime/asthra_runtime.h"
#include "../core/test_comprehensive.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Function declarations for all integration tests
AsthraTestResult test_integration_concurrent_text_processing(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_concurrent_slice_management(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_pattern_matching_with_ffi(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_security_with_concurrency(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_performance_comprehensive(AsthraV12TestContext *ctx);

#endif // ASTHRA_TEST_INTEGRATION_COMMON_H
