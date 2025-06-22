/**
 * Asthra Programming Language Runtime v1.2 - Utilities Module
 * Utility Functions and I/O Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides utility functionality including
 * timing functions and I/O support.
 */

#ifndef ASTHRA_RUNTIME_UTILS_H
#define ASTHRA_RUNTIME_UTILS_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

uint64_t asthra_get_timestamp_ns(void);
uint64_t asthra_get_timestamp_ms(void);
void asthra_sleep_ms(uint64_t milliseconds);
void asthra_sleep_ns(uint64_t nanoseconds);

// =============================================================================
// I/O SUPPORT FUNCTIONS
// =============================================================================

// Standard stream access
FILE *asthra_get_stdin(void);
FILE *asthra_get_stdout(void);
FILE *asthra_get_stderr(void);

// I/O utility functions
void asthra_println(const char *message);
void asthra_eprintln(const char *message);

// Predeclared functions for Asthra language
void asthra_simple_log(const char *message);
void asthra_panic(const char *message) __attribute__((noreturn));

// Type-generic macros using C17 _Generic
#if ASTHRA_HAS_C17
#define asthra_free_typed(ptr) _Generic((ptr), \
    AsthraString*: asthra_string_free, \
    AsthraSliceHeader*: asthra_slice_free, \
    AsthraResult*: asthra_result_free, \
    default: asthra_free \
)(ptr)
#endif

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_UTILS_H 
