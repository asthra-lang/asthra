/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration with C17 Modernization
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 5.2: Static Analysis Integration
 * - C17 annotation attributes for static analysis hints
 * - Extensive _Static_assert for compile-time validation
 * - Improved const correctness with C17 patterns
 * - Restrict pointer annotations for better analysis
 */

#ifndef ASTHRA_STATIC_ANALYSIS_H
#define ASTHRA_STATIC_ANALYSIS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdarg.h>

// C17 feature detection for static analysis
#if __STDC_VERSION__ >= 201710L
#define ASTHRA_C17_AVAILABLE 1
#else
#define ASTHRA_C17_AVAILABLE 0
#endif

// C17 static assertions for static analysis assumptions
#if ASTHRA_C17_AVAILABLE
#include <assert.h>
#define ASTHRA_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define ASTHRA_STATIC_ASSERT(cond, msg) /* No static assert support */
#endif

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 ANNOTATION ATTRIBUTES FOR STATIC ANALYSIS
// =============================================================================

// Compiler-specific attribute detection
#ifdef __GNUC__
    #define ASTHRA_HAS_GCC_ATTRIBUTES 1
#else
    #define ASTHRA_HAS_GCC_ATTRIBUTES 0
#endif

#ifdef __clang__
    #define ASTHRA_HAS_CLANG_ATTRIBUTES 1
#else
    #define ASTHRA_HAS_CLANG_ATTRIBUTES 0
#endif

// Function attributes for static analysis
#if ASTHRA_HAS_GCC_ATTRIBUTES || ASTHRA_HAS_CLANG_ATTRIBUTES
    #define ASTHRA_PURE                __attribute__((pure))
    #define ASTHRA_CONST               __attribute__((const))
    #define ASTHRA_MALLOC              __attribute__((malloc))
    #define ASTHRA_NONNULL(...)        __attribute__((nonnull(__VA_ARGS__)))
    #define ASTHRA_RETURNS_NONNULL     __attribute__((returns_nonnull))
    #define ASTHRA_WARN_UNUSED_RESULT  __attribute__((warn_unused_result))
    #define ASTHRA_NORETURN            __attribute__((noreturn))
    #define ASTHRA_DEPRECATED(msg)     __attribute__((deprecated(msg)))
    #define ASTHRA_LIKELY(x)           __builtin_expect(!!(x), 1)
    #define ASTHRA_UNLIKELY(x)         __builtin_expect(!!(x), 0)
#else
    #define ASTHRA_PURE
    #define ASTHRA_CONST
    #define ASTHRA_MALLOC
    #define ASTHRA_NONNULL(...)
    #define ASTHRA_RETURNS_NONNULL
    #define ASTHRA_WARN_UNUSED_RESULT
    #define ASTHRA_NORETURN
    #define ASTHRA_DEPRECATED(msg)
    #define ASTHRA_LIKELY(x)           (x)
    #define ASTHRA_UNLIKELY(x)         (x)
#endif

// Memory safety attributes
#if ASTHRA_HAS_CLANG_ATTRIBUTES
    #define ASTHRA_NO_SANITIZE(x)      __attribute__((no_sanitize(x)))
    #define ASTHRA_FALLTHROUGH         __attribute__((fallthrough))
    #define ASTHRA_ASSUME_ALIGNED(n)   __attribute__((assume_aligned(n)))
#else
    #define ASTHRA_NO_SANITIZE(x)
    #define ASTHRA_FALLTHROUGH
    #define ASTHRA_ASSUME_ALIGNED(n)
#endif

// Ownership and lifetime annotations
#define ASTHRA_OWNER               /* Indicates ownership transfer */
#define ASTHRA_BORROWED            /* Indicates borrowed reference */
#define ASTHRA_SHARED              /* Indicates shared ownership */
#define ASTHRA_UNIQUE              /* Indicates unique ownership */

// Thread safety annotations
#define ASTHRA_THREAD_SAFE         /* Function is thread-safe */
#define ASTHRA_NOT_THREAD_SAFE     /* Function is not thread-safe */
#define ASTHRA_REQUIRES_LOCK(...)  /* Requires specific locks */
#define ASTHRA_EXCLUDES_LOCK(...)  /* Must not hold specific locks */

// =============================================================================
// EXTENSIVE STATIC ASSERTIONS FOR COMPILE-TIME VALIDATION
// =============================================================================

// Platform and architecture assumptions
ASTHRA_STATIC_ASSERT(sizeof(void*) == 8, "64-bit architecture required for Asthra");
ASTHRA_STATIC_ASSERT(sizeof(size_t) == sizeof(void*), "size_t must match pointer size");
ASTHRA_STATIC_ASSERT(sizeof(ptrdiff_t) == sizeof(void*), "ptrdiff_t must match pointer size");
ASTHRA_STATIC_ASSERT(sizeof(uintptr_t) == sizeof(void*), "uintptr_t must match pointer size");

