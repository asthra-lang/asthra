/**
 * Asthra Programming Language Runtime v1.2
 * Common Test Utilities and Macros
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../runtime/asthra_runtime.h"

// Test framework macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "FAIL: %s\n", message);                                                \
            return 0;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_PASS(message)                                                                         \
    do {                                                                                           \
        printf("PASS: %s\n", message);                                                             \
        return 1;                                                                                  \
    } while (0)

#endif // TEST_COMMON_H
