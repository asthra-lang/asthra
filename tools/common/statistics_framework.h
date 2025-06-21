/**
 * Asthra Programming Language
 * Common Statistics Framework for Tools
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_STATISTICS_FRAMEWORK_H
#define ASTHRA_STATISTICS_FRAMEWORK_H

#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Maximum number of statistics counters per tool
#define STATS_MAX_COUNTERS 16

// Statistics counter definition
typedef struct {
    const char *name;           // Counter name (e.g., "lines_formatted")
    const char *description;    // Human-readable description
    _Atomic(uint64_t) value;   // Atomic counter value
    bool is_time_counter;      // Whether this is a time measurement
} StatsCounter;

// Statistics framework structure
typedef struct {
    const char *tool_name;      // Tool name for reporting
    StatsCounter counters[STATS_MAX_COUNTERS];  // Counter array
    size_t counter_count;       // Number of active counters
} StatsFramework;

// Statistics framework functions
StatsFramework* stats_create_framework(const char *tool_name);
void stats_destroy_framework(StatsFramework *stats);

// Counter management
int stats_add_counter(StatsFramework *stats, const char *name, 
                     const char *description, bool is_time_counter);

// Counter operations (thread-safe)
void stats_increment(StatsFramework *stats, const char *name, uint64_t value);
void stats_set(StatsFramework *stats, const char *name, uint64_t value);
uint64_t stats_get(const StatsFramework *stats, const char *name);

// Convenience functions for common patterns
void stats_increment_by_one(StatsFramework *stats, const char *name);
void stats_add_time_ms(StatsFramework *stats, const char *name, uint64_t milliseconds);

// Reporting functions
void stats_print_summary(const StatsFramework *stats, bool verbose);
void stats_print_json(const StatsFramework *stats);

// Utility functions
uint64_t stats_get_total_time(const StatsFramework *stats);
double stats_get_rate(const StatsFramework *stats, const char *numerator, const char *denominator);

#endif // ASTHRA_STATISTICS_FRAMEWORK_H 
