/**
 * Asthra Programming Language Runtime Safety System
 * Security and Performance Monitoring Module - Security enforcement and monitoring implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_security.h"
#include "../asthra_safety_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

// External references to global state
// Use accessor function instead of direct global
#define g_safety_config (*asthra_safety_get_config_ptr())
extern pthread_mutex_t g_safety_mutex;
extern AsthraSafetyPerformanceMetrics g_performance_metrics;
extern AsthraFaultInjectionConfig g_fault_configs[8];

// Thread-local stack canary storage
static __thread AsthraStackCanary *g_thread_canary = NULL;

// =============================================================================
// STACK CANARY IMPLEMENTATION
// =============================================================================

int asthra_safety_install_stack_canary(void) {
    if (!g_safety_config.enable_stack_canaries) {
        return 0;
    }
    
    if (g_thread_canary) {
        return 0; // Already installed
    }
    
    g_thread_canary = malloc(sizeof(AsthraStackCanary));
    if (!g_thread_canary) {
        return -1;
    }
    
    g_thread_canary->canary_value = asthra_safety_generate_canary_value();
    g_thread_canary->stack_base = &g_thread_canary; // Approximate stack location
    g_thread_canary->stack_size = 8192; // Default stack size estimate
    g_thread_canary->thread_id = pthread_self();
    g_thread_canary->creation_timestamp = asthra_get_timestamp_ns();
    g_thread_canary->is_active = true;
    
    return 0;
}

int asthra_safety_check_stack_canary(void) {
    if (!g_safety_config.enable_stack_canaries || !g_thread_canary) {
        return 0;
    }
    
    if (!g_thread_canary->is_active) {
        return -1; // Canary was deactivated
    }
    
    // In a real implementation, this would check for stack corruption
    // For now, we'll assume the canary is intact unless explicitly corrupted
    return 0;
}

void asthra_safety_remove_stack_canary(void) {
    if (g_thread_canary) {
        g_thread_canary->is_active = false;
        free(g_thread_canary);
        g_thread_canary = NULL;
    }
}

// =============================================================================
// CONSTANT-TIME OPERATION VERIFICATION IMPLEMENTATION
// =============================================================================

AsthraConstantTimeVerification asthra_safety_verify_constant_time_operation(const char *operation_name, 
                                                                            void (*operation)(void), 
                                                                            uint64_t expected_duration_ns) {
    AsthraConstantTimeVerification verification = {0};
    verification.operation_name = operation_name;
    verification.expected_duration_ns = expected_duration_ns;
    
    if (!g_safety_config.enable_constant_time_verification || !operation) {
        verification.is_constant_time = true;
        return verification;
    }
    
    const size_t num_iterations = 100;
    uint64_t timings[num_iterations];
    
    // Perform multiple iterations to measure timing variance
    for (size_t i = 0; i < num_iterations; i++) {
        verification.start_timestamp_ns = asthra_get_timestamp_ns();
        operation();
        verification.end_timestamp_ns = asthra_get_timestamp_ns();
        timings[i] = verification.end_timestamp_ns - verification.start_timestamp_ns;
    }
    
    // Calculate statistics
    uint64_t total_time = 0;
    uint64_t min_time = timings[0];
    uint64_t max_time = timings[0];
    
    for (size_t i = 0; i < num_iterations; i++) {
        total_time += timings[i];
        if (timings[i] < min_time) min_time = timings[i];
        if (timings[i] > max_time) max_time = timings[i];
    }
    
    verification.actual_duration_ns = total_time / num_iterations;
    verification.timing_variance = (double)(max_time - min_time) / verification.actual_duration_ns;
    
    // Consider constant-time if variance is less than 10%
    verification.is_constant_time = (verification.timing_variance < 0.1);
    
    snprintf(verification.timing_analysis, sizeof(verification.timing_analysis),
            "Min: %lu ns, Max: %lu ns, Avg: %lu ns, Variance: %.2f%%",
            min_time, max_time, verification.actual_duration_ns, 
            verification.timing_variance * 100.0);
    
    return verification;
}

// =============================================================================
// SECURE MEMORY ZEROING VALIDATION IMPLEMENTATION
// =============================================================================

AsthraSecureZeroingValidation asthra_safety_validate_secure_zeroing(void *ptr, size_t size) {
    AsthraSecureZeroingValidation validation = {0};
    validation.memory_ptr = ptr;
    validation.memory_size = size;
    validation.zeroing_timestamp_ns = asthra_get_timestamp_ns();
    
    if (!g_safety_config.enable_secure_memory_validation || !ptr || size == 0) {
        validation.was_properly_zeroed = true;
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
    
    if (validation.was_properly_zeroed) {
        snprintf(validation.validation_details, sizeof(validation.validation_details),
                "Memory properly zeroed: %zu bytes", size);
    } else {
        snprintf(validation.validation_details, sizeof(validation.validation_details),
                "Memory not properly zeroed: %zu non-zero bytes out of %zu total",
                validation.non_zero_bytes_found, size);
    }
    
    return validation;
}

// =============================================================================
// FAULT INJECTION TESTING IMPLEMENTATION
// =============================================================================

int asthra_safety_enable_fault_injection(AsthraFaultType fault_type, double probability) {
    if (!g_safety_config.enable_fault_injection_testing) {
        return -1;
    }
    
    if (fault_type >= 8 || probability < 0.0 || probability > 1.0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    
    g_fault_configs[fault_type].fault_type = fault_type;
    g_fault_configs[fault_type].injection_probability = probability;
    g_fault_configs[fault_type].is_enabled = true;
    g_fault_configs[fault_type].injection_count = 0;
    g_fault_configs[fault_type].total_opportunities = 0;
    
    pthread_mutex_unlock(&g_safety_mutex);
    
    return 0;
}

int asthra_safety_disable_fault_injection(AsthraFaultType fault_type) {
    if (fault_type >= 8) {
        return -1;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    g_fault_configs[fault_type].is_enabled = false;
    pthread_mutex_unlock(&g_safety_mutex);
    
    return 0;
}

bool asthra_safety_should_inject_fault(AsthraFaultType fault_type) {
    if (!g_safety_config.enable_fault_injection_testing || fault_type >= 8) {
        return false;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    
    AsthraFaultInjectionConfig *config = &g_fault_configs[fault_type];
    config->total_opportunities++;
    
    if (!config->is_enabled) {
        pthread_mutex_unlock(&g_safety_mutex);
        return false;
    }
    
    // Simple pseudo-random number generation for fault injection
    static uint32_t seed = 12345;
    seed = seed * 1103515245 + 12345;
    double random_value = (double)(seed % 1000000) / 1000000.0;
    
    bool should_inject = (random_value < config->injection_probability);
    
    if (should_inject) {
        config->injection_count++;
    }
    
    pthread_mutex_unlock(&g_safety_mutex);
    
    return should_inject;
}

void asthra_safety_record_fault_injection(AsthraFaultType fault_type) {
    if (!g_safety_config.enable_fault_injection_testing || fault_type >= 8) {
        return;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    g_fault_configs[fault_type].injection_count++;
    pthread_mutex_unlock(&g_safety_mutex);
}

// =============================================================================
// PERFORMANCE MONITORING IMPLEMENTATION
// =============================================================================

void asthra_safety_start_performance_measurement(const char *check_name) {
    if (!g_safety_config.enable_performance_monitoring) {
        return;
    }
    
    // In a full implementation, this would track per-check performance
    // For now, we'll just increment the global counter
    __sync_fetch_and_add(&g_performance_metrics.safety_check_count, 1);
}

void asthra_safety_end_performance_measurement(const char *check_name) {
    if (!g_safety_config.enable_performance_monitoring) {
        return;
    }
    
    // In a full implementation, this would calculate and record timing
    uint64_t current_time = asthra_get_timestamp_ns();
    static uint64_t last_time = 0;
    
    if (last_time > 0) {
        uint64_t check_duration = current_time - last_time;
        __sync_fetch_and_add(&g_performance_metrics.total_check_time_ns, check_duration);
        
        // Update min/max times (simplified, not thread-safe)
        if (g_performance_metrics.min_check_time_ns == 0 || 
            check_duration < g_performance_metrics.min_check_time_ns) {
            g_performance_metrics.min_check_time_ns = check_duration;
        }
        if (check_duration > g_performance_metrics.max_check_time_ns) {
            g_performance_metrics.max_check_time_ns = check_duration;
        }
    }
    
    last_time = current_time;
}

AsthraSafetyPerformanceMetrics asthra_safety_get_performance_metrics(void) {
    AsthraSafetyPerformanceMetrics metrics = g_performance_metrics;
    
    // Calculate average
    if (metrics.safety_check_count > 0) {
        metrics.average_check_time_ns = (double)metrics.total_check_time_ns / metrics.safety_check_count;
    }
    
    return metrics;
}

void asthra_safety_reset_performance_metrics(void) {
    pthread_mutex_lock(&g_safety_mutex);
    memset(&g_performance_metrics, 0, sizeof(g_performance_metrics));
    pthread_mutex_unlock(&g_safety_mutex);
}

// =============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// =============================================================================

uint64_t asthra_safety_generate_canary_value(void) {
    // Use system random number generator for canary values
    uint64_t canary = 0;
    
    #if defined(__linux__) || defined(__APPLE__)
        FILE *urandom = fopen("/dev/urandom", "rb");
        if (urandom) {
            size_t bytes_read = fread(&canary, sizeof(canary), 1, urandom);
            fclose(urandom);
            if (bytes_read != 1) {
                // Fallback if read failed
                canary = (uint64_t)time(NULL) ^ (uint64_t)getpid() ^ (uint64_t)pthread_self();
            }
        } else {
            // Fallback to time-based seed
            canary = (uint64_t)time(NULL) ^ (uint64_t)getpid() ^ (uint64_t)pthread_self();
        }
    #else
        // Fallback for other systems
        canary = (uint64_t)time(NULL) ^ (uint64_t)getpid();
    #endif
    
    return canary;
}

bool asthra_safety_validate_side_channel_resistance(void *memory_access_pattern, size_t pattern_size) {
    if (!g_safety_config.enable_security_enforcement || !memory_access_pattern) {
        return true;
    }
    
    // This is a simplified implementation
    // Real side-channel analysis would be much more complex
    return true;
}

bool asthra_safety_check_timing_attack_resistance(const char *operation_name, 
                                                  void (*operation)(void *), void *data, size_t iterations) {
    if (!g_safety_config.enable_constant_time_verification || !operation) {
        return true;
    }
    
    // Perform timing analysis similar to constant-time verification
    uint64_t timings[iterations];
    
    for (size_t i = 0; i < iterations; i++) {
        uint64_t start = asthra_get_timestamp_ns();
        operation(data);
        uint64_t end = asthra_get_timestamp_ns();
        timings[i] = end - start;
    }
    
    // Calculate variance
    uint64_t total = 0;
    for (size_t i = 0; i < iterations; i++) {
        total += timings[i];
    }
    uint64_t average = total / iterations;
    
    uint64_t variance_sum = 0;
    for (size_t i = 0; i < iterations; i++) {
        uint64_t diff = (timings[i] > average) ? (timings[i] - average) : (average - timings[i]);
        variance_sum += diff * diff;
    }
    
    double variance = (double)variance_sum / iterations;
    double coefficient_variation = variance / average;
    
    // Consider timing-attack resistant if coefficient of variation is low
    return coefficient_variation < 0.05; // 5% threshold
}

bool asthra_safety_validate_control_flow_integrity(void *expected_return_address) {
    if (!g_safety_config.enable_security_enforcement) {
        return true;
    }
    
    // This would require platform-specific implementation
    // For now, just return true as a placeholder
    return true;
} 
