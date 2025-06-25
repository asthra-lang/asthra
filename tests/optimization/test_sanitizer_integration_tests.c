/**
 * Asthra Programming Language
 * Sanitizer Integration Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module contains integration tests for sanitizer infrastructure.
 */

#include "test_sanitizer_common.h"

AsthraTestResult test_sanitizer_environment_setup(AsthraTestContext *context) {
    // Test that sanitizer environment variables are properly set

    const char *asan_options = getenv("ASAN_OPTIONS");
    const char *ubsan_options = getenv("UBSAN_OPTIONS");
    const char *tsan_options = getenv("TSAN_OPTIONS");
    const char *msan_options = getenv("MSAN_OPTIONS");

    // These might not be set in all environments, so we just log their status
    printf("Sanitizer environment status:\n");
    printf("  ASAN_OPTIONS: %s\n", asan_options ? asan_options : "not set");
    printf("  UBSAN_OPTIONS: %s\n", ubsan_options ? ubsan_options : "not set");
    printf("  TSAN_OPTIONS: %s\n", tsan_options ? tsan_options : "not set");
    printf("  MSAN_OPTIONS: %s\n", msan_options ? msan_options : "not set");

    // The test passes regardless of environment setup
    // This is informational for debugging sanitizer issues
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_sanitizer_symbol_availability(AsthraTestContext *context) {
    // Test that sanitizer symbols are available when compiled with sanitizers

    // This is a compile-time test - if we're running, the symbols are available
    printf("Sanitizer symbols are available (test is running)\n");

// Check for common sanitizer macros
#ifdef __has_feature
#if __has_feature(address_sanitizer)
    printf("  AddressSanitizer: ENABLED\n");
#else
    printf("  AddressSanitizer: not detected\n");
#endif

#if __has_feature(thread_sanitizer)
    printf("  ThreadSanitizer: ENABLED\n");
#else
    printf("  ThreadSanitizer: not detected\n");
#endif

#if __has_feature(memory_sanitizer)
    printf("  MemorySanitizer: ENABLED\n");
#else
    printf("  MemorySanitizer: not detected\n");
#endif
#else
    printf("  __has_feature not available (GCC?)\n");
#endif

#ifdef __SANITIZE_ADDRESS__
    printf("  __SANITIZE_ADDRESS__ defined\n");
#endif

#ifdef __SANITIZE_THREAD__
    printf("  __SANITIZE_THREAD__ defined\n");
#endif

    return ASTHRA_TEST_PASS;
}
