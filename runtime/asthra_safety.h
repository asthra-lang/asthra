/**
 * Asthra Programming Language Runtime Safety Checks & Debugging Aids v1.2
 * Comprehensive Safety Mechanisms with v1.2 Feature Integration
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Enhanced Grammar and Type Safety validation
 * - Memory and FFI Safety with annotation verification
 * - String and Slice Safety with bounds checking
 * - Concurrency and Error Handling debugging aids
 * - Security Enforcement with stack canaries and validation
 * - Portable C17 implementation with configurable safety levels
 * - AI-focused rapid feedback for safety violations
 * - Strict standard compliance using -std=c17 -pedantic-errors
 */

#ifndef ASTHRA_SAFETY_H
#define ASTHRA_SAFETY_H

#include "asthra_runtime.h"
#include <pthread.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SAFETY CONFIGURATION AND LEVELS
// =============================================================================

typedef enum {
    ASTHRA_SAFETY_LEVEL_NONE = 0,     // No safety checks (production)
    ASTHRA_SAFETY_LEVEL_BASIC = 1,    // Basic bounds and null checks
    ASTHRA_SAFETY_LEVEL_STANDARD = 2, // Standard safety checks
    ASTHRA_SAFETY_LEVEL_ENHANCED = 3, // Enhanced debugging aids
    ASTHRA_SAFETY_LEVEL_PARANOID = 4  // Maximum safety validation
} AsthraSafetyLevel;

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

// =============================================================================
// ENHANCED GRAMMAR AND TYPE SAFETY
// =============================================================================

// Parser validation structures
typedef enum {
    ASTHRA_GRAMMAR_VALID,
    ASTHRA_GRAMMAR_AMBIGUOUS,
    ASTHRA_GRAMMAR_INVALID_SYNTAX,
    ASTHRA_GRAMMAR_TYPE_MISMATCH,
    ASTHRA_GRAMMAR_SEMANTIC_ERROR
} AsthraGrammarValidationResult;

typedef struct {
    AsthraGrammarValidationResult result;
    char error_message[512];
    const char *source_location;
    int line_number;
    int column_number;
} AsthraGrammarValidation;

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

// Type safety validation
typedef struct {
    bool is_valid;
    uint32_t expected_type_id;
    uint32_t actual_type_id;
    char type_error_message[256];
    const char *context;
} AsthraTypeSafetyCheck;

// Function declarations for grammar and type safety
AsthraGrammarValidation asthra_safety_validate_grammar(const char *source_code, size_t code_length);
AsthraPatternCompletenessCheck asthra_safety_check_pattern_completeness(AsthraMatchArm *arms,
                                                                        size_t arm_count,
                                                                        uint32_t result_type_id);
AsthraTypeSafetyCheck asthra_safety_validate_result_type_usage(AsthraResult result,
                                                               uint32_t expected_type_id);
AsthraTypeSafetyCheck asthra_safety_validate_slice_type_safety(AsthraSliceHeader slice,
                                                               uint32_t expected_element_type_id);

// =============================================================================
// MEMORY AND FFI SAFETY
// =============================================================================

// FFI annotation verification
typedef enum {
    ASTHRA_FFI_ANNOTATION_VALID,
    ASTHRA_FFI_ANNOTATION_MISMATCH,
    ASTHRA_FFI_ANNOTATION_MISSING,
    ASTHRA_FFI_ANNOTATION_INVALID_TRANSFER,
    ASTHRA_FFI_ANNOTATION_LIFETIME_VIOLATION
} AsthraFFIAnnotationResult;

typedef struct {
    AsthraFFIAnnotationResult result;
    AsthraTransferType expected_transfer;
    AsthraTransferType actual_transfer;
    bool is_borrowed;
    char violation_message[256];
    const char *function_name;
    int parameter_index;
} AsthraFFIAnnotationCheck;

