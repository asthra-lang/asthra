// This file provides stub implementations for command functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "fast_check_command.h"
#include "fast_check_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// =============================================================================
// COMMAND FUNCTIONS
// =============================================================================

char **fast_check_discover_files(const char *directory, const char **include_patterns,
                                 size_t include_count, const char **exclude_patterns,
                                 size_t exclude_count, size_t *file_count) {
    // Stub implementation - return empty list
    *file_count = 0;
    return NULL;
}

void fast_check_free_file_list(char **files, size_t file_count) {
    // Stub implementation
    (void)files;
    (void)file_count;
}

FileCheckResult *fast_check_single_file_detailed(FastCheckEngine *engine, const char *file_path,
                                                 FastCheckConfig *config) {
    (void)engine;
    (void)config;

    FileCheckResult *result = calloc(1, sizeof(FileCheckResult));
    if (!result)
        return NULL;

    result->file_path = strdup(file_path);
    result->status = FILE_STATUS_OK;
    result->error_count = 0;
    result->warning_count = 0;
    result->check_time_ms = 1.0;
    result->was_cached = false;
    result->error_message = NULL;

    return result;
}

FastCheckCommandResult *fast_check_run_command(FastCheckEngine *engine, char **files,
                                               size_t file_count, FastCheckConfig *config) {
    (void)engine;
    (void)config;

    FastCheckCommandResult *result = calloc(1, sizeof(FastCheckCommandResult));
    if (!result)
        return NULL;

    result->files_checked = file_count;
    result->files_passed = file_count; // Assume all pass in stub
    result->files_failed = 0;
    result->total_time_ms = file_count * 1.0; // 1ms per file
    result->avg_time_per_file = 1.0;
    result->file_results = NULL; // Don't populate detailed results in stub
    result->result_count = 0;

    return result;
}

FastCheckCommandResult *fast_check_run_on_directory(FastCheckEngine *engine, const char *directory,
                                                    FastCheckConfig *config) {
    (void)engine;
    (void)directory;
    (void)config;

    FastCheckCommandResult *result = calloc(1, sizeof(FastCheckCommandResult));
    if (!result)
        return NULL;

    // Simulate finding 2 files in any directory
    result->files_checked = 2;
    result->files_passed = 2;
    result->files_failed = 0;
    result->total_time_ms = 2.0;
    result->avg_time_per_file = 1.0;
    result->file_results = NULL;
    result->result_count = 0;

    return result;
}

void fast_check_file_result_destroy(FileCheckResult *result) {
    if (result) {
        free(result->file_path);
        free(result->error_message);
        free(result);
    }
}

void fast_check_command_result_destroy(FastCheckCommandResult *result) {
    if (result) {
        if (result->file_results) {
            for (size_t i = 0; i < result->result_count; i++) {
                fast_check_file_result_destroy(result->file_results[i]);
            }
            free(result->file_results);
        }
        free(result);
    }
}

// Output formatting functions
char *fast_check_format_result_human(FileCheckResult *result) {
    if (!result)
        return NULL;

    char *output = malloc(256);
    if (!output)
        return NULL;

    snprintf(output, 256, "%s: %s (errors: %d, warnings: %d, %.1fms)", result->file_path,
             (result->status == FILE_STATUS_OK) ? "OK" : "ERROR", result->error_count,
             result->warning_count, result->check_time_ms);

    return output;
}

char *fast_check_format_result_json(FileCheckResult *result) {
    if (!result)
        return NULL;

    char *output = malloc(512);
    if (!output)
        return NULL;

    snprintf(output, 512,
             "{"
             "\"file_path\":\"%s\","
             "\"status\":\"%s\","
             "\"error_count\":%d,"
             "\"warning_count\":%d,"
             "\"check_time_ms\":%.1f,"
             "\"was_cached\":%s"
             "}",
             result->file_path, (result->status == FILE_STATUS_OK) ? "ok" : "error",
             result->error_count, result->warning_count, result->check_time_ms,
             result->was_cached ? "true" : "false");

    return output;
}

char *fast_check_format_result_compact(FileCheckResult *result) {
    if (!result)
        return NULL;

    char *output = malloc(128);
    if (!output)
        return NULL;

    snprintf(output, 128, "%s:%s", result->file_path,
             (result->status == FILE_STATUS_OK) ? "OK" : "ERR");

    return output;
}

// Statistics functions
FastCheckStatistics *fast_check_get_statistics(FastCheckEngine *engine) {
    if (!engine)
        return NULL;

    FastCheckStatistics *stats = malloc(sizeof(FastCheckStatistics));
    if (!stats)
        return NULL;

    stats->total_checks_performed = engine->stats.total_checks;
    stats->total_files_processed = engine->stats.total_checks; // Same in stub
    stats->cache_hits = engine->stats.cache_hits;
    stats->cache_misses = engine->stats.cache_misses;
    stats->total_analysis_time_ms =
        engine->stats.average_check_time_ms * engine->stats.total_checks;
    stats->avg_check_time_ms = engine->stats.average_check_time_ms;

    return stats;
}

void fast_check_statistics_destroy(FastCheckStatistics *stats) {
    free(stats);
}

// Utility functions
double fast_check_get_file_modification_time(const char *file_path) {
    if (!file_path)
        return 0.0;

    struct stat st;
    if (stat(file_path, &st) == 0) {
        return (double)st.st_mtime;
    }
    return 0.0;
}

char *fast_check_get_relative_path(const char *file_path, const char *base_path) {
    if (!file_path || !base_path)
        return NULL;

    size_t base_len = strlen(base_path);
    if (strncmp(file_path, base_path, base_len) == 0) {
        const char *relative = file_path + base_len;
        if (*relative == '/')
            relative++; // Skip leading slash
        return strdup(relative);
    }

    return strdup(file_path); // Return copy if not relative
}

bool fast_check_should_check_file(const char *file_path, FastCheckConfig *config) {
    (void)config; // Ignore config patterns in stub

    if (!file_path)
        return false;

    const char *ext = strrchr(file_path, '.');
    return (ext && strcmp(ext, ".asthra") == 0);
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_command_dummy = 0;

#endif // FAST_CHECK_USE_STUBS