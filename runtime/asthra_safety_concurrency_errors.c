/**
 * Asthra Safety System - Concurrency and Error Handling Module
 * Task lifecycle events and error tracking
 */

#include "asthra_safety_common.h"
#include "asthra_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asthra_safety_concurrency_errors.h"

// =============================================================================
// CONCURRENCY DEBUGGING STATE
// =============================================================================

static AsthraTaskLifecycleEvent *g_task_events = NULL;
static size_t g_task_event_count = 0;
static size_t g_task_event_capacity = 0;

// =============================================================================
// TASK LIFECYCLE EVENT LOGGING IMPLEMENTATION
// =============================================================================

void asthra_safety_log_task_lifecycle_event(uint64_t task_id, AsthraTaskEvent event, const char *details) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_concurrency_debugging) {
        return;
    }
    
    pthread_mutex_t* mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);
    
    // Expand capacity if needed
    if (g_task_event_count >= g_task_event_capacity) {
        size_t new_capacity = g_task_event_capacity == 0 ? 256 : g_task_event_capacity * 2;
        AsthraTaskLifecycleEvent *new_events = realloc(g_task_events, 
                                                       new_capacity * sizeof(AsthraTaskLifecycleEvent));
        if (!new_events) {
            pthread_mutex_unlock(mutex);
            return;
        }
        g_task_events = new_events;
        g_task_event_capacity = new_capacity;
    }
    
    // Log the event
    AsthraTaskLifecycleEvent *log_event = &g_task_events[g_task_event_count++];
    log_event->task_id = task_id;
    log_event->event = event;
    log_event->timestamp_ns = asthra_get_timestamp_ns();
    log_event->thread_id = pthread_self();
    log_event->task_name = NULL; // Could be enhanced to track task names
    log_event->task_data = NULL;
    log_event->task_data_size = 0;
    
    // Initialize result to default values
    log_event->task_result.tag = ASTHRA_RESULT_OK;
    log_event->task_result.data.ok.value_type_id = 0;
    log_event->task_result.data.ok.value = NULL;
    
    if (details) {
        strncpy(log_event->event_details, details, sizeof(log_event->event_details) - 1);
        log_event->event_details[sizeof(log_event->event_details) - 1] = '\0';
    } else {
        log_event->event_details[0] = '\0';
    }
    
    pthread_mutex_unlock(mutex);
    
    // Also log to the standard logging system
    const char* event_names[] = {
        "SPAWNED", "STARTED", "SUSPENDED", "RESUMED", 
        "COMPLETED", "FAILED", "CANCELLED"
    };
    
    const char* event_name = (event < sizeof(event_names)/sizeof(event_names[0])) 
                            ? event_names[event] : "UNKNOWN";
    
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_CONCURRENCY,
              "Task %llu event %s: %s", task_id, event_name, details ? details : "");
}

// =============================================================================
// TASK EVENT ANALYSIS
// =============================================================================

static size_t asthra_safety_get_task_event_count(uint64_t task_id) {
    pthread_mutex_t* mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);
    
    size_t count = 0;
    for (size_t i = 0; i < g_task_event_count; i++) {
        if (g_task_events[i].task_id == task_id) {
            count++;
        }
    }
    
    pthread_mutex_unlock(mutex);
    return count;
}

static AsthraTaskLifecycleEvent* asthra_safety_get_last_task_event(uint64_t task_id) {
    pthread_mutex_t* mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);
    
    AsthraTaskLifecycleEvent* last_event = NULL;
    uint64_t latest_timestamp = 0;
    
    for (size_t i = 0; i < g_task_event_count; i++) {
        if (g_task_events[i].task_id == task_id && 
            g_task_events[i].timestamp_ns > latest_timestamp) {
            last_event = &g_task_events[i];
            latest_timestamp = g_task_events[i].timestamp_ns;
        }
    }
    
    pthread_mutex_unlock(mutex);
    return last_event;
}

// =============================================================================
// TASK STATE VALIDATION
// =============================================================================