// Ownership tracking for FFI pointers
typedef struct {
    void *ptr;
    size_t size;
    AsthraTransferType transfer_type;
    AsthraOwnershipHint ownership;
    bool is_borrowed;
    uint64_t allocation_timestamp;
    uint64_t last_access_timestamp;
    const char *allocation_source;
    int allocation_line;
    pthread_t owning_thread;
    atomic_int reference_count;
} AsthraFFIPointerTracker;

// Enhanced boundary checks for slices
typedef struct {
    bool is_valid;
    size_t attempted_index;
    size_t slice_length;
    size_t element_size;
    bool is_out_of_bounds;
    bool is_null_pointer;
    bool is_corrupted_header;
    char error_details[256];
} AsthraBoundaryCheck;

// Variadic function safety validation
typedef struct {
    bool is_valid;
    size_t expected_arg_count;
    size_t actual_arg_count;
    AsthraVarArgType *expected_types;
    AsthraVarArgType *actual_types;
    char type_mismatch_details[512];
} AsthraVariadicValidation;

// Function declarations for memory and FFI safety
AsthraFFIAnnotationCheck asthra_safety_verify_ffi_annotation(void *func_ptr, void **args,
                                                             size_t arg_count,
                                                             AsthraTransferType *expected_transfers,
                                                             bool *is_borrowed);
AsthraBoundaryCheck asthra_safety_enhanced_boundary_check(AsthraSliceHeader slice, size_t index);
int asthra_safety_register_ffi_pointer(void *ptr, size_t size, AsthraTransferType transfer,
                                       AsthraOwnershipHint ownership, bool is_borrowed,
                                       const char *source, int line);
int asthra_safety_unregister_ffi_pointer(void *ptr);
AsthraFFIPointerTracker *asthra_safety_get_ffi_pointer_info(void *ptr);
AsthraVariadicValidation asthra_safety_validate_variadic_call(void *func_ptr, AsthraVarArg *args,
                                                              size_t arg_count,
                                                              AsthraVarArgType *expected_types,
                                                              size_t expected_count);

// =============================================================================
// STRING AND SLICE SAFETY
// =============================================================================

// String operation validation
typedef struct {
    bool is_deterministic;
    bool has_overflow_risk;
    bool has_encoding_issues;
    size_t result_length;
    size_t max_safe_length;
    char validation_message[256];
} AsthraStringOperationValidation;

// Memory layout validation for SliceHeader
typedef struct {
    bool is_valid;
    bool has_correct_alignment;
    bool has_valid_pointer;
    bool has_consistent_length;
    bool has_valid_capacity;
    bool has_correct_element_size;
    size_t detected_corruption_offset;
    char corruption_details[256];
} AsthraMemoryLayoutValidation;

// Function declarations for string and slice safety
AsthraStringOperationValidation asthra_safety_validate_string_concatenation(AsthraString *strings,
                                                                            size_t count);
AsthraBoundaryCheck asthra_safety_slice_bounds_check(AsthraSliceHeader slice, size_t index);
AsthraMemoryLayoutValidation asthra_safety_validate_slice_header(AsthraSliceHeader slice);

// =============================================================================
// CONCURRENCY AND ERROR HANDLING
// =============================================================================

// Task lifecycle tracking
typedef enum {
    ASTHRA_TASK_EVENT_SPAWNED,
    ASTHRA_TASK_EVENT_STARTED,
    ASTHRA_TASK_EVENT_SUSPENDED,
    ASTHRA_TASK_EVENT_RESUMED,
    ASTHRA_TASK_EVENT_COMPLETED,
    ASTHRA_TASK_EVENT_FAILED,
    ASTHRA_TASK_EVENT_CANCELLED
} AsthraTaskEvent;

typedef struct {
    uint64_t task_id;
    AsthraTaskEvent event;
    uint64_t timestamp_ns;
    pthread_t thread_id;
    const char *task_name;
    void *task_data;
    size_t task_data_size;
    AsthraResult task_result;
    char event_details[256];
} AsthraTaskLifecycleEvent;

