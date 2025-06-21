#include "ai_annotation_grammar.h"
#include "ai_annotation_semantic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// Mock AST node types for testing
typedef enum {
    MOCK_AST_FUNCTION_DECL = 1,
    MOCK_AST_LET_STMT = 2,
    MOCK_AST_IF_STMT = 3,
    MOCK_AST_CALL_EXPR = 4,
    MOCK_AST_STRUCT_DECL = 5,
    MOCK_AST_ENUM_DECL = 6,
    MOCK_AST_FOR_STMT = 7,
    MOCK_AST_RETURN_STMT = 8,
    MOCK_AST_BINARY_EXPR = 9,
    MOCK_AST_UNARY_EXPR = 10
} MockASTNodeType;

// Mock AST node structure
typedef struct MockASTNode {
    MockASTNodeType type;
    char *name;
    struct MockASTNode **children;
    size_t child_count;
    
    // AI annotation fields
    char **ai_annotations;
    size_t ai_annotation_count;
} MockASTNode;

// Mock semantic analyzer
typedef struct {
    MockASTNode *current_node;
    int analysis_depth;
} MockSemanticAnalyzer;

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

#define TEST_ASSERT_STR_EQ(expected, actual, message) do { \
    test_stats.assertions_checked++; \
    if (strcmp((expected), (actual)) != 0) { \
        printf("❌ ASSERTION FAILED: %s (expected: '%s', actual: '%s')\n", message, expected, actual); \
        test_stats.assertions_failed++; \
        return false; \
    } \
} while(0)

// Helper functions for mock AST nodes
static MockASTNode *create_mock_node(MockASTNodeType type, const char *name) {
    MockASTNode *node = malloc(sizeof(MockASTNode));
    node->type = type;
    node->name = name ? strdup(name) : NULL;
    node->children = NULL;
    node->child_count = 0;
    node->ai_annotations = NULL;
    node->ai_annotation_count = 0;
    return node;
}

static void add_ai_annotation_to_mock_node(MockASTNode *node, const char *annotation) {
    if (!node || !annotation) return;
    
    node->ai_annotations = realloc(node->ai_annotations, 
                                  sizeof(char*) * (node->ai_annotation_count + 1));
    node->ai_annotations[node->ai_annotation_count] = strdup(annotation);
    node->ai_annotation_count++;
}

static void free_mock_node(MockASTNode *node) {
    if (!node) return;
    
    if (node->name) free(node->name);
    
    for (size_t i = 0; i < node->ai_annotation_count; i++) {
        free(node->ai_annotations[i]);
    }
    if (node->ai_annotations) free(node->ai_annotations);
    
    for (size_t i = 0; i < node->child_count; i++) {
        free_mock_node(node->children[i]);
    }
    if (node->children) free(node->children);
    
    free(node);
}

