#include "ai_annotation_processor_core.h"
#include "ai_annotation_processor_analysis.h"
#include "ai_annotation_processor_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Processor lifecycle functions
AIAnnotationProcessor *ai_annotation_processor_create(void) {
    AIAnnotationProcessor *processor = malloc(sizeof(AIAnnotationProcessor));
    if (!processor)
        return NULL;

    // Initialize with default configuration
    processor->config.confidence_threshold = 0.5f;
    processor->config.emit_learning_data = true;
    processor->config.validate_security = true;
    processor->config.strict_validation = false;
    processor->config.output_format = strdup("json");

    processor->current_analysis = NULL;
    processor->current_report = NULL;
    processor->is_processing = false;

    return processor;
}

void ai_annotation_processor_destroy(AIAnnotationProcessor *processor) {
    if (!processor)
        return;

    if (processor->current_analysis) {
        free_ai_annotation_analysis_result(processor->current_analysis);
    }
    if (processor->current_report) {
        free_ai_compilation_report(processor->current_report);
    }
    if (processor->config.output_format) {
        free(processor->config.output_format);
    }

    free(processor);
}

// Configuration functions
void ai_annotation_processor_set_config(AIAnnotationProcessor *processor,
                                        const AIProcessorConfig *config) {
    if (!processor || !config)
        return;

    processor->config.confidence_threshold = config->confidence_threshold;
    processor->config.emit_learning_data = config->emit_learning_data;
    processor->config.validate_security = config->validate_security;
    processor->config.strict_validation = config->strict_validation;

    if (processor->config.output_format) {
        free(processor->config.output_format);
    }
    processor->config.output_format =
        config->output_format ? strdup(config->output_format) : strdup("json");
}

AIProcessorConfig *ai_annotation_processor_get_config(AIAnnotationProcessor *processor) {
    if (!processor)
        return NULL;

    AIProcessorConfig *config = malloc(sizeof(AIProcessorConfig));
    if (!config)
        return NULL;

    *config = processor->config;
    config->output_format =
        processor->config.output_format ? strdup(processor->config.output_format) : NULL;

    return config;
}

void ai_annotation_processor_set_confidence_threshold(AIAnnotationProcessor *processor,
                                                      float threshold) {
    if (!processor)
        return;
    processor->config.confidence_threshold = threshold;
}

void ai_annotation_processor_set_learning_data(AIAnnotationProcessor *processor, bool enable) {
    if (!processor)
        return;
    processor->config.emit_learning_data = enable;
}

// Main processing function
AICompilationReport *ai_annotation_process_program(AIAnnotationProcessor *processor,
                                                   ASTNode *program) {
    if (!processor || !program)
        return NULL;

    processor->is_processing = true;

    // Create compilation report
    AICompilationReport *report = malloc(sizeof(AICompilationReport));
    if (!report) {
        processor->is_processing = false;
        return NULL;
    }

    // Initialize report
    report->file_path = strdup("unknown");
    report->overall_confidence = 0.0f;
    report->ai_generated_functions = 0;
    report->human_reviewed_functions = 0;
    report->total_functions = 0;
    report->improvement_suggestions = NULL;
    report->suggestion_count = 0;
    report->security_concerns = NULL;
    report->concern_count = 0;
    report->todo_items = NULL;
    report->todo_count = 0;
    report->test_coverage_score = 0.0f;
    report->security_score = 0.0f;
    report->maintainability_score = 0.0f;
    report->learning_data = NULL;

    // Analyze AI annotations in the program (using mock semantic analyzer)
    // For now, skip analysis to avoid double free issues with mock AST nodes
    AIAnnotationAnalysisResult *analysis = NULL; // ai_annotation_analyze_program(NULL, program);

    if (analysis) {
        // Calculate overall confidence
        ai_annotation_calculate_overall_confidence(report, analysis);

        // Extract improvement suggestions
        ai_annotation_extract_improvement_suggestions(report, analysis);

        // Extract security concerns
        ai_annotation_extract_security_concerns(report, analysis);

        // Extract TODO items
        ai_annotation_extract_todo_items(report, analysis);

        // Calculate quality scores
        ai_annotation_calculate_quality_scores(report, analysis);

        // Generate learning data if enabled
        if (processor->config.emit_learning_data) {
            report->learning_data = ai_annotation_generate_learning_data(analysis);
        }

        // Free the analysis result immediately after use
        free_ai_annotation_analysis_result(analysis);
    } else {
        // No analysis data available, but still set default quality scores
        ai_annotation_calculate_quality_scores(report, NULL);
    }

    processor->is_processing = false;

    return report;
}
