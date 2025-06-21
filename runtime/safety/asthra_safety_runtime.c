/**
 * Asthra Programming Language Runtime Safety System
 * Runtime Validation Systems Module - String, pattern matching, and result validation implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_runtime.h"
#include "../asthra_safety_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External references to global state
// Use accessor function instead of direct global
#define g_safety_config (*asthra_safety_get_config_ptr())
extern pthread_mutex_t g_safety_mutex;

// External references to tracking arrays (defined in asthra_safety.c)
extern AsthraResultTracker *g_result_trackers;
extern size_t g_result_tracker_count;
extern size_t g_result_tracker_capacity;

// Task event tracking (we'll need to define these globals)
static AsthraTaskEvent *g_task_events = NULL;
static size_t g_task_event_count = 0;
static size_t g_task_event_capacity = 0;

// =============================================================================
// STRING OPERATION VALIDATION IMPLEMENTATION
// =============================================================================

AsthraStringOperationValidation asthra_safety_validate_string_concatenation(AsthraString *strings, size_t count) {
    AsthraStringOperationValidation validation = {0};
    
    if (!g_safety_config.enable_string_operation_validation) {
        validation.is_deterministic = true;
        return validation;
    }
    
    if (!strings || count == 0) {
        validation.has_overflow_risk = false;
        validation.is_deterministic = true;
        return validation;
    }
    
    size_t total_length = 0;
    validation.is_deterministic = true;
    
    // Calculate total length and check for potential issues
    for (size_t i = 0; i < count; i++) {
        if (!strings[i].data) {
            validation.has_overflow_risk = true;
            snprintf(validation.validation_message, sizeof(validation.validation_message),
                    "Null string data at index %zu", i);
            return validation;
        }
        
        // Check for potential overflow
        if (total_length > SIZE_MAX - strings[i].len) {
            validation.has_overflow_risk = true;
            snprintf(validation.validation_message, sizeof(validation.validation_message),
                    "String concatenation would overflow at index %zu", i);
            return validation;
        }
        
        total_length += strings[i].len;
        
        // Check encoding validity
        if (!asthra_safety_validate_string_encoding(strings[i].data, strings[i].len)) {
            validation.has_encoding_issues = true;
            snprintf(validation.validation_message, sizeof(validation.validation_message),
                    "Invalid UTF-8 encoding in string at index %zu", i);
        }
    }
    
    validation.result_length = total_length;
    validation.max_safe_length = SIZE_MAX / 2; // Conservative limit
    
    if (total_length > validation.max_safe_length) {
        validation.has_overflow_risk = true;
        snprintf(validation.validation_message, sizeof(validation.validation_message),
                "Total length %zu exceeds safe limit %zu", total_length, validation.max_safe_length);
    }
    
    return validation;
}

// String interpolation safety validation removed - feature deprecated for AI generation efficiency

// =============================================================================
// TASK AND SCHEDULER EVENT LOGGING IMPLEMENTATION
// =============================================================================

void asthra_safety_log_task_lifecycle_event(uint64_t task_id, AsthraTaskEvent event, const char *details) {
    if (!g_safety_config.enable_concurrency_debugging) {
        return;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    
    // Expand capacity if needed
    if (g_task_event_count >= g_task_event_capacity) {
        size_t new_capacity = g_task_event_capacity == 0 ? 128 : g_task_event_capacity * 2;
        AsthraTaskEvent *new_events = realloc(g_task_events, new_capacity * sizeof(AsthraTaskEvent));
        if (!new_events) {
            pthread_mutex_unlock(&g_safety_mutex);
            return;
        }
        g_task_events = new_events;
        g_task_event_capacity = new_capacity;
    }
    
    // Log the event (simplified - in full implementation would store complete event data)
    g_task_events[g_task_event_count++] = event;
    
    pthread_mutex_unlock(&g_safety_mutex);
    
    // Log to standard logging system
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_CONCURRENCY,
              "Task %lu: event %d - %s", task_id, event, details ? details : "");
}

void asthra_safety_log_scheduler_event(AsthraSchedulerEvent event, const char *details) {
    if (!g_safety_config.enable_concurrency_debugging) {
        return;
    }
    
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_CONCURRENCY,
              "Scheduler: event %d - %s", event, details ? details : "");
}

void asthra_safety_log_c_task_interaction(uint64_t asthra_task_id, pthread_t c_thread_id, 
                                          const char *interaction_type, void *data, size_t data_size) {
    if (!g_safety_config.enable_concurrency_debugging) {
        return;
    }
    
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_CONCURRENCY,
              "C-Task interaction: Asthra task %lu, C thread %p, type %s, data %p (%zu bytes)",
              asthra_task_id, (void*)c_thread_id, interaction_type ? interaction_type : "unknown",
              data, data_size);
}

// =============================================================================
// RESULT TRACKING IMPLEMENTATION
// =============================================================================

int asthra_safety_register_result_tracker(AsthraResult result, const char *location) {
    if (!g_safety_config.enable_error_handling_aids) {
        return 0;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    
    // Expand capacity if needed
    if (g_result_tracker_count >= g_result_tracker_capacity) {
        size_t new_capacity = g_result_tracker_capacity == 0 ? 64 : g_result_tracker_capacity * 2;
        AsthraResultTracker *new_trackers = realloc(g_result_trackers, 
                                                   new_capacity * sizeof(AsthraResultTracker));
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
    
    if (result.tag == ASTHRA_RESULT_ERR) {
        snprintf(tracker->error_context, sizeof(tracker->error_context),
                "Error result created at %s", location ? location : "unknown location");
    } else {
        snprintf(tracker->error_context, sizeof(tracker->error_context),
                "Ok result created at %s", location ? location : "unknown location");
    }
    
    pthread_mutex_unlock(&g_safety_mutex);
    return (int)tracker->result_id;
}

int asthra_safety_mark_result_handled(uint64_t result_id, const char *location) {
    if (!g_safety_config.enable_error_handling_aids) {
        return 0;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    
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
    
    uint64_t current_time = asthra_get_timestamp_ns();
    uint64_t stale_threshold = 1000000000ULL; // 1 second in nanoseconds
    
    for (size_t i = 0; i < g_result_tracker_count; i++) {
        AsthraResultTracker *tracker = &g_result_trackers[i];
        
        if (!tracker->was_handled && tracker->result.tag == ASTHRA_RESULT_ERR) {
            uint64_t age = current_time - tracker->creation_timestamp_ns;
            
            if (age > stale_threshold) {
                asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                             "Unhandled error result detected", 
                                             tracker->creation_location, 0, "result_tracker", 
                                             &tracker->result, sizeof(AsthraResult));
            }
        }
    }
    
    pthread_mutex_unlock(&g_safety_mutex);
}

// =============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// =============================================================================

bool asthra_safety_validate_string_encoding(const char *str, size_t length) {
    if (!str || length == 0) {
        return true; // Empty string is valid
    }
    
    // Basic UTF-8 validation
    for (size_t i = 0; i < length; ) {
        unsigned char c = (unsigned char)str[i];
        
        if (c < 0x80) {
            // ASCII character
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8 sequence
            if (i + 1 >= length || (str[i + 1] & 0xC0) != 0x80) {
                return false;
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8 sequence
            if (i + 2 >= length || (str[i + 1] & 0xC0) != 0x80 || (str[i + 2] & 0xC0) != 0x80) {
                return false;
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte UTF-8 sequence
            if (i + 3 >= length || (str[i + 1] & 0xC0) != 0x80 || 
                (str[i + 2] & 0xC0) != 0x80 || (str[i + 3] & 0xC0) != 0x80) {
                return false;
            }
            i += 4;
        } else {
            // Invalid UTF-8 start byte
            return false;
        }
    }
    
    return true;
}

bool asthra_safety_check_string_overflow(size_t current_length, size_t additional_length, size_t max_length) {
    if (current_length > max_length - additional_length) {
        return true; // Overflow detected
    }
    return false;
}

bool asthra_safety_validate_interpolation_template(const char *template, size_t expected_args) {
    if (!template) {
        return false;
    }
    
    size_t placeholder_count = 0;
    const char *p = template;
    
    while (*p) {
        if (*p == '{' && *(p + 1) == '}') {
            placeholder_count++;
            p += 2;
        } else {
            p++;
        }
    }
    
    return placeholder_count == expected_args;
}

size_t asthra_safety_get_task_event_count(void) {
    if (!g_safety_config.enable_concurrency_debugging) {
        return 0;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    size_t count = g_task_event_count;
    pthread_mutex_unlock(&g_safety_mutex);
    
    return count;
}

size_t asthra_safety_get_result_tracker_count(void) {
    if (!g_safety_config.enable_error_handling_aids) {
        return 0;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    size_t count = g_result_tracker_count;
    pthread_mutex_unlock(&g_safety_mutex);
    
    return count;
}

void asthra_safety_cleanup_old_task_events(uint64_t max_age_ns) {
    if (!g_safety_config.enable_concurrency_debugging) {
        return;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    
    // For simplified implementation, just clear all events older than threshold
    // In full implementation, would maintain timestamps and selectively remove
    if (g_task_event_count > 1000) { // Arbitrary threshold
        g_task_event_count = g_task_event_count / 2; // Keep recent half
    }
    
    pthread_mutex_unlock(&g_safety_mutex);
}

void asthra_safety_cleanup_handled_result_trackers(void) {
    if (!g_safety_config.enable_error_handling_aids) {
        return;
    }
    
    pthread_mutex_lock(&g_safety_mutex);
    
    size_t write_index = 0;
    for (size_t read_index = 0; read_index < g_result_tracker_count; read_index++) {
        if (!g_result_trackers[read_index].was_handled) {
            // Keep unhandled results
            if (write_index != read_index) {
                g_result_trackers[write_index] = g_result_trackers[read_index];
            }
            write_index++;
        }
        // Skip handled results (effectively removing them)
    }
    
    g_result_tracker_count = write_index;
    
    pthread_mutex_unlock(&g_safety_mutex);
} 
