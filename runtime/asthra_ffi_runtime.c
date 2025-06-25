/**
 * Asthra Programming Language FFI Runtime v1.2
 * FFI Memory Management, Type System, and Logging
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * IMPLEMENTATION FEATURES:
 * - FFI memory management with transfer semantics
 * - Type system support and registration
 * - Observability and logging implementation
 * - Variadic function support
 */

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asthra_runtime.h"
#include "concurrency/asthra_concurrency_atomics.h"

// =============================================================================
// TYPE SYSTEM SUPPORT
// =============================================================================

typedef struct AsthraTypeInfo {
    uint32_t type_id;
    char name[64];
    size_t size;
    void (*destructor)(void *);
    struct AsthraTypeInfo *next;
} AsthraTypeInfo;

static AsthraTypeInfo *g_type_registry = NULL;
static uint32_t g_next_type_id = ASTHRA_TYPE_RESULT + 1;
static pthread_mutex_t g_type_mutex = PTHREAD_MUTEX_INITIALIZER;

uint32_t asthra_register_type(const char *name, size_t size, void (*destructor)(void *)) {
    pthread_mutex_lock(&g_type_mutex);

    AsthraTypeInfo *type_info = malloc(sizeof(AsthraTypeInfo));
    if (!type_info) {
        pthread_mutex_unlock(&g_type_mutex);
        return 0;
    }

    type_info->type_id = g_next_type_id++;
    strncpy(type_info->name, name, sizeof(type_info->name) - 1);
    type_info->name[sizeof(type_info->name) - 1] = '\0';
    type_info->size = size;
    type_info->destructor = destructor;
    type_info->next = g_type_registry;
    g_type_registry = type_info;

    uint32_t type_id = type_info->type_id;
    pthread_mutex_unlock(&g_type_mutex);

    return type_id;
}

