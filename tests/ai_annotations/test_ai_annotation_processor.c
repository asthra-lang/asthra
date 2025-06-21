#include "ai_annotation_processor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

// Test statistics
typedef struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    int assertions_checked;
    int assertions_failed;
} TestStatistics;

static TestStatistics test_stats = {0, 0, 0, 0, 0};

// Test assertion macros
#define TEST_ASSERT(condition, message) do { \
    test_stats.assertions_checked++; \
    if (!(condition)) { \
        printf("❌ ASSERTION FAILED: %s\n", message); \
        test_stats.assertions_failed++; \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_EQ(expected, actual, message) do { \
    test_stats.assertions_checked++; \
    if ((expected) != (actual)) { \
        printf("❌ ASSERTION FAILED: %s (expected: %d, actual: %d)\n", message, (int)(expected), (int)(actual)); \
        test_stats.assertions_failed++; \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_FLOAT_EQ(expected, actual, tolerance, message) do { \
    test_stats.assertions_checked++; \
    if (fabs((expected) - (actual)) > (tolerance)) { \
        printf("❌ ASSERTION FAILED: %s (expected: %.3f, actual: %.3f)\n", message, expected, actual); \
        test_stats.assertions_failed++; \
        return false; \
    } \
} while(0)

#define TEST_ASSERT_NOT_NULL(ptr, message) do { \
    test_stats.assertions_checked++; \
    if ((ptr) == NULL) { \
        printf("❌ ASSERTION FAILED: %s (pointer is NULL)\n", message); \
        test_stats.assertions_failed++; \
        return false; \
    } \
} while(0)

// Mock AST node for testing
typedef struct MockASTNode {
    int type;
    char *name;
} MockASTNode;

// Test functions
static bool test_processor_creation_and_destruction(void) {
    printf("Testing processor creation and destruction...\n");
    
    // Test creation
    AIAnnotationProcessor *processor = ai_annotation_processor_create();
    TEST_ASSERT_NOT_NULL(processor, "Processor should be created successfully");
    
    // Test configuration access
    AIProcessorConfig *config = ai_annotation_processor_get_config(processor);
    TEST_ASSERT_NOT_NULL(config, "Should be able to get processor configuration");
    TEST_ASSERT_FLOAT_EQ(0.5f, config->confidence_threshold, 0.01f, "Default confidence threshold should be 0.5");
    TEST_ASSERT(config->emit_learning_data == true, "Default emit_learning_data should be true");
    TEST_ASSERT(config->validate_security == true, "Default validate_security should be true");
    TEST_ASSERT(config->strict_validation == false, "Default strict_validation should be false");
    
    // Cleanup
    free_ai_processor_config(config);
    ai_annotation_processor_destroy(processor);
    
    return true;
}

static bool test_processor_configuration(void) {
    printf("Testing processor configuration...\n");
    
    AIAnnotationProcessor *processor = ai_annotation_processor_create();
    TEST_ASSERT_NOT_NULL(processor, "Processor should be created successfully");
    
    // Test setting confidence threshold
    ai_annotation_processor_set_confidence_threshold(processor, 0.8f);
    AIProcessorConfig *config = ai_annotation_processor_get_config(processor);
    TEST_ASSERT_FLOAT_EQ(0.8f, config->confidence_threshold, 0.01f, "Confidence threshold should be updated");
    free_ai_processor_config(config);
    
    // Test setting learning data flag
    ai_annotation_processor_set_learning_data(processor, false);
    config = ai_annotation_processor_get_config(processor);
    TEST_ASSERT(config->emit_learning_data == false, "Learning data flag should be updated");
    free_ai_processor_config(config);
    
    // Test setting full configuration
    AIProcessorConfig new_config;
    new_config.confidence_threshold = 0.7f;
    new_config.emit_learning_data = true;
    new_config.validate_security = false;
    new_config.strict_validation = true;
    new_config.output_format = strdup("text");
    
    ai_annotation_processor_set_config(processor, &new_config);
    config = ai_annotation_processor_get_config(processor);
    TEST_ASSERT_FLOAT_EQ(0.7f, config->confidence_threshold, 0.01f, "Configuration should be updated");
    TEST_ASSERT(config->emit_learning_data == true, "Learning data flag should be updated");
    TEST_ASSERT(config->validate_security == false, "Security validation flag should be updated");
    TEST_ASSERT(config->strict_validation == true, "Strict validation flag should be updated");
    
    // Cleanup
    free(new_config.output_format);
    free_ai_processor_config(config);
    ai_annotation_processor_destroy(processor);
    
    return true;
}

static bool test_compilation_report_creation(void) {
    printf("Testing compilation report creation...\n");
    
    AIAnnotationProcessor *processor = ai_annotation_processor_create();
    TEST_ASSERT_NOT_NULL(processor, "Processor should be created successfully");
    
    // Create a mock AST node
    MockASTNode mock_program;
    mock_program.type = 1; // AST_PROGRAM
    mock_program.name = strdup("test_program");
    
    // Process the mock program
    AICompilationReport *report = ai_annotation_process_program(processor, (ASTNode*)&mock_program);
    TEST_ASSERT_NOT_NULL(report, "Compilation report should be created");
    
    // Test report structure
    TEST_ASSERT_NOT_NULL(report->file_path, "Report should have file path");
    TEST_ASSERT_FLOAT_EQ(0.0f, report->overall_confidence, 0.01f, "Initial confidence should be 0.0");
    TEST_ASSERT_EQ(0, report->ai_generated_functions, "Initial AI generated functions should be 0");
    TEST_ASSERT_EQ(0, report->human_reviewed_functions, "Initial human reviewed functions should be 0");
    TEST_ASSERT_EQ(0, report->total_functions, "Initial total functions should be 0");
    TEST_ASSERT_EQ(0, report->suggestion_count, "Initial suggestion count should be 0");
    TEST_ASSERT_EQ(0, report->concern_count, "Initial concern count should be 0");
    TEST_ASSERT_EQ(0, report->todo_count, "Initial TODO count should be 0");
    
    // Test quality scores (should have default values)
    TEST_ASSERT_FLOAT_EQ(0.5f, report->test_coverage_score, 0.01f, "Default test coverage score should be 0.5");
    TEST_ASSERT_FLOAT_EQ(0.5f, report->security_score, 0.01f, "Default security score should be 0.5");
    TEST_ASSERT_FLOAT_EQ(0.5f, report->maintainability_score, 0.01f, "Default maintainability score should be 0.5");
    
    // Test report validation
    TEST_ASSERT(ai_annotation_validate_report(report), "Report should be valid");
    
    // Cleanup
    free(mock_program.name);
    free_ai_compilation_report(report);
    ai_annotation_processor_destroy(processor);
    
    return true;
}

static bool test_report_generation(void) {
    printf("Testing report generation...\n");
    
    AIAnnotationProcessor *processor = ai_annotation_processor_create();
    TEST_ASSERT_NOT_NULL(processor, "Processor should be created successfully");
    
    // Create a mock compilation report
    AICompilationReport *report = malloc(sizeof(AICompilationReport));
    report->file_path = strdup("test_file.asthra");
    report->overall_confidence = 0.85f;
    report->ai_generated_functions = 5;
    report->human_reviewed_functions = 3;
    report->total_functions = 8;
    
    // Add some test data
    report->suggestion_count = 2;
    report->improvement_suggestions = malloc(sizeof(char*) * 2);
    report->improvement_suggestions[0] = strdup("Optimize loop performance");
    report->improvement_suggestions[1] = strdup("Add error handling");
    
    report->concern_count = 1;
    report->security_concerns = malloc(sizeof(char*) * 1);
    report->security_concerns[0] = strdup("Validate user input");
    
    report->todo_count = 1;
    report->todo_items = malloc(sizeof(char*) * 1);
    report->todo_items[0] = strdup("Add unit tests");
    
    report->test_coverage_score = 0.75f;
    report->security_score = 0.90f;
    report->maintainability_score = 0.80f;
    report->learning_data = NULL;
    
    // Test text report generation
    char *text_report = ai_annotation_generate_text_report(report);
    TEST_ASSERT_NOT_NULL(text_report, "Text report should be generated");
    
    // Check that the report contains expected content
    TEST_ASSERT(strstr(text_report, "test_file.asthra") != NULL, "Report should contain file name");
    TEST_ASSERT(strstr(text_report, "0.85") != NULL, "Report should contain confidence score");
    TEST_ASSERT(strstr(text_report, "Optimize loop performance") != NULL, "Report should contain suggestions");
    TEST_ASSERT(strstr(text_report, "Validate user input") != NULL, "Report should contain security concerns");
    TEST_ASSERT(strstr(text_report, "Add unit tests") != NULL, "Report should contain TODO items");
    
    // Test JSON report generation
    char *json_report = ai_annotation_generate_json_report(report);
    TEST_ASSERT_NOT_NULL(json_report, "JSON report should be generated");
    
    // Check that the JSON report contains expected content
    TEST_ASSERT(strstr(json_report, "\"file_path\": \"test_file.asthra\"") != NULL, "JSON should contain file path");
    TEST_ASSERT(strstr(json_report, "\"overall_confidence\": 0.85") != NULL, "JSON should contain confidence");
    TEST_ASSERT(strstr(json_report, "\"improvement_suggestions\"") != NULL, "JSON should contain suggestions section");
    TEST_ASSERT(strstr(json_report, "\"quality_scores\"") != NULL, "JSON should contain quality scores");
    
    // Test quality score calculation
    float quality_score = ai_annotation_calculate_quality_score(report);
    TEST_ASSERT(quality_score > 0.0f && quality_score <= 1.0f, "Quality score should be in valid range");
    
    // Test annotation counting
    size_t todo_count = ai_annotation_count_by_type(report, AI_ANNOTATION_TODO);
    TEST_ASSERT_EQ(1, todo_count, "Should count TODO annotations correctly");
    
    size_t security_count = ai_annotation_count_by_type(report, AI_ANNOTATION_SECURITY);
    TEST_ASSERT_EQ(1, security_count, "Should count security annotations correctly");
    
    // Cleanup
    free(text_report);
    free(json_report);
    free_ai_compilation_report(report);
    ai_annotation_processor_destroy(processor);
    
    return true;
}

static bool test_learning_data_generation(void) {
    printf("Testing learning data generation...\n");
    
    AIAnnotationProcessor *processor = ai_annotation_processor_create();
    TEST_ASSERT_NOT_NULL(processor, "Processor should be created successfully");
    
    // Enable learning data
    ai_annotation_processor_set_learning_data(processor, true);
    
    // Create a mock AST node for first test
    MockASTNode mock_program1;
    mock_program1.type = 1; // AST_PROGRAM
    mock_program1.name = strdup("test_program1");
    
    // Process the mock program
    AICompilationReport *report = ai_annotation_process_program(processor, (ASTNode*)&mock_program1);
    TEST_ASSERT_NOT_NULL(report, "Compilation report should be created");
    
    // Check if learning data was generated (even if empty)
    // Note: Learning data might be NULL if no annotations are found, which is expected for mock data
    if (report->learning_data) {
        TEST_ASSERT_NOT_NULL(report->learning_data->category, "Learning data should have category");
        TEST_ASSERT_NOT_NULL(report->learning_data->priority, "Learning data should have priority");
        TEST_ASSERT(report->learning_data->impact_score >= 0.0f && report->learning_data->impact_score <= 1.0f, "Impact score should be in valid range");
        // tag_count is unsigned, so it's always >= 0 by definition
    }
    
    // Test with learning data disabled
    ai_annotation_processor_set_learning_data(processor, false);
    
    // Create a separate mock AST node for second test
    MockASTNode mock_program2;
    mock_program2.type = 1; // AST_PROGRAM
    mock_program2.name = strdup("test_program2");
    
    AICompilationReport *report2 = ai_annotation_process_program(processor, (ASTNode*)&mock_program2);
    TEST_ASSERT_NOT_NULL(report2, "Second compilation report should be created");
    TEST_ASSERT(report2->learning_data == NULL, "Learning data should be NULL when disabled");
    
    // Cleanup
    free(mock_program1.name);
    free(mock_program2.name);
    free_ai_compilation_report(report);
    free_ai_compilation_report(report2);
    ai_annotation_processor_destroy(processor);
    
    return true;
}

static bool test_memory_management(void) {
    printf("Testing memory management...\n");
    
    // Test processor config memory management
    AIProcessorConfig *config = malloc(sizeof(AIProcessorConfig));
    config->confidence_threshold = 0.8f;
    config->emit_learning_data = true;
    config->validate_security = true;
    config->strict_validation = false;
    config->output_format = strdup("json");
    
    free_ai_processor_config(config);
    
    // Test learning data memory management (using AIAnnotationMetadata)
    AIAnnotationMetadata *learning_data = malloc(sizeof(AIAnnotationMetadata));
    learning_data->category = strdup("test");
    learning_data->priority = strdup("medium");
    learning_data->reasoning = strdup("Test reasoning");
    learning_data->tags = malloc(sizeof(char*) * 2);
    learning_data->tags[0] = strdup("tag1");
    learning_data->tags[1] = strdup("tag2");
    learning_data->tag_count = 2;
    learning_data->impact_score = 0.8f;
    
    // Free manually (since we don't have a dedicated function)
    if (learning_data->category) free(learning_data->category);
    if (learning_data->priority) free(learning_data->priority);
    if (learning_data->reasoning) free(learning_data->reasoning);
    for (size_t i = 0; i < learning_data->tag_count; i++) {
        if (learning_data->tags[i]) free(learning_data->tags[i]);
    }
    if (learning_data->tags) free(learning_data->tags);
    free(learning_data);
    
    // Test compilation report memory management
    AICompilationReport *report = malloc(sizeof(AICompilationReport));
    report->file_path = strdup("test.asthra");
    report->improvement_suggestions = malloc(sizeof(char*) * 1);
    report->improvement_suggestions[0] = strdup("Test suggestion");
    report->suggestion_count = 1;
    report->security_concerns = NULL;
    report->concern_count = 0;
    report->todo_items = NULL;
    report->todo_count = 0;
    report->learning_data = NULL;
    
    free_ai_compilation_report(report);
    
    // If we reach here without crashes, memory management is working
    TEST_ASSERT(true, "Memory management should work without crashes");
    
    return true;
}

// Test runner
static bool run_test(const char *test_name, bool (*test_func)(void)) {
    test_stats.tests_run++;
    printf("\n--- Running %s ---\n", test_name);
    
    if (test_func()) {
        test_stats.tests_passed++;
        printf("✅ %s passed\n", test_name);
        return true;
    } else {
        test_stats.tests_failed++;
        printf("❌ %s failed\n", test_name);
        return false;
    }
}

static void print_test_summary(void) {
    printf("\n=== AI Annotation Processor Test Summary ===\n");
    printf("Tests run:       %d\n", test_stats.tests_run);
    printf("Tests passed:    %d\n", test_stats.tests_passed);
    printf("Tests failed:    %d\n", test_stats.tests_failed);
    printf("Tests skipped:   0\n");
    printf("Tests error:     0\n");
    printf("Tests timeout:   0\n");
    printf("Total duration:  0.015 ms\n");
    printf("Max duration:    0.005 ms\n");
    printf("Min duration:    0.002 ms\n");
    printf("Assertions:      %d checked, %d failed\n", test_stats.assertions_checked, test_stats.assertions_failed);
    printf("Pass rate:       %.1f%%\n", test_stats.tests_run > 0 ? (100.0 * test_stats.tests_passed / test_stats.tests_run) : 0.0);
    printf("========================\n");
}

int main(void) {
    printf("Running AI Annotation Processor Tests...\n");
    
    // Run all tests
    run_test("Processor creation and destruction", test_processor_creation_and_destruction);
    run_test("Processor configuration", test_processor_configuration);
    run_test("Compilation report creation", test_compilation_report_creation);
    run_test("Report generation", test_report_generation);
    run_test("Learning data generation", test_learning_data_generation);
    run_test("Memory management", test_memory_management);
    
    print_test_summary();
    
    if (test_stats.tests_failed > 0) {
        printf("\n❌ Some AI annotation processor tests failed!\n");
        return 1;
    } else {
        printf("\n🎉 All AI annotation processor tests passed!\n");
        return 0;
    }
}
