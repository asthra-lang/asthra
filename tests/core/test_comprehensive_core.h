/**
 * Asthra Programming Language Comprehensive Test Suite - Core Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core type definitions and function declarations for the comprehensive test suite.
 */

#ifndef ASTHRA_TEST_COMPREHENSIVE_CORE_H
#define ASTHRA_TEST_COMPREHENSIVE_CORE_H

#include "../framework/test_framework.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST CONSTANTS
// =============================================================================

#define ASTHRA_TEST_TIMEOUT_DEFAULT_NS (5000000000ULL) // 5 seconds
#define ASTHRA_TEST_TIMEOUT_LONG_NS (30000000000ULL)   // 30 seconds

// =============================================================================
// TEST CATEGORIES AND ENUMS
// =============================================================================

// Test categories for features
typedef enum {
    ASTHRA_CATEGORY_GRAMMAR = 0,
    ASTHRA_CATEGORY_PATTERN_MATCHING,
    ASTHRA_CATEGORY_STRING_OPERATIONS,
    ASTHRA_CATEGORY_SLICE_MANAGEMENT,
    ASTHRA_CATEGORY_ENHANCED_FFI,
    ASTHRA_CATEGORY_CONCURRENCY,
    ASTHRA_CATEGORY_SECURITY,
    ASTHRA_CATEGORY_INTEGRATION,
    ASTHRA_CATEGORY_PERFORMANCE,
    ASTHRA_CATEGORY_COUNT
} AsthraTestCategory;

// Test complexity levels
typedef enum {
    ASTHRA_COMPLEXITY_BASIC = 0,
    ASTHRA_COMPLEXITY_INTERMEDIATE,
    ASTHRA_COMPLEXITY_ADVANCED,
    ASTHRA_COMPLEXITY_STRESS
} AsthraTestComplexity;

// Test execution modes
typedef enum {
    ASTHRA_MODE_UNIT = 0,
    ASTHRA_MODE_INTEGRATION,
    ASTHRA_MODE_PERFORMANCE,
    ASTHRA_MODE_SECURITY
} AsthraTestMode;

// =============================================================================
// TEST STRUCTURES
// =============================================================================

// Performance benchmark results
typedef struct {
    uint64_t min_duration_ns;
    uint64_t max_duration_ns;
    uint64_t avg_duration_ns;
    uint64_t median_duration_ns;
    uint64_t std_deviation_ns;
    size_t iterations;
    size_t memory_peak_bytes;
    size_t memory_avg_bytes;
    double throughput_ops_per_sec;
} AsthraBenchmarkResult;

// Security validation results
typedef struct {
    bool constant_time_verified;
    bool side_channel_resistant;
    bool memory_secure_zeroed;
    bool entropy_sufficient;
    uint64_t timing_variance_ns;
    size_t timing_samples;
    const char *security_notes;
} AsthraSecurityResult;

// Extended test metadata for features
typedef struct {
    AsthraTestMetadata base;
    AsthraTestCategory category;
    AsthraTestComplexity complexity;
    AsthraTestMode mode;
    const char *feature_description;
    const char *ai_feedback_notes;
    bool requires_c17_compliance;
    bool requires_security_validation;
    bool requires_performance_benchmark;
    uint64_t expected_max_duration_ns;
    size_t memory_limit_bytes;
} AsthraExtendedTestMetadata;

// Comprehensive test context
typedef struct {
    AsthraTestContext base;
    AsthraExtendedTestMetadata extended_metadata;
    AsthraBenchmarkResult benchmark;
    AsthraSecurityResult security;
    void *test_data;
    size_t test_data_size;
    _Atomic(bool) should_abort;
    uint64_t iteration_count;
} AsthraExtendedTestContext;

// =============================================================================
// CORE FUNCTION DECLARATIONS
// =============================================================================

// Context management
AsthraExtendedTestContext *
asthra_test_context_create_extended(const AsthraExtendedTestMetadata *metadata);
void asthra_test_context_destroy_extended(AsthraExtendedTestContext *ctx);
void asthra_test_context_reset_extended(AsthraExtendedTestContext *ctx);

// Global statistics management
void asthra_set_global_stats(AsthraTestStatistics *stats);
AsthraTestStatistics *asthra_get_global_stats(void);

// =============================================================================
// BACKWARD COMPATIBILITY ALIASES
// =============================================================================

// Legacy type aliases for backward compatibility
typedef AsthraTestCategory AsthraV12TestCategory;
typedef AsthraTestComplexity AsthraV12TestComplexity;
typedef AsthraTestMode AsthraV12TestMode;
typedef AsthraBenchmarkResult AsthraV12BenchmarkResult;
typedef AsthraSecurityResult AsthraV12SecurityResult;
typedef AsthraExtendedTestMetadata AsthraV12TestMetadata;
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
#define ASTHRA_V1_2_CATEGORY_INTEGRATION ASTHRA_CATEGORY_INTEGRATION
#define ASTHRA_V1_2_CATEGORY_PERFORMANCE ASTHRA_CATEGORY_PERFORMANCE
#define ASTHRA_V1_2_CATEGORY_COUNT ASTHRA_CATEGORY_COUNT

#define ASTHRA_V1_2_COMPLEXITY_BASIC ASTHRA_COMPLEXITY_BASIC
#define ASTHRA_V1_2_COMPLEXITY_INTERMEDIATE ASTHRA_COMPLEXITY_INTERMEDIATE
#define ASTHRA_V1_2_COMPLEXITY_ADVANCED ASTHRA_COMPLEXITY_ADVANCED
#define ASTHRA_V1_2_COMPLEXITY_STRESS ASTHRA_COMPLEXITY_STRESS

#define ASTHRA_V1_2_MODE_UNIT ASTHRA_MODE_UNIT
#define ASTHRA_V1_2_MODE_INTEGRATION ASTHRA_MODE_INTEGRATION
#define ASTHRA_V1_2_MODE_PERFORMANCE ASTHRA_MODE_PERFORMANCE
#define ASTHRA_V1_2_MODE_SECURITY ASTHRA_MODE_SECURITY

// Legacy function aliases
#define asthra_test_context_create asthra_test_context_create_extended
#define asthra_v12_test_context_destroy asthra_test_context_destroy_extended
#define asthra_v12_test_context_reset asthra_test_context_reset_extended
#define asthra_v12_set_global_stats asthra_set_global_stats
#define asthra_v12_get_global_stats asthra_get_global_stats

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_COMPREHENSIVE_CORE_H
