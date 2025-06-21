#ifndef PIPELINE_PHASES_H
#define PIPELINE_PHASES_H

#include "pipeline_orchestrator.h"
#include <stdbool.h>

// Phase execution functions
bool pipeline_execute_lexer_phase(PipelineOrchestrator *orchestrator,
                                 SourceFileContext *source_context);
bool pipeline_execute_parser_phase(PipelineOrchestrator *orchestrator,
                                  SourceFileContext *source_context);
bool pipeline_execute_semantic_phase(PipelineOrchestrator *orchestrator,
                                    SourceFileContext *source_context);
bool pipeline_execute_codegen_phase(PipelineOrchestrator *orchestrator,
                                   SourceFileContext *source_context);
bool pipeline_execute_linking_phase(PipelineOrchestrator *orchestrator,
                                   const char *output_executable);

#endif // PIPELINE_PHASES_H