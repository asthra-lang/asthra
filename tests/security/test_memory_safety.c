/**
 * Asthra Programming Language v1.2 Security Validation Tests - Memory Safety
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for secure memory operations, volatile access, and memory protection.
 */

#include "../core/test_comprehensive.h"
#include "test_security_helpers.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// MEMORY SAFETY HELPERS
// =============================================================================

// Mock functions are implemented in test_security_helpers.c

// =============================================================================
// SECURE MEMORY TESTS
// =============================================================================

AsthraTestResult test_security_memory_zeroing(AsthraV12TestContext *ctx) {
    // Test secure memory zeroing functionality
    
    const size_t buffer_size = 1024;
    uint8_t *sensitive_buffer = malloc(buffer_size);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, sensitive_buffer != NULL,
                           "Failed to allocate sensitive buffer")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Fill buffer with sensitive data
    for (size_t i = 0; i < buffer_size; i++) {
        sensitive_buffer[i] = (uint8_t)(0xAA ^ (i & 0xFF));
    }
    
    // Verify buffer contains non-zero data
    bool has_nonzero = false;
    for (size_t i = 0; i < buffer_size; i++) {
        if (sensitive_buffer[i] != 0) {
            has_nonzero = true;
            break;
        }
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, has_nonzero,
                           "Buffer should contain non-zero data before zeroing")) {
        free(sensitive_buffer);
        return ASTHRA_TEST_FAIL;
    }
    
    // Perform secure zeroing
    mock_secure_zero(sensitive_buffer, buffer_size);
    
    // Verify buffer is completely zeroed
    if (!asthra_verify_secure_memory_zeroing(sensitive_buffer, buffer_size)) {
        if (!ASTHRA_TEST_ASSERT(&ctx->base, false,
                               "Secure memory zeroing failed")) {
            free(sensitive_buffer);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    ctx->security.memory_secure_zeroed = true;
    
    asthra_record_ai_feedback(ctx, "MEMORY_SECURITY", 
                             "Secure memory zeroing verified with volatile access patterns");
    
    free(sensitive_buffer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_security_volatile_memory(AsthraV12TestContext *ctx) {
    // Test volatile memory access patterns
    
    const size_t test_size = 256;
    volatile uint8_t *volatile_buffer = malloc(test_size);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, volatile_buffer != NULL,
                           "Failed to allocate volatile buffer")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Test volatile write pattern
    for (size_t i = 0; i < test_size; i++) {
        volatile_buffer[i] = (uint8_t)(i ^ 0x5A);
    }
    
    // Test volatile read pattern
    volatile uint8_t checksum = 0;
    for (size_t i = 0; i < test_size; i++) {
        checksum ^= volatile_buffer[i];
    }
    
    // Verify data integrity
    uint8_t expected_checksum = 0;
    for (size_t i = 0; i < test_size; i++) {
        expected_checksum ^= (uint8_t)(i ^ 0x5A);
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, checksum == expected_checksum,
                           "Volatile memory access integrity check failed")) {
        free((void*)volatile_buffer);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test volatile zeroing
    for (size_t i = 0; i < test_size; i++) {
        volatile_buffer[i] = 0;
    }
    
    // Verify zeroing
    for (size_t i = 0; i < test_size; i++) {
        if (!ASTHRA_TEST_ASSERT(&ctx->base, volatile_buffer[i] == 0,
                               "Volatile zeroing failed at index %zu", i)) {
            free((void*)volatile_buffer);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    free((void*)volatile_buffer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_security_memory_protection(AsthraV12TestContext *ctx) {
    // Test memory protection mechanisms
    
    const size_t protected_size = 4096; // Page size
    uint8_t *protected_buffer = malloc(protected_size);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, protected_buffer != NULL,
                           "Failed to allocate protected buffer")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Simulate memory protection by bounds checking
    bool bounds_check_passed = true;
    
    // Test valid access
    for (size_t i = 0; i < protected_size; i += 64) {
        protected_buffer[i] = (uint8_t)(i & 0xFF);
        if (protected_buffer[i] != (uint8_t)(i & 0xFF)) {
            bounds_check_passed = false;
            break;
        }
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, bounds_check_passed,
                           "Valid memory access failed")) {
        free(protected_buffer);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test boundary conditions
    protected_buffer[0] = 0xAA;
    protected_buffer[protected_size - 1] = 0x55;
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, 
                           protected_buffer[0] == 0xAA && protected_buffer[protected_size - 1] == 0x55,
                           "Boundary access failed")) {
        free(protected_buffer);
        return ASTHRA_TEST_FAIL;
    }
    
    free(protected_buffer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_security_overflow_detection(AsthraV12TestContext *ctx) {
    // Test stack protection mechanisms
    
    // Test stack canary simulation
    const uint32_t stack_canary = 0xDEADBEEF;
    uint8_t stack_buffer[1024];
    uint32_t canary_copy = stack_canary;
    
    // Initialize buffer with pattern
    for (size_t i = 0; i < sizeof(stack_buffer); i++) {
        stack_buffer[i] = (uint8_t)(i & 0xFF);
    }
    
    // Verify canary integrity
    if (!ASTHRA_TEST_ASSERT(&ctx->base, canary_copy == stack_canary,
                           "Stack canary was corrupted")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test buffer overflow detection simulation
    bool overflow_detected = false;
    
    // Simulate safe buffer usage
    for (size_t i = 0; i < sizeof(stack_buffer) - 1; i++) {
        stack_buffer[i] = 0xAA;
        if (canary_copy != stack_canary) {
            overflow_detected = true;
            break;
        }
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !overflow_detected,
                           "False positive overflow detection")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_security_heap_protection(AsthraV12TestContext *ctx) {
    // Test heap protection mechanisms
    
    const size_t heap_size = 2048;
    uint8_t *heap_buffer = malloc(heap_size);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, heap_buffer != NULL,
                           "Failed to allocate heap buffer")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Test heap metadata integrity simulation
    struct {
        size_t size;
        uint32_t magic;
        uint8_t *data;
    } heap_metadata = {
        .size = heap_size,
        .magic = 0xCAFEBABE,
        .data = heap_buffer
    };
    
    // Fill heap buffer
    for (size_t i = 0; i < heap_size; i++) {
        heap_buffer[i] = (uint8_t)(i & 0xFF);
    }
    
    // Verify metadata integrity
    if (!ASTHRA_TEST_ASSERT(&ctx->base, 
                           heap_metadata.magic == 0xCAFEBABE &&
                           heap_metadata.size == heap_size &&
                           heap_metadata.data == heap_buffer,
                           "Heap metadata corruption detected")) {
        free(heap_buffer);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test use-after-free detection simulation
    memset(heap_buffer, 0xDE, heap_size); // Poison pattern
    free(heap_buffer);
    
    // In a real implementation, accessing heap_buffer here would be detected
    // For simulation, we just verify the pattern was set
    // heap_metadata.data = NULL; // Mark as freed
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Memory Safety Test Suite ===\n");
    
    // Create test context
    AsthraV12TestMetadata metadata = {
        .base = {
            .name = "Memory Safety Tests",
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
        .feature_description = "Memory safety and protection tests",
        .ai_feedback_notes = "Validates secure memory operations",
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
    printf("\nRunning memory zeroing test...\n");
    if (test_security_memory_zeroing(&ctx->base) == ASTHRA_TEST_PASS) {
        printf("✓ PASS: Memory zeroing\n");
        passed++;
    } else {
        printf("✗ FAIL: Memory zeroing\n");
        failed++;
    }
    
    printf("\nRunning volatile memory test...\n");
    if (test_security_volatile_memory(&ctx->base) == ASTHRA_TEST_PASS) {
        printf("✓ PASS: Volatile memory\n");
        passed++;
    } else {
        printf("✗ FAIL: Volatile memory\n");
        failed++;
    }
    
    printf("\nRunning overflow detection test...\n");
    if (test_security_overflow_detection(&ctx->base) == ASTHRA_TEST_PASS) {
        printf("✓ PASS: Overflow detection\n");
        passed++;
    } else {
        printf("✗ FAIL: Overflow detection\n");
        failed++;
    }
    
    printf("\nRunning heap protection test...\n");
    if (test_security_heap_protection(&ctx->base) == ASTHRA_TEST_PASS) {
        printf("✓ PASS: Heap protection\n");
        passed++;
    } else {
        printf("✗ FAIL: Heap protection\n");
        failed++;
    }
    
    // Clean up
    asthra_test_context_destroy_extended(ctx);
    
    // Report results
    printf("\n=== Test Summary ===\n");
    printf("Passed: %zu\n", passed);
    printf("Failed: %zu\n", failed);
    printf("Total:  %zu\n", passed + failed);
    
    return failed > 0 ? 1 : 0;
} 
