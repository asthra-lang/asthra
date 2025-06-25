#ifndef ASTHRA_AI_ANNOTATION_PROCESSOR_H
#define ASTHRA_AI_ANNOTATION_PROCESSOR_H

#include "ai_annotation_semantic.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct AIAnnotationProcessor AIAnnotationProcessor;
typedef struct AsthraCompilerContext AsthraCompilerContext;

// Compile-time annotation processing results
typedef struct {
    char *file_path;
    float overall_confidence; // Average confidence across all annotations
    size_t ai_generated_functions;
    size_t human_reviewed_functions;
    size_t total_functions;

    // Improvement suggestions
    char **improvement_suggestions;
    size_t suggestion_count;

    // Security concerns
    char **security_concerns;
    size_t concern_count;

    // TODO items
    char **todo_items;
    size_t todo_count;

    // Quality metrics
    float test_coverage_score;
    float security_score;
    float maintainability_score;

    // Metadata for AI learning
    AIAnnotationMetadata *learning_data;
} AICompilationReport;

// Processor configuration
typedef struct {
    float confidence_threshold; // Minimum confidence for acceptance (default: 0.5)
    bool emit_learning_data;    // Whether to generate learning data (default: true)
    bool validate_security;     // Whether to validate security annotations (default: true)
    bool strict_validation;     // Whether to use strict validation (default: false)
    char *output_format;        // Output format: "json", "text", "xml" (default: "json")
} AIProcessorConfig;

// Processor interface
AIAnnotationProcessor *ai_annotation_processor_create(void);
void ai_annotation_processor_destroy(AIAnnotationProcessor *processor);

// Configuration functions
void ai_annotation_processor_set_config(AIAnnotationProcessor *processor,
                                        const AIProcessorConfig *config);
AIProcessorConfig *ai_annotation_processor_get_config(AIAnnotationProcessor *processor);
void ai_annotation_processor_set_confidence_threshold(AIAnnotationProcessor *processor,
                                                      float threshold);
void ai_annotation_processor_set_learning_data(AIAnnotationProcessor *processor, bool enable);

// Processing functions
AICompilationReport *ai_annotation_process_program(AIAnnotationProcessor *processor,
                                                   ASTNode *program);
bool ai_annotation_emit_metadata(AIAnnotationProcessor *processor, const char *output_path,
                                 AICompilationReport *report);
char *ai_annotation_generate_learning_report(AIAnnotationProcessor *processor,
                                             AICompilationReport *report);

// Report generation
char *ai_annotation_generate_json_report(AICompilationReport *report);
char *ai_annotation_generate_text_report(AICompilationReport *report);
void ai_annotation_print_summary(AICompilationReport *report);

// Memory management
void free_ai_compilation_report(AICompilationReport *report);
void free_ai_processor_config(AIProcessorConfig *config);

// Utility functions
bool ai_annotation_validate_report(AICompilationReport *report);
float ai_annotation_calculate_quality_score(AICompilationReport *report);
size_t ai_annotation_count_by_type(AICompilationReport *report, AIAnnotationType type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_PROCESSOR_H
