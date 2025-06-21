/**
 * Minimal includes for safety modules to avoid circular dependencies
 */
#ifndef ASTHRA_SAFETY_MINIMAL_INCLUDES_H
#define ASTHRA_SAFETY_MINIMAL_INCLUDES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdarg.h>

// Forward declarations of essential types from asthra_runtime.h
typedef enum {
    ASTHRA_OWNERSHIP_GC,
    ASTHRA_OWNERSHIP_C,
    ASTHRA_OWNERSHIP_PINNED
} AsthraOwnershipHint;

typedef enum {
    ASTHRA_TRANSFER_FULL,
    ASTHRA_TRANSFER_NONE,
    ASTHRA_TRANSFER_BORROWED
} AsthraTransferType;

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
    ASTHRA_LOG_CATEGORY_FFI,
    ASTHRA_LOG_CATEGORY_TASK,
    ASTHRA_LOG_CATEGORY_SYNC,
    ASTHRA_LOG_CATEGORY_SAFETY,
    ASTHRA_LOG_CATEGORY_SECURITY
} AsthraLogCategory;

// Forward declaration of logging function
void asthra_log(AsthraLogLevel level, AsthraLogCategory category, const char *format, ...);

// Get timestamp function
uint64_t asthra_get_timestamp_ns(void);

#endif // ASTHRA_SAFETY_MINIMAL_INCLUDES_H