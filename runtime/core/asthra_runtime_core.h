/**
 * Asthra Programming Language Runtime v1.2 - Core Module
 * Core Types, Version Information, and C17 Feature Detection
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides the foundational types and feature detection
 * for the Asthra runtime system.
 */

#ifndef ASTHRA_RUNTIME_CORE_H
#define ASTHRA_RUNTIME_CORE_H

#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// C17 modernization includes
#include <stdalign.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// VERSION AND BUILD INFORMATION
// =============================================================================

#define ASTHRA_RUNTIME_VERSION_MAJOR 1
#define ASTHRA_RUNTIME_VERSION_MINOR 0
#define ASTHRA_RUNTIME_VERSION_PATCH 0
#define ASTHRA_RUNTIME_VERSION "1.0.0"

// =============================================================================
// C17 FEATURE DETECTION
// =============================================================================

// Feature detection for C17 capabilities
#if __STDC_VERSION__ >= 201710L
#define ASTHRA_HAS_C17 1
#define ASTHRA_HAS_ALIGNED_ALLOC 1
#define ASTHRA_HAS_ATOMICS 1
// Check for threads.h availability (not available on all platforms)
#if defined(__STDC_NO_THREADS__) || defined(__APPLE__)
#define ASTHRA_HAS_THREAD_LOCAL 1
#define ASTHRA_HAS_C11_THREADS 0
#else
#define ASTHRA_HAS_THREAD_LOCAL 1
#define ASTHRA_HAS_C11_THREADS 1
#include <threads.h>
#endif
#else
#define ASTHRA_HAS_C17 0
#define ASTHRA_HAS_ALIGNED_ALLOC 0
#define ASTHRA_HAS_THREAD_LOCAL 0
#define ASTHRA_HAS_ATOMICS 0
#define ASTHRA_HAS_C11_THREADS 0
#endif

// C17 thread-local storage for GC optimization
#if ASTHRA_HAS_THREAD_LOCAL
#define ASTHRA_THREAD_LOCAL _Thread_local
#else
#define ASTHRA_THREAD_LOCAL __thread
#endif

// C17 static assertions for compile-time validation
#define ASTHRA_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

// Compile-time validation of version consistency
ASTHRA_STATIC_ASSERT(ASTHRA_RUNTIME_VERSION_MAJOR >= 1, "Major version must be at least 1");
ASTHRA_STATIC_ASSERT(ASTHRA_RUNTIME_VERSION_MINOR >= 0, "Minor version must be non-negative");

// =============================================================================
// C17 ATOMIC TYPE DEFINITIONS
// =============================================================================

// C17 atomic types for thread-safe counters
#if ASTHRA_HAS_ATOMICS
typedef atomic_size_t asthra_atomic_size_t;
typedef atomic_uint_fast64_t asthra_atomic_counter_t;
typedef atomic_bool asthra_atomic_bool;
typedef atomic_flag asthra_atomic_flag;
#else
typedef size_t asthra_atomic_size_t;
typedef uint_fast64_t asthra_atomic_counter_t;
typedef bool asthra_atomic_bool;
typedef struct {
    bool flag;
} asthra_atomic_flag;
#endif

// =============================================================================
// CORE TYPE DEFINITIONS
// =============================================================================

// Memory ownership and zone management
typedef enum {
    ASTHRA_OWNERSHIP_GC,    // Asthra-managed (garbage collected)
    ASTHRA_OWNERSHIP_C,     // C-managed (manual, GC ignores)
    ASTHRA_OWNERSHIP_PINNED // Pinned memory (GC observes, C manages)
} AsthraOwnershipHint;

typedef enum {
    ASTHRA_ZONE_GC,     // GC-managed heap
    ASTHRA_ZONE_MANUAL, // Manual memory zone (C-compatible)
    ASTHRA_ZONE_PINNED, // Pinned memory zone
    ASTHRA_ZONE_STACK   // Stack-allocated (temporary)
} AsthraMemoryZone;

// FFI transfer semantics
typedef enum {
    ASTHRA_TRANSFER_FULL,    // Full ownership transfer
    ASTHRA_TRANSFER_NONE,    // No ownership transfer
    ASTHRA_TRANSFER_BORROWED // Borrowed reference (temporary)
} AsthraTransferType;

// =============================================================================
// BUILT-IN TYPE SYSTEM SUPPORT
// =============================================================================

// Built-in type IDs
#define ASTHRA_TYPE_VOID 0
#define ASTHRA_TYPE_SLICE 1
#define ASTHRA_TYPE_STRING 2
#define ASTHRA_TYPE_RESULT 3
#define ASTHRA_TYPE_BOOL 4
#define ASTHRA_TYPE_I8 5
#define ASTHRA_TYPE_U8 6
#define ASTHRA_TYPE_I16 7
#define ASTHRA_TYPE_U16 8
#define ASTHRA_TYPE_I32 9
#define ASTHRA_TYPE_U32 10
#define ASTHRA_TYPE_I64 11
#define ASTHRA_TYPE_U64 12
#define ASTHRA_TYPE_F32 13
#define ASTHRA_TYPE_F64 14

// Type system functions
uint32_t asthra_register_type(const char *name, size_t size, void (*destructor)(void *));
const char *asthra_get_type_name(uint32_t type_id);
size_t asthra_get_type_size(uint32_t type_id);

// =============================================================================
// RUNTIME INFORMATION FUNCTIONS
// =============================================================================

// Runtime information
const char *asthra_runtime_version(void);
const char *asthra_runtime_build_info(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_CORE_H
