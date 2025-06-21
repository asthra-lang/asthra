#ifndef ASTHRA_AI_ANNOTATION_PROCESSOR_REPORTS_H
#define ASTHRA_AI_ANNOTATION_PROCESSOR_REPORTS_H

#include "ai_annotation_processor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Report generation functions
char *ai_annotation_generate_text_report(AICompilationReport *report);
char *ai_annotation_generate_json_report(AICompilationReport *report);
void ai_annotation_print_summary(AICompilationReport *report);

// Additional processing functions (for compatibility)
bool ai_annotation_emit_metadata(AIAnnotationProcessor *processor, 
                                const char *output_path, 
                                AICompilationReport *report);
char *ai_annotation_generate_learning_report(AIAnnotationProcessor *processor, 
                                            AICompilationReport *report);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_PROCESSOR_REPORTS_H 
