#ifndef PIPELINE_CORE_H
#define PIPELINE_CORE_H

#include "pipeline_orchestrator.h"
#include <stdbool.h>
#include <stddef.h>

// Core orchestrator lifecycle and API functions
PipelineOrchestrator *pipeline_orchestrator_create(AsthraCompilerContext *context);
void pipeline_orchestrator_destroy(PipelineOrchestrator *orchestrator);

// Main compilation API functions
bool pipeline_orchestrator_compile_file(PipelineOrchestrator *orchestrator,
                                       const char *input_file,
                                       const char *output_file);
bool pipeline_orchestrator_compile_files(PipelineOrchestrator *orchestrator,
                                        const char **input_files,
                                        size_t file_count,
                                        const char *output_file);

#endif // PIPELINE_CORE_H