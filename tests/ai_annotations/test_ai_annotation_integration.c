#include "ai_annotation_grammar.h"
#include "ai_annotation_semantic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// Mock AST node for testing
typedef struct MockASTNode {
    int type;  // Using int instead of ASTNodeType for simplicity
    char *name;
    struct MockASTNode **children;
    size_t child_count;
} MockASTNode;

// Mock AST node types (simplified)
#define MOCK_AST_FUNCTION_DECL 1
#define MOCK_AST_LET_STMT 2
#define MOCK_AST_IF_STMT 3
#define MOCK_AST_CALL_EXPR 4

MockASTNode *create_mock_function_node(const char *name) {
    MockASTNode *node = malloc(sizeof(MockASTNode));
    node->type = MOCK_AST_FUNCTION_DECL;
    node->name = strdup(name);
    node->children = NULL;
    node->child_count = 0;
    return node;
}

MockASTNode *create_mock_statement_node(int type) {
    MockASTNode *node = malloc(sizeof(MockASTNode));
    node->type = type;
    node->name = NULL;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

void free_mock_node(MockASTNode *node) {
    if (!node) return;
    
    if (node->name) free(node->name);
    if (node->children) {
        for (size_t i = 0; i < node->child_count; i++) {
            free_mock_node(node->children[i]);
        }
        free(node->children);
    }
    free(node);
}

void test_ai_annotation_context_validation_with_mock_nodes(void) {
    printf("Testing AI annotation context validation with mock AST nodes...\n");
    
    // Test function context
    MockASTNode *func_node = create_mock_function_node("test_function");
    
    // All AI annotations should be valid for functions
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_CONFIDENCE) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_HYPOTHESIS) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_REVIEW_NEEDED) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_TODO) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_OPTIMIZE) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_TEST_COVERAGE) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_SECURITY) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_PATTERN) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_COMPLEXITY) == true);
    assert(ai_annotation_valid_for_function(AI_ANNOTATION_REFINEMENT) == true);
    
    // Test statement context
    MockASTNode *stmt_node = create_mock_statement_node(MOCK_AST_IF_STMT);
    
    // Only some annotations should be valid for statements
    assert(ai_annotation_valid_for_statement(AI_ANNOTATION_CONFIDENCE) == true);
    assert(ai_annotation_valid_for_statement(AI_ANNOTATION_TODO) == true);
    assert(ai_annotation_valid_for_statement(AI_ANNOTATION_OPTIMIZE) == true);
    assert(ai_annotation_valid_for_statement(AI_ANNOTATION_SECURITY) == true);
    
    // These should not be valid for statements
    assert(ai_annotation_valid_for_statement(AI_ANNOTATION_HYPOTHESIS) == false);
    assert(ai_annotation_valid_for_statement(AI_ANNOTATION_TEST_COVERAGE) == false);
    
    // Test expression context
    MockASTNode *expr_node = create_mock_statement_node(MOCK_AST_CALL_EXPR);
    
    // Only some annotations should be valid for expressions
    assert(ai_annotation_valid_for_expression(AI_ANNOTATION_CONFIDENCE) == true);
    assert(ai_annotation_valid_for_expression(AI_ANNOTATION_OPTIMIZE) == true);
    
    // These should not be valid for expressions
    assert(ai_annotation_valid_for_expression(AI_ANNOTATION_HYPOTHESIS) == false);
    assert(ai_annotation_valid_for_expression(AI_ANNOTATION_TODO) == false);
    
    free_mock_node(func_node);
    free_mock_node(stmt_node);
    free_mock_node(expr_node);
    
    printf("✅ AI annotation context validation with mock nodes passed\n");
}

void test_ai_annotation_metadata_extraction(void) {
    printf("Testing AI annotation metadata extraction...\n");
    
    // Test confidence annotation
    AIAnnotationParam confidence_param = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"),
        .value.string_value = strdup("0.85")
    };
    
    // Validate confidence parameter
    assert(validate_ai_confidence_annotation(&confidence_param, 1) == true);
    
    // Test hypothesis annotation
    AIAnnotationParam hypothesis_param = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("hypothesis"),
        .value.string_value = strdup("This function validates user input")
    };
    
    // Validate hypothesis parameter
    assert(validate_ai_hypothesis_annotation(&hypothesis_param, 1) == true);
    
    // Test invalid confidence (out of range)
    AIAnnotationParam invalid_confidence = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"),
        .value.string_value = strdup("1.5")
    };
    
    assert(validate_ai_confidence_annotation(&invalid_confidence, 1) == false);
    
    // Test empty hypothesis
    AIAnnotationParam empty_hypothesis = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("hypothesis"),
        .value.string_value = strdup("")
    };
    
    assert(validate_ai_hypothesis_annotation(&empty_hypothesis, 1) == false);
    
    // Cleanup
    free(confidence_param.name);
    free(confidence_param.value.string_value);
    free(hypothesis_param.name);
    free(hypothesis_param.value.string_value);
    free(invalid_confidence.name);
    free(invalid_confidence.value.string_value);
    free(empty_hypothesis.name);
    free(empty_hypothesis.value.string_value);
    
    printf("✅ AI annotation metadata extraction tests passed\n");
}

void test_ai_annotation_type_mapping(void) {
    printf("Testing AI annotation type mapping...\n");
    
    // Test all supported AI annotation types
    const char *annotation_names[] = {
        "ai_confidence",
        "ai_hypothesis", 
        "ai_review_needed",
        "ai_todo",
        "ai_optimize",
        "ai_test_coverage",
        "ai_security_review",
        "ai_pattern",
        "ai_complexity",
        "ai_refinement_step"
    };
    
    AIAnnotationType expected_types[] = {
        AI_ANNOTATION_CONFIDENCE,
        AI_ANNOTATION_HYPOTHESIS,
        AI_ANNOTATION_REVIEW_NEEDED,
        AI_ANNOTATION_TODO,
        AI_ANNOTATION_OPTIMIZE,
        AI_ANNOTATION_TEST_COVERAGE,
        AI_ANNOTATION_SECURITY,
        AI_ANNOTATION_PATTERN,
        AI_ANNOTATION_COMPLEXITY,
        AI_ANNOTATION_REFINEMENT
    };
    
    size_t annotation_count = sizeof(annotation_names) / sizeof(annotation_names[0]);
    
    for (size_t i = 0; i < annotation_count; i++) {
        // Test recognition
        assert(is_ai_annotation(annotation_names[i]) == true);
        
        // Test type resolution
        AIAnnotationType resolved_type = resolve_ai_annotation_type(annotation_names[i]);
        assert(resolved_type == expected_types[i]);
    }
    
    // Test invalid annotations
    assert(is_ai_annotation("not_ai_annotation") == false);
    assert(is_ai_annotation("ai_unknown") == false);
    assert(is_ai_annotation(NULL) == false);
    assert(is_ai_annotation("") == false);
    
    assert(resolve_ai_annotation_type("invalid") == 0);
    assert(resolve_ai_annotation_type(NULL) == 0);
    
    printf("✅ AI annotation type mapping tests passed\n");
}

int main(void) {
    printf("Running AI Annotation Integration Tests...\n\n");
    
    test_ai_annotation_context_validation_with_mock_nodes();
    test_ai_annotation_metadata_extraction();
    test_ai_annotation_type_mapping();
    
    printf("\n🎉 All AI annotation integration tests passed!\n");
    return 0;
} 
