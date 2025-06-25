#ifndef ASTHRA_AI_ANNOTATION_PROCESSOR_TYPES_H
#define ASTHRA_AI_ANNOTATION_PROCESSOR_TYPES_H

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

// Internal processor structure
struct AIAnnotationProcessor {
    AIProcessorConfig config;
    AIAnnotationAnalysisResult *current_analysis;
    AICompilationReport *current_report;
    bool is_processing;
};

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_ANNOTATION_PROCESSOR_TYPES_H
