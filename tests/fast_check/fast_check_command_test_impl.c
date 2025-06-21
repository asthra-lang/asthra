#include "fast_check_command.h"
#include "fast_check_engine.h"
#include "parser_string_interface.h"
#include "semantic_analyzer_core.h"
#include "semantic_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

// =============================================================================
// FAST CHECK COMMAND IMPLEMENTATION
// =============================================================================

FastCheckCommandConfig* fast_check_command_config_create(void) {
    FastCheckCommandConfig *config = calloc(1, sizeof(FastCheckCommandConfig));
    if (!config) return NULL;
    
    config->watch_mode = false;
    config->syntax_only = false;
    config->output_format = strdup("human");
    config->max_errors = 100;
    config->enable_cache = true;
    config->verbose = false;
    
    return config;
}

void fast_check_command_config_destroy(FastCheckCommandConfig *config) {
    if (!config) return;
    
    free(config->output_format);
    free(config);
}

// =============================================================================
// FILE CHECK FUNCTIONS
// =============================================================================

static double get_time_ms_precise(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static FileCheckResult* create_file_check_result(const char* file_path) {
    FileCheckResult* result = calloc(1, sizeof(FileCheckResult));
    if (!result) return NULL;
    
    result->file_path = strdup(file_path);
    result->status = FILE_STATUS_OK;
    result->error_count = 0;
    result->warning_count = 0;
    result->check_time_ms = 0.0;
    result->was_cached = false;
    result->error_message = NULL;
    
    return result;
}

static void destroy_file_check_result(FileCheckResult* result) {
    if (!result) return;
    
    free(result->file_path);
    free(result->error_message);
    free(result);
}

static bool file_exists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static char* read_file_content(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc((size_t)length + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(content, 1, (size_t)length, file);
    content[read_size] = '\0';
    fclose(file);
    
    return content;
}

/**
 * Perform fast check on a single file
 */
static FileCheckResult* fast_check_single_file(const char* file_path, FastCheckCommandConfig* config) {
    if (!file_path || !config) return NULL;
    
    FileCheckResult* result = create_file_check_result(file_path);
    if (!result) return NULL;
    
    double start_time = get_time_ms_precise();
    
    // Check if file exists
    if (!file_exists(file_path)) {
        result->status = FILE_STATUS_ERROR;
        result->error_count = 1;
        result->error_message = strdup("File not found");
        goto cleanup;
    }
    
    // Read file content
    char* content = read_file_content(file_path);
    if (!content) {
        result->status = FILE_STATUS_ERROR;
        result->error_count = 1;
        result->error_message = strdup("Failed to read file");
        goto cleanup;
    }
    
    // Quick syntax validation
    ParseResult parse_result = parse_string(content);
    if (!parse_result.success || !parse_result.ast) {
        result->status = FILE_STATUS_ERROR;
        result->error_count = 1;
        result->error_message = strdup("Syntax error");
        free(content);
        goto cleanup;
    }
    
    // If not syntax-only mode, perform semantic analysis
    if (!config->syntax_only) {
        SemanticAnalyzer* analyzer = semantic_analyzer_create();
        if (analyzer) {
            bool semantic_success = semantic_analyze_program(analyzer, parse_result.ast);
            if (!semantic_success) {
                result->status = FILE_STATUS_WARNING;
                result->warning_count = 1;
                if (!result->error_message) {
                    result->error_message = strdup("Semantic warnings");
                }
            }
            semantic_analyzer_destroy(analyzer);
        }
    }
    
    // Cleanup parser result
    cleanup_parse_result(&parse_result);
    free(content);
    
cleanup:
    result->check_time_ms = get_time_ms_precise() - start_time;
    return result;
}

/**
 * Run fast check command on multiple files
 */
FastCheckCommandResult* fast_check_run_command_with_engine(FastCheckEngine *engine, char** files, size_t file_count, FastCheckConfig* config) {
    if (!engine || !files || file_count == 0 || !config) return NULL;
    
    FastCheckCommandResult* cmd_result = calloc(1, sizeof(FastCheckCommandResult));
    if (!cmd_result) return NULL;
    
    cmd_result->file_results = calloc(file_count, sizeof(FileCheckResult*));
    if (!cmd_result->file_results) {
        free(cmd_result);
        return NULL;
    }
    
    double total_start_time = get_time_ms_precise();
    
    // Process each file
    for (size_t i = 0; i < file_count; i++) {
        FileCheckResult* file_result = fast_check_single_file(files[i], config);
        if (file_result) {
            cmd_result->file_results[cmd_result->result_count++] = file_result;
            
            if (file_result->status == FILE_STATUS_OK || file_result->status == FILE_STATUS_WARNING) {
                cmd_result->files_passed++;
            } else {
                cmd_result->files_failed++;
            }
        }
    }
    
    cmd_result->files_checked = cmd_result->result_count;
    cmd_result->total_time_ms = get_time_ms_precise() - total_start_time;
    cmd_result->avg_time_per_file = cmd_result->files_checked > 0 ? 
        cmd_result->total_time_ms / cmd_result->files_checked : 0.0;
    
    return cmd_result;
}

// fast_check_command_result_destroy is implemented in main library

/**
 * Print command results in human-readable format
 */
void fast_check_print_results(FastCheckCommandResult* result, bool verbose) {
    if (!result) return;
    
    printf("\n=== Fast Check Results ===\n");
    printf("Files checked: %zu\n", result->files_checked);
    printf("Files passed: %zu\n", result->files_passed);
    printf("Files failed: %zu\n", result->files_failed);
    printf("Total time: %.2f ms\n", result->total_time_ms);
    printf("Average time per file: %.2f ms\n", result->avg_time_per_file);
    
    if (verbose) {
        printf("\nFile Details:\n");
        for (size_t i = 0; i < result->result_count; i++) {
            FileCheckResult* file_result = result->file_results[i];
            printf("  %s: ", file_result->file_path);
            
            switch (file_result->status) {
                case FILE_STATUS_OK:
                    printf("✅ OK");
                    break;
                case FILE_STATUS_WARNING:
                    printf("⚠️  WARNING (%d warnings)", file_result->warning_count);
                    break;
                case FILE_STATUS_ERROR:
                    printf("❌ ERROR (%d errors)", file_result->error_count);
                    break;
                case FILE_STATUS_SKIP:
                    printf("⏭️  SKIPPED");
                    break;
            }
            
            printf(" (%.2f ms)", file_result->check_time_ms);
            if (file_result->error_message) {
                printf(" - %s", file_result->error_message);
            }
            printf("\n");
        }
    }
    
    printf("========================\n\n");
}

/**
 * IDE integration support - quick syntax validation
 */
bool fast_check_syntax_only(const char* file_path) {
    if (!file_path) return false;
    
    FastCheckCommandConfig config = {
        .syntax_only = true,
        .watch_mode = false,
        .enable_cache = false,
        .verbose = false,
        .max_errors = 1,
        .output_format = NULL
    };
    
    FileCheckResult* result = fast_check_single_file(file_path, &config);
    if (!result) return false;
    
    bool success = (result->status == FILE_STATUS_OK);
    destroy_file_check_result(result);
    
    return success;
}

/**
 * Incremental compilation support
 */
bool fast_check_incremental_validate(char** changed_files, size_t file_count) {
    if (!changed_files || file_count == 0) return true;
    
    FastCheckConfig* config = fast_check_config_create();
    if (!config) return false;
    
    config->syntax_only = false;
    config->enable_cache = true;
    
    FastCheckEngine* engine = fast_check_engine_create();
    FastCheckCommandResult* result = fast_check_run_command_with_engine(engine, changed_files, file_count, config);
    
    bool all_passed = (result && result->files_failed == 0);
    
    fast_check_command_result_destroy(result);
    fast_check_engine_destroy(engine);
    fast_check_config_destroy(config);
    
    return all_passed;
}

// fast_check_run_command is implemented in main library with different signature 
