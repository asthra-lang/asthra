/**
 * Asthra Programming Language Runtime v1.2 - Errors Module
 * Error Handling and Reporting
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides error handling functionality including
 * error codes, error structures, and error reporting functions.
 */

#ifndef ASTHRA_RUNTIME_ERRORS_H
#define ASTHRA_RUNTIME_ERRORS_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ERROR HANDLING
// =============================================================================

typedef enum {
    ASTHRA_ERROR_NONE = 0,
    ASTHRA_ERROR_OUT_OF_MEMORY,
    ASTHRA_ERROR_INVALID_ARGUMENT,
    ASTHRA_ERROR_NULL_POINTER,
    ASTHRA_ERROR_BOUNDS_CHECK,
    ASTHRA_ERROR_TYPE_MISMATCH,
    ASTHRA_ERROR_OWNERSHIP_VIOLATION,
    ASTHRA_ERROR_THREAD_ERROR,
    ASTHRA_ERROR_IO_ERROR,
    ASTHRA_ERROR_CRYPTO_ERROR,
    ASTHRA_ERROR_RUNTIME_ERROR
} AsthraErrorCode;

typedef struct {
    AsthraErrorCode code;
    char message[256];
    const char *file;
    int line;
    const char *function;
} AsthraError;

AsthraError asthra_get_last_error(void);
void asthra_set_error(AsthraErrorCode code, const char *message, const char *file, int line, const char *function);
void asthra_clear_error(void);
const char *asthra_error_string(AsthraErrorCode code);

// Error handling macros
#define ASTHRA_SET_ERROR(code, msg) \
    asthra_set_error(code, msg, __FILE__, __LINE__, __func__)

#define ASTHRA_RETURN_IF_ERROR(expr) \
    do { \
        AsthraError _err = asthra_get_last_error(); \
        if (_err.code != ASTHRA_ERROR_NONE) return (expr); \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_ERRORS_H 
