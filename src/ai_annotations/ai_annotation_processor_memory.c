#include "ai_annotation_processor_memory.h"
#include <stdlib.h>

// Memory management functions
void free_ai_compilation_report(AICompilationReport *report) {
    if (!report)
        return;

    if (report->file_path)
        free(report->file_path);

    for (size_t i = 0; i < report->suggestion_count; i++) {
        if (report->improvement_suggestions[i]) {
            free(report->improvement_suggestions[i]);
        }
    }
    if (report->improvement_suggestions)
        free(report->improvement_suggestions);

    for (size_t i = 0; i < report->concern_count; i++) {
        if (report->security_concerns[i]) {
            free(report->security_concerns[i]);
        }
    }
    if (report->security_concerns)
        free(report->security_concerns);

    for (size_t i = 0; i < report->todo_count; i++) {
        if (report->todo_items[i]) {
            free(report->todo_items[i]);
        }
    }
    if (report->todo_items)
        free(report->todo_items);

    if (report->learning_data) {
        // Free AIAnnotationMetadata
        if (report->learning_data->category)
            free(report->learning_data->category);
        if (report->learning_data->priority)
            free(report->learning_data->priority);
        if (report->learning_data->reasoning)
            free(report->learning_data->reasoning);
        for (size_t i = 0; i < report->learning_data->tag_count; i++) {
            if (report->learning_data->tags[i])
                free(report->learning_data->tags[i]);
        }
        if (report->learning_data->tags)
            free(report->learning_data->tags);
        free(report->learning_data);
    }

    free(report);
}

void free_ai_processor_config(AIProcessorConfig *config) {
    if (!config)
        return;

    if (config->output_format)
        free(config->output_format);
    free(config);
}

// Utility functions
bool ai_annotation_validate_report(AICompilationReport *report) {
    if (!report)
        return false;

    // Basic validation checks
    if (report->overall_confidence < 0.0f || report->overall_confidence > 1.0f)
        return false;
    if (report->test_coverage_score < 0.0f || report->test_coverage_score > 1.0f)
        return false;
    if (report->security_score < 0.0f || report->security_score > 1.0f)
        return false;
    if (report->maintainability_score < 0.0f || report->maintainability_score > 1.0f)
        return false;

    return true;
}

float ai_annotation_calculate_quality_score(AICompilationReport *report) {
    if (!report)
        return 0.0f;

    // Weighted average of quality metrics
    float quality_score = (report->test_coverage_score * 0.3f) + (report->security_score * 0.4f) +
                          (report->maintainability_score * 0.3f);

    return quality_score;
}

size_t ai_annotation_count_by_type(AICompilationReport *report, AIAnnotationType type) {
    if (!report)
        return 0;

    // This would need access to the analysis result to count by type
    // For now, return counts based on what we have in the report
    switch (type) {
    case AI_ANNOTATION_TODO:
        return report->todo_count;
    case AI_ANNOTATION_SECURITY:
        return report->concern_count;
    case AI_ANNOTATION_OPTIMIZE:
        return report->suggestion_count; // Approximation
    default:
        return 0;
    }
}
