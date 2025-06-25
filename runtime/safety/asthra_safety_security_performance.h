/**
 * Asthra Programming Language Runtime Safety - Security & Performance Monitoring
 * Header file for constant-time verification, secure zeroing, stack canaries, fault injection, and
 * performance monitoring
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_SECURITY_PERFORMANCE_H
#define ASTHRA_SAFETY_SECURITY_PERFORMANCE_H

#include "asthra_safety.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SECURITY & PERFORMANCE MONITORING FUNCTIONS
// =============================================================================

/**
 * Verifies that an operation executes in constant time
 * @param operation_name Name of the operation for logging
 * @param operation Function pointer to the operation to test
 * @param expected_duration_ns Expected duration in nanoseconds
 * @return Constant-time verification result with timing analysis
 */
AsthraConstantTimeVerification
asthra_safety_verify_constant_time_operation(const char *operation_name, void (*operation)(void),
                                             uint64_t expected_duration_ns);

/**
 * Validates that memory has been securely zeroed
 * @param ptr Pointer to memory to validate
 * @param size Size of memory region in bytes
 * @return Secure zeroing validation result
 */
AsthraSecureZeroingValidation asthra_safety_validate_secure_zeroing(void *ptr, size_t size);

/**
 * Removes stack canary protection for current thread
 */
void asthra_safety_remove_stack_canary(void);

/**
 * Enables fault injection for testing
 * @param fault_type Type of fault to inject
 * @param probability Probability of fault injection (0.0 to 1.0)
 * @return 0 on success, -1 on error
 */
int asthra_safety_enable_fault_injection(AsthraFaultType fault_type, double probability);

/**
 * Disables fault injection for a specific fault type
 * @param fault_type Type of fault to disable
 * @return 0 on success, -1 on error
 */
int asthra_safety_disable_fault_injection(AsthraFaultType fault_type);

/**
 * Checks if a fault should be injected
 * @param fault_type Type of fault to check
 * @return true if fault should be injected, false otherwise
 */
bool asthra_safety_should_inject_fault(AsthraFaultType fault_type);

/**
 * Records that a fault was injected for statistics
 * @param fault_type Type of fault that was injected
 */
void asthra_safety_record_fault_injection(AsthraFaultType fault_type);

/**
 * Starts performance measurement for a safety check
 * @param check_name Name of the safety check being measured
 */
void asthra_safety_start_performance_measurement(const char *check_name);

/**
 * Ends performance measurement for a safety check
 * @param check_name Name of the safety check being measured
 */
void asthra_safety_end_performance_measurement(const char *check_name);

/**
 * Sets the safety configuration
 * @param config New safety configuration
 * @return 0 on success, -1 on error
 */
int asthra_safety_set_config(const AsthraSafetyConfig *config);

/**
 * Gets the current safety configuration
 * @return Current safety configuration
 */
AsthraSafetyConfig asthra_safety_get_config(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_SECURITY_PERFORMANCE_H