static bool asthra_safety_validate_task_state_transition(uint64_t task_id, AsthraTaskEvent new_event) {
    AsthraTaskLifecycleEvent* last_event = asthra_safety_get_last_task_event(task_id);
    
    if (!last_event) {
        // First event for this task - should be SPAWNED
        return (new_event == ASTHRA_TASK_EVENT_SPAWNED);
    }
    
    AsthraTaskEvent last_state = last_event->event;
    
    // Define valid state transitions
    switch (last_state) {
        case ASTHRA_TASK_EVENT_SPAWNED:
            return (new_event == ASTHRA_TASK_EVENT_STARTED || 
                   new_event == ASTHRA_TASK_EVENT_CANCELLED);
            
        case ASTHRA_TASK_EVENT_STARTED:
            return (new_event == ASTHRA_TASK_EVENT_SUSPENDED ||
                   new_event == ASTHRA_TASK_EVENT_COMPLETED ||
                   new_event == ASTHRA_TASK_EVENT_FAILED ||
                   new_event == ASTHRA_TASK_EVENT_CANCELLED);
            
        case ASTHRA_TASK_EVENT_SUSPENDED:
            return (new_event == ASTHRA_TASK_EVENT_RESUMED ||
                   new_event == ASTHRA_TASK_EVENT_CANCELLED);
            
        case ASTHRA_TASK_EVENT_RESUMED:
            return (new_event == ASTHRA_TASK_EVENT_SUSPENDED ||
                   new_event == ASTHRA_TASK_EVENT_COMPLETED ||
                   new_event == ASTHRA_TASK_EVENT_FAILED ||
                   new_event == ASTHRA_TASK_EVENT_CANCELLED);
            
        case ASTHRA_TASK_EVENT_COMPLETED:
        case ASTHRA_TASK_EVENT_FAILED:
        case ASTHRA_TASK_EVENT_CANCELLED:
            // Terminal states - no further transitions allowed
            return false;
            
        default:
            return false;
    }
}

// =============================================================================
// ENHANCED TASK LIFECYCLE LOGGING
// =============================================================================

void asthra_safety_log_task_lifecycle_event_enhanced(uint64_t task_id, AsthraTaskEvent event, 
                                                     const char *details, AsthraResult *result) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_concurrency_debugging) {
        return;
    }
    
    // Validate state transition
    if (!asthra_safety_validate_task_state_transition(task_id, event)) {
        char violation_msg[256];
        snprintf(violation_msg, sizeof(violation_msg),
                "Invalid task state transition for task %llu to event %d", task_id, event);
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_CONCURRENCY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                     violation_msg, __FILE__, __LINE__, __func__,
                                     &task_id, sizeof(task_id));
    }
    
    // Log the event with enhanced information
    pthread_mutex_t* mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);
    
    // Expand capacity if needed
    if (g_task_event_count >= g_task_event_capacity) {
        size_t new_capacity = g_task_event_capacity == 0 ? 256 : g_task_event_capacity * 2;
        AsthraTaskLifecycleEvent *new_events = realloc(g_task_events, 
                                                       new_capacity * sizeof(AsthraTaskLifecycleEvent));
        if (!new_events) {
            pthread_mutex_unlock(mutex);
            return;
        }
        g_task_events = new_events;
        g_task_event_capacity = new_capacity;
    }
    
    // Log the event
    AsthraTaskLifecycleEvent *log_event = &g_task_events[g_task_event_count++];
    log_event->task_id = task_id;
    log_event->event = event;
    log_event->timestamp_ns = asthra_get_timestamp_ns();
    log_event->thread_id = pthread_self();
    log_event->task_name = NULL;
    log_event->task_data = NULL;
    log_event->task_data_size = 0;
    
    // Copy result if provided
    if (result) {
        log_event->task_result = *result;
    } else {
        log_event->task_result.tag = ASTHRA_RESULT_OK;
        log_event->task_result.data.ok.value_type_id = 0;
        log_event->task_result.data.ok.value = NULL;
    }
    
    if (details) {
        strncpy(log_event->event_details, details, sizeof(log_event->event_details) - 1);
        log_event->event_details[sizeof(log_event->event_details) - 1] = '\0';
    } else {
        log_event->event_details[0] = '\0';
    }
    
    pthread_mutex_unlock(mutex);
}

// =============================================================================
// MODULE CLEANUP
// =============================================================================

void asthra_safety_concurrency_errors_cleanup(void) {
    pthread_mutex_t* mutex = asthra_safety_get_mutex();
    pthread_mutex_lock(mutex);
    
    // Clean up task events
    free(g_task_events);
    g_task_events = NULL;
    g_task_event_count = 0;
    g_task_event_capacity = 0;
    
    pthread_mutex_unlock(mutex);
}
