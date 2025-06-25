#ifndef FAST_CHECK_COMMAND_H
#define FAST_CHECK_COMMAND_H

#include "fast_check_engine.h"
#include <stdbool.h>
#include <stddef.h>

// File status constants
typedef enum {
    FILE_STATUS_OK = 0,
    FILE_STATUS_ERROR,
    FILE_STATUS_WARNING,
    FILE_STATUS_SKIP
} FileStatus;

// File check result structure (different from FastCheckResult in engine)
typedef struct {
    char *file_path;
    FileStatus status;
    int error_count;
    int warning_count;
    double check_time_ms;
    bool was_cached;
    char *error_message;
} FileCheckResult;

// Command result structure
typedef struct {
    size_t files_checked;
    size_t files_passed;
    size_t files_failed;
    double total_time_ms;
    double avg_time_per_file;
    FileCheckResult **file_results;
    size_t result_count;
} FastCheckCommandResult;

// Statistics structure
typedef struct {
    size_t total_checks_performed;
    size_t total_files_processed;
    size_t cache_hits;
    size_t cache_misses;
    double total_analysis_time_ms;
    double avg_check_time_ms;
} FastCheckStatistics;

// Command configuration structure
typedef struct {
    bool watch_mode;
    bool syntax_only;
    char *output_format;
    int max_errors;
    bool enable_cache;
    bool verbose;
} FastCheckCommandConfig;

// Stub functions
FastCheckCommandConfig *fast_check_command_config_create(void);
void fast_check_command_config_destroy(FastCheckCommandConfig *config);
FastCheckCommandResult *fast_check_run_command_with_engine(FastCheckEngine *engine, char **files,
                                                           size_t file_count,
                                                           FastCheckConfig *config);

// Additional command functions
FastCheckConfig *fast_check_config_create(void);
void fast_check_config_set_watch_mode(FastCheckConfig *config, bool watch_mode);
void fast_check_config_set_syntax_only(FastCheckConfig *config, bool syntax_only);
void fast_check_config_set_timeout(FastCheckConfig *config, int timeout_ms);
void fast_check_config_set_output_format(FastCheckConfig *config, const char *format);
void fast_check_config_add_include_pattern(FastCheckConfig *config, const char *pattern);
void fast_check_config_add_exclude_pattern(FastCheckConfig *config, const char *pattern);

// File discovery functions
char **fast_check_discover_files(const char *directory, const char **include_patterns,
                                 size_t include_count, const char **exclude_patterns,
                                 size_t exclude_count, size_t *file_count);
void fast_check_free_file_list(char **files, size_t file_count);

// Advanced command functions
FileCheckResult *fast_check_single_file_detailed(FastCheckEngine *engine, const char *file_path,
                                                 FastCheckConfig *config);

// Note: fast_check_run_command is implemented in main library
void fast_check_command_result_destroy(FastCheckCommandResult *result);
void fast_check_print_results(FastCheckCommandResult *result, bool verbose);
bool fast_check_syntax_only(const char *file_path);
bool fast_check_incremental_validate(char **changed_files, size_t file_count);
FastCheckCommandResult *fast_check_run_on_directory(FastCheckEngine *engine, const char *directory,
                                                    FastCheckConfig *config);

// Result management functions
void fast_check_file_result_destroy(FileCheckResult *result);
void fast_check_command_result_destroy(FastCheckCommandResult *result);

// Output formatting functions
char *fast_check_format_result_human(FileCheckResult *result);
char *fast_check_format_result_json(FileCheckResult *result);
char *fast_check_format_result_compact(FileCheckResult *result);

// Statistics functions
FastCheckStatistics *fast_check_get_statistics(FastCheckEngine *engine);
void fast_check_statistics_destroy(FastCheckStatistics *stats);

// Utility functions
double fast_check_get_file_modification_time(const char *file_path);
char *fast_check_get_relative_path(const char *file_path, const char *base_path);
bool fast_check_should_check_file(const char *file_path, FastCheckConfig *config);

#endif // FAST_CHECK_COMMAND_H
