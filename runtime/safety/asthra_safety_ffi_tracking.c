/**
 * Asthra Programming Language Runtime Safety - FFI Safety & Tracking
 * Implementation of FFI pointer tracking, variadic validation, and FFI call logging
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_ffi_tracking.h"
#include "../asthra_safety_common.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

// Use accessor functions instead of direct globals
#define g_safety_config (*asthra_safety_get_config_ptr())
#define g_safety_mutex (*asthra_safety_get_mutex())
#define g_performance_metrics (*asthra_safety_get_metrics_ptr())

// =============================================================================
// FFI POINTER TRACKING IMPLEMENTATION
// =============================================================================

int asthra_safety_unregister_ffi_pointer(void *ptr) {
    if (!g_safety_config.enable_ownership_tracking || !ptr) {
        return 0;
    }

    pthread_mutex_lock(&g_safety_mutex);

    extern AsthraFFIPointerTracker *g_ffi_pointers;
    extern size_t g_ffi_pointer_count;

    for (size_t i = 0; i < g_ffi_pointer_count; i++) {
        if (g_ffi_pointers[i].ptr == ptr) {
            // Decrement reference count
            int old_count = atomic_fetch_sub(&g_ffi_pointers[i].reference_count, 1);

            if (old_count <= 1) {
                // Remove from tracking array
                memmove(&g_ffi_pointers[i], &g_ffi_pointers[i + 1],
                        (g_ffi_pointer_count - i - 1) * sizeof(AsthraFFIPointerTracker));
                g_ffi_pointer_count--;
            }

            pthread_mutex_unlock(&g_safety_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_safety_mutex);

    // Pointer not found - potential double-free or untracked pointer
    asthra_safety_report_violation(ASTHRA_VIOLATION_FFI_SAFETY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                   "Attempting to unregister unknown FFI pointer", __FILE__,
                                   __LINE__, __func__, ptr, sizeof(void *));
    return -1;
}

AsthraFFIPointerTracker *asthra_safety_get_ffi_pointer_info(void *ptr) {
    if (!g_safety_config.enable_ownership_tracking || !ptr) {
        return NULL;
    }

    pthread_mutex_lock(&g_safety_mutex);

    extern AsthraFFIPointerTracker *g_ffi_pointers;
    extern size_t g_ffi_pointer_count;

    for (size_t i = 0; i < g_ffi_pointer_count; i++) {
        if (g_ffi_pointers[i].ptr == ptr) {
            g_ffi_pointers[i].last_access_timestamp = asthra_get_timestamp_ns();
            pthread_mutex_unlock(&g_safety_mutex);
            return &g_ffi_pointers[i];
        }
    }

    pthread_mutex_unlock(&g_safety_mutex);
    return NULL;
}

// =============================================================================
// VARIADIC FUNCTION SAFETY IMPLEMENTATION
// =============================================================================

AsthraVariadicValidation asthra_safety_validate_variadic_call(void *func_ptr, AsthraVarArg *args,
                                                              size_t arg_count,
                                                              AsthraVarArgType *expected_types,
                                                              size_t expected_count) {
    AsthraVariadicValidation validation = {0};

    if (!g_safety_config.enable_variadic_validation) {
        validation.is_valid = true;
        return validation;
    }

    validation.expected_arg_count = expected_count;
    validation.actual_arg_count = arg_count;
    validation.expected_types = expected_types;

    if (!func_ptr) {
        validation.is_valid = false;
        snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                 "Null function pointer for variadic call");
        return validation;
    }

    if (arg_count != expected_count) {
        validation.is_valid = false;
        snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                 "Argument count mismatch: expected %zu, got %zu", expected_count, arg_count);
        return validation;
    }

    if (!args && arg_count > 0) {
        validation.is_valid = false;
        snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                 "Null arguments array with non-zero count");
        return validation;
    }

    // Validate argument types
    for (size_t i = 0; i < arg_count && i < expected_count; i++) {
        if (args[i].type != expected_types[i]) {
            validation.is_valid = false;
            snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                     "Type mismatch at argument %zu: expected %d, got %d", i, expected_types[i],
                     args[i].type);
            return validation;
        }

        // Additional validation for pointer types
        if (args[i].type == ASTHRA_VARARG_PTR && !args[i].value.ptr_val) {
            validation.is_valid = false;
            snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                     "Null pointer value at argument %zu", i);
            return validation;
        }
    }

    validation.is_valid = true;
    return validation;
}

// =============================================================================
// FFI CALL LOGGING IMPLEMENTATION
// =============================================================================

void asthra_safety_log_ffi_call(const char *function_name, void *function_ptr, void **args,
                                size_t arg_count) {
    if (!g_safety_config.enable_ffi_call_logging) {
        return;
    }

    static uint64_t call_id_counter = 0;
    uint64_t call_id = __sync_fetch_and_add(&call_id_counter, 1);

    // Create FFI call log entry
    AsthraFFICallLog call_log = {0};
    call_log.call_id = call_id;
    call_log.function_name = function_name;
    call_log.function_ptr = function_ptr;
    call_log.arguments = args;
    call_log.argument_count = arg_count;
    call_log.call_timestamp_ns = asthra_get_timestamp_ns();
    call_log.is_successful = true; // Assume success unless proven otherwise

    snprintf(call_log.call_details, sizeof(call_log.call_details),
             "FFI call %llu: %s(%p) with %zu arguments", (unsigned long long)call_id,
             function_name ? function_name : "unknown", function_ptr, arg_count);

    // Log to the standard logging system
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_FFI, "FFI call: %s", call_log.call_details);

    // Update performance metrics
    __sync_fetch_and_add(&g_performance_metrics.safety_check_count, 1);
}
