/**
 * Asthra Programming Language Runtime Safety System
 * Security and Performance Monitoring Module - Security enforcement and monitoring
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_SECURITY_H
#define ASTHRA_SAFETY_SECURITY_H

#include "asthra_safety_minimal_includes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SECURITY STRUCTURES
// =============================================================================

// Stack canary management
typedef struct {
    uint64_t canary_value;
    void *stack_base;
    size_t stack_size;
    pthread_t thread_id;
    uint64_t creation_timestamp;
    bool is_active;
} AsthraStackCanary;

// Constant-time operation verification
typedef struct {
    const char *operation_name;
    uint64_t start_timestamp_ns;
    uint64_t end_timestamp_ns;
    uint64_t expected_duration_ns;
    uint64_t actual_duration_ns;
    bool is_constant_time;
    double timing_variance;
    char timing_analysis[256];
} AsthraConstantTimeVerification;

// Secure memory zeroing validation
typedef struct {
    void *memory_ptr;
    size_t memory_size;
    uint64_t zeroing_timestamp_ns;
    bool was_properly_zeroed;
    size_t non_zero_bytes_found;
    char validation_details[256];
} AsthraSecureZeroingValidation;

// Fault injection testing
typedef enum {
    ASTHRA_FAULT_MEMORY_ALLOCATION,
    ASTHRA_FAULT_FFI_CALL,
    ASTHRA_FAULT_SLICE_ACCESS,
    ASTHRA_FAULT_STRING_OPERATION,
    ASTHRA_FAULT_TASK_SPAWN,
    ASTHRA_FAULT_PATTERN_MATCH,
    ASTHRA_FAULT_TYPE_CHECK,
    ASTHRA_FAULT_SECURITY_CHECK
} AsthraFaultType;

typedef struct {
    AsthraFaultType fault_type;
    double injection_probability;
    bool is_enabled;
    uint64_t injection_count;
    uint64_t total_opportunities;
} AsthraFaultInjectionConfig;

// Performance monitoring
typedef struct {
    uint64_t safety_check_count;
    uint64_t total_check_time_ns;
    uint64_t max_check_time_ns;
    uint64_t min_check_time_ns;
    double average_check_time_ns;
} AsthraSafetyPerformanceMetrics;

// =============================================================================
// SECURITY FUNCTIONS
// =============================================================================

/**
 * Install stack canary for current thread
 */
int asthra_safety_install_stack_canary(void);

/**
 * Check stack canary integrity
 */
int asthra_safety_check_stack_canary(void);

/**
 * Remove stack canary for current thread
 */
void asthra_safety_remove_stack_canary(void);

/**
 * Verify constant-time operation
 */
AsthraConstantTimeVerification asthra_safety_verify_constant_time_operation(const char *operation_name, 
                                                                            void (*operation)(void), 
                                                                            uint64_t expected_duration_ns);

/**
 * Validate secure memory zeroing
 */
AsthraSecureZeroingValidation asthra_safety_validate_secure_zeroing(void *ptr, size_t size);

/**
 * Enable fault injection for testing
 */
int asthra_safety_enable_fault_injection(AsthraFaultType fault_type, double probability);

/**
 * Disable fault injection
 */
int asthra_safety_disable_fault_injection(AsthraFaultType fault_type);

/**
 * Check if fault should be injected
 */
bool asthra_safety_should_inject_fault(AsthraFaultType fault_type);

/**
 * Record fault injection occurrence
 */
void asthra_safety_record_fault_injection(AsthraFaultType fault_type);

/**
 * Start performance measurement for a safety check
 */
void asthra_safety_start_performance_measurement(const char *check_name);

/**
 * End performance measurement for a safety check
 */
void asthra_safety_end_performance_measurement(const char *check_name);

/**
 * Get performance metrics
 */
AsthraSafetyPerformanceMetrics asthra_safety_get_performance_metrics(void);

/**
 * Reset performance metrics
 */
void asthra_safety_reset_performance_metrics(void);

/**
 * Generate cryptographically secure random canary value
 */
uint64_t asthra_safety_generate_canary_value(void);

/**
 * Validate memory access patterns for side-channel resistance
 */
bool asthra_safety_validate_side_channel_resistance(void *memory_access_pattern, size_t pattern_size);

/**
 * Check for timing attack vulnerabilities
 */
bool asthra_safety_check_timing_attack_resistance(const char *operation_name, 
                                                  void (*operation)(void *), void *data, size_t iterations);

/**
 * Validate control flow integrity
 */
bool asthra_safety_validate_control_flow_integrity(void *expected_return_address);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_SECURITY_H 
