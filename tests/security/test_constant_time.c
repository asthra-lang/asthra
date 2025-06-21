/**
 * Asthra Programming Language v1.2 Constant-Time Operation Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for constant-time operations and timing attack resistance.
 */

#include "test_security_helpers.h"

// Declaration for stub function
bool asthra_verify_constant_time(AsthraV12TestContext *ctx, 
                                void (*func)(void*), void *data, size_t iterations);

// =============================================================================
// CONSTANT-TIME OPERATION TESTS
// =============================================================================

AsthraTestResult test_security_constant_time_verification(AsthraV12TestContext *ctx) {
    // Test constant-time operation verification using timing analysis

    const size_t data_size = 32; // 256-bit keys
    const size_t num_samples = 1000;
    
    uint8_t *data1 = malloc(data_size);
    uint8_t *data2 = malloc(data_size);
    uint8_t *data3 = malloc(data_size);
    
    if (!data1 || !data2 || !data3) {
        free(data1);
        free(data2);
        free(data3);
        return ASTHRA_TEST_ERROR;
    }
    
    // Initialize test data
    memset(data1, 0xAA, data_size);
    memset(data2, 0xAA, data_size);
    memset(data3, 0x55, data_size);
    
    // Test 1: Constant-time comparison timing
    uint64_t *ct_times_same = malloc(num_samples * sizeof(uint64_t));
    uint64_t *ct_times_diff = malloc(num_samples * sizeof(uint64_t));
    
    if (!ct_times_same || !ct_times_diff) {
        free(data1);
        free(data2);
        free(data3);
        free(ct_times_same);
        free(ct_times_diff);
        return ASTHRA_TEST_ERROR;
    }
    
    // Measure constant-time comparison with same data
    for (size_t i = 0; i < num_samples; i++) {
        uint64_t start = rdtsc();
        volatile int result = mock_constant_time_compare(data1, data2, data_size);
        uint64_t end = rdtsc();
        ct_times_same[i] = end - start;
        (void)result; // Prevent optimization
    }
    
    // Measure constant-time comparison with different data
    for (size_t i = 0; i < num_samples; i++) {
        uint64_t start = rdtsc();
        volatile int result = mock_constant_time_compare(data1, data3, data_size);
        uint64_t end = rdtsc();
        ct_times_diff[i] = end - start;
        (void)result; // Prevent optimization
    }
    
    // Calculate statistics for constant-time operations
    double ct_same_mean = 0.0, ct_diff_mean = 0.0;
    for (size_t i = 0; i < num_samples; i++) {
        ct_same_mean += (double)ct_times_same[i];
        ct_diff_mean += (double)ct_times_diff[i];
    }
    ct_same_mean /= (double)num_samples;
    ct_diff_mean /= (double)num_samples;
    
    double ct_same_variance = 0.0, ct_diff_variance = 0.0;
    for (size_t i = 0; i < num_samples; i++) {
        double diff_same = (double)ct_times_same[i] - ct_same_mean;
        double diff_diff = (double)ct_times_diff[i] - ct_diff_mean;
        ct_same_variance += diff_same * diff_same;
        ct_diff_variance += diff_diff * diff_diff;
    }
    ct_same_variance /= (double)(num_samples - 1);
    ct_diff_variance /= (double)(num_samples - 1);
    
    double ct_same_cv = sqrt(ct_same_variance) / ct_same_mean;
    double ct_diff_cv = sqrt(ct_diff_variance) / ct_diff_mean;
    
    // Test 2: Variable-time comparison for contrast
    uint64_t *vt_times_same = malloc(num_samples * sizeof(uint64_t));
    uint64_t *vt_times_diff = malloc(num_samples * sizeof(uint64_t));
    
    if (!vt_times_same || !vt_times_diff) {
        free(data1);
        free(data2);
        free(data3);
        free(ct_times_same);
        free(ct_times_diff);
        free(vt_times_same);
        free(vt_times_diff);
        return ASTHRA_TEST_ERROR;
    }
    
    // Measure variable-time comparison
    for (size_t i = 0; i < num_samples; i++) {
        uint64_t start = rdtsc();
        volatile int result = mock_variable_time_compare(data1, data2, data_size);
        uint64_t end = rdtsc();
        vt_times_same[i] = end - start;
        (void)result;
    }
    
    for (size_t i = 0; i < num_samples; i++) {
        uint64_t start = rdtsc();
        volatile int result = mock_variable_time_compare(data1, data3, data_size);
        uint64_t end = rdtsc();
        vt_times_diff[i] = end - start;
        (void)result;
    }
    
    // Calculate statistics for variable-time operations
    double vt_same_mean = 0.0, vt_diff_mean = 0.0;
    for (size_t i = 0; i < num_samples; i++) {
        vt_same_mean += (double)vt_times_same[i];
        vt_diff_mean += (double)vt_times_diff[i];
    }
    vt_same_mean /= (double)num_samples;
    vt_diff_mean /= (double)num_samples;
    
    // Constant-time operations should have similar timing regardless of data
    double ct_timing_difference = fabs(ct_same_mean - ct_diff_mean) / ct_same_mean;
    double vt_timing_difference = fabs(vt_same_mean - vt_diff_mean) / vt_same_mean;
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, ct_timing_difference < 0.1,
                           "Constant-time timing difference too high: %.3f", ct_timing_difference)) {
        goto cleanup;
    }
    
    // Variable-time should show more timing difference (for validation)
    if (!ASTHRA_TEST_ASSERT(&ctx->base, vt_timing_difference > ct_timing_difference,
                           "Variable-time should show more timing variation")) {
        goto cleanup;
    }
    
    // Coefficient of variation should be low for constant-time
    if (!ASTHRA_TEST_ASSERT(&ctx->base, ct_same_cv < 0.2 && ct_diff_cv < 0.2,
                           "Constant-time coefficient of variation too high: %.3f, %.3f", 
                           ct_same_cv, ct_diff_cv)) {
        goto cleanup;
    }
    
    // Update security context
    ctx->security.constant_time_verified = true;
    ctx->security.timing_variance_ns = (uint64_t)(ct_same_variance + ct_diff_variance) / 2;
    ctx->security.timing_samples = num_samples * 2;
    
    asthra_v12_record_ai_feedback(ctx, "SECURITY_VALIDATION", 
                                 "Constant-time operations verified with statistical analysis");
    
    free(data1);
    free(data2);
    free(data3);
    free(ct_times_same);
    free(ct_times_diff);
    free(vt_times_same);
    free(vt_times_diff);
    
    return ASTHRA_TEST_PASS;

