#include "benchmark.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/task.h>
#elif defined(__linux__)
#include <sys/resource.h>
#endif

// =============================================================================
// PLATFORM-SPECIFIC TIMING IMPLEMENTATION
// =============================================================================

// High-resolution timer calibration
static uint64_t g_timer_frequency = 0;
static bool g_timer_initialized = false;

static void initialize_timer(void) {
    if (g_timer_initialized) {
        return;
    }

#ifdef __APPLE__
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    g_timer_frequency = ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND * timebase.denom / timebase.numer;
#elif defined(__linux__)
    struct timespec res;
    if (clock_getres(CLOCK_MONOTONIC, &res) == 0) {
        g_timer_frequency = ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND;
    } else {
        g_timer_frequency = ASTHRA_BENCHMARK_MICROSECONDS_PER_SECOND;
    }
#else
    g_timer_frequency = ASTHRA_BENCHMARK_MICROSECONDS_PER_SECOND;
#endif

    g_timer_initialized = true;
}

uint64_t asthra_benchmark_get_time_ns(void) {
    initialize_timer();

#ifdef __APPLE__
    return mach_absolute_time();
#elif defined(__linux__)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (uint64_t)ts.tv_sec * ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND + (uint64_t)ts.tv_nsec;
    }
    // Fallback to gettimeofday
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND +
           (uint64_t)tv.tv_usec * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND +
           (uint64_t)tv.tv_usec * 1000;
#endif
}

uint64_t asthra_benchmark_get_cpu_time_ns(void) {
#ifdef __APPLE__
    struct task_thread_times_info thread_info;
    mach_msg_type_number_t thread_info_count = TASK_THREAD_TIMES_INFO_COUNT;

    if (task_info(mach_task_self(), TASK_THREAD_TIMES_INFO, (task_info_t)&thread_info,
                  &thread_info_count) == KERN_SUCCESS) {
        return (uint64_t)(thread_info.user_time.seconds + thread_info.system_time.seconds) *
                   ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND +
               (uint64_t)(thread_info.user_time.microseconds +
                          thread_info.system_time.microseconds) *
                   1000;
    }
#elif defined(__linux__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return (uint64_t)(usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) *
                   ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND +
               (uint64_t)(usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) * 1000;
    }
#endif

    // Fallback to wall clock time
    return asthra_benchmark_get_time_ns();
}

// =============================================================================
// TIMER FUNCTIONS
// =============================================================================

AsthraBenchmarkTimer asthra_benchmark_timer_start(void) {
    AsthraBenchmarkTimer timer = {.start_ns = asthra_benchmark_get_time_ns(),
                                  .end_ns = 0,
                                  .duration_ns = 0,
                                  .is_valid = true};
    return timer;
}

void asthra_benchmark_timer_end(AsthraBenchmarkTimer *timer) {
    if (!timer || !timer->is_valid) {
        return;
    }

    timer->end_ns = asthra_benchmark_get_time_ns();
    timer->duration_ns = timer->end_ns - timer->start_ns;
}

uint64_t asthra_benchmark_timer_duration_ns(const AsthraBenchmarkTimer *timer) {
    return timer && timer->is_valid ? timer->duration_ns : 0;
}

// =============================================================================
// TIME CONVERSION UTILITIES
// =============================================================================

double asthra_benchmark_ns_to_seconds(uint64_t nanoseconds) {
    return (double)nanoseconds / (double)ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND;
}

double asthra_benchmark_ns_to_milliseconds(uint64_t nanoseconds) {
    return (double)nanoseconds / (double)(ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND /
                                          ASTHRA_BENCHMARK_MILLISECONDS_PER_SECOND);
}

double asthra_benchmark_ns_to_microseconds(uint64_t nanoseconds) {
    return (double)nanoseconds / (double)(ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND /
                                          ASTHRA_BENCHMARK_MICROSECONDS_PER_SECOND);
}