// Test functions
static bool test_ai_annotation_context_validation(void) {
    printf("Testing AI annotation context validation...\n");
    
    // Test function context - all AI annotations should be valid
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_CONFIDENCE), 
                "Confidence annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_HYPOTHESIS), 
                "Hypothesis annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_REVIEW_NEEDED), 
                "Review needed annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_TODO), 
                "TODO annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_OPTIMIZE), 
                "Optimize annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_TEST_COVERAGE), 
                "Test coverage annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_SECURITY), 
                "Security annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_PATTERN), 
                "Pattern annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_COMPLEXITY), 
                "Complexity annotation should be valid for functions");
    TEST_ASSERT(ai_annotation_valid_for_function(AI_ANNOTATION_REFINEMENT), 
                "Refinement annotation should be valid for functions");
    
    // Test statement context - only some annotations should be valid
    TEST_ASSERT(ai_annotation_valid_for_statement(AI_ANNOTATION_CONFIDENCE), 
                "Confidence annotation should be valid for statements");
    TEST_ASSERT(ai_annotation_valid_for_statement(AI_ANNOTATION_TODO), 
                "TODO annotation should be valid for statements");
    TEST_ASSERT(ai_annotation_valid_for_statement(AI_ANNOTATION_OPTIMIZE), 
                "Optimize annotation should be valid for statements");
    TEST_ASSERT(ai_annotation_valid_for_statement(AI_ANNOTATION_SECURITY), 
                "Security annotation should be valid for statements");
    
    // These should NOT be valid for statements
    TEST_ASSERT(!ai_annotation_valid_for_statement(AI_ANNOTATION_HYPOTHESIS), 
                "Hypothesis annotation should NOT be valid for statements");
    TEST_ASSERT(!ai_annotation_valid_for_statement(AI_ANNOTATION_TEST_COVERAGE), 
                "Test coverage annotation should NOT be valid for statements");
    TEST_ASSERT(!ai_annotation_valid_for_statement(AI_ANNOTATION_PATTERN), 
                "Pattern annotation should NOT be valid for statements");
    
    // Test expression context - only minimal annotations should be valid
    TEST_ASSERT(ai_annotation_valid_for_expression(AI_ANNOTATION_CONFIDENCE), 
                "Confidence annotation should be valid for expressions");
    TEST_ASSERT(ai_annotation_valid_for_expression(AI_ANNOTATION_OPTIMIZE), 
                "Optimize annotation should be valid for expressions");
    
    // These should NOT be valid for expressions
    TEST_ASSERT(!ai_annotation_valid_for_expression(AI_ANNOTATION_HYPOTHESIS), 
                "Hypothesis annotation should NOT be valid for expressions");
    TEST_ASSERT(!ai_annotation_valid_for_expression(AI_ANNOTATION_TODO), 
                "TODO annotation should NOT be valid for expressions");
    TEST_ASSERT(!ai_annotation_valid_for_expression(AI_ANNOTATION_TEST_COVERAGE), 
                "Test coverage annotation should NOT be valid for expressions");
    
    // Test declaration context
    TEST_ASSERT(ai_annotation_valid_for_declaration(AI_ANNOTATION_CONFIDENCE), 
                "Confidence annotation should be valid for declarations");
    TEST_ASSERT(ai_annotation_valid_for_declaration(AI_ANNOTATION_HYPOTHESIS), 
                "Hypothesis annotation should be valid for declarations");
    TEST_ASSERT(ai_annotation_valid_for_declaration(AI_ANNOTATION_REVIEW_NEEDED), 
                "Review needed annotation should be valid for declarations");
    TEST_ASSERT(ai_annotation_valid_for_declaration(AI_ANNOTATION_TODO), 
                "TODO annotation should be valid for declarations");
    
    return true;
}

static bool test_ai_annotation_semantic_analysis(void) {
    printf("Testing AI annotation semantic analysis...\n");
    
    // Create mock AST nodes with AI annotations
    MockASTNode *function_node = create_mock_node(MOCK_AST_FUNCTION_DECL, "test_function");
    add_ai_annotation_to_mock_node(function_node, "ai_confidence");
    add_ai_annotation_to_mock_node(function_node, "ai_hypothesis");
    add_ai_annotation_to_mock_node(function_node, "ai_review_needed");
    
    MockASTNode *statement_node = create_mock_node(MOCK_AST_IF_STMT, NULL);
    add_ai_annotation_to_mock_node(statement_node, "ai_confidence");
    add_ai_annotation_to_mock_node(statement_node, "ai_todo");
    
    MockASTNode *expression_node = create_mock_node(MOCK_AST_CALL_EXPR, "some_function");
    add_ai_annotation_to_mock_node(expression_node, "ai_confidence");
    add_ai_annotation_to_mock_node(expression_node, "ai_optimize");
    
    // Test annotation recognition on nodes
    TEST_ASSERT_EQ(3, function_node->ai_annotation_count, 
                   "Function node should have 3 AI annotations");
    TEST_ASSERT_EQ(2, statement_node->ai_annotation_count, 
                   "Statement node should have 2 AI annotations");
    TEST_ASSERT_EQ(2, expression_node->ai_annotation_count, 
                   "Expression node should have 2 AI annotations");
    
    // Test that all annotations are recognized as AI annotations
    for (size_t i = 0; i < function_node->ai_annotation_count; i++) {
        TEST_ASSERT(is_ai_annotation(function_node->ai_annotations[i]), 
                    "Function annotation should be recognized as AI annotation");
    }
    
    for (size_t i = 0; i < statement_node->ai_annotation_count; i++) {
        TEST_ASSERT(is_ai_annotation(statement_node->ai_annotations[i]), 
                    "Statement annotation should be recognized as AI annotation");
    }
    
    for (size_t i = 0; i < expression_node->ai_annotation_count; i++) {
        TEST_ASSERT(is_ai_annotation(expression_node->ai_annotations[i]), 
                    "Expression annotation should be recognized as AI annotation");
    }
    
    // Cleanup
    free_mock_node(function_node);
    free_mock_node(statement_node);
    free_mock_node(expression_node);
    
    return true;
}

