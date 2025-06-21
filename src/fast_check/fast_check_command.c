#include "fast_check_command.h"
#include "fast_check_engine.h"
#include "dependency_tracker.h"
#include "fast_semantic_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

// Command Interface Implementation
FastCheckCommandResult *fast_check_run_command(
    FastCheckEngine *engine, 
    const char **files, 
    size_t file_count,
    const FastCheckConfig *config) {
    
    if (!engine || !files || !config) return NULL;
    
    FastCheckCommandResult *cmd_result = calloc(1, sizeof(FastCheckCommandResult));
    if (!cmd_result) return NULL;
    
    clock_t start_time = clock();
    
    // Clear cache if requested
    if (config->clear_cache) {
        fast_check_engine_clear_cache(engine);
    }
    
    // Process each file
    for (size_t i = 0; i < file_count; i++) {
        // Check timeout
        clock_t current_time = clock();
        double elapsed_ms = (double)(current_time - start_time) / (double)CLOCKS_PER_SEC * 1000.0;
        if (elapsed_ms > config->timeout_ms) {
            break;
        }
        
        // Skip files that don't match patterns
        if (!fast_check_should_check_file(files[i], config)) {
            continue;
        }
        
        // Check the file (syntax_only flag is handled inside fast_check_file)
        FastCheckResult *result = fast_check_file(engine, files[i]);
        
        if (result) {
            cmd_result->files_checked++;
            cmd_result->total_errors += result->error_count;
            cmd_result->total_warnings += result->warning_count;
            cmd_result->total_time_ms += result->check_time_ms;
            
            if (result->was_cached) {
                cmd_result->cache_hits++;
            } else {
                cmd_result->cache_misses++;
            }
            
            fast_check_result_destroy(result);
        }
    }
    
    // Calculate statistics
    if (cmd_result->files_checked > 0) {
        cmd_result->avg_time_per_file = cmd_result->total_time_ms / (double)cmd_result->files_checked;
    }
    
    cmd_result->success = (cmd_result->total_errors == 0);
    
    // Create summary message
    char summary_buffer[512];
    snprintf(summary_buffer, sizeof(summary_buffer),
             "Checked %zu files in %.1fms (avg %.1fms/file). "
             "Errors: %zu, Warnings: %zu, Cache hits: %zu",
             cmd_result->files_checked,
             cmd_result->total_time_ms,
             cmd_result->avg_time_per_file,
             cmd_result->total_errors,
             cmd_result->total_warnings,
             cmd_result->cache_hits);
    
    cmd_result->summary_message = strdup(summary_buffer);
    
    return cmd_result;
}

FastCheckCommandResult *fast_check_run_on_directory(
    FastCheckEngine *engine,
    const char *directory_path,
    const FastCheckConfig *config) {
    
    if (!engine || !directory_path || !config) return NULL;
    
    // Discover files in directory
    size_t file_count;
    char **files = fast_check_discover_files(
        directory_path,
        config->include_patterns,
        config->include_count,
        config->exclude_patterns,
        config->exclude_count,
        &file_count
    );
    
    if (!files || file_count == 0) {
        FastCheckCommandResult *result = calloc(1, sizeof(FastCheckCommandResult));
        if (result) {
            result->success = true;
            result->summary_message = strdup("No files found to check");
        }
        return result;
    }
    
    // Run command on discovered files
    FastCheckCommandResult *result = fast_check_run_command(
        engine, (const char**)files, file_count, config);
    
    // Cleanup
    fast_check_free_file_list(files, file_count);
    
    return result;
}

FileCheckResult *fast_check_single_file_detailed(
    FastCheckEngine *engine,
    const char *file_path,
    const FastCheckConfig *config) {
    
    if (!engine || !file_path || !config) return NULL;
    
    FileCheckResult *file_result = calloc(1, sizeof(FileCheckResult));
    if (!file_result) return NULL;
    
    file_result->file_path = strdup(file_path);
    
    // Check if file exists
    struct stat file_stat;
    if (stat(file_path, &file_stat) != 0) {
        file_result->status = FILE_STATUS_NOT_FOUND;
        file_result->error_message = strdup("File not found");
        return file_result;
    }
    
    // Check file
    FastCheckResult *result = fast_check_file(engine, file_path);
    if (!result) {
        file_result->status = FILE_STATUS_CACHE_ERROR;
        file_result->error_message = strdup("Check operation failed");
        return file_result;
    }
    
    // Copy result data
    file_result->error_count = result->error_count;
    file_result->warning_count = result->warning_count;
    file_result->check_time_ms = result->check_time_ms;
    file_result->was_cached = result->was_cached;
    
    // Determine status
    if (result->error_count > 0) {
        file_result->status = FILE_STATUS_ERROR;
    } else if (result->warning_count > 0) {
        file_result->status = FILE_STATUS_WARNING;
    } else {
        file_result->status = FILE_STATUS_OK;
    }
    
    fast_check_result_destroy(result);
    return file_result;
}






 
