/**
 * Asthra Programming Language Compiler
 * Cross-Platform Abstraction Layer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This header provides platform-specific abstractions for:
 * - macOS (Clang)
 * - Linux (Clang)
 *
 * Features C17 compatibility and compile-time validation.
 */

#ifndef ASTHRA_PLATFORM_H
#define ASTHRA_PLATFORM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// C17 feature detection
#if __STDC_VERSION__ >= 201710L
#define ASTHRA_C17_AVAILABLE 1
#include <assert.h>
#define ASTHRA_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define ASTHRA_C17_AVAILABLE 0
#define ASTHRA_STATIC_ASSERT(cond, msg) /* No static assert support */
#endif

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PLATFORM DETECTION
// =============================================================================

#if defined(_WIN32) || defined(_WIN64)
#define ASTHRA_PLATFORM_WINDOWS 1
#define ASTHRA_PLATFORM_UNIX 0
#define ASTHRA_PLATFORM_MACOS 0
#define ASTHRA_PLATFORM_LINUX 0
#define ASTHRA_PLATFORM_NAME "Windows"
#elif defined(__APPLE__) && defined(__MACH__)
#define ASTHRA_PLATFORM_WINDOWS 0
#define ASTHRA_PLATFORM_UNIX 1
#define ASTHRA_PLATFORM_MACOS 1
#define ASTHRA_PLATFORM_LINUX 0
#define ASTHRA_PLATFORM_NAME "macOS"
#elif defined(__linux__)
#define ASTHRA_PLATFORM_WINDOWS 0
#define ASTHRA_PLATFORM_UNIX 1
#define ASTHRA_PLATFORM_MACOS 0
#define ASTHRA_PLATFORM_LINUX 1
#define ASTHRA_PLATFORM_NAME "Linux"
#else
#define ASTHRA_PLATFORM_WINDOWS 0
#define ASTHRA_PLATFORM_UNIX 1
#define ASTHRA_PLATFORM_MACOS 0
#define ASTHRA_PLATFORM_LINUX 0
#define ASTHRA_PLATFORM_NAME "Unknown"
#endif

// =============================================================================
// COMPILER DETECTION
// =============================================================================

#if defined(__clang__)
#define ASTHRA_COMPILER_CLANG 1
#define ASTHRA_COMPILER_NAME "Clang"
#define ASTHRA_COMPILER_VERSION                                                                    \
    (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#else
#error "Unsupported compiler. Asthra requires Clang/LLVM."
#endif

// =============================================================================
// PLATFORM-SPECIFIC INCLUDES
// =============================================================================

#if !ASTHRA_PLATFORM_WINDOWS
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#if ASTHRA_PLATFORM_LINUX
#include <linux/limits.h>
#include <sys/prctl.h>
#endif
#endif

// =============================================================================
// FILE SYSTEM ABSTRACTIONS
// =============================================================================

// File system abstractions - Unix only
#define ASTHRA_PATH_SEPARATOR '/'
#define ASTHRA_PATH_SEPARATOR_STR "/"
#define ASTHRA_PATH_LIST_SEPARATOR ':'
#define ASTHRA_PATH_LIST_SEPARATOR_STR ":"
#define ASTHRA_EXE_EXT ""
#define ASTHRA_OBJ_EXT ".o"
#define ASTHRA_LIB_EXT ".a"
#if ASTHRA_PLATFORM_MACOS
#define ASTHRA_DLL_EXT ".dylib"
#else
#define ASTHRA_DLL_EXT ".so"
#endif
#define ASTHRA_LIB_PREFIX "lib"
#define ASTHRA_MAX_PATH 4096

// =============================================================================
// THREAD AND PROCESS ABSTRACTIONS
// =============================================================================

// Thread and process abstractions - Unix only
typedef pthread_t asthra_thread_t;
typedef pthread_mutex_t asthra_mutex_t;
typedef pid_t asthra_process_t;
typedef pthread_t asthra_thread_id_t;
typedef pid_t asthra_process_id_t;
#define ASTHRA_THREAD_INVALID ((pthread_t)0)
#define ASTHRA_PROCESS_INVALID ((pid_t) - 1)

// =============================================================================
// MEMORY MANAGEMENT ABSTRACTIONS
// =============================================================================

// Memory management abstractions - Unix only
#include <stdlib.h>
#define asthra_alloc_aligned(size, alignment) aligned_alloc(alignment, size)
#define asthra_free_aligned(ptr) free(ptr)
#define asthra_page_size() getpagesize()

// =============================================================================
// FUNCTION ATTRIBUTES AND CALLING CONVENTIONS
// =============================================================================

// Clang-only attributes
#define ASTHRA_INLINE static inline __attribute__((always_inline))
#define ASTHRA_NOINLINE __attribute__((noinline))
#define ASTHRA_NORETURN __attribute__((noreturn))
#define ASTHRA_RESTRICT __restrict__
#define ASTHRA_ALIGN(n) __attribute__((aligned(n)))
#define ASTHRA_PACKED __attribute__((packed))
#define ASTHRA_EXPORT __attribute__((visibility("default")))
#define ASTHRA_IMPORT
#define ASTHRA_CDECL
#define ASTHRA_STDCALL
#define ASTHRA_FASTCALL

// =============================================================================
// ATOMIC OPERATIONS (C17)
// =============================================================================

#if ASTHRA_C17_AVAILABLE
#include <stdatomic.h>
#define ASTHRA_ATOMIC(type) _Atomic(type)
#define asthra_atomic_load(ptr) atomic_load(ptr)
#define asthra_atomic_store(ptr, val) atomic_store(ptr, val)
#define asthra_atomic_fetch_add(ptr, val) atomic_fetch_add(ptr, val)
#define asthra_atomic_fetch_sub(ptr, val) atomic_fetch_sub(ptr, val)
#define asthra_atomic_compare_exchange(ptr, expected, desired)                                     \
    atomic_compare_exchange_strong(ptr, expected, desired)
#else
#define ASTHRA_ATOMIC(type) volatile type
// Fallback implementations for Clang builtins
#define asthra_atomic_load(ptr) __sync_fetch_and_add(ptr, 0)
#define asthra_atomic_store(ptr, val) __sync_lock_test_and_set(ptr, val)
#define asthra_atomic_fetch_add(ptr, val) __sync_fetch_and_add(ptr, val)
#define asthra_atomic_fetch_sub(ptr, val) __sync_fetch_and_sub(ptr, val)
#endif

// =============================================================================
// ERROR HANDLING ABSTRACTIONS
// =============================================================================

// Error handling abstractions - Unix only
typedef int asthra_error_t;
#define ASTHRA_ERROR_SUCCESS 0
#define asthra_get_system_error() errno
#define asthra_set_system_error(err) (errno = (err))

/**
 * @brief Get error message string for error code
 * @param error Error code to convert to string
 * @return Human-readable error message
 */
const char *asthra_get_error_string(asthra_error_t error);

// =============================================================================
// UNICODE AND STRING HANDLING
// =============================================================================

// Unicode and string handling - Unix only
#include <wchar.h>
typedef wchar_t asthra_wchar_t;
#define ASTHRA_WCHAR_MAX WCHAR_MAX
#define asthra_wcslen wcslen
#define asthra_wcscpy wcscpy
#define asthra_wcscat wcscat
#define asthra_wcscmp wcscmp

// =============================================================================
// COMPILE-TIME VALIDATIONS (C17)
// =============================================================================

// Validate platform detection
ASTHRA_STATIC_ASSERT(ASTHRA_PLATFORM_WINDOWS + ASTHRA_PLATFORM_UNIX == 1,
                     "Exactly one platform must be detected");

// Validate compiler detection
ASTHRA_STATIC_ASSERT(ASTHRA_COMPILER_CLANG == 1, "Clang compiler must be detected");

// Validate path separator consistency
#if ASTHRA_PLATFORM_WINDOWS
ASTHRA_STATIC_ASSERT(ASTHRA_PATH_SEPARATOR == '\\', "Windows path separator must be backslash");
#else
ASTHRA_STATIC_ASSERT(ASTHRA_PATH_SEPARATOR == '/', "Unix path separator must be forward slash");
#endif

// Validate type sizes for cross-platform compatibility
ASTHRA_STATIC_ASSERT(sizeof(void *) == 8, "64-bit pointers required");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= 4, "size_t must be at least 32-bit");
ASTHRA_STATIC_ASSERT(sizeof(int) == 4, "int must be 32-bit for ABI compatibility");

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Get platform information string
 * @return Platform name and compiler information
 */