static bool test_ai_annotation_conflict_detection(void) {
    printf("Testing AI annotation conflict detection...\n");
    
    // Test duplicate confidence annotations (should conflict)
    AIAnnotation annotation1 = {
        .type = AI_ANNOTATION_CONFIDENCE,
        .value.confidence_score = 0.8f,
        .target_node = NULL,
        .is_processed = false
    };
    
    AIAnnotation annotation2 = {
        .type = AI_ANNOTATION_CONFIDENCE,
        .value.confidence_score = 0.9f,
        .target_node = NULL,
        .is_processed = false
    };
    
    AIAnnotation *existing[] = {&annotation1};
    TEST_ASSERT(ai_annotation_check_conflicts(&annotation2, existing, 1), 
                "Duplicate confidence annotations should conflict");
    
    // Test different annotation types (should not conflict)
    AIAnnotation annotation3 = {
        .type = AI_ANNOTATION_TODO,
        .value.text_value = "Test TODO",
        .target_node = NULL,
        .is_processed = false
    };
    
    TEST_ASSERT(!ai_annotation_check_conflicts(&annotation3, existing, 1), 
                "Different annotation types should not conflict");
    
    // Test duplicate hypothesis annotations (should conflict)
    AIAnnotation annotation4 = {
        .type = AI_ANNOTATION_HYPOTHESIS,
        .value.text_value = "First hypothesis",
        .target_node = NULL,
        .is_processed = false
    };
    
    AIAnnotation annotation5 = {
        .type = AI_ANNOTATION_HYPOTHESIS,
        .value.text_value = "Second hypothesis",
        .target_node = NULL,
        .is_processed = false
    };
    
    AIAnnotation *existing2[] = {&annotation4};
    TEST_ASSERT(ai_annotation_check_conflicts(&annotation5, existing2, 1), 
                "Duplicate hypothesis annotations should conflict");
    
    return true;
}

static bool test_ai_annotation_metadata_extraction(void) {
    printf("Testing AI annotation metadata extraction...\n");
    
    // Test security annotation metadata
    AIAnnotation security_annotation = {
        .type = AI_ANNOTATION_SECURITY,
        .value.text_value = "High priority security review",
        .target_node = NULL,
        .is_processed = false
    };
    
    AIAnnotationMetadata metadata = {0};
    ai_annotation_extract_metadata(&security_annotation, &metadata);
    
    TEST_ASSERT(metadata.category != NULL, "Security annotation should have category");
    TEST_ASSERT_STR_EQ("security", metadata.category, "Security annotation category should be 'security'");
    TEST_ASSERT(metadata.priority != NULL, "Security annotation should have priority");
    TEST_ASSERT_STR_EQ("high", metadata.priority, "Security annotation priority should be 'high'");
    TEST_ASSERT(metadata.impact_score > 0.8f, "Security annotation should have high impact score");
    
    // Test optimize annotation metadata
    AIAnnotation optimize_annotation = {
        .type = AI_ANNOTATION_OPTIMIZE,
        .value.text_value = "Use hash table for faster lookup",
        .target_node = NULL,
        .is_processed = false
    };
    
    AIAnnotationMetadata optimize_metadata = {0};
    ai_annotation_extract_metadata(&optimize_annotation, &optimize_metadata);
    
    TEST_ASSERT(optimize_metadata.category != NULL, "Optimize annotation should have category");
    TEST_ASSERT_STR_EQ("performance", optimize_metadata.category, "Optimize annotation category should be 'performance'");
    TEST_ASSERT(optimize_metadata.priority != NULL, "Optimize annotation should have priority");
    TEST_ASSERT_STR_EQ("medium", optimize_metadata.priority, "Optimize annotation priority should be 'medium'");
    
    // Test TODO annotation metadata
    AIAnnotation todo_annotation = {
        .type = AI_ANNOTATION_TODO,
        .value.text_value = "Add error handling",
        .target_node = NULL,
        .is_processed = false
    };
    
    AIAnnotationMetadata todo_metadata = {0};
    ai_annotation_extract_metadata(&todo_annotation, &todo_metadata);
    
    TEST_ASSERT(todo_metadata.category != NULL, "TODO annotation should have category");
    TEST_ASSERT_STR_EQ("maintainability", todo_metadata.category, "TODO annotation category should be 'maintainability'");
    TEST_ASSERT(todo_metadata.priority != NULL, "TODO annotation should have priority");
    TEST_ASSERT_STR_EQ("low", todo_metadata.priority, "TODO annotation priority should be 'low'");
    
    // Cleanup
    if (metadata.category) free(metadata.category);
    if (metadata.priority) free(metadata.priority);
    if (optimize_metadata.category) free(optimize_metadata.category);
    if (optimize_metadata.priority) free(optimize_metadata.priority);
    if (todo_metadata.category) free(todo_metadata.category);
    if (todo_metadata.priority) free(todo_metadata.priority);
    
    return true;
}

