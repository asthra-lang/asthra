#ifndef PIPELINE_UTILS_H
#define PIPELINE_UTILS_H

#include "pipeline_orchestrator.h"
#include <stdbool.h>
#include <stddef.h>

// Helper and utility functions
double pipeline_get_current_time_ms(void);
const char *pipeline_get_phase_name(AsthraCompilerPhase phase);
void pipeline_clear_state(PipelineOrchestrator *orchestrator);
bool pipeline_is_ready_for_execution(PipelineOrchestrator *orchestrator);
void pipeline_orchestrator_get_statistics(PipelineOrchestrator *orchestrator, size_t *total_files,
                                          size_t *total_lines, size_t *total_errors,
                                          double *total_time_ms);
const PipelinePhaseResult *
pipeline_orchestrator_get_phase_results(PipelineOrchestrator *orchestrator, const char *filename);

// Internal helper functions
bool create_intermediate_directory(const char *output_path);
char *read_source_file(const char *filepath, size_t *out_size);
void initialize_phase_result(PipelinePhaseResult *result, AsthraCompilerPhase phase);
void finalize_phase_result(PipelinePhaseResult *result, bool success, const char *error_msg);

#endif // PIPELINE_UTILS_H