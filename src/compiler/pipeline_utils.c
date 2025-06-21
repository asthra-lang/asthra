/**
 * Asthra Programming Language Compiler
 * Pipeline Orchestrator - Utility Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Helper and utility functions for pipeline orchestration
 */

#include "pipeline_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Platform-specific includes for timing
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

// =============================================================================
// TIMING AND MEASUREMENT FUNCTIONS
// =============================================================================

/**
 * Get current timestamp in milliseconds
 */
double pipeline_get_current_time_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)(counter.QuadPart * 1000.0) / frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double)tv.tv_sec * 1000.0) + ((double)tv.tv_usec / 1000.0);
#endif
}

// =============================================================================
// FILE AND DIRECTORY MANAGEMENT
// =============================================================================

/**
 * Create intermediate directory for build artifacts
 */
bool create_intermediate_directory(const char *output_path) {
    if (!output_path) return false;
    
    // Extract directory from output path
    char *dir_path = strdup(output_path);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        
        // Create directory if it doesn't exist
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            if (mkdir(dir_path, 0755) != 0) {
                free(dir_path);
                return false;
            }
        }
    }
    
    free(dir_path);
    return true;
}

/**
 * Read source file into memory
 */
char *read_source_file(const char *filepath, size_t *out_size) {
    FILE *file = fopen(filepath, "r");
    if (!file) return NULL;
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }
    
    // Allocate buffer
    char *source = malloc((size_t)file_size + 1);
    if (!source) {
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(source, 1, (size_t)file_size, file);
    source[bytes_read] = '\0';
    fclose(file);
    
    if (out_size) *out_size = bytes_read;
    return source;
}

// =============================================================================
// PHASE RESULT MANAGEMENT
// =============================================================================

/**
 * Initialize phase result structure
 */
void initialize_phase_result(PipelinePhaseResult *result, AsthraCompilerPhase phase) {
    memset(result, 0, sizeof(PipelinePhaseResult));
    result->phase = phase;
    result->success = false;
    result->start_time = pipeline_get_current_time_ms();
}

/**
 * Finalize phase result structure
 */
void finalize_phase_result(PipelinePhaseResult *result, bool success, const char *error_msg) {
    result->end_time = pipeline_get_current_time_ms();
    result->execution_time_ms = result->end_time - result->start_time;
    result->success = success;
    
    if (error_msg) {
        result->error_message = strdup(error_msg);
    }
}

// =============================================================================
// PIPELINE STATE MANAGEMENT
// =============================================================================

/**
 * Get phase name as string
 */
const char *pipeline_get_phase_name(AsthraCompilerPhase phase) {
    switch (phase) {
        case ASTHRA_PHASE_LEXING:
            return "Lexing";
        case ASTHRA_PHASE_PARSING:
            return "Parsing";
        case ASTHRA_PHASE_SEMANTIC_ANALYSIS:
            return "Semantic Analysis";
        case ASTHRA_PHASE_OPTIMIZATION:
            return "Optimization";
        case ASTHRA_PHASE_CODE_GENERATION:
            return "Code Generation";
        case ASTHRA_PHASE_LINKING:
            return "Linking";
        default:
            return "Unknown";
    }
}

/**
 * Clear pipeline state for next compilation
 */
void pipeline_clear_state(PipelineOrchestrator *orchestrator) {
    if (!orchestrator) return;
    
    orchestrator->pipeline_active = false;
    orchestrator->error_occurred = false;
    orchestrator->current_phase = ASTHRA_PHASE_LEXING;
    orchestrator->source_file_count = 0;
    orchestrator->total_start_time = 0.0;
    orchestrator->total_end_time = 0.0;
    
    // Clear phase times
    memset(orchestrator->phase_times, 0, sizeof(orchestrator->phase_times));
    
    // Clear statistics
    memset(&orchestrator->statistics, 0, sizeof(orchestrator->statistics));
    
    // Free source file contexts
    SourceFileContext *current = orchestrator->source_files;
    while (current) {
        SourceFileContext *next = current->next;
        free(current->source_code);
        free(current);
        current = next;
    }
    orchestrator->source_files = NULL;
    
    // Clear object files
    if (orchestrator->object_files) {
        for (size_t i = 0; i < orchestrator->object_file_count; i++) {
            free(orchestrator->object_files[i]);
        }
        free(orchestrator->object_files);
        orchestrator->object_files = NULL;
        orchestrator->object_file_count = 0;
    }
}

/**
 * Check if pipeline is in valid state for execution
 */
bool pipeline_is_ready_for_execution(PipelineOrchestrator *orchestrator) {
    if (!orchestrator) return false;
    if (!orchestrator->compiler_context) return false;
    if (orchestrator->pipeline_active) return false;
    return true;
}

// =============================================================================
// STATISTICS AND REPORTING
// =============================================================================

void pipeline_orchestrator_get_statistics(PipelineOrchestrator *orchestrator,
                                         size_t *total_files,
                                         size_t *total_lines,
                                         size_t *total_errors,
                                         double *total_time_ms) {
    if (!orchestrator) return;
    
    if (total_files) *total_files = orchestrator->source_file_count;
    if (total_lines) *total_lines = orchestrator->statistics.total_lines_processed;
    if (total_errors) *total_errors = orchestrator->statistics.total_errors;
    if (total_time_ms) *total_time_ms = orchestrator->total_end_time - orchestrator->total_start_time;
}

const PipelinePhaseResult *pipeline_orchestrator_get_phase_results(
    PipelineOrchestrator *orchestrator, 
    const char *filename) {
    if (!orchestrator || !filename) return NULL;
    
    // Find the source file context
    SourceFileContext *current = orchestrator->source_files;
    while (current) {
        if (strcmp(current->input_path, filename) == 0) {
            return current->phases;
        }
        current = current->next;
    }
    
    return NULL;
}