#include "fast_check_command.h"
#include "fast_check_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Statistics and Performance Management

FastCheckStatistics *fast_check_get_statistics(FastCheckEngine *engine) {
    if (!engine)
        return NULL;

    FastCheckStatistics *stats = calloc(1, sizeof(FastCheckStatistics));
    if (!stats)
        return NULL;

    // Get statistics from engine (simplified)
    stats->total_checks_performed = engine->total_checks;
    stats->total_files_processed = engine->total_checks; // Simplified
    stats->total_check_time_ms = engine->total_check_time;
    stats->average_check_time_ms =
        engine->total_checks > 0 ? (double)engine->total_check_time / (double)engine->total_checks
                                 : 0.0;

    size_t total_operations = engine->cache_hits + engine->cache_misses;
    stats->cache_hit_rate_percent =
        total_operations > 0 ? (engine->cache_hits * 100) / total_operations : 0;

    stats->syntax_only_checks = 0; // Would need to track separately
    stats->full_semantic_checks = engine->total_checks;

    return stats;
}

void fast_check_reset_statistics(FastCheckEngine *engine) {
    if (!engine)
        return;

    engine->total_checks = 0;
    engine->total_check_time = 0.0;
    engine->cache_hits = 0;
    engine->cache_misses = 0;
}

void fast_check_statistics_destroy(FastCheckStatistics *stats) {
    free(stats);
}

// Result Management Functions

void fast_check_command_result_destroy(FastCheckCommandResult *result) {
    if (!result)
        return;

    free(result->summary_message);
    free(result);
}

void fast_check_file_result_destroy(FileCheckResult *result) {
    if (!result)
        return;

    free(result->file_path);
    free(result->error_message);
    free(result);
}