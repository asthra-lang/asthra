/**
 * Asthra Programming Language - Runtime Function Stubs
 *
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Runtime function stubs for test framework compatibility
 */

#include "concurrency_integration_common.h"

// =============================================================================
// RUNTIME FUNCTION STUBS
// =============================================================================

// Note: Asthra_register_c_thread and Asthra_unregister_c_thread are provided
// by the real runtime implementation in asthra_concurrency_thread_registry.c
// No stubs needed here to avoid duplicate symbol errors.

/**
 * Create error result with FFI signature
 * Matches the signature in asthra_ffi_memory.h
 */
AsthraFFIResult Asthra_result_err(int error_code, const char *error_message,
                                  const char *error_source, void *error_context) {
    AsthraFFIResult result;
    result.tag = ASTHRA_FFI_RESULT_ERR;
    result.data.err.error_code = error_code;

    // Safely copy error message
    if (error_message) {
        strncpy(result.data.err.error_message, error_message, 255);
        result.data.err.error_message[255] = '\0';
    } else {
        strcpy(result.data.err.error_message, "Unknown error");
    }

    result.data.err.error_source = error_source;
    result.data.err.error_context = error_context;

    return result;
}

// =============================================================================
// TEST FRAMEWORK FUNCTION STUBS
// =============================================================================

/**
 * Initialize test context
 * Provides basic initialization for integration tests
 */
void asthra_test_context_init(AsthraTestContext *context) {
    if (!context) {
        return;
    }

    // Initialize context structure
    memset(context, 0, sizeof(AsthraTestContext));
    context->result = CONCURRENCY_TEST_PASS;

    // Initialize metadata with default values
    context->metadata.name = "integration_test";
    context->metadata.file = __FILE__;
    context->metadata.line = __LINE__;
    context->metadata.function = "asthra_test_context_init";
    context->metadata.severity = ASTHRA_TEST_SEVERITY_MEDIUM;
    context->metadata.timeout_ns = 30000000000ULL; // 30 seconds
    context->metadata.skip = false;
    context->metadata.skip_reason = NULL;

    // Initialize timing
    context->start_time_ns = asthra_test_get_time_ns();
    context->end_time_ns = 0;
    context->duration_ns = 0;
    context->error_message = NULL;
    context->error_message_allocated = false;
    context->assertions_in_test = 0;
    context->global_stats = NULL;
}

// Note: All test framework functions (reset_test_counters, print_test_statistics,
// tests_run, tests_passed, run_tier1_tests, run_tier2_tests, etc.) are provided
// by the actual test modules, so no stubs needed here.