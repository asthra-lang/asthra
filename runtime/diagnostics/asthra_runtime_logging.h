/**
 * Asthra Programming Language Runtime v1.2 - Diagnostics Module
 * Logging and Diagnostic Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides logging and diagnostic functionality
 * for runtime monitoring and debugging.
 */

#ifndef ASTHRA_RUNTIME_LOGGING_H
#define ASTHRA_RUNTIME_LOGGING_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// LOGGING AND DIAGNOSTICS
// =============================================================================

typedef enum {
    ASTHRA_LOG_TRACE,
    ASTHRA_LOG_DEBUG,
    ASTHRA_LOG_INFO,
    ASTHRA_LOG_WARN,
    ASTHRA_LOG_ERROR,
    ASTHRA_LOG_FATAL
} AsthraLogLevel;

typedef enum {
    ASTHRA_LOG_CATEGORY_GENERAL,
    ASTHRA_LOG_CATEGORY_MEMORY,
    ASTHRA_LOG_CATEGORY_GC,
    ASTHRA_LOG_CATEGORY_FFI,
    ASTHRA_LOG_CATEGORY_CONCURRENCY,
    ASTHRA_LOG_CATEGORY_SECURITY,
    ASTHRA_LOG_CATEGORY_PERFORMANCE
} AsthraLogCategory;

// Logging operations
void asthra_log(AsthraLogLevel level, AsthraLogCategory category, const char *format, ...);
void asthra_log_va(AsthraLogLevel level, AsthraLogCategory category, const char *format,
                   va_list args);
void asthra_log_set_level(AsthraLogLevel min_level);
void asthra_log_enable_category(AsthraLogCategory category, bool enabled);

// Diagnostic logging
void asthra_log_memory_zones(void);
void asthra_log_gc_stats(void);
void asthra_log_ffi_calls(void);
void asthra_log_task_stats(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_LOGGING_H
