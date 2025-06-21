/**
 * Asthra Programming Language Runtime Safety System
 * Runtime Validation Systems Module - String, pattern matching, and result validation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_RUNTIME_H
#define ASTHRA_SAFETY_RUNTIME_H

#include "asthra_safety_minimal_includes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// RUNTIME VALIDATION STRUCTURES
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

// Task lifecycle event tracking
typedef enum {
    ASTHRA_TASK_EVENT_CREATED,
    ASTHRA_TASK_EVENT_STARTED,
    ASTHRA_TASK_EVENT_SUSPENDED,
    ASTHRA_TASK_EVENT_RESUMED,
    ASTHRA_TASK_EVENT_COMPLETED,
    ASTHRA_TASK_EVENT_FAILED,
    ASTHRA_TASK_EVENT_CANCELLED
} AsthraTaskEvent;

// Scheduler event tracking
typedef enum {
    ASTHRA_SCHEDULER_EVENT_STARTED,
    ASTHRA_SCHEDULER_EVENT_STOPPED,
    ASTHRA_SCHEDULER_EVENT_WORKER_ADDED,
    ASTHRA_SCHEDULER_EVENT_WORKER_REMOVED,
    ASTHRA_SCHEDULER_EVENT_QUEUE_FULL,
    ASTHRA_SCHEDULER_EVENT_QUEUE_EMPTY
} AsthraSchedulerEvent;

// Result tracking for error handling validation
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

// =============================================================================
// RUNTIME VALIDATION FUNCTIONS
// =============================================================================

/**
 * Validate string concatenation operation safety
 */
AsthraStringOperationValidation asthra_safety_validate_string_concatenation(AsthraString *strings, size_t count);

/**
 * Log task lifecycle events for debugging
 */
void asthra_safety_log_task_lifecycle_event(uint64_t task_id, AsthraTaskEvent event, const char *details);

/**
 * Log scheduler events for debugging
 */
void asthra_safety_log_scheduler_event(AsthraSchedulerEvent event, const char *details);

/**
 * Log C task interaction events
 */
void asthra_safety_log_c_task_interaction(uint64_t asthra_task_id, pthread_t c_thread_id, 
                                          const char *interaction_type, void *data, size_t data_size);

/**
 * Register result for tracking unhandled errors
 */
int asthra_safety_register_result_tracker(AsthraResult result, const char *location);

/**
 * Mark result as handled
 */
int asthra_safety_mark_result_handled(uint64_t result_id, const char *location);

/**
 * Check for unhandled results and report violations
 */
void asthra_safety_check_unhandled_results(void);

/**
 * Validate string encoding (UTF-8)
 */
bool asthra_safety_validate_string_encoding(const char *str, size_t length);

/**
 * Check string operation for potential buffer overflow
 */
bool asthra_safety_check_string_overflow(size_t current_length, size_t additional_length, size_t max_length);

/**
 * Validate interpolation template format
 */
bool asthra_safety_validate_interpolation_template(const char *template, size_t expected_args);

/**
 * Get task event statistics
 */
size_t asthra_safety_get_task_event_count(void);

/**
 * Get result tracker statistics
 */
size_t asthra_safety_get_result_tracker_count(void);

/**
 * Clean up old task events
 */
void asthra_safety_cleanup_old_task_events(uint64_t max_age_ns);

/**
 * Clean up handled result trackers
 */
void asthra_safety_cleanup_handled_result_trackers(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_RUNTIME_H 
