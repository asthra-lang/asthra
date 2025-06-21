/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 5.2: Static Analysis Integration Implementation
 * - Const-correct string and memory operations
 * - Restrict pointer optimized functions
 * - Static analysis configuration and error reporting
 */

#include "static_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

// C17 compile-time validation of implementation assumptions
ASTHRA_STATIC_ASSERT(sizeof(void*) == 8, "Implementation assumes 64-bit pointers");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(uint32_t), "size_t must be at least 32-bit");

// Global static analysis configuration
static StaticAnalysisConfig g_static_analysis_config = ASTHRA_STATIC_ANALYSIS_DEFAULT_CONFIG;
static bool g_static_analysis_initialized = false;

// =============================================================================
// CONST-CORRECT STRING OPERATIONS WITH C17 PATTERNS
// =============================================================================

ASTHRA_PURE ASTHRA_NONNULL(1)
size_t asthra_string_length(const char *restrict str) {
    // No NULL check needed - parameter is marked nonnull
    
    // Use optimized strlen with restrict hint
    return strlen(str);
}

ASTHRA_PURE ASTHRA_NONNULL(1, 2)
int asthra_string_compare(const char *restrict str1, const char *restrict str2) {
    // No NULL check needed - parameters are marked nonnull
    
    // Use optimized strcmp with restrict hints
    return strcmp(str1, str2);
}

ASTHRA_NONNULL(1, 2)
char *asthra_string_copy(char *restrict dest, const char *restrict src, size_t dest_size) {
    // No NULL check needed - parameters are marked nonnull
    
    if (ASTHRA_UNLIKELY(dest_size == 0)) {
        return dest;
    }
    
    // Use C17 compound literal for bounds checking
    size_t src_len = asthra_string_length(src);
    size_t copy_len = (src_len < dest_size - 1) ? src_len : dest_size - 1;
    
    // Optimized copy with restrict pointers
    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';
    
    return dest;
}

ASTHRA_NONNULL(1, 2)
char *asthra_string_concatenate(char *restrict dest, const char *restrict src, 
                               size_t dest_size, size_t dest_used) {
    // No NULL check needed - parameters are marked nonnull
    ASTHRA_BOUNDS_CHECK(dest_used, dest_size);
    
    if (ASTHRA_UNLIKELY(dest_used >= dest_size - 1)) {
        return dest; // No space for concatenation
    }
    
    size_t remaining = dest_size - dest_used - 1;
    size_t src_len = asthra_string_length(src);
    size_t copy_len = (src_len < remaining) ? src_len : remaining;
    
    // Optimized concatenation with restrict pointers
    memcpy(dest + dest_used, src, copy_len);
    dest[dest_used + copy_len] = '\0';
    
    return dest;
}

// =============================================================================
// CONST-CORRECT MEMORY OPERATIONS WITH RESTRICT ANNOTATIONS
// =============================================================================

ASTHRA_PURE ASTHRA_NONNULL(1, 2)
int asthra_memory_compare(const void *restrict ptr1, const void *restrict ptr2, size_t size) {
    // No NULL check needed - parameters are marked nonnull
    
    if (ASTHRA_UNLIKELY(size == 0)) {
        return 0;
    }
    
    // Use optimized memcmp with restrict hints
    return memcmp(ptr1, ptr2, size);
}

ASTHRA_NONNULL(1, 2)
void *asthra_memory_copy(void *restrict dest, const void *restrict src, size_t size) {
    // No NULL check needed - parameters are marked nonnull
    
    if (ASTHRA_UNLIKELY(size == 0)) {
        return dest;
    }
    
    // Use optimized memcpy with restrict pointers for better optimization
    return memcpy(dest, src, size);
}

ASTHRA_NONNULL(1)
void *asthra_memory_set(void *restrict ptr, int value, size_t size) {
    // No NULL check needed - parameter is marked nonnull
    
    if (ASTHRA_UNLIKELY(size == 0)) {
        return ptr;
    }
    
    // Use optimized memset
    return memset(ptr, value, size);
}

ASTHRA_NONNULL(1, 2)
void *asthra_memory_move(void *dest, const void *src, size_t size) {
    // No NULL check needed - parameters are marked nonnull
    
    if (ASTHRA_UNLIKELY(size == 0)) {
        return dest;
    }
    
    // Use memmove for potentially overlapping memory regions
    return memmove(dest, src, size);
}

// =============================================================================
// CONST-CORRECT ARRAY OPERATIONS
// =============================================================================

ASTHRA_PURE ASTHRA_NONNULL(1)
size_t asthra_array_length(const void *const *restrict array) {
    // No NULL check needed - parameter is marked nonnull
    
    size_t count = 0;
    while (array[count] != NULL) {
        count++;
        // Prevent infinite loops with reasonable limit
        if (ASTHRA_UNLIKELY(count > SIZE_MAX / 2)) {
            asthra_abort_with_message("Array length overflow");
        }
    }
    
    return count;
}

