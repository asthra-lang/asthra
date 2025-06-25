#include "fast_check_command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Output Formatting Implementation

char *fast_check_format_result_human(const FileCheckResult *result) {
    if (!result)
        return NULL;

    char *buffer = malloc(1024);
    if (!buffer)
        return NULL;

    const char *status_str;
    switch (result->status) {
    case FILE_STATUS_OK:
        status_str = "OK";
        break;
    case FILE_STATUS_WARNING:
        status_str = "WARN";
        break;
    case FILE_STATUS_ERROR:
        status_str = "ERROR";
        break;
    case FILE_STATUS_TIMEOUT:
        status_str = "TIMEOUT";
        break;
    case FILE_STATUS_NOT_FOUND:
        status_str = "NOT_FOUND";
        break;
    case FILE_STATUS_PERMISSION:
        status_str = "PERMISSION";
        break;
    case FILE_STATUS_CACHE_ERROR:
        status_str = "CACHE_ERROR";
        break;
    default:
        status_str = "UNKNOWN";
        break;
    }

    snprintf(buffer, 1024, "%s %s (%.1fms)%s\n  Errors: %zu, Warnings: %zu%s", status_str,
             result->file_path, result->check_time_ms, result->was_cached ? " [cached]" : "",
             result->error_count, result->warning_count,
             result->error_message ? result->error_message : "");

    return buffer;
}

char *fast_check_format_result_json(const FileCheckResult *result) {
    if (!result)
        return NULL;

    char *buffer = malloc(2048);
    if (!buffer)
        return NULL;

    snprintf(buffer, 2048,
             "{\n"
             "  \"file_path\": \"%s\",\n"
             "  \"status\": \"%d\",\n"
             "  \"error_count\": %zu,\n"
             "  \"warning_count\": %zu,\n"
             "  \"check_time_ms\": %.1f,\n"
             "  \"was_cached\": %s,\n"
             "  \"error_message\": \"%s\"\n"
             "}",
             result->file_path, result->status, result->error_count, result->warning_count,
             result->check_time_ms, result->was_cached ? "true" : "false",
             result->error_message ? result->error_message : "");

    return buffer;
}

char *fast_check_format_result_compact(const FileCheckResult *result) {
    if (!result)
        return NULL;

    char *buffer = malloc(512);
    if (!buffer)
        return NULL;

    snprintf(buffer, 512, "%s:%d:E%zu:W%zu:%.0fms", result->file_path, result->status,
             result->error_count, result->warning_count, result->check_time_ms);

    return buffer;
}

char *fast_check_format_summary_human(const FastCheckCommandResult *result) {
    if (!result)
        return NULL;

    char *buffer = malloc(1024);
    if (!buffer)
        return NULL;

    snprintf(buffer, 1024,
             "\nSummary: %s\n"
             "Files checked: %zu\n"
             "Total time: %.1fms\n"
             "Average time per file: %.1fms\n"
             "Errors: %zu\n"
             "Warnings: %zu\n"
             "Cache hits: %zu\n"
             "Cache misses: %zu\n",
             result->summary_message ? result->summary_message : "No summary",
             result->files_checked, result->total_time_ms, result->avg_time_per_file,
             result->total_errors, result->total_warnings, result->cache_hits,
             result->cache_misses);

    return buffer;
}

char *fast_check_format_summary_json(const FastCheckCommandResult *result) {
    if (!result)
        return NULL;

    char *buffer = malloc(2048);
    if (!buffer)
        return NULL;

    snprintf(buffer, 2048,
             "{\n"
             "  \"files_checked\": %zu,\n"
             "  \"total_errors\": %zu,\n"
             "  \"total_warnings\": %zu,\n"
             "  \"total_time_ms\": %.1f,\n"
             "  \"avg_time_per_file\": %.1f,\n"
             "  \"cache_hits\": %zu,\n"
             "  \"cache_misses\": %zu,\n"
             "  \"success\": %s,\n"
             "  \"summary_message\": \"%s\"\n"
             "}",
             result->files_checked, result->total_errors, result->total_warnings,
             result->total_time_ms, result->avg_time_per_file, result->cache_hits,
             result->cache_misses, result->success ? "true" : "false",
             result->summary_message ? result->summary_message : "");

    return buffer;
}