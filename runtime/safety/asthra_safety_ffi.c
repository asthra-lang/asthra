/**
 * Asthra Programming Language Runtime Safety System
 * FFI Safety and Tracking Module - Foreign Function Interface safety implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_ffi.h"
#include "../asthra_safety_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External references to global state
// Use accessor function instead of direct global
#define g_safety_config (*asthra_safety_get_config_ptr())
extern pthread_mutex_t g_safety_mutex;
extern AsthraFFIPointerTracker *g_ffi_pointers;
extern size_t g_ffi_pointer_count;
extern size_t g_ffi_pointer_capacity;

// =============================================================================
// FFI SAFETY AND TRACKING IMPLEMENTATION
// =============================================================================

AsthraFFIAnnotationCheck asthra_safety_verify_ffi_annotation(void *func_ptr, void **args,
                                                             size_t arg_count,
                                                             AsthraTransferType *expected_transfers,
                                                             bool *is_borrowed) {
    AsthraFFIAnnotationCheck check = {0};

    if (!g_safety_config.enable_ffi_annotation_verification) {
        check.result = ASTHRA_FFI_ANNOTATION_VALID;
        return check;
    }

    if (!func_ptr) {
        check.result = ASTHRA_FFI_ANNOTATION_INVALID_TRANSFER;
        snprintf(check.violation_message, sizeof(check.violation_message), "Null function pointer");
        return check;
    }

    if (arg_count > 0 && (!args || !expected_transfers || !is_borrowed)) {
        check.result = ASTHRA_FFI_ANNOTATION_MISSING;
        snprintf(check.violation_message, sizeof(check.violation_message),
                 "Missing annotation information for %zu arguments", arg_count);
        return check;
    }

    // Validate each argument's transfer semantics
    for (size_t i = 0; i < arg_count; i++) {
        if (!args[i] && expected_transfers[i] != ASTHRA_TRANSFER_NONE) {
            check.result = ASTHRA_FFI_ANNOTATION_LIFETIME_VIOLATION;
            check.parameter_index = (int)i;
            snprintf(check.violation_message, sizeof(check.violation_message),
                     "Null pointer for parameter %zu with transfer semantics", i);
            return check;
        }

        // Check if borrowed pointers are being transferred
        if (is_borrowed[i] && expected_transfers[i] == ASTHRA_TRANSFER_FULL) {
            check.result = ASTHRA_FFI_ANNOTATION_LIFETIME_VIOLATION;
            check.parameter_index = (int)i;
            snprintf(check.violation_message, sizeof(check.violation_message),
                     "Attempting to transfer ownership of borrowed pointer at parameter %zu", i);
            return check;
        }
    }

    check.result = ASTHRA_FFI_ANNOTATION_VALID;
    return check;
}

int asthra_safety_register_ffi_pointer(void *ptr, size_t size, AsthraTransferType transfer,
                                       AsthraOwnershipHint ownership, bool is_borrowed,
                                       const char *source, int line) {
    if (!g_safety_config.enable_ownership_tracking) {
        return 0;
    }

    pthread_mutex_lock(&g_safety_mutex);

    // Expand capacity if needed
    if (g_ffi_pointer_count >= g_ffi_pointer_capacity) {
        size_t new_capacity = g_ffi_pointer_capacity == 0 ? 64 : g_ffi_pointer_capacity * 2;
        AsthraFFIPointerTracker *new_pointers =
            realloc(g_ffi_pointers, new_capacity * sizeof(AsthraFFIPointerTracker));
        if (!new_pointers) {
            pthread_mutex_unlock(&g_safety_mutex);
            return -1;
        }
        g_ffi_pointers = new_pointers;
        g_ffi_pointer_capacity = new_capacity;
    }

    // Register the pointer
    AsthraFFIPointerTracker *tracker = &g_ffi_pointers[g_ffi_pointer_count++];
    tracker->ptr = ptr;
    tracker->size = size;
    tracker->transfer_type = transfer;
    tracker->ownership = ownership;
    tracker->is_borrowed = is_borrowed;
    tracker->allocation_timestamp = asthra_get_timestamp_ns();
    tracker->last_access_timestamp = tracker->allocation_timestamp;
    tracker->allocation_source = source;
    tracker->allocation_line = line;
    tracker->owning_thread = pthread_self();
    atomic_init(&tracker->reference_count, 1);

    pthread_mutex_unlock(&g_safety_mutex);
    return 0;
}

int asthra_safety_unregister_ffi_pointer(void *ptr) {
    if (!g_safety_config.enable_ownership_tracking || !ptr) {
        return 0;
    }

    pthread_mutex_lock(&g_safety_mutex);

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
                 "Null function pointer");
        return validation;
    }

    if (arg_count != expected_count) {
        validation.is_valid = false;
        snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                 "Argument count mismatch: expected %zu, got %zu", expected_count, arg_count);
        return validation;
    }

    if (!args || !expected_types) {
        validation.is_valid = false;
        snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                 "Null arguments or expected types array");
        return validation;
    }

    // Validate argument types
    for (size_t i = 0; i < arg_count; i++) {
        if (args[i].type != expected_types[i]) {
            validation.is_valid = false;
            snprintf(validation.type_mismatch_details, sizeof(validation.type_mismatch_details),
                     "Type mismatch at argument %zu: expected %d, got %d", i, expected_types[i],
                     args[i].type);
            return validation;
        }
    }

    validation.is_valid = true;
    return validation;
}

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
}

bool asthra_safety_is_ffi_pointer_valid(void *ptr) {
    if (!g_safety_config.enable_ownership_tracking || !ptr) {
        return false;
    }

    AsthraFFIPointerTracker *tracker = asthra_safety_get_ffi_pointer_info(ptr);
    return tracker != NULL;
}

void asthra_safety_update_ffi_pointer_access(void *ptr) {
    if (!g_safety_config.enable_ownership_tracking || !ptr) {
        return;
    }

    pthread_mutex_lock(&g_safety_mutex);

    for (size_t i = 0; i < g_ffi_pointer_count; i++) {
        if (g_ffi_pointers[i].ptr == ptr) {
            g_ffi_pointers[i].last_access_timestamp = asthra_get_timestamp_ns();
            break;
        }
    }

    pthread_mutex_unlock(&g_safety_mutex);
}

bool asthra_safety_validate_ffi_transfer(void *ptr, AsthraTransferType transfer_type) {
    if (!g_safety_config.enable_ownership_tracking || !ptr) {
        return true; // Allow if tracking is disabled
    }

    AsthraFFIPointerTracker *tracker = asthra_safety_get_ffi_pointer_info(ptr);
    if (!tracker) {
        return false; // Untracked pointer
    }

    // Check if transfer is compatible with current tracking
    if (tracker->is_borrowed && transfer_type == ASTHRA_TRANSFER_FULL) {
        return false; // Cannot transfer borrowed pointer
    }

    return true;
}

bool asthra_safety_check_ffi_pointer_ownership(void *ptr, pthread_t thread_id) {
    if (!g_safety_config.enable_ownership_tracking || !ptr) {
        return true; // Allow if tracking is disabled
    }

    AsthraFFIPointerTracker *tracker = asthra_safety_get_ffi_pointer_info(ptr);
    if (!tracker) {
        return false; // Untracked pointer
    }

    return pthread_equal(tracker->owning_thread, thread_id);
}

size_t asthra_safety_get_ffi_pointer_count(void) {
    if (!g_safety_config.enable_ownership_tracking) {
        return 0;
    }

    pthread_mutex_lock(&g_safety_mutex);
    size_t count = g_ffi_pointer_count;
    pthread_mutex_unlock(&g_safety_mutex);

    return count;
}

void asthra_safety_cleanup_expired_ffi_pointers(uint64_t max_age_ns) {
    if (!g_safety_config.enable_ownership_tracking) {
        return;
    }

    pthread_mutex_lock(&g_safety_mutex);

    uint64_t current_time = asthra_get_timestamp_ns();
    size_t write_index = 0;

    for (size_t read_index = 0; read_index < g_ffi_pointer_count; read_index++) {
        AsthraFFIPointerTracker *tracker = &g_ffi_pointers[read_index];
        uint64_t age = current_time - tracker->last_access_timestamp;

        if (age <= max_age_ns) {
            // Keep this pointer
            if (write_index != read_index) {
                g_ffi_pointers[write_index] = g_ffi_pointers[read_index];
            }
            write_index++;
        }
        // Otherwise, skip this pointer (effectively removing it)
    }

    g_ffi_pointer_count = write_index;

    pthread_mutex_unlock(&g_safety_mutex);
}