ASTHRA_PURE ASTHRA_NONNULL(1, 3)
const void *asthra_array_find(const void *const *restrict array, size_t count,
                              const void *restrict element,
                              int (*compare)(const void *, const void *)) {
    // No NULL check needed - parameters are marked nonnull
    
    for (size_t i = 0; i < count; i++) {
        if (array[i] != NULL && compare(array[i], element) == 0) {
            return array[i];
        }
    }
    
    return NULL;
}

ASTHRA_NONNULL(1, 3)
void asthra_array_sort(void **restrict array, size_t count,
                      int (*compare)(const void *, const void *)) {
    // No NULL check needed - parameters are marked nonnull
    
    if (count <= 1) {
        return;
    }
    
    // Use standard library qsort for now
    // TODO: Implement optimized sort with restrict annotations
    qsort(array, count, sizeof(void*), compare);
}

// =============================================================================
// MEMORY ALLOCATION WITH RESTRICT ANNOTATIONS
// =============================================================================

ASTHRA_MALLOC ASTHRA_WARN_UNUSED_RESULT
void *asthra_alloc_aligned(size_t size, size_t alignment) {
    if (ASTHRA_UNLIKELY(size == 0)) {
        return NULL;
    }
    
    // Validate alignment is power of 2
    if (ASTHRA_UNLIKELY((alignment & (alignment - 1)) != 0)) {
        errno = EINVAL;
        return NULL;
    }
    
#if ASTHRA_C17_AVAILABLE && defined(__STDC_LIB_EXT1__)
    // Use C17 aligned_alloc if available
    return aligned_alloc(alignment, size);
#else
    // Fallback implementation
    void *ptr;
    if (posix_memalign(&ptr, alignment, size) == 0) {
        return ptr;
    }
    return NULL;
#endif
}

ASTHRA_MALLOC ASTHRA_WARN_UNUSED_RESULT
void *asthra_alloc_array(size_t element_size, size_t count) {
    if (ASTHRA_UNLIKELY(element_size == 0 || count == 0)) {
        return NULL;
    }
    
    // Check for overflow
    if (ASTHRA_UNLIKELY(count > SIZE_MAX / element_size)) {
        errno = ENOMEM;
        return NULL;
    }
    
    return calloc(count, element_size);
}

ASTHRA_WARN_UNUSED_RESULT ASTHRA_NONNULL(1)
void *asthra_realloc_array(void *restrict ptr, size_t old_count, 
                          size_t new_count, size_t element_size) {
    // No NULL check needed - parameter is marked nonnull
    
    if (ASTHRA_UNLIKELY(element_size == 0 || new_count == 0)) {
        free(ptr);
        return NULL;
    }
    
    // Check for overflow
    if (ASTHRA_UNLIKELY(new_count > SIZE_MAX / element_size)) {
        errno = ENOMEM;
        return NULL;
    }
    
    size_t new_size = element_size * new_count;
    void *new_ptr = realloc(ptr, new_size);
    
    if (new_ptr != NULL && new_count > old_count) {
        // Zero-initialize new elements
        size_t old_size = element_size * old_count;
        memset((char*)new_ptr + old_size, 0, new_size - old_size);
    }
    
    return new_ptr;
}

ASTHRA_NONNULL(1)
void asthra_free_aligned(void *restrict ptr) {
    // No NULL check needed - parameter is marked nonnull
    free(ptr);
}

// =============================================================================
// BUFFER OPERATIONS WITH RESTRICT ANNOTATIONS
// =============================================================================

ASTHRA_NONNULL(1, 3)
size_t asthra_buffer_copy(char *restrict dest, size_t dest_size,
                         const char *restrict src, size_t src_size) {
    // No NULL check needed - parameters are marked nonnull
    
    if (ASTHRA_UNLIKELY(dest_size == 0)) {
        return 0;
    }
    
    size_t copy_size = (src_size < dest_size) ? src_size : dest_size;
    memcpy(dest, src, copy_size);
    
    return copy_size;
}

ASTHRA_NONNULL(1, 4)
size_t asthra_buffer_append(char *restrict dest, size_t dest_size, size_t dest_used,
                           const char *restrict src, size_t src_size) {
    // No NULL check needed - parameters are marked nonnull
    ASTHRA_BOUNDS_CHECK(dest_used, dest_size);
    
    if (ASTHRA_UNLIKELY(dest_used >= dest_size)) {
        return 0;
    }
    
    size_t remaining = dest_size - dest_used;
    size_t copy_size = (src_size < remaining) ? src_size : remaining;
    
    memcpy(dest + dest_used, src, copy_size);
    
    return copy_size;
}