// Scheduler event tracking
typedef enum {
    ASTHRA_SCHEDULER_EVENT_TASK_QUEUED,
    ASTHRA_SCHEDULER_EVENT_TASK_DEQUEUED,
    ASTHRA_SCHEDULER_EVENT_WORKER_STARTED,
    ASTHRA_SCHEDULER_EVENT_WORKER_STOPPED,
    ASTHRA_SCHEDULER_EVENT_LOAD_BALANCED,
    ASTHRA_SCHEDULER_EVENT_DEADLOCK_DETECTED
} AsthraSchedulerEvent;

typedef struct {
    AsthraSchedulerEvent event;
    uint64_t timestamp_ns;
    size_t active_tasks;
    size_t queued_tasks;
    size_t worker_threads;
    double cpu_utilization;
    char scheduler_details[256];
} AsthraSchedulerEventLog;

// C-Asthra task interaction tracking
typedef struct {
    uint64_t interaction_id;
    uint64_t asthra_task_id;
    pthread_t c_thread_id;
    const char *interaction_type;
    void *shared_data;
    size_t data_size;
    uint64_t start_timestamp_ns;
    uint64_t end_timestamp_ns;
    bool is_successful;
    char interaction_details[256];
} AsthraCTaskInteraction;

// Error handling aids
typedef struct {
    uint64_t result_id;
    AsthraResult result;
    bool was_handled;
    uint64_t creation_timestamp_ns;
    uint64_t handling_timestamp_ns;
    const char *creation_location;
    const char *handling_location;
    char error_context[256];
} AsthraResultTracker;

// Function declarations for concurrency and error handling
void asthra_safety_log_task_lifecycle_event(uint64_t task_id, AsthraTaskEvent event,
                                            const char *details);
void asthra_safety_log_scheduler_event(AsthraSchedulerEvent event, const char *details);
void asthra_safety_log_c_task_interaction(uint64_t asthra_task_id, pthread_t c_thread_id,
                                          const char *interaction_type, void *data,
                                          size_t data_size);
int asthra_safety_register_result_tracker(AsthraResult result, const char *location);
int asthra_safety_mark_result_handled(uint64_t result_id, const char *location);
void asthra_safety_check_unhandled_results(void);
AsthraPatternCompletenessCheck asthra_safety_verify_match_exhaustiveness(AsthraMatchArm *arms,
                                                                         size_t arm_count,
                                                                         uint32_t result_type_id);

// =============================================================================
// SECURITY ENFORCEMENT
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

// FFI call logging and validation
typedef struct {
    uint64_t call_id;
    const char *function_name;
    void *function_ptr;
    void **arguments;
    size_t argument_count;
    AsthraTransferType *transfer_types;
    bool *is_borrowed;
    uint64_t call_timestamp_ns;
    uint64_t return_timestamp_ns;
    void *return_value;
    bool is_successful;
    char call_details[512];
} AsthraFFICallLog;

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

// Function declarations for security enforcement
int asthra_safety_install_stack_canary(void);
int asthra_safety_check_stack_canary(void);
void asthra_safety_remove_stack_canary(void);
void asthra_safety_log_ffi_call(const char *function_name, void *function_ptr, void **args,
                                size_t arg_count);
AsthraConstantTimeVerification
asthra_safety_verify_constant_time_operation(const char *operation_name, void (*operation)(void),
                                             uint64_t expected_duration_ns);
AsthraSecureZeroingValidation asthra_safety_validate_secure_zeroing(void *ptr, size_t size);

// =============================================================================
// FAULT INJECTION TESTING
// =============================================================================

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
    uint64_t injection_count;
    uint64_t total_opportunities;
    bool is_enabled;
    char fault_description[256];
} AsthraFaultInjectionConfig;

// Function declarations for fault injection
int asthra_safety_enable_fault_injection(AsthraFaultType fault_type, double probability);
int asthra_safety_disable_fault_injection(AsthraFaultType fault_type);
bool asthra_safety_should_inject_fault(AsthraFaultType fault_type);
void asthra_safety_record_fault_injection(AsthraFaultType fault_type);

