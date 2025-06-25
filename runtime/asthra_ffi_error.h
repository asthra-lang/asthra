/**
 * Asthra FFI Error Handling v1.0
 * Mapping C error codes (primarily errno values) to Asthra's Result types
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Type-safe FFI error handling
 * - Comprehensive error context for debugging
 * - Integration with Asthra Result system
 * - Thread-safe operation
 * - Cross-platform compatibility
 */

#ifndef ASTHRA_FFI_ERROR_H
#define ASTHRA_FFI_ERROR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Define our own string structure for standalone usage
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} AsthraCFFIString;

/**
 * Core FFI error representation
 */
typedef struct {
    int32_t code;                  // Original C error code
    AsthraCFFIString *subsystem;   // Subsystem identifier (e.g., "libc", "openssl")
    AsthraCFFIString *message;     // Human-readable error message
    AsthraCFFIString *source_file; // Source file where error occurred
    int32_t line;                  // Line number where error occurred
} AsthraCFFIError;

/**
 * Initialize FFIError struct from error code
 * @param code Error code (usually from errno)
 * @param subsystem Subsystem identifier string
 * @return Initialized FFIError struct
 */
AsthraCFFIError *asthra_ffi_error_create(int32_t code, const char *subsystem);

/**
 * Free FFIError resources
 * @param error Pointer to FFIError struct
 */
void asthra_ffi_error_free(AsthraCFFIError *error);

/**
 * Create FFIError from current errno
 * @param subsystem Subsystem identifier
 * @return Initialized FFIError struct
 */
AsthraCFFIError *asthra_ffi_error_from_errno(const char *subsystem);

/**
 * Create FFIError with source location information
 * @param code Error code
 * @param subsystem Subsystem identifier
 * @param file Source file
 * @param line Line number
 * @return Initialized FFIError struct
 */
AsthraCFFIError *asthra_ffi_error_create_with_location(int32_t code, const char *subsystem,
                                                       const char *file, int32_t line);

/**
 * Set source location for error
 * @param error FFIError struct
 * @param file Source file
 * @param line Line number
 */
void asthra_ffi_error_set_location(AsthraCFFIError *error, const char *file, int32_t line);

/**
 * Get error message
 * @param error FFIError struct
 * @return Error message string (caller must not free)
 */
const char *asthra_ffi_error_get_message(AsthraCFFIError *error);

/**
 * Get error code
 * @param error FFIError struct
 * @return Error code
 */
int32_t asthra_ffi_error_get_code(AsthraCFFIError *error);

/**
 * Register the FFIError type with the runtime
 * @return Type ID for FFIError
 */
uint32_t asthra_ffi_error_register_type(void);

/**
 * Create a Result.Err value from an FFIError
 * Note: This function is meant to be called by the Asthra language runtime
 * @param error FFIError struct
 * @return Error value (to be used with asthra_result_err)
 */
void *asthra_ffi_error_create_err_value(AsthraCFFIError *error);

/**
 * Check if error code represents a temporary error
 * @param code Error code
 * @return true if temporary, false otherwise
 */
bool asthra_ffi_error_is_temporary(int32_t code);

/**
 * Check if error code represents a permission denied error
 * @param code Error code
 * @return true if permission denied, false otherwise
 */
bool asthra_ffi_error_is_permission_denied(int32_t code);

/**
 * Check if error code represents a not found error
 * @param code Error code
 * @return true if not found, false otherwise
 */
bool asthra_ffi_error_is_not_found(int32_t code);

/**
 * Convert FFIError to string representation
 * @param error FFIError struct
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of characters written (excluding null terminator)
 */
size_t asthra_ffi_error_to_string(AsthraCFFIError *error, char *buffer, size_t buffer_size);

/**
 * Builtin function handlers for Asthra language integration
 * These functions are called by the Asthra runtime when the corresponding builtins are used
 */

/**
 * Creates FFIError from current errno (called by from_errno() builtin)
 * @param subsystem Subsystem identifier
 * @return Initialized FFIError struct with source location
 */
AsthraCFFIError *asthra_builtin_from_errno(const char *subsystem);

/**
 * Creates FFIError from specific error code (called by from_error_code() builtin)
 * @param code Error code
 * @param subsystem Subsystem identifier
 * @return Initialized FFIError struct with source location
 */
AsthraCFFIError *asthra_builtin_from_error_code(int32_t code, const char *subsystem);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_ERROR_H