cleanup:
    free(data1);
    free(data2);
    free(data3);
    free(ct_times_same);
    free(ct_times_diff);
    free(vt_times_same);
    free(vt_times_diff);
    return ASTHRA_TEST_FAIL;
}

// Helper function for constant-time selection test
static void constant_time_select_test(void *data) {
    uint8_t *test_data = (uint8_t*)data;
    uint8_t a = test_data[0];
    uint8_t b = test_data[1];
    uint8_t condition = test_data[2];
    
    // Constant-time selection: result = condition ? a : b
    uint8_t mask = -(condition & 1); // 0x00 or 0xFF
    volatile uint8_t result = (a & mask) | (b & ~mask);
    test_data[3] = result;
}

AsthraTestResult test_security_constant_time_operations(AsthraV12TestContext *ctx) {
    // Test specific constant-time operations
    
    uint8_t test_data[4] = {0xAA, 0x55, 1, 0};
    
    if (!asthra_v12_verify_constant_time(ctx, constant_time_select_test, test_data, 1000)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (test_data[3] != 0xAA) {
        printf("Constant-time selection failed: expected 0xAA, got 0x%02X\n", test_data[3]);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test with condition = 0
    test_data[2] = 0;
    test_data[3] = 0;
    constant_time_select_test(test_data);
    
    if (test_data[3] != 0x55) {
        printf("Constant-time selection failed: expected 0x55, got 0x%02X\n", test_data[3]);
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_security_timing_attack_resistance(AsthraV12TestContext *ctx) {
    // Test resistance to timing attacks
    
    const size_t key_size = 32;
    const size_t num_tests = 500;
    
    uint8_t secret_key[32];
    memset(secret_key, 0x42, sizeof(secret_key));
    
    uint64_t *timing_samples = malloc(num_tests * sizeof(uint64_t));
    if (!timing_samples) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Test timing with various inputs
    for (size_t i = 0; i < num_tests; i++) {
        uint8_t test_key[32];
        
        // Create test keys with different patterns
        if (i < num_tests / 4) {
            // All zeros
            memset(test_key, 0x00, sizeof(test_key));
        } else if (i < num_tests / 2) {
            // All ones
            memset(test_key, 0xFF, sizeof(test_key));
        } else if (i < 3 * num_tests / 4) {
            // Random pattern
            for (size_t j = 0; j < sizeof(test_key); j++) {
                test_key[j] = (uint8_t)(rand() & 0xFF);
            }
        } else {
            // Partially matching secret
            memcpy(test_key, secret_key, sizeof(test_key));
            test_key[i % sizeof(test_key)] ^= 0x01; // Flip one bit
        }
        
        uint64_t start = rdtsc();
        volatile int result = mock_constant_time_compare(secret_key, test_key, key_size);
        uint64_t end = rdtsc();
        
        timing_samples[i] = end - start;
        (void)result;
    }
    
    // Analyze timing distribution
    double mean = 0.0;
    for (size_t i = 0; i < num_tests; i++) {
        mean += (double)timing_samples[i];
    }
    mean /= (double)num_tests;
    
    double variance = 0.0;
    for (size_t i = 0; i < num_tests; i++) {
        double diff = (double)timing_samples[i] - mean;
        variance += diff * diff;
    }
    variance /= (double)(num_tests - 1);
    
    double cv = sqrt(variance) / mean;
    
    // Timing should be consistent (low coefficient of variation)
    if (!ASTHRA_TEST_ASSERT(&ctx->base, cv < 0.15,
                           "Timing attack vulnerability detected: CV = %.3f", cv)) {
        free(timing_samples);
        return ASTHRA_TEST_FAIL;
    }
    
    ctx->security.timing_variance_ns = (uint64_t)variance;
    ctx->security.side_channel_resistant = true;
    
    free(timing_samples);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Constant Time Operations Test ===\n");
    
    // Create test context
    AsthraV12TestMetadata metadata = {
        .base = {
            .name = "Constant Time Operations",
            .file = __FILE__,
            .line = __LINE__,
            .function = "main",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 30000000000ULL,
            .skip = false,
            .skip_reason = NULL
        },
        .category = ASTHRA_V1_2_CATEGORY_SECURITY,
        .complexity = ASTHRA_V1_2_COMPLEXITY_ADVANCED,
        .mode = ASTHRA_V1_2_MODE_SECURITY,
        .feature_description = "Constant-time operation security tests",
        .ai_feedback_notes = "Validates timing attack resistance",
        .requires_c17_compliance = true,
        .requires_security_validation = true,
        .requires_performance_benchmark = false,
        .expected_max_duration_ns = 30000000000ULL,
        .memory_limit_bytes = 10 * 1024 * 1024
    };
    
    AsthraV12TestContext *ctx = asthra_test_context_create_extended(&metadata);
    if (!ctx) {
        fprintf(stderr, "Failed to create test context\n");
        return 1;
    }
    
    size_t passed = 0;
    size_t failed = 0;
    
    // Run tests
    // DISABLED: Constant-time verification requires hardware support
    // printf("\nRunning constant-time verification test...\n");
    // if (test_security_constant_time_verification(ctx) == ASTHRA_TEST_PASS) {
    //     printf("✓ PASS: Constant-time verification\n");
    //     passed++;
    // } else {
    //     printf("✗ FAIL: Constant-time verification\n");
    //     failed++;
    // }
    
    printf("\nRunning constant-time operations test...\n");
    if (test_security_constant_time_operations(ctx) == ASTHRA_TEST_PASS) {
        printf("✓ PASS: Constant-time operations\n");
        passed++;
    } else {
        printf("✗ FAIL: Constant-time operations\n");
        failed++;
    }
    
    // DISABLED: Timing attack resistance requires hardware support
    // printf("\nRunning timing attack resistance test...\n");
    // if (test_security_timing_attack_resistance(ctx) == ASTHRA_TEST_PASS) {
    //     printf("✓ PASS: Timing attack resistance\n");
    //     passed++;
    // } else {
    //     printf("✗ FAIL: Timing attack resistance\n");
    //     failed++;
    // }
    
    printf("\n=== Constant Time Tests Partially DISABLED ===\n");
    printf("Hardware-dependent tests disabled. See GITHUB_ISSUE_SECURITY.md\n");
    
    // Clean up
    asthra_test_context_destroy_extended(ctx);
    
    // Report results
    printf("\n=== Test Summary ===\n");
    printf("Passed: %zu\n", passed);
    printf("Failed: %zu\n", failed);
    printf("Total:  %zu\n", passed + failed);
    
    return failed > 0 ? 1 : 0;
} 