// Integer type size assumptions
ASTHRA_STATIC_ASSERT(sizeof(char) == 1, "char must be 1 byte");
ASTHRA_STATIC_ASSERT(sizeof(short) >= 2, "short must be at least 2 bytes");
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 4 bytes");
ASTHRA_STATIC_ASSERT(sizeof(long) >= 8, "long must be at least 8 bytes on 64-bit");
ASTHRA_STATIC_ASSERT(sizeof(long long) >= 8, "long long must be at least 8 bytes");

// Floating point assumptions
ASTHRA_STATIC_ASSERT(sizeof(float) == 4, "float must be 4 bytes (IEEE 754)");
ASTHRA_STATIC_ASSERT(sizeof(double) == 8, "double must be 8 bytes (IEEE 754)");

// Boolean type assumptions
ASTHRA_STATIC_ASSERT(sizeof(bool) == 1, "bool must be 1 byte");
ASTHRA_STATIC_ASSERT(sizeof(_Bool) == 1, "_Bool must be 1 byte");

// Atomic type assumptions
#if ASTHRA_C17_AVAILABLE && !defined(__STDC_NO_ATOMICS__)
ASTHRA_STATIC_ASSERT(sizeof(atomic_bool) >= sizeof(bool), "atomic_bool size");
ASTHRA_STATIC_ASSERT(sizeof(atomic_int) >= sizeof(int), "atomic_int size");
ASTHRA_STATIC_ASSERT(sizeof(atomic_size_t) >= sizeof(size_t), "atomic_size_t size");
ASTHRA_STATIC_ASSERT(sizeof(atomic_uintptr_t) >= sizeof(uintptr_t), "atomic_uintptr_t size");
#endif

// Alignment assumptions
ASTHRA_STATIC_ASSERT(_Alignof(max_align_t) >= 8, "max_align_t must be at least 8-byte aligned");
ASTHRA_STATIC_ASSERT(_Alignof(void*) >= 8, "pointer alignment must be at least 8 bytes");

// Character encoding assumptions
ASTHRA_STATIC_ASSERT('\0' == 0, "null character must be zero");
ASTHRA_STATIC_ASSERT('0' == 48, "ASCII digit zero assumption");
ASTHRA_STATIC_ASSERT('A' == 65, "ASCII uppercase A assumption");
ASTHRA_STATIC_ASSERT('a' == 97, "ASCII lowercase a assumption");

// Endianness detection (compile-time)
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define ASTHRA_LITTLE_ENDIAN 1
        #define ASTHRA_BIG_ENDIAN 0
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define ASTHRA_LITTLE_ENDIAN 0
        #define ASTHRA_BIG_ENDIAN 1
    #else
        #error "Unknown byte order"
    #endif
#else
    // Fallback endianness detection
    #define ASTHRA_LITTLE_ENDIAN 1
    #define ASTHRA_BIG_ENDIAN 0
#endif

// =============================================================================
// CONST CORRECTNESS IMPROVEMENTS WITH C17 PATTERNS
// =============================================================================

// Const-correct string operations
ASTHRA_PURE ASTHRA_NONNULL(1)
size_t asthra_string_length(const char *restrict str);

ASTHRA_PURE ASTHRA_NONNULL(1, 2)
int asthra_string_compare(const char *restrict str1, const char *restrict str2);

ASTHRA_NONNULL(1, 2)
char *asthra_string_copy(char *restrict dest, const char *restrict src, size_t dest_size);

ASTHRA_NONNULL(1, 2)
char *asthra_string_concatenate(char *restrict dest, const char *restrict src, 
                               size_t dest_size, size_t dest_used);

// Const-correct memory operations
ASTHRA_PURE ASTHRA_NONNULL(1, 2)
int asthra_memory_compare(const void *restrict ptr1, const void *restrict ptr2, size_t size);

ASTHRA_NONNULL(1, 2)
void *asthra_memory_copy(void *restrict dest, const void *restrict src, size_t size);

ASTHRA_NONNULL(1)
void *asthra_memory_set(void *restrict ptr, int value, size_t size);

ASTHRA_NONNULL(1, 2)
void *asthra_memory_move(void *dest, const void *src, size_t size);

// Const-correct array operations
ASTHRA_PURE ASTHRA_NONNULL(1)
size_t asthra_array_length(const void *const *restrict array);

ASTHRA_PURE ASTHRA_NONNULL(1, 3)
const void *asthra_array_find(const void *const *restrict array, size_t count,
                              const void *restrict element,
                              int (*compare)(const void *, const void *));

ASTHRA_NONNULL(1, 3)
void asthra_array_sort(void **restrict array, size_t count,
                      int (*compare)(const void *, const void *));

// =============================================================================
// RESTRICT POINTER ANNOTATIONS FOR BETTER ANALYSIS
// =============================================================================

