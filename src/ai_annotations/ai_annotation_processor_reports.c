#include "ai_annotation_processor_reports.h"
#include "ai_annotation_processor_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Report generation functions
char *ai_annotation_generate_text_report(AICompilationReport *report) {
    if (!report)
        return NULL;

    // Calculate buffer size needed
    size_t buffer_size = 1024; // Base size
    buffer_size += strlen(report->file_path ? report->file_path : "unknown") + 100;
    buffer_size += report->suggestion_count * 100;
    buffer_size += report->concern_count * 100;
    buffer_size += report->todo_count * 100;

    char *buffer = malloc(buffer_size);
    if (!buffer)
        return NULL;

    int offset = 0;

    // Header
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "AI Annotation Analysis Report\n");
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "=============================\n");
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "File: %s\n",
                       report->file_path ? report->file_path : "unknown");
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "Overall Confidence: %.2f\n", report->overall_confidence);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "AI Generated Functions: %zu\n", report->ai_generated_functions);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "Human Reviewed Functions: %zu\n", report->human_reviewed_functions);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "Total Functions: %zu\n\n", report->total_functions);

    // Improvement suggestions
    if (report->suggestion_count > 0) {
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                           "Improvement Suggestions:\n");
        for (size_t i = 0; i < report->suggestion_count; i++) {
            offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  - %s\n",
                               report->improvement_suggestions[i]);
        }
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "\n");
    }

    // Security concerns
    if (report->concern_count > 0) {
        offset +=
            snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "Security Concerns:\n");
        for (size_t i = 0; i < report->concern_count; i++) {
            offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  - %s\n",
                               report->security_concerns[i]);
        }
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "\n");
    }

    // TODO items
    if (report->todo_count > 0) {
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "TODO Items:\n");
        for (size_t i = 0; i < report->todo_count; i++) {
            offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  - %s\n",
                               report->todo_items[i]);
        }
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "\n");
    }

    // Quality scores
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "Quality Scores:\n");
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  Test Coverage: %.2f\n", report->test_coverage_score);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  Security: %.2f\n",
                       report->security_score);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  Maintainability: %.2f\n", report->maintainability_score);

    return buffer;
}

char *ai_annotation_generate_json_report(AICompilationReport *report) {
    if (!report)
        return NULL;

    // Calculate required buffer size
    size_t buffer_size = 2048; // Initial size
    char *buffer = malloc(buffer_size);
    if (!buffer)
        return NULL;

    int offset = 0;
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "{\n");
    offset +=
        snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                 "  \"file_path\": \"%s\",\n", report->file_path ? report->file_path : "unknown");
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  \"overall_confidence\": %.2f,\n", report->overall_confidence);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  \"ai_generated_functions\": %zu,\n", report->ai_generated_functions);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  \"human_reviewed_functions\": %zu,\n", report->human_reviewed_functions);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  \"total_functions\": %zu,\n", report->total_functions);

    // Improvement suggestions
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  \"improvement_suggestions\": [\n");
    if (report->suggestion_count > 0) {
        for (size_t i = 0; i < report->suggestion_count; i++) {
            offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                               "    \"%s\"%s\n", report->improvement_suggestions[i],
                               i < report->suggestion_count - 1 ? "," : "");
        }
    }
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  ],\n");

    // Security concerns
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  \"security_concerns\": [\n");
    if (report->concern_count > 0) {
        for (size_t i = 0; i < report->concern_count; i++) {
            offset +=
                snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "    \"%s\"%s\n",
                         report->security_concerns[i], i < report->concern_count - 1 ? "," : "");
        }
    }
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  ],\n");

    // TODO items
    offset +=
        snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  \"todo_items\": [\n");
    if (report->todo_count > 0) {
        for (size_t i = 0; i < report->todo_count; i++) {
            offset +=
                snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "    \"%s\"%s\n",
                         report->todo_items[i], i < report->todo_count - 1 ? "," : "");
        }
    }
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  ],\n");

    // Quality scores
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "  \"quality_scores\": {\n");
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "    \"test_coverage\": %.2f,\n", report->test_coverage_score);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "    \"security\": %.2f,\n", report->security_score);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "    \"maintainability\": %.2f\n", report->maintainability_score);
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  },\n");

    // Learning data
    offset +=
        snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  \"learning_data\": ");
    if (report->learning_data) {
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "{\n");
        offset += snprintf(
            buffer + (size_t)offset, buffer_size - (size_t)offset, "    \"category\": \"%s\",\n",
            report->learning_data->category ? report->learning_data->category : "unknown");
        offset += snprintf(
            buffer + (size_t)offset, buffer_size - (size_t)offset, "    \"priority\": \"%s\",\n",
            report->learning_data->priority ? report->learning_data->priority : "unknown");
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                           "    \"impact_score\": %.2f,\n", report->learning_data->impact_score);
        offset += snprintf(
            buffer + (size_t)offset, buffer_size - (size_t)offset, "    \"reasoning\": \"%s\"\n",
            report->learning_data->reasoning ? report->learning_data->reasoning : "");
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "  }\n");
    } else {
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "null\n");
    }

    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "}\n");

    return buffer;
}

void ai_annotation_print_summary(AICompilationReport *report) {
    if (!report)
        return;

    char *text_report = ai_annotation_generate_text_report(report);
    if (text_report) {
        printf("%s\n", text_report);
        free(text_report);
    }
}

// Additional processing functions for compatibility
bool ai_annotation_emit_metadata(AIAnnotationProcessor *processor, const char *output_path,
                                 AICompilationReport *report) {
    if (!processor || !output_path || !report)
        return false;

    // Generate JSON report
    char *json_report = ai_annotation_generate_json_report(report);
    if (!json_report)
        return false;

    // Write to file
    FILE *file = fopen(output_path, "w");
    if (!file) {
        free(json_report);
        return false;
    }

    fprintf(file, "%s", json_report);
    fclose(file);
    free(json_report);

    return true;
}

char *ai_annotation_generate_learning_report(AIAnnotationProcessor *processor,
                                             AICompilationReport *report) {
    if (!processor || !report)
        return NULL;

    // Generate a learning-focused report based on the compilation report
    size_t buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    if (!buffer)
        return NULL;

    int offset = 0;
    offset +=
        snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "AI Learning Report\n");
    offset +=
        snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "==================\n");
    offset +=
        snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                 "Overall Quality Score: %.2f\n", ai_annotation_calculate_quality_score(report));
    offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                       "Confidence Level: %.2f\n", report->overall_confidence);

    if (report->learning_data) {
        offset += snprintf(
            buffer + (size_t)offset, buffer_size - (size_t)offset, "Learning Category: %s\n",
            report->learning_data->category ? report->learning_data->category : "unknown");
        offset +=
            snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset, "Priority: %s\n",
                     report->learning_data->priority ? report->learning_data->priority : "unknown");
        offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                           "Impact Score: %.2f\n", report->learning_data->impact_score);
        if (report->learning_data->reasoning) {
            offset += snprintf(buffer + (size_t)offset, buffer_size - (size_t)offset,
                               "Reasoning: %s\n", report->learning_data->reasoning);
        }
    }

    return buffer;
}
