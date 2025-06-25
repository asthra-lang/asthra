#ifndef ASTHRA_AI_ANNOTATION_PROCESSOR_MEMORY_H
#define ASTHRA_AI_ANNOTATION_PROCESSOR_MEMORY_H

#include "ai_annotation_processor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Memory management functions
void free_ai_compilation_report(AICompilationReport *report);
void free_ai_processor_config(AIProcessorConfig *config);

// Utility functions
bool ai_annotation_validate_report(AICompilationReport *report);
float ai_annotation_calculate_quality_score(AICompilationReport *report);
size_t ai_annotation_count_by_type(AICompilationReport *report, AIAnnotationType type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_PROCESSOR_MEMORY_H
