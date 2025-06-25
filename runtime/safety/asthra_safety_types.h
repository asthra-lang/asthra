/**
 * Asthra Programming Language Runtime Safety System
 * Core Type Safety Module - Type validation and checking
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_TYPES_H
#define ASTHRA_SAFETY_TYPES_H

#include "asthra_safety_minimal_includes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SAFETY SYSTEM CONFIGURATION
// =============================================================================

// Safety levels for different use cases
typedef enum {
    ASTHRA_SAFETY_LEVEL_NONE = 0,     // No safety checks (production)
    ASTHRA_SAFETY_LEVEL_BASIC = 1,    // Basic bounds and null checks
    ASTHRA_SAFETY_LEVEL_STANDARD = 2, // Standard safety checks
    ASTHRA_SAFETY_LEVEL_ENHANCED = 3, // Enhanced debugging aids
    ASTHRA_SAFETY_LEVEL_PARANOID = 4  // Maximum safety validation
} AsthraSafetyLevel;

// Comprehensive safety configuration
typedef struct {
    AsthraSafetyLevel level;
    bool enable_parser_validation;
    bool enable_pattern_matching_checks;
    bool enable_type_safety_checks;
    bool enable_ffi_annotation_verification;
    bool enable_boundary_checks;
    bool enable_ownership_tracking;
    bool enable_variadic_validation;
    bool enable_string_operation_validation;
    bool enable_slice_bounds_checking;
    bool enable_memory_layout_validation;
    bool enable_concurrency_debugging;
    bool enable_error_handling_aids;
    bool enable_security_enforcement;
    bool enable_stack_canaries;
    bool enable_ffi_call_logging;
    bool enable_constant_time_verification;
    bool enable_secure_memory_validation;
    bool enable_fault_injection_testing;
    bool enable_performance_monitoring;
} AsthraSafetyConfig;

// Default safety configurations for different environments
extern const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_DEBUG;
extern const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_RELEASE;
extern const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_TESTING;
extern const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_PARANOID;

// Violation types for safety reporting
typedef enum {
    ASTHRA_VIOLATION_GRAMMAR,
    ASTHRA_VIOLATION_TYPE_SAFETY,
    ASTHRA_VIOLATION_MEMORY_SAFETY,
    ASTHRA_VIOLATION_FFI_SAFETY,
    ASTHRA_VIOLATION_CONCURRENCY,
    ASTHRA_VIOLATION_SECURITY
} AsthraViolationType;

// Safety violation structure
typedef struct {
    AsthraViolationType violation_type;
    AsthraSafetyLevel severity;
    uint64_t timestamp_ns;
    const char *source_location;
    int line_number;
    const char *function_name;
    char violation_message[512];
    void *context_data;
    size_t context_size;
} AsthraSafetyViolation;

// =============================================================================
// TYPE SAFETY STRUCTURES
// =============================================================================

// Type safety validation result
typedef struct {
    bool is_valid;
    uint32_t expected_type_id;
    uint32_t actual_type_id;
    char type_error_message[256];
    const char *context;
} AsthraTypeSafetyCheck;

// Pattern matching completeness checking
typedef enum {
    ASTHRA_PATTERN_COMPLETE,
    ASTHRA_PATTERN_INCOMPLETE,
    ASTHRA_PATTERN_UNREACHABLE,
    ASTHRA_PATTERN_REDUNDANT
} AsthraPatternCompletenessResult;

typedef struct {
    AsthraPatternCompletenessResult result;
    char missing_patterns[1024];
    char unreachable_patterns[1024];
    size_t pattern_count;
    size_t covered_patterns;
} AsthraPatternCompletenessCheck;

// =============================================================================
// TYPE SAFETY FUNCTIONS
// =============================================================================

/**
 * Validate slice element type safety
 */
AsthraTypeSafetyCheck asthra_safety_validate_slice_type_safety(AsthraSliceHeader slice,
                                                               uint32_t expected_element_type_id);

/**
 * Validate Result<T,E> type usage
 */
AsthraTypeSafetyCheck asthra_safety_validate_result_type_usage(AsthraResult result,
                                                               uint32_t expected_type_id);

/**
 * Check pattern matching completeness
 */
AsthraPatternCompletenessCheck asthra_safety_check_pattern_completeness(AsthraMatchArm *arms,
                                                                        size_t arm_count,
                                                                        uint32_t result_type_id);

/**
 * Verify match exhaustiveness for specific result types
 */
AsthraPatternCompletenessCheck asthra_safety_verify_match_exhaustiveness(AsthraMatchArm *arms,
                                                                         size_t arm_count,
                                                                         uint32_t result_type_id);

/**
 * Validate type ID consistency and registration
 */
bool asthra_safety_validate_type_id(uint32_t type_id);

/**
 * Get type name from type ID for error messages
 */
const char *asthra_safety_get_type_name_safe(uint32_t type_id);

// =============================================================================
// SAFETY SYSTEM CORE FUNCTIONS
// =============================================================================

/**
 * Initialize the safety system with configuration
 */
int asthra_safety_init(const AsthraSafetyConfig *config);

/**
 * Cleanup the safety system
 */
void asthra_safety_cleanup(void);

/**
 * Set safety configuration
 */
int asthra_safety_set_config(const AsthraSafetyConfig *config);

/**
 * Get current safety configuration
 */
AsthraSafetyConfig asthra_safety_get_config(void);

/**
 * Report a safety violation
 */
void asthra_safety_report_violation(AsthraViolationType type, AsthraSafetyLevel severity,
                                    const char *message, const char *location, int line,
                                    const char *function, void *context, size_t context_size);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_TYPES_H
