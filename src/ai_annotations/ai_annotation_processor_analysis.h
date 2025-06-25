#ifndef ASTHRA_AI_ANNOTATION_PROCESSOR_ANALYSIS_H
#define ASTHRA_AI_ANNOTATION_PROCESSOR_ANALYSIS_H

#include "ai_annotation_processor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Analysis functions
void ai_annotation_calculate_overall_confidence(AICompilationReport *report,
                                                AIAnnotationAnalysisResult *analysis);
void ai_annotation_extract_improvement_suggestions(AICompilationReport *report,
                                                   AIAnnotationAnalysisResult *analysis);
void ai_annotation_extract_security_concerns(AICompilationReport *report,
                                             AIAnnotationAnalysisResult *analysis);
void ai_annotation_extract_todo_items(AICompilationReport *report,
                                      AIAnnotationAnalysisResult *analysis);
void ai_annotation_calculate_quality_scores(AICompilationReport *report,
                                            AIAnnotationAnalysisResult *analysis);
AIAnnotationMetadata *ai_annotation_generate_learning_data(AIAnnotationAnalysisResult *analysis);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_PROCESSOR_ANALYSIS_H
