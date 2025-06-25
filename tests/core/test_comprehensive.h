/**
 * Asthra Programming Language Comprehensive Test Suite - Main Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main header file that includes all modular comprehensive test components.
 * This header maintains backward compatibility while providing access to
 * the new modular structure.
 *
 * Comprehensive test suite covering all language features:
 * - Grammar disambiguation and parsing
 * - Pattern matching and Result<T,E> error handling
 * - String operations (concatenation and interpolation)
 * - Slice management and FFI integration
 * - Enhanced FFI with ownership annotations
 * - Memory safety and GC interaction
 * - Concurrency bridge (spawn) functionality
 * - Security features (constant-time, volatile memory)
 *
 * AI Focus: Critical feedback for AI code generation, covering edge cases
 * and providing structured feedback for all features.
 */

#ifndef ASTHRA_TEST_COMPREHENSIVE_H
#define ASTHRA_TEST_COMPREHENSIVE_H

// Include all modular components
#include "test_comprehensive_benchmark.h"
#include "test_comprehensive_core.h"
#include "test_comprehensive_reporting.h"
#include "test_comprehensive_runners.h"
#include "test_comprehensive_security.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BACKWARD COMPATIBILITY ALIASES (Legacy V12 -> New Generic Names)
// =============================================================================

// Legacy type aliases for backward compatibility
typedef AsthraTestCategory AsthraV12TestCategory;
typedef AsthraTestComplexity AsthraV12TestComplexity;
typedef AsthraTestMode AsthraV12TestMode;
typedef AsthraExtendedTestMetadata AsthraV12TestMetadata;
typedef AsthraBenchmarkResult AsthraV12BenchmarkResult;
typedef AsthraSecurityResult AsthraV12SecurityResult;
typedef AsthraExtendedTestContext AsthraV12TestContext;

// Legacy constant aliases
#define ASTHRA_V1_2_TEST_TIMEOUT_DEFAULT_NS ASTHRA_TEST_TIMEOUT_DEFAULT_NS
#define ASTHRA_V1_2_TEST_TIMEOUT_LONG_NS ASTHRA_TEST_TIMEOUT_LONG_NS

// Legacy enum value aliases
#define ASTHRA_V1_2_CATEGORY_GRAMMAR ASTHRA_CATEGORY_GRAMMAR
#define ASTHRA_V1_2_CATEGORY_PATTERN_MATCHING ASTHRA_CATEGORY_PATTERN_MATCHING
#define ASTHRA_V1_2_CATEGORY_STRING_OPERATIONS ASTHRA_CATEGORY_STRING_OPERATIONS
#define ASTHRA_V1_2_CATEGORY_SLICE_MANAGEMENT ASTHRA_CATEGORY_SLICE_MANAGEMENT
#define ASTHRA_V1_2_CATEGORY_ENHANCED_FFI ASTHRA_CATEGORY_ENHANCED_FFI
#define ASTHRA_V1_2_CATEGORY_CONCURRENCY ASTHRA_CATEGORY_CONCURRENCY
#define ASTHRA_V1_2_CATEGORY_SECURITY ASTHRA_CATEGORY_SECURITY
#define ASTHRA_V1_2_CATEGORY_PERFORMANCE ASTHRA_CATEGORY_PERFORMANCE
#define ASTHRA_V1_2_CATEGORY_INTEGRATION ASTHRA_CATEGORY_INTEGRATION
#define ASTHRA_V1_2_CATEGORY_COUNT ASTHRA_CATEGORY_COUNT

#define ASTHRA_V1_2_COMPLEXITY_BASIC ASTHRA_COMPLEXITY_BASIC
#define ASTHRA_V1_2_COMPLEXITY_INTERMEDIATE ASTHRA_COMPLEXITY_INTERMEDIATE
#define ASTHRA_V1_2_COMPLEXITY_ADVANCED ASTHRA_COMPLEXITY_ADVANCED
#define ASTHRA_V1_2_COMPLEXITY_STRESS ASTHRA_COMPLEXITY_STRESS

#define ASTHRA_V1_2_MODE_UNIT ASTHRA_MODE_UNIT
#define ASTHRA_V1_2_MODE_INTEGRATION ASTHRA_MODE_INTEGRATION
#define ASTHRA_V1_2_MODE_PERFORMANCE ASTHRA_MODE_PERFORMANCE
#define ASTHRA_V1_2_MODE_SECURITY ASTHRA_MODE_SECURITY

// =============================================================================
// LEGACY FUNCTION ALIASES
// =============================================================================

// Legacy function aliases for backward compatibility
#define asthra_test_context_create asthra_test_context_create_extended
#define asthra_v12_test_context_destroy asthra_test_context_destroy_extended
#define asthra_v12_test_context_reset asthra_test_context_reset_extended

// Note: asthra_benchmark_* functions are used directly (no aliases needed)

// Removed circular macro definitions - using v12 function names directly
// Functions are implemented as asthra_v12_* in test_safety_stubs.c
// Compatibility macros for legacy code
#define asthra_verify_secure_memory_zeroing asthra_v12_verify_secure_memory_zeroing
#define asthra_verify_entropy_quality asthra_v12_verify_entropy_quality
#define asthra_verify_constant_time asthra_v12_verify_constant_time

