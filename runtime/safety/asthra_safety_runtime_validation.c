/**
 * Asthra Programming Language Runtime Safety - Runtime Validation Systems
 * Implementation of result tracking, pattern matching, scheduler events, and C-task interactions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_runtime_validation.h"
#include "asthra_runtime.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External references to global state from asthra_safety.c
extern AsthraSafetyConfig g_safety_config;
extern pthread_mutex_t g_safety_mutex;

// =============================================================================
// SCHEDULER EVENT LOGGING IMPLEMENTATION
// =============================================================================

void asthra_safety_log_scheduler_event(AsthraSchedulerEvent event, const char *details) {
    if (!g_safety_config.enable_concurrency_debugging) {
        return;
    }

    // Create scheduler event log entry
    AsthraSchedulerEventLog log_entry = {0};
    log_entry.event = event;
    log_entry.timestamp_ns = asthra_get_timestamp_ns();

    // Get current system statistics (simplified)
    log_entry.active_tasks = 0;      // Would be populated from scheduler state
    log_entry.queued_tasks = 0;      // Would be populated from scheduler state
    log_entry.worker_threads = 0;    // Would be populated from scheduler state
    log_entry.cpu_utilization = 0.0; // Would be calculated from system metrics

    if (details) {
        strncpy(log_entry.scheduler_details, details, sizeof(log_entry.scheduler_details) - 1);
        log_entry.scheduler_details[sizeof(log_entry.scheduler_details) - 1] = '\0';
    }

    // Log to the standard logging system
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_CONCURRENCY, "Scheduler event %d at %lu: %s",
               event, log_entry.timestamp_ns, details ? details : "");
}

// =============================================================================
// C-ASTHRA TASK INTERACTION TRACKING IMPLEMENTATION
// =============================================================================

void asthra_safety_log_c_task_interaction(uint64_t asthra_task_id, pthread_t c_thread_id,
                                          const char *interaction_type, void *data,
                                          size_t data_size) {
    if (!g_safety_config.enable_concurrency_debugging) {
        return;
    }

    static uint64_t interaction_counter = 0;
    uint64_t interaction_id = __sync_fetch_and_add(&interaction_counter, 1);

    // Create interaction log entry
    AsthraCTaskInteraction interaction = {0};
    interaction.interaction_id = interaction_id;
    interaction.asthra_task_id = asthra_task_id;
    interaction.c_thread_id = c_thread_id;
    interaction.interaction_type = interaction_type;
    interaction.shared_data = data;
    interaction.data_size = data_size;
    interaction.start_timestamp_ns = asthra_get_timestamp_ns();
    interaction.is_successful = true; // Assume success unless proven otherwise

    snprintf(interaction.interaction_details, sizeof(interaction.interaction_details),
             "C-Asthra interaction: task=%lu, thread=%lu, type=%s, data_size=%zu", asthra_task_id,
             (unsigned long)c_thread_id, interaction_type ? interaction_type : "unknown",
             data_size);

    // Log to the standard logging system
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_CONCURRENCY, "C-Task interaction %lu: %s",
               interaction_id, interaction.interaction_details);
}

// =============================================================================
// RESULT TRACKING IMPLEMENTATION
// =============================================================================

int asthra_safety_register_result_tracker(AsthraResult result, const char *location) {
    if (!g_safety_config.enable_error_handling_aids) {
        return 0;
    }

    pthread_mutex_lock(&g_safety_mutex);

    extern AsthraResultTracker *g_result_trackers;
    extern size_t g_result_tracker_count;
    extern size_t g_result_tracker_capacity;

    // Expand capacity if needed
    if (g_result_tracker_count >= g_result_tracker_capacity) {
        size_t new_capacity = g_result_tracker_capacity == 0 ? 64 : g_result_tracker_capacity * 2;
        AsthraResultTracker *new_trackers =
            realloc(g_result_trackers, new_capacity * sizeof(AsthraResultTracker));
        if (!new_trackers) {
            pthread_mutex_unlock(&g_safety_mutex);
            return -1;
        }
        g_result_trackers = new_trackers;
        g_result_tracker_capacity = new_capacity;
    }

    // Register the result
    static uint64_t result_id_counter = 0;
    AsthraResultTracker *tracker = &g_result_trackers[g_result_tracker_count++];
    tracker->result_id = __sync_fetch_and_add(&result_id_counter, 1);
    tracker->result = result;
    tracker->was_handled = false;
    tracker->creation_timestamp_ns = asthra_get_timestamp_ns();
    tracker->handling_timestamp_ns = 0;
    tracker->creation_location = location;
    tracker->handling_location = NULL;

    pthread_mutex_unlock(&g_safety_mutex);
    return (int)tracker->result_id;
}

int asthra_safety_mark_result_handled(uint64_t result_id, const char *location) {
    if (!g_safety_config.enable_error_handling_aids) {
        return 0;
    }

    pthread_mutex_lock(&g_safety_mutex);

    extern AsthraResultTracker *g_result_trackers;
    extern size_t g_result_tracker_count;

    for (size_t i = 0; i < g_result_tracker_count; i++) {
        if (g_result_trackers[i].result_id == result_id) {
            g_result_trackers[i].was_handled = true;
            g_result_trackers[i].handling_timestamp_ns = asthra_get_timestamp_ns();
            g_result_trackers[i].handling_location = location;
            pthread_mutex_unlock(&g_safety_mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&g_safety_mutex);
    return -1; // Result ID not found
}

void asthra_safety_check_unhandled_results(void) {
    if (!g_safety_config.enable_error_handling_aids) {
        return;
    }

    pthread_mutex_lock(&g_safety_mutex);

    extern AsthraResultTracker *g_result_trackers;
    extern size_t g_result_tracker_count;

    uint64_t current_time = asthra_get_timestamp_ns();
    uint64_t timeout_ns = 5000000000ULL; // 5 seconds

    for (size_t i = 0; i < g_result_tracker_count; i++) {
        AsthraResultTracker *tracker = &g_result_trackers[i];

        if (!tracker->was_handled && (current_time - tracker->creation_timestamp_ns) > timeout_ns) {
            asthra_safety_report_violation(ASTHRA_VIOLATION_TYPE_SAFETY,
                                           ASTHRA_SAFETY_LEVEL_STANDARD,
                                           "Unhandled Result detected", tracker->creation_location,
                                           0, "unknown", &tracker->result, sizeof(AsthraResult));
        }
    }

    pthread_mutex_unlock(&g_safety_mutex);
}

// =============================================================================
// PATTERN MATCHING VERIFICATION IMPLEMENTATION
// =============================================================================

AsthraPatternCompletenessCheck asthra_safety_verify_match_exhaustiveness(AsthraMatchArm *arms,
                                                                         size_t arm_count,
                                                                         uint32_t result_type_id) {
    // This is the same as asthra_safety_check_pattern_completeness
    // but with additional verification for specific result types
    AsthraPatternCompletenessCheck check =
        asthra_safety_check_pattern_completeness(arms, arm_count, result_type_id);

    if (!g_safety_config.enable_pattern_matching_checks) {
        return check;
    }

    // Additional verification based on result type
    if (result_type_id != 0) {
        // Could add type-specific pattern completeness checks here
        // For now, we use the general completeness check
    }

    return check;
}
