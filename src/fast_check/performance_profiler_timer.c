/*
 * Asthra Fast Check Performance Profiler Timer Implementation
 * Timer management functions
 */

#include "performance_profiler.h"
#include <stdio.h>
#include <time.h>

// =============================================================================
// Timer Management
// =============================================================================

void performance_timer_start(PerformanceTimer *timer) {
    if (!timer)
        return;

    clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
    timer->is_active = true;
}

void performance_timer_stop(PerformanceTimer *timer) {
    if (!timer || !timer->is_active)
        return;

    clock_gettime(CLOCK_MONOTONIC, &timer->end_time);
    timer->duration_ms = timespec_diff_ms(&timer->start_time, &timer->end_time);
    timer->is_active = false;
}

double performance_timer_get_duration_ms(const PerformanceTimer *timer) {
    if (!timer)
        return 0.0;

    if (timer->is_active) {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        return timespec_diff_ms(&timer->start_time, &current_time);
    }

    return timer->duration_ms;
}