// =============================================================================
// PERFORMANCE MONITORING
// =============================================================================

typedef struct {
    uint64_t safety_check_count;
    uint64_t safety_check_time_ns;
    uint64_t violations_detected;
    uint64_t false_positives;
    double average_check_time_ns;
    double overhead_percentage;
} AsthraSafetyPerformanceMetrics;

// Function declarations for performance monitoring
AsthraSafetyPerformanceMetrics asthra_safety_get_performance_metrics(void);
void asthra_safety_reset_performance_metrics(void);
void asthra_safety_start_performance_measurement(const char *check_name);
void asthra_safety_end_performance_measurement(const char *check_name);

// =============================================================================
// SAFETY SYSTEM INITIALIZATION AND MANAGEMENT
// =============================================================================

// Main safety system functions
int asthra_safety_init(const AsthraSafetyConfig *config);
void asthra_safety_cleanup(void);
int asthra_safety_set_config(const AsthraSafetyConfig *config);
AsthraSafetyConfig asthra_safety_get_config(void);

// Safety violation reporting
typedef enum {
    ASTHRA_VIOLATION_GRAMMAR,
    ASTHRA_VIOLATION_TYPE_SAFETY,
    ASTHRA_VIOLATION_MEMORY_SAFETY,
    ASTHRA_VIOLATION_FFI_SAFETY,
    ASTHRA_VIOLATION_CONCURRENCY,
    ASTHRA_VIOLATION_SECURITY
} AsthraViolationType;

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

void asthra_safety_report_violation(AsthraViolationType type, AsthraSafetyLevel severity,
                                    const char *message, const char *location, int line,
                                    const char *function, void *context, size_t context_size);

// Safety macros for convenient usage
#define ASTHRA_SAFETY_CHECK(condition, violation_type, message)                                    \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            asthra_safety_report_violation(violation_type, ASTHRA_SAFETY_LEVEL_STANDARD, message,  \
                                           __FILE__, __LINE__, __func__, NULL, 0);                 \
        }                                                                                          \
    } while (0)

#define ASTHRA_SAFETY_ASSERT(condition, message)                                                   \
    ASTHRA_SAFETY_CHECK(condition, ASTHRA_VIOLATION_GRAMMAR, message)

#define ASTHRA_SAFETY_FFI_CHECK(condition, message)                                                \
    ASTHRA_SAFETY_CHECK(condition, ASTHRA_VIOLATION_FFI_SAFETY, message)

#define ASTHRA_SAFETY_MEMORY_CHECK(condition, message)                                             \
    ASTHRA_SAFETY_CHECK(condition, ASTHRA_VIOLATION_MEMORY_SAFETY, message)

#define ASTHRA_SAFETY_TYPE_CHECK(condition, message)                                               \
    ASTHRA_SAFETY_CHECK(condition, ASTHRA_VIOLATION_TYPE_SAFETY, message)

#define ASTHRA_SAFETY_CONCURRENCY_CHECK(condition, message)                                        \
    ASTHRA_SAFETY_CHECK(condition, ASTHRA_VIOLATION_CONCURRENCY, message)

#define ASTHRA_SAFETY_SECURITY_CHECK(condition, message)                                           \
    ASTHRA_SAFETY_CHECK(condition, ASTHRA_VIOLATION_SECURITY, message)

// Conditional compilation for safety checks based on build configuration
#ifdef ASTHRA_ENABLE_SAFETY_CHECKS
#define ASTHRA_SAFETY_ENABLED 1
#else
#define ASTHRA_SAFETY_ENABLED 0
#endif

#if ASTHRA_SAFETY_ENABLED
#define ASTHRA_SAFETY_CALL(func) func
#define ASTHRA_SAFETY_CONDITIONAL(code) code
#else
#define ASTHRA_SAFETY_CALL(func) ((void)0)
#define ASTHRA_SAFETY_CONDITIONAL(code) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_H
