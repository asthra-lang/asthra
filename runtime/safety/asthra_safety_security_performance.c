/**
 * Asthra Programming Language Runtime Safety - Security & Performance Monitoring
 * Implementation of constant-time verification, secure zeroing, stack canaries, fault injection,
 * and performance monitoring
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_security_performance.h"
#include "asthra_runtime.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External references to global state from asthra_safety.c
extern AsthraSafetyConfig g_safety_config;
extern pthread_mutex_t g_safety_mutex;
extern AsthraSafetyPerformanceMetrics g_performance_metrics;
extern AsthraFaultInjectionConfig g_fault_configs[8];

// =============================================================================
// CONSTANT-TIME OPERATION VERIFICATION IMPLEMENTATION
// =============================================================================

AsthraConstantTimeVerification
asthra_safety_verify_constant_time_operation(const char *operation_name, void (*operation)(void),
                                             uint64_t expected_duration_ns) {
    AsthraConstantTimeVerification verification = {0};

    if (!g_safety_config.enable_constant_time_verification || !operation) {
        verification.is_constant_time = true;
        return verification;
    }

    verification.operation_name = operation_name;
    verification.expected_duration_ns = expected_duration_ns;

    // Measure operation timing multiple times
    const int num_measurements = 10;
    uint64_t measurements[num_measurements];

    for (int i = 0; i < num_measurements; i++) {
        uint64_t start = asthra_get_timestamp_ns();
        operation();
        uint64_t end = asthra_get_timestamp_ns();
        measurements[i] = end - start;
    }

    // Calculate statistics
    uint64_t total = 0;
    uint64_t min_time = measurements[0];
    uint64_t max_time = measurements[0];

    for (int i = 0; i < num_measurements; i++) {
        total += measurements[i];
        if (measurements[i] < min_time)
            min_time = measurements[i];
        if (measurements[i] > max_time)
            max_time = measurements[i];
    }

    verification.actual_duration_ns = total / num_measurements;
    verification.timing_variance = (double)(max_time - min_time) / verification.actual_duration_ns;

    // Consider constant-time if variance is below threshold (e.g., 10%)
    verification.is_constant_time = verification.timing_variance < 0.1;

    verification.start_timestamp_ns = asthra_get_timestamp_ns();
    verification.end_timestamp_ns = verification.start_timestamp_ns;

    snprintf(verification.timing_analysis, sizeof(verification.timing_analysis),
             "Operation '%s': avg=%lu ns, variance=%.2f%%, constant_time=%s",
             operation_name ? operation_name : "unknown", verification.actual_duration_ns,
             verification.timing_variance * 100.0, verification.is_constant_time ? "yes" : "no");

    if (!verification.is_constant_time) {
        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                       "Non-constant-time operation detected", __FILE__, __LINE__,
                                       __func__, &verification, sizeof(verification));
    }

    return verification;
}

// =============================================================================
// SECURE MEMORY ZEROING VALIDATION IMPLEMENTATION
// =============================================================================

AsthraSecureZeroingValidation asthra_safety_validate_secure_zeroing(void *ptr, size_t size) {
    AsthraSecureZeroingValidation validation = {0};

    if (!g_safety_config.enable_secure_memory_validation) {
        validation.was_properly_zeroed = true;
        return validation;
    }

    validation.memory_ptr = ptr;
    validation.memory_size = size;
    validation.zeroing_timestamp_ns = asthra_get_timestamp_ns();

    if (!ptr || size == 0) {
        validation.was_properly_zeroed = false;
        snprintf(validation.validation_details, sizeof(validation.validation_details),
                 "Invalid parameters: ptr=%p, size=%zu", ptr, size);
        return validation;
    }

    // Check if memory is actually zeroed
    unsigned char *bytes = (unsigned char *)ptr;
    validation.non_zero_bytes_found = 0;

    for (size_t i = 0; i < size; i++) {
        if (bytes[i] != 0) {
            validation.non_zero_bytes_found++;
        }
    }

    validation.was_properly_zeroed = (validation.non_zero_bytes_found == 0);

    if (!validation.was_properly_zeroed) {
        snprintf(validation.validation_details, sizeof(validation.validation_details),
                 "Found %zu non-zero bytes out of %zu total bytes", validation.non_zero_bytes_found,
                 size);

        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                       "Secure memory not properly zeroed", __FILE__, __LINE__,
                                       __func__, &validation, sizeof(validation));
    }

    return validation;
}

// =============================================================================
// STACK CANARY MANAGEMENT IMPLEMENTATION
// =============================================================================

void asthra_safety_remove_stack_canary(void) {
    if (!g_safety_config.enable_stack_canaries) {
        return;
    }

    extern ASTHRA_THREAD_LOCAL AsthraStackCanary *g_thread_canary;

    if (g_thread_canary) {
        g_thread_canary->is_active = false;
        free(g_thread_canary);
        g_thread_canary = NULL;
    }
}

// =============================================================================
// FAULT INJECTION TESTING IMPLEMENTATION
// =============================================================================

int asthra_safety_enable_fault_injection(AsthraFaultType fault_type, double probability) {
    if (!g_safety_config.enable_fault_injection_testing || fault_type >= 8 || probability < 0.0 ||
        probability > 1.0) {
        return -1;
    }

    g_fault_configs[fault_type].injection_probability = probability;
    g_fault_configs[fault_type].is_enabled = true;
    g_fault_configs[fault_type].injection_count = 0;
    g_fault_configs[fault_type].total_opportunities = 0;

    snprintf(g_fault_configs[fault_type].fault_description,
             sizeof(g_fault_configs[fault_type].fault_description),
             "Fault injection enabled for type %d with probability %.2f", fault_type, probability);

    return 0;
}

int asthra_safety_disable_fault_injection(AsthraFaultType fault_type) {
    if (fault_type >= 8) {
        return -1;
    }

    g_fault_configs[fault_type].is_enabled = false;
    g_fault_configs[fault_type].injection_probability = 0.0;

    return 0;
}

bool asthra_safety_should_inject_fault(AsthraFaultType fault_type) {
    if (!g_safety_config.enable_fault_injection_testing || fault_type >= 8 ||
        !g_fault_configs[fault_type].is_enabled) {
        return false;
    }

    __sync_fetch_and_add(&g_fault_configs[fault_type].total_opportunities, 1);

    // Simple random number generation for fault injection
    static uint32_t seed = 1;
    seed = seed * 1103515245 + 12345;
    double random_value = (double)(seed % 1000) / 1000.0;

    if (random_value < g_fault_configs[fault_type].injection_probability) {
        __sync_fetch_and_add(&g_fault_configs[fault_type].injection_count, 1);
        return true;
    }

    return false;
}

void asthra_safety_record_fault_injection(AsthraFaultType fault_type) {
    if (fault_type < 8) {
        asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_GENERAL,
                   "Fault injection recorded for type %d", fault_type);
    }
}

// =============================================================================
// PERFORMANCE MONITORING IMPLEMENTATION
// =============================================================================

void asthra_safety_start_performance_measurement(const char *check_name) {
    if (!g_safety_config.enable_performance_monitoring) {
        return;
    }

    // In a full implementation, this would store per-check timing information
    // For now, we just increment the check count
    __sync_fetch_and_add(&g_performance_metrics.safety_check_count, 1);
}

void asthra_safety_end_performance_measurement(const char *check_name) {
    if (!g_safety_config.enable_performance_monitoring) {
        return;
    }

    // In a full implementation, this would calculate and store timing information
    // For now, we just update the average check time with a placeholder value
    uint64_t check_time_ns = 1000; // Placeholder: 1 microsecond

    uint64_t old_time = g_performance_metrics.safety_check_time_ns;
    uint64_t old_count = g_performance_metrics.safety_check_count;

    if (old_count > 0) {
        g_performance_metrics.average_check_time_ns =
            (double)(old_time + check_time_ns) / old_count;
    }

    __sync_fetch_and_add(&g_performance_metrics.safety_check_time_ns, check_time_ns);
}

// =============================================================================
// CONFIGURATION MANAGEMENT IMPLEMENTATION
// =============================================================================

int asthra_safety_set_config(const AsthraSafetyConfig *config) {
    if (!config) {
        return -1;
    }

    pthread_mutex_lock(&g_safety_mutex);
    g_safety_config = *config;
    pthread_mutex_unlock(&g_safety_mutex);

    asthra_log(ASTHRA_LOG_INFO, ASTHRA_LOG_CATEGORY_GENERAL,
               "Safety configuration updated to level %d", config->level);

    return 0;
}

AsthraSafetyConfig asthra_safety_get_config(void) {
    pthread_mutex_lock(&g_safety_mutex);
    AsthraSafetyConfig config = g_safety_config;
    pthread_mutex_unlock(&g_safety_mutex);

    return config;
}