// Memory allocation with restrict annotations
ASTHRA_MALLOC ASTHRA_WARN_UNUSED_RESULT
void *asthra_alloc_aligned(size_t size, size_t alignment) ASTHRA_ASSUME_ALIGNED(16);

ASTHRA_MALLOC ASTHRA_WARN_UNUSED_RESULT
void *asthra_alloc_array(size_t element_size, size_t count);

ASTHRA_WARN_UNUSED_RESULT ASTHRA_NONNULL(1)
void *asthra_realloc_array(void *restrict ptr, size_t old_count, 
                          size_t new_count, size_t element_size);

ASTHRA_NONNULL(1)
void asthra_free_aligned(void *restrict ptr);

// Buffer operations with restrict annotations
ASTHRA_NONNULL(1, 3)
size_t asthra_buffer_copy(char *restrict dest, size_t dest_size,
                         const char *restrict src, size_t src_size);

ASTHRA_NONNULL(1, 4)
size_t asthra_buffer_append(char *restrict dest, size_t dest_size, size_t dest_used,
                           const char *restrict src, size_t src_size);

ASTHRA_PURE ASTHRA_NONNULL(1, 3)
bool asthra_buffer_equal(const char *restrict buf1, size_t size1,
                        const char *restrict buf2, size_t size2);

// String formatting with restrict annotations
ASTHRA_NONNULL(1, 3)
int asthra_format_string(char *restrict buffer, size_t buffer_size,
                        const char *restrict format, ...);

ASTHRA_NONNULL(1, 3)
int asthra_format_string_va(char *restrict buffer, size_t buffer_size,
                           const char *restrict format, va_list args);

// =============================================================================
// STATIC ANALYSIS HELPER MACROS
// =============================================================================

// Assertion macros for static analysis
#define ASTHRA_ASSERT_NOT_NULL(ptr) \
    ASTHRA_STATIC_ASSERT(sizeof(ptr) == sizeof(void*), "Must be pointer type")

#define ASTHRA_ASSERT_ARRAY_SIZE(array, expected_size) \
    ASTHRA_STATIC_ASSERT(sizeof(array) / sizeof((array)[0]) == (expected_size), \
                        "Array size mismatch")

#define ASTHRA_ASSERT_STRUCT_SIZE(type, expected_size) \
    ASTHRA_STATIC_ASSERT(sizeof(type) == (expected_size), \
                        "Struct size changed - update dependent code")

#define ASTHRA_ASSERT_ENUM_SIZE(type, max_value) \
    ASTHRA_STATIC_ASSERT((max_value) < 256, "Enum too large for uint8_t storage")

// Memory safety macros
#define ASTHRA_BOUNDS_CHECK(index, size) \
    do { \
        if (ASTHRA_UNLIKELY((index) >= (size))) { \
            asthra_abort_with_message("Array bounds violation"); \
        } \
    } while (0)

#define ASTHRA_NULL_CHECK(ptr) \
    do { \
        if (ASTHRA_UNLIKELY((ptr) == NULL)) { \
            asthra_abort_with_message("Null pointer dereference"); \
        } \
    } while (0)

// =============================================================================
// STATIC ANALYSIS CONFIGURATION
// =============================================================================

// Static analysis tool configuration
typedef struct {
    bool enable_bounds_checking;
    bool enable_null_checking;
    bool enable_ownership_tracking;
    bool enable_const_checking;
    bool enable_thread_safety_analysis;
    bool enable_memory_leak_detection;
    bool strict_const_correctness;
    bool warn_on_implicit_conversions;
} StaticAnalysisConfig;

// Default static analysis configuration
#define ASTHRA_STATIC_ANALYSIS_DEFAULT_CONFIG { \
    .enable_bounds_checking = true, \
    .enable_null_checking = true, \
    .enable_ownership_tracking = true, \
    .enable_const_checking = true, \
    .enable_thread_safety_analysis = true, \
    .enable_memory_leak_detection = true, \
    .strict_const_correctness = true, \
    .warn_on_implicit_conversions = true \
}

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Static analysis initialization
ASTHRA_NONNULL(1)
void asthra_static_analysis_init(const StaticAnalysisConfig *restrict config);

// Static analysis cleanup
void asthra_static_analysis_cleanup(void);

// Error reporting for static analysis
ASTHRA_NORETURN ASTHRA_NONNULL(1)
void asthra_abort_with_message(const char *restrict message);

ASTHRA_NONNULL(1)
void asthra_static_analysis_warning(const char *restrict message);

ASTHRA_NONNULL(1)
void asthra_static_analysis_error(const char *restrict message);

// Helper function to get current static analysis configuration
const StaticAnalysisConfig *asthra_get_static_analysis_config(void);

// Helper function to check if static analysis is enabled
bool asthra_is_static_analysis_enabled(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_STATIC_ANALYSIS_H 