const char *asthra_get_type_name(uint32_t type_id) {
    pthread_mutex_lock(&g_type_mutex);

    AsthraTypeInfo *current = g_type_registry;
    while (current) {
        if (current->type_id == type_id) {
            const char *name = current->name;
            pthread_mutex_unlock(&g_type_mutex);
            return name;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&g_type_mutex);
    return NULL;
}

size_t asthra_get_type_size(uint32_t type_id) {
    pthread_mutex_lock(&g_type_mutex);

    AsthraTypeInfo *current = g_type_registry;
    while (current) {
        if (current->type_id == type_id) {
            size_t size = current->size;
            pthread_mutex_unlock(&g_type_mutex);
            return size;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&g_type_mutex);
    return 0;
}

// =============================================================================
// FFI MEMORY MANAGEMENT IMPLEMENTATION
// =============================================================================

void *asthra_ffi_alloc(size_t size, AsthraTransferType transfer) {
    AsthraMemoryZone zone =
        (transfer == ASTHRA_TRANSFER_FULL) ? ASTHRA_ZONE_MANUAL : ASTHRA_ZONE_GC;

    // TODO: Update FFI stats when stats system is available
    // asthra_atomic_stats_update_ffi_call();

    return asthra_alloc(size, zone);
}

void asthra_ffi_free(void *ptr, AsthraTransferType transfer) {
    AsthraMemoryZone zone;
    switch (transfer) {
    case ASTHRA_TRANSFER_FULL:
        zone = ASTHRA_ZONE_GC;
        break;
    case ASTHRA_TRANSFER_NONE:
        zone = ASTHRA_ZONE_MANUAL;
        break;
    case ASTHRA_TRANSFER_BORROWED:
        zone = ASTHRA_ZONE_PINNED;
        break;
    default:
        zone = ASTHRA_ZONE_MANUAL;
        break;
    }

    asthra_free(ptr, zone);
}

int asthra_ffi_register_external(void *ptr, size_t size, void (*cleanup)(void *)) {
    // TODO: Implement external pointer registration
    (void)ptr;
    (void)size;
    (void)cleanup;
    return 0;
}

void asthra_ffi_unregister_external(void *ptr) {
    // TODO: Implement external pointer unregistration
    (void)ptr;
}

// =============================================================================
// VARIADIC FUNCTION SUPPORT IMPLEMENTATION
// =============================================================================

int asthra_call_variadic_int(void *func_ptr, AsthraVarArg *args, size_t arg_count) {
    // TODO: Implement variadic function calls
    // This is a complex feature that requires platform-specific assembly
    // or libffi integration
    (void)func_ptr;
    (void)args;
    (void)arg_count;
    return 0;
}

void *asthra_call_variadic_ptr(void *func_ptr, AsthraVarArg *args, size_t arg_count) {
    // TODO: Implement variadic function calls
    (void)func_ptr;
    (void)args;
    (void)arg_count;
    return NULL;
}

double asthra_call_variadic_double(void *func_ptr, AsthraVarArg *args, size_t arg_count) {
    // TODO: Implement variadic function calls
    (void)func_ptr;
    (void)args;
    (void)arg_count;
    return 0.0;
}

// =============================================================================
// OBSERVABILITY AND LOGGING IMPLEMENTATION
// =============================================================================

static const char *log_level_names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static const char *log_category_names[] = {"GENERAL",     "MEMORY",   "GC",         "FFI",
                                           "CONCURRENCY", "SECURITY", "PERFORMANCE"};

void asthra_log(AsthraLogLevel level, AsthraLogCategory category, const char *format, ...) {
    printf("[%s:%s] ", log_level_names[level], log_category_names[category]);

    va_list args;
    va_start(args, format);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    vprintf(format, args);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    va_end(args);

    printf("\n");
}

void asthra_log_va(AsthraLogLevel level, AsthraLogCategory category, const char *format,
                   va_list args) {
    printf("[%s:%s] ", log_level_names[level], log_category_names[category]);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    vprintf(format, args);
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    printf("\n");
}

void asthra_log_set_level(AsthraLogLevel min_level) {
    // TODO: Implement log level filtering
    (void)min_level;
}

void asthra_log_enable_category(AsthraLogCategory category, bool enabled) {
    // TODO: Implement category filtering
    (void)category;
    (void)enabled;
}

void asthra_log_ffi_calls(void) {
    printf("[FFI] FFI call statistics:\n");
    AsthraRuntimeStats stats = asthra_get_runtime_stats();
    printf("[FFI]   Total calls: %" PRIu64 "\n", ASTHRA_ATOMIC_LOAD(&stats.ffi_calls));
}

// =============================================================================
// ERROR STRING CONVERSION
// =============================================================================

const char *asthra_error_string(AsthraErrorCode code) {
    switch (code) {
    case ASTHRA_ERROR_NONE:
        return "No error";
    case ASTHRA_ERROR_OUT_OF_MEMORY:
        return "Out of memory";
    case ASTHRA_ERROR_INVALID_ARGUMENT:
        return "Invalid argument";
    case ASTHRA_ERROR_NULL_POINTER:
        return "Null pointer";
    case ASTHRA_ERROR_BOUNDS_CHECK:
        return "Bounds check failed";
    case ASTHRA_ERROR_TYPE_MISMATCH:
        return "Type mismatch";
    case ASTHRA_ERROR_OWNERSHIP_VIOLATION:
        return "Ownership violation";
    case ASTHRA_ERROR_THREAD_ERROR:
        return "Thread error";
    case ASTHRA_ERROR_IO_ERROR:
        return "I/O error";
    case ASTHRA_ERROR_CRYPTO_ERROR:
        return "Cryptographic error";
    case ASTHRA_ERROR_RUNTIME_ERROR:
        return "Runtime error";
    default:
        return "Unknown error";
    }
}

// =============================================================================
// RUNTIME STATISTICS RESET
// =============================================================================

void asthra_reset_runtime_stats(void) {
    // TODO: Implement proper stats reset
    // Note: This function needs to be reimplemented to work with
    // the actual runtime stats storage mechanism
    // The current asthra_get_runtime_stats() returns by value, not reference
    printf("[FFI] Warning: asthra_reset_runtime_stats not implemented\n");
}

// =============================================================================
// FFI SLICE OPERATIONS IMPLEMENTATION
// =============================================================================

AsthraSliceHeader asthra_slice_new(size_t element_size, size_t len, AsthraOwnershipHint ownership) {
    if (element_size == 0 || len == 0) {
        // Return empty slice using C17 compound literal
        return (AsthraSliceHeader){.ptr = NULL,
                                   .len = 0,
                                   .cap = 0,
                                   .element_size = element_size,
                                   .ownership = ownership,
                                   .is_mutable = false,
                                   .type_id = ASTHRA_TYPE_SLICE};
    }

    size_t total_size = element_size * len;
    AsthraMemoryZone zone =
        (ownership == ASTHRA_OWNERSHIP_GC) ? ASTHRA_ZONE_GC : ASTHRA_ZONE_MANUAL;
    void *restrict ptr = asthra_alloc(total_size, zone);

    if (!ptr) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_OUT_OF_MEMORY, "Failed to allocate slice memory");
        return (AsthraSliceHeader){0}; // C17 zero initialization
    }

    // Use C17 compound literal for initialization
    return (AsthraSliceHeader){.ptr = ptr,
                               .len = len,
                               .cap = len,
                               .element_size = element_size,
                               .ownership = ownership,
                               .is_mutable = true,
                               .type_id = ASTHRA_TYPE_SLICE};
}