#define asthra_v12_record_ai_feedback asthra_record_ai_feedback
#define asthra_v12_generate_test_report asthra_generate_test_report
#define asthra_v12_verify_c17_compliance asthra_verify_c17_compliance
#define asthra_v12_run_static_analysis asthra_run_static_analysis
#define asthra_v12_print_comprehensive_summary asthra_print_comprehensive_summary

// Test suite functions - use direct v1_2 functions (remove circular aliases)

// Additional legacy aliases for existing code (avoiding redefinition)
// Note: asthra_test_context_create already defined above

// Note: asthra_benchmark_* functions are used directly (no macros needed)

#define asthra_record_ai_feedback(ctx, type, msg) asthra_v12_record_ai_feedback(ctx, type, msg)
#define asthra_generate_test_report(file) asthra_v12_generate_test_report(file)
#define asthra_verify_c17_compliance(file) asthra_v12_verify_c17_compliance(file)
#define asthra_run_static_analysis(file) asthra_v12_run_static_analysis(file)
#define asthra_print_comprehensive_summary() asthra_v12_print_comprehensive_summary()

#define run_comprehensive_test_suite() run_v1_2_comprehensive_test_suite()
#define run_category_tests(cat) run_v1_2_category_tests(cat)
#define run_performance_benchmarks() run_v1_2_performance_benchmarks()
#define run_security_validation() run_v1_2_security_validation()

// =============================================================================
// ORIGINAL TEST DECLARATIONS (for compatibility)
// =============================================================================

// Grammar and parsing tests
AsthraTestResult test_grammar_postfix_expr_disambiguation(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_precedence_rules(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_zero_parsing_conflicts(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_semantic_tags_parsing(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_ffi_annotations_parsing(AsthraV12TestContext *ctx);

// Pattern matching tests
AsthraTestResult test_pattern_match_statement_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_if_let_constructs(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_destructuring_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_result_type_syntax(AsthraV12TestContext *ctx);

// String operations tests
AsthraTestResult test_string_concatenation_syntax(AsthraV12TestContext *ctx);
// String interpolation test removed - feature deprecated for AI generation efficiency
AsthraTestResult test_string_edge_case_parsing(AsthraV12TestContext *ctx);

// Slice syntax tests
AsthraTestResult test_slice_creation_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_len_access_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_array_literal_syntax(AsthraV12TestContext *ctx);

// Pattern matching and error handling
AsthraTestResult test_result_exhaustive_matching(AsthraV12TestContext *ctx);
AsthraTestResult test_result_all_cases_handled(AsthraV12TestContext *ctx);
AsthraTestResult test_result_nested_matching(AsthraV12TestContext *ctx);
AsthraTestResult test_result_complex_patterns(AsthraV12TestContext *ctx);

// Error propagation tests
AsthraTestResult test_error_propagation_ffi_boundaries(AsthraV12TestContext *ctx);
AsthraTestResult test_error_propagation_task_boundaries(AsthraV12TestContext *ctx);
AsthraTestResult test_error_propagation_deep_call_stack(AsthraV12TestContext *ctx);

// Performance tests
AsthraTestResult test_pattern_matching_zero_cost(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_matching_benchmark(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_matching_optimization(AsthraV12TestContext *ctx);

// String operations
AsthraTestResult test_string_deterministic_concatenation(AsthraTestContext *ctx);
AsthraTestResult test_string_deterministic_interpolation(AsthraTestContext *ctx);
AsthraTestResult test_string_cross_platform_consistency(AsthraTestContext *ctx);

// String performance tests
AsthraTestResult test_string_concatenation_performance(AsthraTestContext *ctx);
// String interpolation performance test removed - feature deprecated for AI generation efficiency
AsthraTestResult test_string_performance_comparison(AsthraTestContext *ctx);

// Memory safety tests
AsthraTestResult test_string_gc_interaction(AsthraTestContext *ctx);
AsthraTestResult test_string_memory_management(AsthraTestContext *ctx);
AsthraTestResult test_string_large_operations(AsthraTestContext *ctx);

// FFI integration tests
AsthraTestResult test_string_c_string_conversion(AsthraTestContext *ctx);
AsthraTestResult test_string_ffi_ownership_transfer(AsthraTestContext *ctx);
AsthraTestResult test_string_ffi_borrowed_references(AsthraTestContext *ctx);

// Security tests
AsthraTestResult test_security_constant_time_verification(AsthraV12TestContext *ctx);
AsthraTestResult test_security_constant_time_operations(AsthraV12TestContext *ctx);
AsthraTestResult test_security_timing_attack_resistance(AsthraV12TestContext *ctx);

AsthraTestResult test_security_memory_zeroing(AsthraV12TestContext *ctx);
AsthraTestResult test_security_volatile_memory_access(AsthraV12TestContext *ctx);
AsthraTestResult test_security_memory_protection(AsthraV12TestContext *ctx);

AsthraTestResult test_security_csprng_functionality(AsthraV12TestContext *ctx);
AsthraTestResult test_security_entropy_testing(AsthraV12TestContext *ctx);
AsthraTestResult test_security_random_quality(AsthraV12TestContext *ctx);

AsthraTestResult test_security_side_channel_resistance(AsthraV12TestContext *ctx);
AsthraTestResult test_security_cache_timing_resistance(AsthraV12TestContext *ctx);
AsthraTestResult test_security_power_analysis_resistance(AsthraV12TestContext *ctx);

// Additional helper functions
void asthra_security_print_separator(void);

// Legacy alias
#define asthra_v12_security_print_separator asthra_security_print_separator

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_COMPREHENSIVE_H
