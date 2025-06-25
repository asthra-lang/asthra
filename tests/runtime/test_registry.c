/**
 * Asthra Programming Language v1.2 Test Registry Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of test registry and summary management
 */

#include "test_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

// Grammar and Parsing Tests
AsthraTestResult test_grammar_disambiguation(AsthraV12TestContext *ctx);
AsthraTestResult test_semantic_tags_parsing(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_matching_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_string_operations_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_syntax(AsthraV12TestContext *ctx);

// Pattern Matching and Error Handling Tests
AsthraTestResult test_exhaustive_matching(AsthraV12TestContext *ctx);
AsthraTestResult test_error_propagation(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_matching_performance(AsthraV12TestContext *ctx);
AsthraTestResult test_nested_result_handling(AsthraV12TestContext *ctx);
AsthraTestResult test_complex_pattern_scenarios(AsthraV12TestContext *ctx);

// String Operations Tests
AsthraTestResult test_string_deterministic_behavior(AsthraV12TestContext *ctx);
AsthraTestResult test_string_performance_characteristics(AsthraV12TestContext *ctx);
AsthraTestResult test_string_memory_safety(AsthraV12TestContext *ctx);
AsthraTestResult test_string_ffi_integration(AsthraV12TestContext *ctx);
AsthraTestResult test_string_large_data_handling(AsthraV12TestContext *ctx);

// Slice Management Tests
AsthraTestResult test_slice_header_structure(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_header_operations(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_header_memory_layout(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_bounds_checking_runtime(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_bounds_checking_edge_cases(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_bounds_checking_performance(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_predictable_memory_layout(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_alignment_requirements(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_memory_efficiency(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_pqc_sized_data(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_large_data_performance(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_large_data_memory_safety(AsthraV12TestContext *ctx);

// Enhanced FFI Tests
AsthraTestResult test_ffi_enhanced_function_calls(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_bidirectional_calls(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_complex_data_structures(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_ownership_borrow(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_ownership_move(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_ownership_copy(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_memory_safety_bounds_checking(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_memory_safety_leak_prevention(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_memory_safety_double_free_prevention(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_c17_compliance_features(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_c17_compliance_types(AsthraV12TestContext *ctx);
AsthraTestResult test_ffi_c17_compliance_functions(AsthraV12TestContext *ctx);

// Concurrency Tests
AsthraTestResult test_concurrency_spawn_basic(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_spawn_with_c_functions(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_c_threads_calling_asthra(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_task_creation(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_task_execution(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_task_cleanup(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_result_propagation(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_error_across_tasks(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_error_handling_stress(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_await_foundation(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_task_handles(AsthraV12TestContext *ctx);
AsthraTestResult test_concurrency_future_compatibility(AsthraV12TestContext *ctx);

// Security Tests
AsthraTestResult test_security_constant_time_operations(AsthraV12TestContext *ctx);
AsthraTestResult test_security_secure_memory(AsthraV12TestContext *ctx);
AsthraTestResult test_security_csprng_functionality(AsthraV12TestContext *ctx);
AsthraTestResult test_security_side_channel_resistance(AsthraV12TestContext *ctx);
AsthraTestResult test_security_timing_attack_resistance(AsthraV12TestContext *ctx);
AsthraTestResult test_security_memory_disclosure_prevention(AsthraV12TestContext *ctx);

// Integration Tests
AsthraTestResult test_integration_concurrent_text_processing(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_concurrent_slice_management(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_pattern_matching_with_ffi(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_security_with_concurrency(AsthraV12TestContext *ctx);
AsthraTestResult test_integration_performance_comprehensive(AsthraV12TestContext *ctx);

// =============================================================================
// TEST REGISTRY DEFINITION
// =============================================================================

static TestRegistryEntry test_registry[] = {
    // Grammar and Parsing Tests
    {"Grammar Disambiguation", "Grammar", test_grammar_disambiguation, false, false},
    {"Semantic Tags Parsing", "Grammar", test_semantic_tags_parsing, false, false},
    {"Pattern Matching Syntax", "Grammar", test_pattern_matching_syntax, false, false},
    {"String Operations Syntax", "Grammar", test_string_operations_syntax, false, false},
    {"Slice Syntax", "Grammar", test_slice_syntax, false, false},

    // Pattern Matching and Error Handling Tests
    {"Exhaustive Matching", "Pattern Matching", test_exhaustive_matching, false, false},
    {"Error Propagation", "Pattern Matching", test_error_propagation, false, false},
    {"Performance Validation", "Pattern Matching", test_pattern_matching_performance, true, false},
    {"Nested Results", "Pattern Matching", test_nested_result_handling, false, false},
    {"Complex Patterns", "Pattern Matching", test_complex_pattern_scenarios, false, false},

    // String Operations Tests
    {"Deterministic Behavior", "String Operations", test_string_deterministic_behavior, false,
     false},
    {"Performance Characteristics", "String Operations", test_string_performance_characteristics,
     true, false},
    {"Memory Safety", "String Operations", test_string_memory_safety, false, true},
    {"FFI Integration", "String Operations", test_string_ffi_integration, false, false},
    {"Large Data Handling", "String Operations", test_string_large_data_handling, true, false},

    // Slice Management Tests
    {"SliceHeader Structure", "Slice Management", test_slice_header_structure, false, false},
    {"SliceHeader Operations", "Slice Management", test_slice_header_operations, false, false},
    {"SliceHeader Memory Layout", "Slice Management", test_slice_header_memory_layout, false,
     false},
    {"Bounds Checking Runtime", "Slice Management", test_slice_bounds_checking_runtime, false,
     true},
    {"Bounds Checking Edge Cases", "Slice Management", test_slice_bounds_checking_edge_cases, false,
     true},
    {"Bounds Checking Performance", "Slice Management", test_slice_bounds_checking_performance,
     true, false},
    {"Predictable Memory Layout", "Slice Management", test_slice_predictable_memory_layout, false,
     false},
    {"Alignment Requirements", "Slice Management", test_slice_alignment_requirements, false, false},
    {"Memory Efficiency", "Slice Management", test_slice_memory_efficiency, false, false},
    {"PQC Sized Data", "Slice Management", test_slice_pqc_sized_data, true, false},
    {"Large Data Performance", "Slice Management", test_slice_large_data_performance, true, false},
    {"Large Data Memory Safety", "Slice Management", test_slice_large_data_memory_safety, false,
     true},

    // Enhanced FFI Tests
    {"Enhanced Function Calls", "Enhanced FFI", test_ffi_enhanced_function_calls, false, false},
    {"Bidirectional Calls", "Enhanced FFI", test_ffi_bidirectional_calls, false, false},
    {"Complex Data Structures", "Enhanced FFI", test_ffi_complex_data_structures, false, false},
    {"Ownership Borrow", "Enhanced FFI", test_ffi_ownership_borrow, false, false},
    {"Ownership Move", "Enhanced FFI", test_ffi_ownership_move, false, false},
    {"Ownership Copy", "Enhanced FFI", test_ffi_ownership_copy, false, false},
    {"Memory Safety Bounds Checking", "Enhanced FFI", test_ffi_memory_safety_bounds_checking, false,
     true},
    {"Memory Safety Leak Prevention", "Enhanced FFI", test_ffi_memory_safety_leak_prevention, false,
     true},
    {"Memory Safety Double Free Prevention", "Enhanced FFI",
     test_ffi_memory_safety_double_free_prevention, false, true},
    {"C17 Compliance Features", "Enhanced FFI", test_ffi_c17_compliance_features, false, false},
    {"C17 Compliance Types", "Enhanced FFI", test_ffi_c17_compliance_types, false, false},
    {"C17 Compliance Functions", "Enhanced FFI", test_ffi_c17_compliance_functions, false, false},

    // Concurrency Tests
    {"Spawn Basic", "Concurrency", test_concurrency_spawn_basic, false, false},
    {"Spawn with C Functions", "Concurrency", test_concurrency_spawn_with_c_functions, false,
     false},
    {"C Threads Calling Asthra", "Concurrency", test_concurrency_c_threads_calling_asthra, false,
     false},
    {"Task Creation", "Concurrency", test_concurrency_task_creation, false, false},
    {"Task Execution", "Concurrency", test_concurrency_task_execution, false, false},
    {"Task Cleanup", "Concurrency", test_concurrency_task_cleanup, false, false},
    {"Result Propagation", "Concurrency", test_concurrency_result_propagation, false, false},
    {"Error Across Tasks", "Concurrency", test_concurrency_error_across_tasks, false, false},
    {"Error Handling Stress", "Concurrency", test_concurrency_error_handling_stress, true, false},
    {"Await Foundation", "Concurrency", test_concurrency_await_foundation, false, false},
    {"Task Handles", "Concurrency", test_concurrency_task_handles, false, false},
    {"Future Compatibility", "Concurrency", test_concurrency_future_compatibility, false, false},

    // Security Tests
    {"Constant Time Operations", "Security", test_security_constant_time_operations, false, true},
    {"Secure Memory", "Security", test_security_secure_memory, false, true},
    {"CSPRNG Functionality", "Security", test_security_csprng_functionality, false, true},
    {"Side Channel Resistance", "Security", test_security_side_channel_resistance, true, true},
    {"Timing Attack Resistance", "Security", test_security_timing_attack_resistance, true, true},
    {"Memory Disclosure Prevention", "Security", test_security_memory_disclosure_prevention, false,
     true},

    // Integration Tests
    {"Concurrent Text Processing", "Integration", test_integration_concurrent_text_processing,
     false, false},
    {"Concurrent Slice Management", "Integration", test_integration_concurrent_slice_management,
     false, false},
    {"Pattern Matching with FFI", "Integration", test_integration_pattern_matching_with_ffi, false,
     false},
    {"Security with Concurrency", "Integration", test_integration_security_with_concurrency, false,
     true},
    {"Performance Comprehensive", "Integration", test_integration_performance_comprehensive, true,
     false},
};

// =============================================================================
// REGISTRY ACCESS FUNCTIONS
// =============================================================================

const TestRegistryEntry *get_test_registry(void) {
    return test_registry;
}

size_t get_test_registry_size(void) {
    return sizeof(test_registry) / sizeof(test_registry[0]);
}

// =============================================================================
// TEST SUMMARY MANAGEMENT
// =============================================================================

void test_summary_init(TestSummary *summary) {
    if (!summary)
        return;

    memset(summary, 0, sizeof(TestSummary));
    summary->failure_messages = NULL;
    summary->failure_count = 0;
}

void test_summary_cleanup(TestSummary *summary) {
    if (!summary)
        return;

    for (int i = 0; i < summary->failure_count; i++) {
        free(summary->failure_messages[i]);
    }
    free(summary->failure_messages);

    memset(summary, 0, sizeof(TestSummary));
}

void test_summary_add_failure(TestSummary *summary, const char *test_name, const char *message) {
    if (!summary || !test_name || !message)
        return;

    summary->failure_messages =
        realloc(summary->failure_messages, (summary->failure_count + 1) * sizeof(char *));

    if (!summary->failure_messages)
        return;

    size_t msg_len = strlen(test_name) + strlen(message) + 10;
    summary->failure_messages[summary->failure_count] = malloc(msg_len);

    if (summary->failure_messages[summary->failure_count]) {
        snprintf(summary->failure_messages[summary->failure_count], msg_len, "%s: %s", test_name,
                 message);
        summary->failure_count++;
    }
}
