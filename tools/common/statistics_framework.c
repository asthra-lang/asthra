/**
 * Asthra Programming Language
 * Common Statistics Framework for Tools - Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "statistics_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StatsFramework *stats_create_framework(const char *tool_name) {
    StatsFramework *stats = malloc(sizeof(StatsFramework));
    if (!stats) {
        return NULL;
    }

    stats->tool_name = tool_name;
    stats->counter_count = 0;

    // Initialize all counters
    for (size_t i = 0; i < STATS_MAX_COUNTERS; i++) {
        stats->counters[i].name = NULL;
        stats->counters[i].description = NULL;
        atomic_init(&stats->counters[i].value, 0);
        stats->counters[i].is_time_counter = false;
    }

    return stats;
}

void stats_destroy_framework(StatsFramework *stats) {
    if (stats) {
        free(stats);
    }
}

int stats_add_counter(StatsFramework *stats, const char *name, const char *description,
                      bool is_time_counter) {
    if (!stats || stats->counter_count >= STATS_MAX_COUNTERS) {
        return -1;
    }

    StatsCounter *counter = &stats->counters[stats->counter_count];
    counter->name = name;
    counter->description = description;
    counter->is_time_counter = is_time_counter;
    atomic_init(&counter->value, 0);

    stats->counter_count++;
    return 0;
}

static StatsCounter *find_counter(const StatsFramework *stats, const char *name) {
    if (!stats || !name) {
        return NULL;
    }

    for (size_t i = 0; i < stats->counter_count; i++) {
        if (stats->counters[i].name && strcmp(stats->counters[i].name, name) == 0) {
            return (StatsCounter *)&stats->counters[i];
        }
    }

    return NULL;
}

void stats_increment(StatsFramework *stats, const char *name, uint64_t value) {
    StatsCounter *counter = find_counter(stats, name);
    if (counter) {
        atomic_fetch_add_explicit(&counter->value, value, memory_order_relaxed);
    }
}

void stats_set(StatsFramework *stats, const char *name, uint64_t value) {
    StatsCounter *counter = find_counter(stats, name);
    if (counter) {
        atomic_store_explicit(&counter->value, value, memory_order_relaxed);
    }
}

uint64_t stats_get(const StatsFramework *stats, const char *name) {
    StatsCounter *counter = find_counter(stats, name);
    if (counter) {
        return atomic_load_explicit(&counter->value, memory_order_relaxed);
    }
    return 0;
}

void stats_increment_by_one(StatsFramework *stats, const char *name) {
    stats_increment(stats, name, 1);
}

void stats_add_time_ms(StatsFramework *stats, const char *name, uint64_t milliseconds) {
    stats_increment(stats, name, milliseconds);
}

void stats_print_summary(const StatsFramework *stats, bool verbose) {
    if (!stats) {
        return;
    }

    printf("\n%s Statistics:\n", stats->tool_name);
    printf("=====================================\n");

    for (size_t i = 0; i < stats->counter_count; i++) {
        const StatsCounter *counter = &stats->counters[i];
        uint64_t value = atomic_load_explicit(&counter->value, memory_order_relaxed);

        if (counter->is_time_counter) {
            printf("  %-20s: %llu ms\n", counter->description, (unsigned long long)value);
        } else {
            printf("  %-20s: %llu\n", counter->description, (unsigned long long)value);
        }

        if (verbose && counter->name) {
            printf("    (counter: %s)\n", counter->name);
        }
    }

    printf("=====================================\n");
}

void stats_print_json(const StatsFramework *stats) {
    if (!stats) {
        return;
    }

    printf("{\n");
    printf("  \"tool\": \"%s\",\n", stats->tool_name);
    printf("  \"statistics\": {\n");

    for (size_t i = 0; i < stats->counter_count; i++) {
        const StatsCounter *counter = &stats->counters[i];
        uint64_t value = atomic_load_explicit(&counter->value, memory_order_relaxed);

        printf("    \"%s\": %llu", counter->name, (unsigned long long)value);
        if (i < stats->counter_count - 1) {
            printf(",");
        }
        printf("\n");
    }

    printf("  }\n");
    printf("}\n");
}

uint64_t stats_get_total_time(const StatsFramework *stats) {
    if (!stats) {
        return 0;
    }

    uint64_t total = 0;
    for (size_t i = 0; i < stats->counter_count; i++) {
        const StatsCounter *counter = &stats->counters[i];
        if (counter->is_time_counter) {
            total += atomic_load_explicit(&counter->value, memory_order_relaxed);
        }
    }

    return total;
}

double stats_get_rate(const StatsFramework *stats, const char *numerator, const char *denominator) {
    if (!stats) {
        return 0.0;
    }

    uint64_t num = stats_get(stats, numerator);
    uint64_t den = stats_get(stats, denominator);

    if (den == 0) {
        return 0.0;
    }

    return (double)num / (double)den;
}
