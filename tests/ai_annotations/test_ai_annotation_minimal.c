#include "ai_annotation_grammar.h"
#include "ai_annotation_semantic.h"
#include "../framework/test_framework.h"
#include <string.h>
#include <stdlib.h>

// Test AI annotation recognition
AsthraTestResult test_ai_annotation_recognition(AsthraTestContext *context) {
    // Test valid AI annotations
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_confidence"), "ai_confidence should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_hypothesis"), "ai_hypothesis should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_review_needed"), "ai_review_needed should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_todo"), "ai_todo should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_optimize"), "ai_optimize should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_test_coverage"), "ai_test_coverage should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_security_review"), "ai_security_review should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_pattern"), "ai_pattern should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_complexity"), "ai_complexity should be recognized");
    ASTHRA_TEST_ASSERT_TRUE(context, is_ai_annotation("ai_refinement_step"), "ai_refinement_step should be recognized");
    
    // Test invalid annotations
    ASTHRA_TEST_ASSERT_FALSE(context, is_ai_annotation("not_ai_annotation"), "not_ai_annotation should not be recognized");
    ASTHRA_TEST_ASSERT_FALSE(context, is_ai_annotation("ai_unknown"), "ai_unknown should not be recognized");
    ASTHRA_TEST_ASSERT_FALSE(context, is_ai_annotation(NULL), "NULL should not be recognized");
    ASTHRA_TEST_ASSERT_FALSE(context, is_ai_annotation(""), "empty string should not be recognized");
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ai_annotation_type_resolution(AsthraTestContext *context) {
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_confidence"), (int)AI_ANNOTATION_CONFIDENCE, "ai_confidence type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_hypothesis"), (int)AI_ANNOTATION_HYPOTHESIS, "ai_hypothesis type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_review_needed"), (int)AI_ANNOTATION_REVIEW_NEEDED, "ai_review_needed type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_todo"), (int)AI_ANNOTATION_TODO, "ai_todo type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_optimize"), (int)AI_ANNOTATION_OPTIMIZE, "ai_optimize type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_test_coverage"), (int)AI_ANNOTATION_TEST_COVERAGE, "ai_test_coverage type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_security_review"), (int)AI_ANNOTATION_SECURITY, "ai_security_review type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_pattern"), (int)AI_ANNOTATION_PATTERN, "ai_pattern type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_complexity"), (int)AI_ANNOTATION_COMPLEXITY, "ai_complexity type resolution");
    asthra_test_assert_int_eq(context, (int)resolve_ai_annotation_type("ai_refinement_step"), (int)AI_ANNOTATION_REFINEMENT, "ai_refinement_step type resolution");
    
    // Test invalid types
    asthra_test_assert_int_eq(context, resolve_ai_annotation_type("invalid"), 0, "invalid type should return 0");
    asthra_test_assert_int_eq(context, resolve_ai_annotation_type(NULL), 0, "NULL type should return 0");
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ai_annotation_context_validation(AsthraTestContext *context) {
    // Test function context
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_CONFIDENCE), "confidence valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_HYPOTHESIS), "hypothesis valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_REVIEW_NEEDED), "review_needed valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_TODO), "todo valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_OPTIMIZE), "optimize valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_TEST_COVERAGE), "test_coverage valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_SECURITY), "security valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_PATTERN), "pattern valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_COMPLEXITY), "complexity valid for function");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_function(AI_ANNOTATION_REFINEMENT), "refinement valid for function");
    
    // Test statement context
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_statement(AI_ANNOTATION_CONFIDENCE), "confidence valid for statement");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_statement(AI_ANNOTATION_TODO), "todo valid for statement");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_statement(AI_ANNOTATION_OPTIMIZE), "optimize valid for statement");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_statement(AI_ANNOTATION_SECURITY), "security valid for statement");
    
    // Test invalid statement contexts
    ASTHRA_TEST_ASSERT_FALSE(context, ai_annotation_valid_for_statement(AI_ANNOTATION_HYPOTHESIS), "hypothesis should not be valid for statement");
    ASTHRA_TEST_ASSERT_FALSE(context, ai_annotation_valid_for_statement(AI_ANNOTATION_TEST_COVERAGE), "test_coverage should not be valid for statement");
    
    // Test expression context
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_expression(AI_ANNOTATION_CONFIDENCE), "confidence valid for expression");
    ASTHRA_TEST_ASSERT_TRUE(context, ai_annotation_valid_for_expression(AI_ANNOTATION_OPTIMIZE), "optimize valid for expression");
    
    // Test invalid expression contexts
    ASTHRA_TEST_ASSERT_FALSE(context, ai_annotation_valid_for_expression(AI_ANNOTATION_HYPOTHESIS), "hypothesis should not be valid for expression");
    ASTHRA_TEST_ASSERT_FALSE(context, ai_annotation_valid_for_expression(AI_ANNOTATION_TODO), "todo should not be valid for expression");
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ai_annotation_parameter_validation(AsthraTestContext *context) {
    // Test confidence annotation validation
    AIAnnotationParam confidence_param = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"),
        .value.string_value = strdup("0.85")
    };
    ASTHRA_TEST_ASSERT_TRUE(context, validate_ai_confidence_annotation(&confidence_param, 1), "valid confidence annotation should pass");
    free(confidence_param.name);
    free(confidence_param.value.string_value);
    
    // Test invalid confidence (out of range)
    AIAnnotationParam invalid_confidence = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"),
        .value.string_value = strdup("1.5")
    };
    ASTHRA_TEST_ASSERT_FALSE(context, validate_ai_confidence_annotation(&invalid_confidence, 1), "out-of-range confidence should fail");
    free(invalid_confidence.name);
    free(invalid_confidence.value.string_value);
    
    // Test hypothesis annotation validation
    AIAnnotationParam hypothesis_param = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("hypothesis"),
        .value.string_value = strdup("This function handles user input validation")
    };
    ASTHRA_TEST_ASSERT_TRUE(context, validate_ai_hypothesis_annotation(&hypothesis_param, 1), "valid hypothesis annotation should pass");
    free(hypothesis_param.name);
    free(hypothesis_param.value.string_value);
    
    // Test empty hypothesis
    AIAnnotationParam empty_hypothesis = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("hypothesis"),
        .value.string_value = strdup("")
    };
    ASTHRA_TEST_ASSERT_FALSE(context, validate_ai_hypothesis_annotation(&empty_hypothesis, 1), "empty hypothesis should fail");
    free(empty_hypothesis.name);
    free(empty_hypothesis.value.string_value);
    
    // Test refinement annotation validation
    AIAnnotationParam refinement_param = {
        .type = ANNOTATION_PARAM_INT,
        .name = strdup("step"),
        .value.int_value = 3
    };
    ASTHRA_TEST_ASSERT_TRUE(context, validate_ai_refinement_annotation(&refinement_param, 1), "valid refinement annotation should pass");
    free(refinement_param.name);
    
    // Test invalid refinement (zero or negative)
    AIAnnotationParam invalid_refinement = {
        .type = ANNOTATION_PARAM_INT,
        .name = strdup("step"),
        .value.int_value = 0
    };
    ASTHRA_TEST_ASSERT_FALSE(context, validate_ai_refinement_annotation(&invalid_refinement, 1), "zero refinement step should fail");
    free(invalid_refinement.name);
    
    return ASTHRA_TEST_PASS;
}

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("AI Annotation Minimal Tests");
    
    asthra_test_suite_add_test(suite, "annotation_recognition", "Test AI annotation recognition", test_ai_annotation_recognition);
    asthra_test_suite_add_test(suite, "type_resolution", "Test AI annotation type resolution", test_ai_annotation_type_resolution);
    asthra_test_suite_add_test(suite, "context_validation", "Test AI annotation context validation", test_ai_annotation_context_validation);
    asthra_test_suite_add_test(suite, "parameter_validation", "Test AI annotation parameter validation", test_ai_annotation_parameter_validation);
    
    return asthra_test_suite_run_and_exit(suite);
} 
