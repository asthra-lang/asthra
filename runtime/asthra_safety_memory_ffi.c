/**
 * Asthra Safety System - Memory and FFI Safety Module
 * FFI annotation verification, pointer tracking, and memory safety checks
 */

#include "asthra_safety_memory_ffi.h"
#include "asthra_runtime.h"
#include "asthra_safety_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// FFI POINTER TRACKING STATE
// =============================================================================

static AsthraFFIPointerTracker *g_ffi_pointers = NULL;
static size_t g_ffi_pointer_count = 0;
static size_t g_ffi_pointer_capacity = 0;

// =============================================================================
// FFI ANNOTATION VERIFICATION IMPLEMENTATION
// =============================================================================

AsthraFFIAnnotationCheck asthra_safety_verify_ffi_annotation(void *func_ptr, void **args,
                                                             size_t arg_count,
                                                             AsthraTransferType *expected_transfers,
                                                             bool *is_borrowed) {
    AsthraFFIAnnotationCheck check = {0};
    AsthraSafetyConfig *config = asthra_safety_get_config_ptr();

    if (!config->enable_ffi_annotation_verification) {
        check.result = ASTHRA_FFI_ANNOTATION_VALID;
        return check;
    }

    if (!func_ptr) {
        check.result = ASTHRA_FFI_ANNOTATION_MISSING;
        snprintf(check.violation_message, sizeof(check.violation_message),
                 "Function pointer is null");
        return check;
    }

    if (!args && arg_count > 0) {
        check.result = ASTHRA_FFI_ANNOTATION_MISMATCH;
        snprintf(check.violation_message, sizeof(check.violation_message),
                 "Arguments array is null but arg_count is %zu", arg_count);
        return check;
    }

    if (!expected_transfers && arg_count > 0) {
        check.result = ASTHRA_FFI_ANNOTATION_MISSING;
        snprintf(check.violation_message, sizeof(check.violation_message),
                 "Expected transfer types array is null");
        return check;
    }

    // Validate each argument's transfer annotation
    for (size_t i = 0; i < arg_count; i++) {
        if (!args[i]) {
            check.result = ASTHRA_FFI_ANNOTATION_MISMATCH;
            check.parameter_index = (int)i;
            snprintf(check.violation_message, sizeof(check.violation_message),
                     "Argument %zu is null pointer", i);
            return check;
        }

        // Check if this pointer is tracked
        AsthraFFIPointerTracker *tracker = asthra_safety_get_ffi_pointer_info(args[i]);
        if (tracker) {
            // Verify transfer type matches
            if (tracker->transfer_type != expected_transfers[i]) {
                check.result = ASTHRA_FFI_ANNOTATION_INVALID_TRANSFER;
                check.parameter_index = (int)i;
                check.expected_transfer = expected_transfers[i];
                check.actual_transfer = tracker->transfer_type;
                snprintf(check.violation_message, sizeof(check.violation_message),
                         "Transfer type mismatch for argument %zu: expected %d, got %d", i,
                         expected_transfers[i], tracker->transfer_type);
                return check;
            }

            // Check borrowing status
            if (is_borrowed && is_borrowed[i] != tracker->is_borrowed) {
                check.result = ASTHRA_FFI_ANNOTATION_LIFETIME_VIOLATION;
                check.parameter_index = (int)i;
                check.is_borrowed = tracker->is_borrowed;
                snprintf(check.violation_message, sizeof(check.violation_message),
                         "Borrowing status mismatch for argument %zu", i);
                return check;
            }

            // Update last access timestamp
            tracker->last_access_timestamp = asthra_get_timestamp_ns();
        }
    }

    check.result = ASTHRA_FFI_ANNOTATION_VALID;
    return check;
}

// =============================================================================
// FFI POINTER TRACKING IMPLEMENTATION
// =============================================================================

int asthra_safety_register_ffi_pointer(void *ptr, size_t size, AsthraTransferType transfer,
                                       AsthraOwnershipHint ownership, bool is_borrowed,
                                       const char *source, int line) {
    AsthraSafetyConfig *config = asthra_safety_get_config_ptr();

    if (!config->enable_ownership_tracking) {
        return 0;
    }

    if (!ptr) {
        return -1;
    }

    pthread_mutex_t *mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);

    // Expand capacity if needed
    if (g_ffi_pointer_count >= g_ffi_pointer_capacity) {
        size_t new_capacity = g_ffi_pointer_capacity == 0 ? 256 : g_ffi_pointer_capacity * 2;
        AsthraFFIPointerTracker *new_pointers =
            realloc(g_ffi_pointers, new_capacity * sizeof(AsthraFFIPointerTracker));
        if (!new_pointers) {
            pthread_mutex_unlock(mutex);
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

    pthread_mutex_unlock(mutex);

    return 0;
}

int asthra_safety_unregister_ffi_pointer(void *ptr) {
    AsthraSafetyConfig *config = asthra_safety_get_config_ptr();

    if (!config->enable_ownership_tracking) {
        return 0;
    }

    if (!ptr) {
        return -1;
    }

    pthread_mutex_t *mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);

    // Find and remove the pointer
    for (size_t i = 0; i < g_ffi_pointer_count; i++) {
        if (g_ffi_pointers[i].ptr == ptr) {
            // Move last element to this position
            if (i < g_ffi_pointer_count - 1) {
                g_ffi_pointers[i] = g_ffi_pointers[g_ffi_pointer_count - 1];
            }
            g_ffi_pointer_count--;
            pthread_mutex_unlock(mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(mutex);

    // Pointer not found - this might be a violation
    asthra_safety_report_violation(ASTHRA_VIOLATION_FFI_SAFETY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                   "Attempted to unregister unknown FFI pointer", __FILE__,
                                   __LINE__, __func__, &ptr, sizeof(ptr));

    return -1;
}

AsthraFFIPointerTracker *asthra_safety_get_ffi_pointer_info(void *ptr) {
    AsthraSafetyConfig *config = asthra_safety_get_config_ptr();

    if (!config->enable_ownership_tracking) {
        return NULL;
    }

    if (!ptr) {
        return NULL;
    }

    pthread_mutex_t *mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);

    // Find the pointer
    for (size_t i = 0; i < g_ffi_pointer_count; i++) {
        if (g_ffi_pointers[i].ptr == ptr) {
            AsthraFFIPointerTracker *tracker = &g_ffi_pointers[i];
            pthread_mutex_unlock(mutex);
            return tracker;
        }
    }

    pthread_mutex_unlock(mutex);
    return NULL;
}

// =============================================================================
// MODULE CLEANUP
// =============================================================================

void asthra_safety_memory_ffi_cleanup(void) {
    pthread_mutex_t *mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);

    // Clean up FFI pointer tracking
    free(g_ffi_pointers);
    g_ffi_pointers = NULL;
    g_ffi_pointer_count = 0;
    g_ffi_pointer_capacity = 0;

    pthread_mutex_unlock(mutex);
}
