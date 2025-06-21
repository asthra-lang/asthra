/**
 * Asthra Programming Language Runtime Safety - Runtime Validation Systems
 * Header file for result tracking, pattern matching, scheduler events, and C-task interactions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_RUNTIME_VALIDATION_H
#define ASTHRA_SAFETY_RUNTIME_VALIDATION_H

#include "asthra_safety.h"
#include <pthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// RUNTIME VALIDATION FUNCTIONS
// =============================================================================

/**
 * Registers a Result for tracking to detect unhandled errors
 * @param result The Result value to track
 * @param location Source location where Result was created
 * @return Result ID for tracking, or -1 on error
 */
int asthra_safety_register_result_tracker(AsthraResult result, const char *location);

/**
 * Marks a Result as handled to prevent unhandled error warnings
 * @param result_id ID of the Result to mark as handled
 * @param location Source location where Result was handled
 * @return 0 on success, -1 if Result ID not found
 */
int asthra_safety_mark_result_handled(uint64_t result_id, const char *location);

/**
 * Checks for unhandled Results and reports violations
 */
void asthra_safety_check_unhandled_results(void);

/**
 * Verifies pattern matching exhaustiveness for match expressions
 * @param arms Array of match arms to check
 * @param arm_count Number of match arms
 * @param result_type_id Type ID for result type validation
 * @return Pattern completeness check result
 */
AsthraPatternCompletenessCheck asthra_safety_verify_match_exhaustiveness(AsthraMatchArm *arms, size_t arm_count, 
                                                                         uint32_t result_type_id);

/**
 * Logs scheduler events for concurrency debugging
 * @param event Type of scheduler event
 * @param details Additional details about the event
 */
void asthra_safety_log_scheduler_event(AsthraSchedulerEvent event, const char *details);

/**
 * Logs C-Asthra task interactions for debugging
 * @param asthra_task_id ID of the Asthra task
 * @param c_thread_id ID of the C thread
 * @param interaction_type Type of interaction
 * @param data Shared data pointer
 * @param data_size Size of shared data
 */
void asthra_safety_log_c_task_interaction(uint64_t asthra_task_id, pthread_t c_thread_id, 
                                          const char *interaction_type, void *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_RUNTIME_VALIDATION_H 