AsthraSliceHeader asthra_slice_from_raw_parts(void *restrict ptr, size_t len, size_t element_size,
                                              bool is_mutable, AsthraOwnershipHint ownership) {
    // Use C17 compound literal with designated initializers
    return (AsthraSliceHeader){.ptr = ptr,
                               .len = len,
                               .cap = len,
                               .element_size = element_size,
                               .ownership = ownership,
                               .is_mutable = is_mutable,
                               .type_id = ASTHRA_TYPE_SLICE};
}

AsthraSliceHeader asthra_slice_subslice(AsthraSliceHeader slice, size_t start, size_t end) {
    if (start > end || end > slice.len) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_BOUNDS_CHECK, "Invalid subslice bounds");
        return (AsthraSliceHeader){0}; // C17 zero initialization
    }

    // Calculate new pointer with restrict qualifier
    char *restrict base_ptr = (char *restrict)slice.ptr;
    void *restrict new_ptr = base_ptr + (start * slice.element_size);

    // Use C17 compound literal for subslice
    return (AsthraSliceHeader){.ptr = new_ptr,
                               .len = end - start,
                               .cap = slice.cap - start,
                               .element_size = slice.element_size,
                               .ownership = ASTHRA_OWNERSHIP_C, // Subslices don't own memory
                               .is_mutable = slice.is_mutable,
                               .type_id = slice.type_id};
}

void asthra_slice_free(AsthraSliceHeader slice) {
    if (slice.ptr && slice.ownership == ASTHRA_OWNERSHIP_GC) {
        AsthraMemoryZone zone = ASTHRA_ZONE_GC;
        asthra_free(slice.ptr, zone);
    }
}

bool asthra_slice_bounds_check(AsthraSliceHeader slice, size_t index) {
    return index < slice.len;
}

void *asthra_slice_get_element(AsthraSliceHeader slice, size_t index) {
    if (!asthra_slice_bounds_check(slice, index)) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_BOUNDS_CHECK, "Slice index out of bounds");
        return NULL;
    }

    char *restrict base_ptr = (char *restrict)slice.ptr;
    return base_ptr + (index * slice.element_size);
}

int asthra_slice_set_element(AsthraSliceHeader slice, size_t index, const void *restrict element) {
    if (!slice.is_mutable) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_INVALID_ARGUMENT, "Cannot modify immutable slice");
        return -1;
    }

    if (!asthra_slice_bounds_check(slice, index)) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_BOUNDS_CHECK, "Slice index out of bounds");
        return -1;
    }

    char *restrict base_ptr = (char *restrict)slice.ptr;
    void *restrict target = base_ptr + (index * slice.element_size);
    memcpy(target, element, slice.element_size);
    return 0;
}

// Slice accessor functions with restrict qualifiers
void *asthra_slice_get_ptr(AsthraSliceHeader slice) {
    return slice.ptr;
}

size_t asthra_slice_get_len(AsthraSliceHeader slice) {
    return slice.len;
}
size_t asthra_slice_get_cap(AsthraSliceHeader slice) {
    return slice.cap;
}
size_t asthra_slice_get_element_size(AsthraSliceHeader slice) {
    return slice.element_size;
}
