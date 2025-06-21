#ifndef ASTHRA_AI_ANNOTATION_PROCESSOR_CORE_H
#define ASTHRA_AI_ANNOTATION_PROCESSOR_CORE_H

#include "ai_annotation_processor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Processor lifecycle functions
AIAnnotationProcessor *ai_annotation_processor_create(void);
void ai_annotation_processor_destroy(AIAnnotationProcessor *processor);

// Configuration functions
void ai_annotation_processor_set_config(AIAnnotationProcessor *processor, const AIProcessorConfig *config);
AIProcessorConfig *ai_annotation_processor_get_config(AIAnnotationProcessor *processor);
void ai_annotation_processor_set_confidence_threshold(AIAnnotationProcessor *processor, float threshold);
void ai_annotation_processor_set_learning_data(AIAnnotationProcessor *processor, bool enable);

// Main processing function
AICompilationReport *ai_annotation_process_program(AIAnnotationProcessor *processor, ASTNode *program);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_PROCESSOR_CORE_H 
