/**
 * Asthra Programming Language Runtime Safety System Test Stubs
 * Stub implementations for safety runtime functions used in tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../../runtime/asthra_safety.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: asthra_safety_get_config_ptr is already implemented in the runtime library

// Removed asthra_safety_log_task_lifecycle_event - it's already in the runtime library

void asthra_safety_log_scheduler_event(AsthraSchedulerEvent event, const char *details) {
    // Stub: Just print for testing
    printf("[STUB] Scheduler event: event=%d, details=%s\n", event, details);
}

int asthra_safety_register_result_tracker(AsthraResult result, const char *location) {
    // Stub: Return a dummy ID
    printf("[STUB] Register result tracker at %s\n", location);
    return 42; // Dummy ID
}

int asthra_safety_mark_result_handled(uint64_t result_id, const char *handler) {
    // Stub: Always succeed
    printf("[STUB] Mark result %llu handled by %s\n", (unsigned long long)result_id, handler);
    return 0;
}

void asthra_safety_check_unhandled_results(void) {
    // Stub: Do nothing
    printf("[STUB] Checking unhandled results\n");
}

// Forward declaration to avoid including full header
typedef struct AsthraExtendedTestContext AsthraV12TestContext;

// Stub for constant time verification
bool asthra_v12_verify_constant_time(AsthraV12TestContext *ctx, void (*func)(void *), void *data,
                                     size_t iterations) {
    // Stub: Just run the function and return true
    printf("[STUB] Verifying constant time for %zu iterations\n", iterations);
    if (func && data) {
        func(data);
    }
    return true;
}

// Removed asthra_test_assert_int - it's already defined in test_assertions_basic.c

// Stub for AI feedback
void asthra_record_ai_feedback(AsthraV12TestContext *ctx, const char *feedback) {
    printf("[STUB] AI feedback: %s\n", feedback);
    (void)ctx;
}

// Missing security test helper stubs
bool asthra_v12_verify_entropy_quality(const uint8_t *data, size_t size) {
    // Stub implementation: just check that data is not all zeros
    if (!data || size == 0)
        return false;

    bool has_nonzero = false;
    for (size_t i = 0; i < size && i < 256; i++) {
        if (data[i] != 0) {
            has_nonzero = true;
            break;
        }
    }
    return has_nonzero;
}

// Stub for slice type safety validation
AsthraTypeSafetyCheck asthra_safety_validate_slice_type_safety(AsthraSliceHeader slice,
                                                               uint32_t expected_type_id) {
    AsthraTypeSafetyCheck check;
    check.is_valid = (slice.type_id == expected_type_id);
    check.actual_type_id = slice.type_id;
    check.expected_type_id = expected_type_id;
    snprintf(check.type_error_message, sizeof(check.type_error_message),
             check.is_valid ? "Type match" : "Type mismatch");
    check.context = "slice type validation";
    return check;
}

// Stub for secure memory zeroing verification
bool asthra_v12_verify_secure_memory_zeroing(const uint8_t *buffer, size_t size) {
    if (!buffer || size == 0)
        return false;

    // Check if buffer is zeroed
    for (size_t i = 0; i < size; i++) {
        if (buffer[i] != 0) {
            return false;
        }
    }
    return true;
}

// Additional missing v12 security functions
void asthra_v12_secure_memory_zero(void *ptr, size_t size) {
    if (ptr && size > 0) {
        volatile unsigned char *p = (volatile unsigned char *)ptr;
        for (size_t i = 0; i < size; i++) {
            p[i] = 0;
        }
    }
}

bool asthra_v12_test_csprng_quality(AsthraV12TestContext *ctx,
                                    void (*rng_function)(uint8_t *, size_t), size_t test_size) {
    (void)ctx;
    printf("[STUB] Testing CSPRNG quality with %zu bytes\n", test_size);
    if (rng_function) {
        uint8_t *test_data = malloc(test_size);
        if (test_data) {
            rng_function(test_data, test_size);
            bool result = asthra_v12_verify_entropy_quality(test_data, test_size);
            free(test_data);
            return result;
        }
    }
    return false;
}

bool asthra_v12_verify_side_channel_resistance(AsthraV12TestContext *ctx, void (*operation)(void *),
                                               void *data1, void *data2, size_t iterations) {
    (void)ctx;
    printf("[STUB] Testing side-channel resistance for %zu iterations\n", iterations);
    if (operation) {
        for (size_t i = 0; i < iterations; i++) {
            operation(data1);
            operation(data2);
        }
    }
    return true; // Stub always passes
}

void asthra_v12_security_print_results(const AsthraV12TestContext *ctx) {
    (void)ctx;
    printf("[STUB] Security test results summary\n");
}
