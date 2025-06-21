#ifndef ASTHRA_AI_ANNOTATION_SEMANTIC_H
#define ASTHRA_AI_ANNOTATION_SEMANTIC_H

#include "ai_annotation_grammar.h"
#include "../analysis/semantic_analyzer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Semantic analysis for AI annotations
typedef struct {
    AIAnnotation *annotations;     // Array of processed annotations
    size_t annotation_count;
    size_t capacity;
    
    // Statistics
    size_t confidence_annotations;
    size_t review_needed_count;
    size_t todo_count;
    size_t security_annotations;
    
    // Validation results
    size_t valid_annotations;
    size_t invalid_annotations;
    char **validation_errors;
    size_t error_count;
} AIAnnotationAnalysisResult;

// Semantic analysis functions
AIAnnotationAnalysisResult *ai_annotation_analyze_program(SemanticAnalyzer *analyzer, ASTNode *program);
bool ai_annotation_validate_placement(AIAnnotation *annotation, ASTNode *target);
bool ai_annotation_check_conflicts(AIAnnotation *annotation, AIAnnotation *existing[], size_t existing_count);
void ai_annotation_extract_metadata(AIAnnotation *annotation, AIAnnotationMetadata *metadata);

// Context validation
bool ai_annotation_valid_for_function(AIAnnotationType type);
bool ai_annotation_valid_for_statement(AIAnnotationType type);
bool ai_annotation_valid_for_expression(AIAnnotationType type);
bool ai_annotation_valid_for_declaration(AIAnnotationType type);

// Helper functions for semantic analysis (implemented in ai_annotation_semantic.c)

// Memory management
void free_ai_annotation_analysis_result(AIAnnotationAnalysisResult *result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_SEMANTIC_H 
