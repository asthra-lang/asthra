#ifndef ASTHRA_SAFETY_COMMON_H
#define ASTHRA_SAFETY_COMMON_H

#include "asthra_runtime.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SHARED SAFETY CONFIGURATION AND TYPES
// =============================================================================

typedef enum {
    ASTHRA_SAFETY_LEVEL_NONE = 0,        // No safety checks (production)
    ASTHRA_SAFETY_LEVEL_BASIC = 1,       // Basic bounds and null checks
    ASTHRA_SAFETY_LEVEL_STANDARD = 2,    // Standard safety checks
    ASTHRA_SAFETY_LEVEL_ENHANCED = 3,    // Enhanced debugging aids
    ASTHRA_SAFETY_LEVEL_PARANOID = 4     // Maximum safety validation
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

typedef enum {
    ASTHRA_VIOLATION_GRAMMAR,
    ASTHRA_VIOLATION_TYPE_SAFETY,
    ASTHRA_VIOLATION_MEMORY_SAFETY,
    ASTHRA_VIOLATION_FFI_SAFETY,
    ASTHRA_VIOLATION_CONCURRENCY,
    ASTHRA_VIOLATION_SECURITY
} AsthraViolationType;

typedef struct {
    uint64_t safety_check_count;
    uint64_t safety_check_time_ns;
    uint64_t violations_detected;
    uint64_t false_positives;
    double average_check_time_ns;
    double overhead_percentage;
} AsthraSafetyPerformanceMetrics;

// =============================================================================
// SHARED GLOBAL STATE ACCESS
// =============================================================================

// Global state accessors (implemented in asthra_safety_core.c)
extern AsthraSafetyConfig* asthra_safety_get_config_ptr(void);
extern bool asthra_safety_is_initialized(void);
extern pthread_mutex_t* asthra_safety_get_mutex(void);
extern AsthraSafetyPerformanceMetrics* asthra_safety_get_metrics_ptr(void);

// Utility functions
extern uint64_t asthra_get_timestamp_ns(void);
extern int asthra_random_bytes(uint8_t* buffer, size_t size);

// =============================================================================
// SHARED VIOLATION REPORTING
// =============================================================================

void asthra_safety_report_violation(AsthraViolationType type, AsthraSafetyLevel severity, 
                                   const char *message, const char *location, int line, 
                                   const char *function, void *context, size_t context_size);

// =============================================================================
// MODULE FUNCTION DECLARATIONS
// =============================================================================

// Grammar and Type Safety (asthra_safety_grammar_types.c)
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

typedef struct {
    bool is_valid;
    uint32_t expected_type_id;
    uint32_t actual_type_id;
    char type_error_message[256];
    const char *context;
} AsthraTypeSafetyCheck;

AsthraGrammarValidation asthra_safety_validate_grammar(const char *source_code, size_t code_length);
AsthraPatternCompletenessCheck asthra_safety_check_pattern_completeness(AsthraMatchArm *arms, size_t arm_count, uint32_t result_type_id);
AsthraTypeSafetyCheck asthra_safety_validate_result_type_usage(AsthraResult result, uint32_t expected_type_id);

// Memory and FFI Safety (asthra_safety_memory_ffi.c)
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

AsthraFFIAnnotationCheck asthra_safety_verify_ffi_annotation(void *func_ptr, void **args, size_t arg_count, 
                                                             AsthraTransferType *expected_transfers, bool *is_borrowed);
int asthra_safety_register_ffi_pointer(void *ptr, size_t size, AsthraTransferType transfer, 
                                       AsthraOwnershipHint ownership, bool is_borrowed, 
                                       const char *source, int line);
int asthra_safety_unregister_ffi_pointer(void *ptr);
AsthraFFIPointerTracker *asthra_safety_get_ffi_pointer_info(void *ptr);

// String and Slice Safety (asthra_safety_strings_slices.c)
typedef struct {
    bool is_deterministic;
    bool has_overflow_risk;
    bool has_encoding_issues;
    size_t result_length;
    size_t max_safe_length;
    char validation_message[256];
} AsthraStringOperationValidation;

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

AsthraStringOperationValidation asthra_safety_validate_string_concatenation(AsthraString *strings, size_t count);
AsthraBoundaryCheck asthra_safety_slice_bounds_check(AsthraSliceHeader slice, size_t index);

// Concurrency and Error Handling (asthra_safety_concurrency_errors.c)
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

void asthra_safety_log_task_lifecycle_event(uint64_t task_id, AsthraTaskEvent event, const char *details);

// Security Enforcement (asthra_safety_security.c)
typedef struct {
    uint64_t canary_value;
    void *stack_base;
    size_t stack_size;
    pthread_t thread_id;
    uint64_t creation_timestamp;
    bool is_active;
} AsthraStackCanary;

int asthra_safety_install_stack_canary(void);
int asthra_safety_check_stack_canary(void);
void asthra_safety_remove_stack_canary(void);

// Core System Management (asthra_safety_core.c)
extern const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_DEBUG;
extern const AsthraSafetyConfig ASTHRA_SAFETY_CONFIG_RELEASE;

int asthra_safety_init(const AsthraSafetyConfig *config);
void asthra_safety_cleanup(void);
AsthraSafetyPerformanceMetrics asthra_safety_get_performance_metrics(void);
void asthra_safety_reset_performance_metrics(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_COMMON_H 