const char *asthra_get_platform_info(void);

/**
 * @brief Check if running on Windows
 * @return true if Windows platform
 */
ASTHRA_INLINE bool asthra_is_windows(void) {
    return ASTHRA_PLATFORM_WINDOWS;
}

/**
 * @brief Check if running on Unix-like system
 * @return true if Unix platform (Linux/macOS)
 */
ASTHRA_INLINE bool asthra_is_unix(void) {
    return ASTHRA_PLATFORM_UNIX;
}

/**
 * @brief Get appropriate file extension for executable
 * @return ".exe" on Windows, "" on Unix
 */
ASTHRA_INLINE const char *asthra_get_exe_extension(void) {
    return ASTHRA_EXE_EXT;
}

/**
 * @brief Get appropriate path separator for platform
 * @return '\\' on Windows, '/' on Unix
 */
ASTHRA_INLINE char asthra_get_path_separator(void) {
    return ASTHRA_PATH_SEPARATOR;
}

/**
 * @brief Build a cross-platform file path
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @param components NULL-terminated array of path components
 * @return Number of characters written, or -1 on error
 */
int asthra_build_path(char *buffer, size_t buffer_size, const char **components);

/**
 * @brief Convert path separators to platform-specific format
 * @param path Path to normalize (modified in-place)
 */
void asthra_normalize_path(char *path);

/**
 * @brief Check if a file exists
 * @param path File path to check
 * @return true if file exists
 */
bool asthra_file_exists(const char *path);

/**
 * @brief Create directory (including parent directories)
 * @param path Directory path to create
 * @return true on success
 */
bool asthra_create_directory(const char *path);

/**
 * @brief Duplicate a string (cross-platform strdup)
 * @param str String to duplicate
 * @return Newly allocated copy of the string, or NULL on failure
 */
char *asthra_strdup(const char *str);

// Additional platform functions
asthra_thread_id_t asthra_get_current_thread_id(void);
asthra_process_id_t asthra_get_current_process_id(void);
void *asthra_realloc_safe(void *ptr, size_t old_size, size_t new_size);
bool asthra_cstring_ends_with(const char *str, const char *suffix);

// asthra_string_starts_with
bool asthra_string_starts_with(const char *str, const char *prefix);

// asthra_get_current_time_ms - Get current time in milliseconds
uint64_t asthra_get_current_time_ms(void);

// asthra_get_high_resolution_time
uint64_t asthra_get_high_resolution_time(void);

// asthra_get_high_resolution_frequency
uint64_t asthra_get_high_resolution_frequency(void);

// asthra_get_elapsed_seconds
double asthra_get_elapsed_seconds(uint64_t start_time, uint64_t end_time);

// asthra_get_system_memory
size_t asthra_get_system_memory(void);

// asthra_get_cpu_count
int asthra_get_cpu_count(void);

#ifdef __cplusplus
}
#endif

#endif /* ASTHRA_PLATFORM_H */
