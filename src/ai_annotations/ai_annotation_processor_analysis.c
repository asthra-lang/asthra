#include "ai_annotation_processor_analysis.h"
#include <stdlib.h>
#include <string.h>

// Analysis function implementations
void ai_annotation_calculate_overall_confidence(AICompilationReport *report, AIAnnotationAnalysisResult *analysis) {
    if (!report || !analysis || analysis->confidence_annotations == 0) {
        report->overall_confidence = 0.0f;
        return;
    }
    
    float total_confidence = 0.0f;
    size_t confidence_count = 0;
    
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        if (annotation->type == AI_ANNOTATION_CONFIDENCE) {
            total_confidence += annotation->value.confidence_score;
            confidence_count++;
        }
    }
    
    report->overall_confidence = confidence_count > 0 ? 
                                total_confidence / (float)confidence_count : 0.0f;
}

void ai_annotation_extract_improvement_suggestions(AICompilationReport *report, AIAnnotationAnalysisResult *analysis) {
    if (!report || !analysis) return;
    
    // Count optimization and TODO annotations
    size_t suggestion_count = 0;
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        if (annotation->type == AI_ANNOTATION_OPTIMIZE || 
            annotation->type == AI_ANNOTATION_TODO) {
            suggestion_count++;
        }
    }
    
    if (suggestion_count == 0) return;
    
    report->improvement_suggestions = malloc(sizeof(char*) * suggestion_count);
    report->suggestion_count = 0;
    
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        if (annotation->type == AI_ANNOTATION_OPTIMIZE || 
            annotation->type == AI_ANNOTATION_TODO) {
            if (annotation->value.text_value) {
                report->improvement_suggestions[report->suggestion_count] = 
                    strdup(annotation->value.text_value);
                report->suggestion_count++;
            }
        }
    }
}

void ai_annotation_extract_security_concerns(AICompilationReport *report, AIAnnotationAnalysisResult *analysis) {
    if (!report || !analysis) return;
    
    // Count security annotations
    size_t concern_count = 0;
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        if (annotation->type == AI_ANNOTATION_SECURITY) {
            concern_count++;
        }
    }
    
    if (concern_count == 0) return;
    
    report->security_concerns = malloc(sizeof(char*) * concern_count);
    report->concern_count = 0;
    
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        if (annotation->type == AI_ANNOTATION_SECURITY) {
            if (annotation->value.text_value) {
                report->security_concerns[report->concern_count] = 
                    strdup(annotation->value.text_value);
                report->concern_count++;
            }
        }
    }
}

void ai_annotation_extract_todo_items(AICompilationReport *report, AIAnnotationAnalysisResult *analysis) {
    if (!report || !analysis) return;
    
    // Count TODO annotations
    size_t todo_count = 0;
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        if (annotation->type == AI_ANNOTATION_TODO) {
            todo_count++;
        }
    }
    
    if (todo_count == 0) return;
    
    report->todo_items = malloc(sizeof(char*) * todo_count);
    report->todo_count = 0;
    
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        if (annotation->type == AI_ANNOTATION_TODO) {
            if (annotation->value.text_value) {
                report->todo_items[report->todo_count] = 
                    strdup(annotation->value.text_value);
                report->todo_count++;
            }
        }
    }
}

void ai_annotation_calculate_quality_scores(AICompilationReport *report, AIAnnotationAnalysisResult *analysis) {
    if (!report) return;
    
    // Set default scores first
    report->test_coverage_score = 0.5f;
    report->security_score = 0.5f;
    report->maintainability_score = 0.5f;
    
    // If no analysis data, keep defaults
    if (!analysis) return;
    
    // Calculate test coverage score based on test coverage annotations
    float test_coverage_total = 0.0f;
    size_t test_coverage_count = 0;
    
    // Calculate security score based on security annotations and confidence
    float security_total = 0.0f;
    size_t security_count = 0;
    
    // Calculate maintainability score based on TODO and optimization annotations
    float maintainability_total = 0.0f;
    size_t maintainability_count = 0;
    
    for (size_t i = 0; i < analysis->annotation_count; i++) {
        AIAnnotation *annotation = &analysis->annotations[i];
        
        switch (annotation->type) {
            case AI_ANNOTATION_TEST_COVERAGE:
                test_coverage_total += 0.8f; // Assume good coverage when annotated
                test_coverage_count++;
                break;
            case AI_ANNOTATION_SECURITY:
                security_total += 0.9f; // High security score for security annotations
                security_count++;
                break;
            case AI_ANNOTATION_TODO:
                maintainability_total += 0.6f; // Medium score for TODOs
                maintainability_count++;
                break;
            case AI_ANNOTATION_OPTIMIZE:
                maintainability_total += 0.7f; // Good score for optimization notes
                maintainability_count++;
                break;
            case AI_ANNOTATION_CONFIDENCE: {
                // Use confidence score to boost all metrics
                float confidence = annotation->value.confidence_score;
                test_coverage_total += confidence * 0.1f;
                security_total += confidence * 0.1f;
                maintainability_total += confidence * 0.1f;
                break;
            }
            default:
                break;
        }
    }
    
    // Calculate final scores (only override defaults if we have data)
    if (test_coverage_count > 0) {
        report->test_coverage_score = (float)test_coverage_total / (float)test_coverage_count;
    }
    if (security_count > 0) {
        report->security_score = (float)security_total / (float)security_count;
    }
    if (maintainability_count > 0) {
        report->maintainability_score = (float)maintainability_total / (float)maintainability_count;
    }
    
    // Ensure scores are within valid range
    if (report->test_coverage_score > 1.0f) report->test_coverage_score = 1.0f;
    if (report->security_score > 1.0f) report->security_score = 1.0f;
    if (report->maintainability_score > 1.0f) report->maintainability_score = 1.0f;
}

AIAnnotationMetadata *ai_annotation_generate_learning_data(AIAnnotationAnalysisResult *analysis) {
    if (!analysis) return NULL;
    
    AIAnnotationMetadata *data = malloc(sizeof(AIAnnotationMetadata));
    if (!data) return NULL;
    
    // Initialize learning data with basic metadata
    data->category = strdup("learning");
    data->priority = strdup("medium");
    data->tags = NULL;
    data->tag_count = 0;
    data->reasoning = NULL;
    data->impact_score = 0.5f;
    
    // Generate reasoning based on analysis results
    if (analysis->valid_annotations > analysis->invalid_annotations) {
        data->reasoning = strdup("High annotation validity rate indicates good AI annotation patterns");
        data->impact_score = 0.8f;
        free(data->priority);  // Free the previous allocation
        data->priority = strdup("low");
    } else if (analysis->invalid_annotations > 0) {
        data->reasoning = strdup("Some invalid annotation placements detected - review needed");
        data->impact_score = 0.3f;
        free(data->priority);  // Free the previous allocation
        data->priority = strdup("high");
    }
    
    // Add tags based on analysis
    if (analysis->confidence_annotations > 0) {
        data->tags = malloc(sizeof(char*) * 2);
        data->tags[0] = strdup("confidence-tracking");
        data->tags[1] = strdup("ai-generated");
        data->tag_count = 2;
    }
    
    return data;
} 
