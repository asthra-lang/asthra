#ifndef FAST_CHECK_COMMAND_H
#define FAST_CHECK_COMMAND_H

#include "fast_check_engine.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fast Check Command Results
 * Aggregated results from checking multiple files
 */
typedef struct {
    size_t files_checked;     // Total files processed
    size_t total_errors;      // Total error count across all files
    size_t total_warnings;    // Total warning count across all files
    double total_time_ms;     // Total check time in milliseconds
    double avg_time_per_file; // Average time per file
    size_t cache_hits;        // Number of cache hits
    size_t cache_misses;      // Number of cache misses
    bool success;             // Overall success status
    char *summary_message;    // Human-readable summary
} FastCheckCommandResult;

/**
 * File Check Status
 * Status for individual file checking
 */
typedef enum {
    FILE_STATUS_OK,         // No issues found
    FILE_STATUS_WARNING,    // Warnings found
    FILE_STATUS_ERROR,      // Errors found
    FILE_STATUS_TIMEOUT,    // Check timed out
    FILE_STATUS_NOT_FOUND,  // File not found
    FILE_STATUS_PERMISSION, // Permission denied
    FILE_STATUS_CACHE_ERROR // Cache operation failed
} FileCheckStatus;

/**
 * Individual File Result
 * Result for checking a single file
 */
typedef struct {
    char *file_path;        // Path to the checked file
    FileCheckStatus status; // Check status
    size_t error_count;     // Number of errors
    size_t warning_count;   // Number of warnings
    double check_time_ms;   // Time taken to check this file
    bool was_cached;        // Whether result came from cache
    char *error_message;    // Error message if check failed
} FileCheckResult;

// Configuration Management
FastCheckConfig *fast_check_config_create(void);
void fast_check_config_destroy(FastCheckConfig *config);
void fast_check_config_set_watch_mode(FastCheckConfig *config, bool enabled);
void fast_check_config_set_syntax_only(FastCheckConfig *config, bool enabled);
void fast_check_config_set_timeout(FastCheckConfig *config, uint64_t timeout_ms);
void fast_check_config_set_output_format(FastCheckConfig *config, const char *format);
void fast_check_config_add_include_pattern(FastCheckConfig *config, const char *pattern);
void fast_check_config_add_exclude_pattern(FastCheckConfig *config, const char *pattern);

// Command Interface
FastCheckCommandResult *fast_check_run_command(FastCheckEngine *engine, const char **files,
                                               size_t file_count, const FastCheckConfig *config);

FastCheckCommandResult *fast_check_run_on_directory(FastCheckEngine *engine,
                                                    const char *directory_path,
                                                    const FastCheckConfig *config);

FileCheckResult *fast_check_single_file_detailed(FastCheckEngine *engine, const char *file_path,
                                                 const FastCheckConfig *config);

// Watch Mode Support
typedef void (*FastCheckWatchCallback)(const char *file_path, FileCheckResult *result,
                                       void *user_data);

int fast_check_start_watch_mode(FastCheckEngine *engine, const char **watch_paths,
                                size_t path_count, FastCheckWatchCallback callback,
                                void *user_data);

void fast_check_stop_watch_mode(void);

// File Discovery
char **fast_check_discover_files(const char *directory_path, const char **include_patterns,
                                 size_t include_count, const char **exclude_patterns,
                                 size_t exclude_count, size_t *file_count);

char **fast_check_get_modified_files(const char *directory_path, int64_t since_timestamp,
                                     size_t *file_count);

// Output Formatting
char *fast_check_format_result_human(const FileCheckResult *result);
char *fast_check_format_result_json(const FileCheckResult *result);
char *fast_check_format_result_compact(const FileCheckResult *result);
char *fast_check_format_summary_human(const FastCheckCommandResult *result);
char *fast_check_format_summary_json(const FastCheckCommandResult *result);

// Utility Functions
bool fast_check_should_check_file(const char *file_path, const FastCheckConfig *config);
double fast_check_get_file_modification_time(const char *file_path);
char *fast_check_get_relative_path(const char *file_path, const char *base_path);

// Result Management
void fast_check_command_result_destroy(FastCheckCommandResult *result);
void fast_check_file_result_destroy(FileCheckResult *result);
void fast_check_free_file_list(char **files, size_t count);

// Statistics and Performance
typedef struct {
    size_t total_checks_performed;
    size_t total_files_processed;
    double total_check_time_ms;
    double average_check_time_ms;
    size_t cache_hit_rate_percent;
    size_t syntax_only_checks;
    size_t full_semantic_checks;
} FastCheckStatistics;

FastCheckStatistics *fast_check_get_statistics(FastCheckEngine *engine);
void fast_check_reset_statistics(FastCheckEngine *engine);
void fast_check_statistics_destroy(FastCheckStatistics *stats);

#ifdef __cplusplus
}
#endif

#endif // FAST_CHECK_COMMAND_H
