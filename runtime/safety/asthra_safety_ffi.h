/**
 * Asthra Programming Language Runtime Safety System
 * FFI Safety and Tracking Module - Foreign Function Interface safety
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_FFI_H
#define ASTHRA_SAFETY_FFI_H

#include "asthra_safety_minimal_includes.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FFI SAFETY STRUCTURES
// =============================================================================

// FFI annotation verification
typedef enum {
    ASTHRA_FFI_ANNOTATION_VALID,
    ASTHRA_FFI_ANNOTATION_MISMATCH,
    ASTHRA_FFI_ANNOTATION_MISSING,
    ASTHRA_FFI_ANNOTATION_INVALID_TRANSFER,
    ASTHRA_FFI_ANNOTATION_LIFETIME_VIOLATION
} AsthraFFIAnnotationResult;

typedef struct {
    AsthraFFIAnnotationResult result;
    AsthraTransferType expected_transfer;
    AsthraTransferType actual_transfer;
    bool is_borrowed;
    char violation_message[256];
    const char *function_name;
    int parameter_index;
} AsthraFFIAnnotationCheck;

// Ownership tracking for FFI pointers
typedef struct {
    void *ptr;
    size_t size;
    AsthraTransferType transfer_type;
    AsthraOwnershipHint ownership;
    bool is_borrowed;
    uint64_t allocation_timestamp;
    uint64_t last_access_timestamp;
    const char *allocation_source;
    int allocation_line;
    pthread_t owning_thread;
    atomic_int reference_count;
} AsthraFFIPointerTracker;

// Variadic function safety validation
typedef struct {
    bool is_valid;
    size_t expected_arg_count;
    size_t actual_arg_count;
    AsthraVarArgType *expected_types;
    AsthraVarArgType *actual_types;
    char type_mismatch_details[512];
} AsthraVariadicValidation;

// FFI call logging and validation
typedef struct {
    uint64_t call_id;
    const char *function_name;
    void *function_ptr;
    void **arguments;
    size_t argument_count;
    AsthraTransferType *transfer_types;
    bool *is_borrowed;
    uint64_t call_timestamp_ns;
    uint64_t return_timestamp_ns;
    void *return_value;
    bool is_successful;
    char call_details[512];
} AsthraFFICallLog;

// =============================================================================
// FFI SAFETY FUNCTIONS
// =============================================================================

/**
 * Verify FFI function annotation compliance
 */
AsthraFFIAnnotationCheck asthra_safety_verify_ffi_annotation(void *func_ptr, void **args,
                                                             size_t arg_count,
                                                             AsthraTransferType *expected_transfers,
                                                             bool *is_borrowed);

/**
 * Register FFI pointer for ownership tracking
 */
int asthra_safety_register_ffi_pointer(void *ptr, size_t size, AsthraTransferType transfer,
                                       AsthraOwnershipHint ownership, bool is_borrowed,
                                       const char *source, int line);

/**
 * Unregister FFI pointer from tracking
 */
int asthra_safety_unregister_ffi_pointer(void *ptr);

/**
 * Get FFI pointer tracking information
 */
AsthraFFIPointerTracker *asthra_safety_get_ffi_pointer_info(void *ptr);

/**
 * Validate variadic function call safety
 */
AsthraVariadicValidation asthra_safety_validate_variadic_call(void *func_ptr, AsthraVarArg *args,
                                                              size_t arg_count,
                                                              AsthraVarArgType *expected_types,
                                                              size_t expected_count);

/**
 * Log FFI function call for debugging and safety tracking
 */
void asthra_safety_log_ffi_call(const char *function_name, void *function_ptr, void **args,
                                size_t arg_count);

/**
 * Check if FFI pointer is still valid and tracked
 */
bool asthra_safety_is_ffi_pointer_valid(void *ptr);

/**
 * Update FFI pointer access timestamp
 */
void asthra_safety_update_ffi_pointer_access(void *ptr);

/**
 * Validate FFI transfer semantics
 */
bool asthra_safety_validate_ffi_transfer(void *ptr, AsthraTransferType transfer_type);

/**
 * Check FFI pointer ownership
 */
bool asthra_safety_check_ffi_pointer_ownership(void *ptr, pthread_t thread_id);

/**
 * Get FFI pointer statistics
 */
size_t asthra_safety_get_ffi_pointer_count(void);

/**
 * Clean up expired FFI pointer tracking entries
 */
void asthra_safety_cleanup_expired_ffi_pointers(uint64_t max_age_ns);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_FFI_H
