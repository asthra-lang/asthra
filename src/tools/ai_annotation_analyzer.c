#include "../ai_annotations/ai_annotation_processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple mock AST node for testing
typedef struct {
    int type;
    char *name;
} SimpleASTNode;

static void print_usage(const char *program_name) {
    printf("Usage: %s [options]\n", program_name);
    printf("Options:\n");
    printf("  --output-json <file>       Output JSON report to file\n");
    printf("  --learning-data            Include learning data in report\n");
    printf("  --confidence-threshold <value>  Set confidence threshold (0.0-1.0)\n");
    printf("  --help                     Show this help message\n");
    printf("  --demo                     Run demonstration with mock data\n");
}

static void print_annotation_summary(AICompilationReport *report) {
    printf("AI Annotation Analysis Report\n");
    printf("=============================\n");
    printf("File: %s\n", report->file_path ? report->file_path : "unknown");
    printf("Overall Confidence: %.2f\n", report->overall_confidence);
    printf("AI Generated Functions: %zu\n", report->ai_generated_functions);
    printf("Human Reviewed Functions: %zu\n", report->human_reviewed_functions);
    printf("Total Functions: %zu\n", report->total_functions);
    
    if (report->suggestion_count > 0) {
        printf("\nImprovement Suggestions:\n");
        for (size_t i = 0; i < report->suggestion_count; i++) {
            printf("  - %s\n", report->improvement_suggestions[i]);
        }
    }
    
    if (report->concern_count > 0) {
        printf("\nSecurity Concerns:\n");
        for (size_t i = 0; i < report->concern_count; i++) {
            printf("  - %s\n", report->security_concerns[i]);
        }
    }
    
    if (report->todo_count > 0) {
        printf("\nTODO Items:\n");
        for (size_t i = 0; i < report->todo_count; i++) {
            printf("  - %s\n", report->todo_items[i]);
        }
    }
    
    printf("\nQuality Scores:\n");
    printf("  Test Coverage: %.2f\n", report->test_coverage_score);
    printf("  Security: %.2f\n", report->security_score);
    printf("  Maintainability: %.2f\n", report->maintainability_score);
    
    if (report->learning_data) {
        printf("\nLearning Data:\n");
        printf("  Category: %s\n", report->learning_data->category ? report->learning_data->category : "unknown");
        printf("  Priority: %s\n", report->learning_data->priority ? report->learning_data->priority : "unknown");
        printf("  Impact Score: %.2f\n", report->learning_data->impact_score);
        if (report->learning_data->reasoning) {
            printf("  Reasoning: %s\n", report->learning_data->reasoning);
        }
    }
}

int main(int argc, char *argv[]) {
    const char *output_file = NULL;
    bool include_learning = false;
    float confidence_threshold = 0.5f;
    bool demo_mode = false;
    
    // Parse command line options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--output-json") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "--learning-data") == 0) {
            include_learning = true;
        } else if (strcmp(argv[i], "--confidence-threshold") == 0 && i + 1 < argc) {
            confidence_threshold = atof(argv[++i]);
            if (confidence_threshold < 0.0f || confidence_threshold > 1.0f) {
                fprintf(stderr, "Error: Confidence threshold must be between 0.0 and 1.0\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--demo") == 0) {
            demo_mode = true;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Create AI annotation processor
    AIAnnotationProcessor *processor = ai_annotation_processor_create();
    if (!processor) {
        fprintf(stderr, "Error: Failed to create annotation processor\n");
        return 1;
    }
    
    // Configure processor
    ai_annotation_processor_set_confidence_threshold(processor, confidence_threshold);
    ai_annotation_processor_set_learning_data(processor, include_learning);
    
    // Create a simple mock AST program for demo
    SimpleASTNode mock_program;
    mock_program.type = 1; // AST_PROGRAM
    mock_program.name = strdup("demo_program");
    
    // Process annotations (with mock data)
    AICompilationReport *report = ai_annotation_process_program(processor, (ASTNode*)&mock_program);
    if (!report) {
        fprintf(stderr, "Error: Failed to process annotations\n");
        ai_annotation_processor_destroy(processor);
        free(mock_program.name);
        return 1;
    }
    
    // For demo, add some mock data to make the report more interesting
    if (demo_mode) {
        // Override some fields for demo
        if (report->file_path) free(report->file_path);
        report->file_path = strdup("demo_source.asthra");
        report->overall_confidence = 0.75f;
        report->ai_generated_functions = 3;
        report->human_reviewed_functions = 2;
        report->total_functions = 5;
        
        // Add mock suggestions
        report->suggestion_count = 2;
        report->improvement_suggestions = malloc(sizeof(char*) * 2);
        report->improvement_suggestions[0] = strdup("Consider adding error handling for edge cases");
        report->improvement_suggestions[1] = strdup("Optimize the sorting algorithm performance");
        
        // Add mock TODO item
        report->todo_count = 1;
        report->todo_items = malloc(sizeof(char*) * 1);
        report->todo_items[0] = strdup("Add comprehensive unit tests");
        
        printf("Running AI Annotation Analyzer Demo\n");
        printf("====================================\n");
        printf("Confidence threshold: %.2f\n", confidence_threshold);
        printf("Learning data enabled: %s\n", include_learning ? "yes" : "no");
        printf("\n");
    }
    
    // Generate output
    if (output_file) {
        char *json_report = ai_annotation_generate_json_report(report);
        if (json_report) {
            FILE *output = fopen(output_file, "w");
            if (output) {
                fprintf(output, "%s", json_report);
                fclose(output);
                printf("JSON report written to: %s\n", output_file);
            } else {
                fprintf(stderr, "Error: Failed to write output file: %s\n", output_file);
            }
            free(json_report);
        } else {
            fprintf(stderr, "Error: Failed to generate JSON report\n");
        }
    } else {
        // Print summary to stdout
        print_annotation_summary(report);
    }
    
    // Cleanup
    free_ai_compilation_report(report);
    ai_annotation_processor_destroy(processor);
    free(mock_program.name);
    
    return 0;
} 
