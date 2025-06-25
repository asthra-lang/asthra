/**
 * Comprehensive Security Tests - Utility Functions
 *
 * Output formatting and utility functions for security test reporting.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "comprehensive_security_common.h"

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

void asthra_v12_security_print_results(const AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    printf("\n=== Security Validation Results ===\n");
    printf("Constant-time verified: %s\n", ctx->security.constant_time_verified ? "PASS" : "FAIL");
    printf("Side-channel resistant: %s\n", ctx->security.side_channel_resistant ? "PASS" : "FAIL");
    printf("Memory secure zeroed: %s\n", ctx->security.memory_secure_zeroed ? "PASS" : "FAIL");
    printf("Entropy sufficient: %s\n", ctx->security.entropy_sufficient ? "PASS" : "FAIL");
    printf("Timing variance: %llu ns\n", (unsigned long long)ctx->security.timing_variance_ns);
    printf("Timing samples: %zu\n", ctx->security.timing_samples);

    if (ctx->security.security_notes) {
        printf("Security notes: %s\n", ctx->security.security_notes);
    }
}

void asthra_v12_security_print_separator(void) {
    printf("="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "="
           "\n");
}