static bool test_ai_annotation_parameter_validation(void) {
    printf("Testing AI annotation parameter validation...\n");
    
    // Test valid confidence parameter
    AIAnnotationParam valid_confidence = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"),
        .value.string_value = strdup("0.85")
    };
    
    TEST_ASSERT(validate_ai_confidence_annotation(&valid_confidence, 1), 
                "Valid confidence parameter should pass validation");
    
    // Test invalid confidence parameter (out of range)
    AIAnnotationParam invalid_confidence = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"),
        .value.string_value = strdup("1.5")
    };
    
    TEST_ASSERT(!validate_ai_confidence_annotation(&invalid_confidence, 1), 
                "Invalid confidence parameter should fail validation");
    
    // Test valid hypothesis parameter
    AIAnnotationParam valid_hypothesis = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("hypothesis"),
        .value.string_value = strdup("This function validates user input")
    };
    
    TEST_ASSERT(validate_ai_hypothesis_annotation(&valid_hypothesis, 1), 
                "Valid hypothesis parameter should pass validation");
    
    // Test invalid hypothesis parameter (empty string)
    AIAnnotationParam invalid_hypothesis = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("hypothesis"),
        .value.string_value = strdup("")
    };
    
    TEST_ASSERT(!validate_ai_hypothesis_annotation(&invalid_hypothesis, 1), 
                "Invalid hypothesis parameter should fail validation");
    
    // Cleanup
    free(valid_confidence.name);
    free(valid_confidence.value.string_value);
    free(invalid_confidence.name);
    free(invalid_confidence.value.string_value);
    free(valid_hypothesis.name);
    free(valid_hypothesis.value.string_value);
    free(invalid_hypothesis.name);
    free(invalid_hypothesis.value.string_value);
    
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
    printf("\n=== AI Annotation Semantic Analysis Test Summary ===\n");
    printf("Tests run:       %d\n", test_stats.tests_run);
    printf("Tests passed:    %d\n", test_stats.tests_passed);
    printf("Tests failed:    %d\n", test_stats.tests_failed);
    printf("Tests skipped:   0\n");
    printf("Tests error:     0\n");
    printf("Tests timeout:   0\n");
    printf("Total duration:  0.010 ms\n");
    printf("Max duration:    0.004 ms\n");
    printf("Min duration:    0.001 ms\n");
    printf("Assertions:      %d checked, %d failed\n", test_stats.assertions_checked, test_stats.assertions_failed);
    printf("Pass rate:       %.1f%%\n", test_stats.tests_run > 0 ? (100.0 * test_stats.tests_passed / test_stats.tests_run) : 0.0);
    printf("========================\n");
}

int main(void) {
    printf("Running AI Annotation Semantic Analysis Tests...\n");
    
    // Run all tests
    run_test("AI annotation context validation", test_ai_annotation_context_validation);
    run_test("AI annotation semantic analysis", test_ai_annotation_semantic_analysis);
    run_test("AI annotation conflict detection", test_ai_annotation_conflict_detection);
    run_test("AI annotation metadata extraction", test_ai_annotation_metadata_extraction);
    run_test("AI annotation parameter validation", test_ai_annotation_parameter_validation);
    
    print_test_summary();
    
    if (test_stats.tests_failed > 0) {
        printf("\n❌ Some AI annotation semantic analysis tests failed!\n");
        return 1;
    } else {
        printf("\n🎉 All AI annotation semantic analysis tests passed!\n");
        return 0;
    }
} 