ASTHRA_PURE ASTHRA_NONNULL(1, 3)
bool asthra_buffer_equal(const char *restrict buf1, size_t size1,
                        const char *restrict buf2, size_t size2) {
    // No NULL check needed - parameters are marked nonnull
    
    if (size1 != size2) {
        return false;
    }
    
    if (size1 == 0) {
        return true;
    }
    
    return memcmp(buf1, buf2, size1) == 0;
}

// =============================================================================
// STRING FORMATTING WITH RESTRICT ANNOTATIONS
// =============================================================================

ASTHRA_NONNULL(1, 3)
int asthra_format_string(char *restrict buffer, size_t buffer_size,
                        const char *restrict format, ...) {
    // No NULL check needed - parameters are marked nonnull
    
    if (ASTHRA_UNLIKELY(buffer_size == 0)) {
        return -1;
    }
    
    va_list args;
    va_start(args, format);
    int result = asthra_format_string_va(buffer, buffer_size, format, args);
    va_end(args);
    
    return result;
}

ASTHRA_NONNULL(1, 3)
int asthra_format_string_va(char *restrict buffer, size_t buffer_size,
                           const char *restrict format, va_list args) {
    // No NULL check needed - parameters are marked nonnull
    
    if (ASTHRA_UNLIKELY(buffer_size == 0)) {
        return -1;
    }
    
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    int result = vsnprintf(buffer, buffer_size, format, args);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    
    // Ensure null termination
    if (result >= 0 && (size_t)result >= buffer_size) {
        buffer[buffer_size - 1] = '\0';
    }
    
    return result;
}

// =============================================================================
// STATIC ANALYSIS CONFIGURATION AND ERROR REPORTING
// =============================================================================

ASTHRA_NONNULL(1)
void asthra_static_analysis_init(const StaticAnalysisConfig *restrict config) {
    // No NULL check needed - parameter is marked nonnull
    
    // Copy configuration with C17 compound literal
    g_static_analysis_config = *config;
    g_static_analysis_initialized = true;
    
    if (config->enable_bounds_checking) {
        // Initialize bounds checking subsystem
        // TODO: Implement bounds checking initialization
    }
    
    if (config->enable_null_checking) {
        // Initialize null pointer checking
        // TODO: Implement null checking initialization
    }
    
    if (config->enable_ownership_tracking) {
        // Initialize ownership tracking
        // TODO: Implement ownership tracking initialization
    }
    
    if (config->enable_thread_safety_analysis) {
        // Initialize thread safety analysis
        // TODO: Implement thread safety analysis initialization
    }
}

void asthra_static_analysis_cleanup(void) {
    if (!g_static_analysis_initialized) {
        return;
    }
    
    // Cleanup static analysis subsystems
    // TODO: Implement cleanup for various analysis systems
    
    // Reset configuration to defaults
    g_static_analysis_config = (StaticAnalysisConfig)ASTHRA_STATIC_ANALYSIS_DEFAULT_CONFIG;
    g_static_analysis_initialized = false;
}

ASTHRA_NORETURN ASTHRA_NONNULL(1)
void asthra_abort_with_message(const char *restrict message) {
    // No NULL check needed - parameter is marked nonnull
    
    fprintf(stderr, "FATAL ERROR: %s\n", message);
    fprintf(stderr, "Aborting execution due to static analysis violation.\n");
    
    // Flush all streams before aborting
    fflush(stderr);
    fflush(stdout);
    
    abort();
}

ASTHRA_NONNULL(1)
void asthra_static_analysis_warning(const char *restrict message) {
    // No NULL check needed - parameter is marked nonnull
    
    if (!g_static_analysis_initialized) {
        return;
    }
    
    fprintf(stderr, "STATIC ANALYSIS WARNING: %s\n", message);
    fflush(stderr);
}

ASTHRA_NONNULL(1)
void asthra_static_analysis_error(const char *restrict message) {
    // No NULL check needed - parameter is marked nonnull
    
    if (!g_static_analysis_initialized) {
        return;
    }
    
    fprintf(stderr, "STATIC ANALYSIS ERROR: %s\n", message);
    fflush(stderr);
    
    // In strict mode, treat errors as fatal
    if (g_static_analysis_config.strict_const_correctness) {
        asthra_abort_with_message("Static analysis error in strict mode");
    }
}

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

// Static analysis configuration validation removed - no validation needed currently

// Helper function to get current static analysis configuration
const StaticAnalysisConfig *asthra_get_static_analysis_config(void) {
    return g_static_analysis_initialized ? &g_static_analysis_config : NULL;
}

// Helper function to check if static analysis is enabled
bool asthra_is_static_analysis_enabled(void) {
    return g_static_analysis_initialized;
} 
