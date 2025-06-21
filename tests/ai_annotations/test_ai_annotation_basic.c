#include "ai_annotation_grammar.h"
#include "../framework/test_framework.h"
#include <stdio.h>
#include <string.h>

// Test AI annotation recognition
ASTHRA_TEST_DEFINE(test_ai_annotation_recognition, ASTHRA_TEST_SEVERITY_HIGH) {
    printf("Testing AI annotation recognition...\n");
    
    // Test valid AI annotations
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_confidence"), "Should recognize ai_confidence")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_hypothesis"), "Should recognize ai_hypothesis")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_review_needed"), "Should recognize ai_review_needed")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_todo"), "Should recognize ai_todo")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_optimize"), "Should recognize ai_optimize")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_test_coverage"), "Should recognize ai_test_coverage")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_security_review"), "Should recognize ai_security_review")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_pattern"), "Should recognize ai_pattern")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_complexity"), "Should recognize ai_complexity")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, is_ai_annotation("ai_refinement_step"), "Should recognize ai_refinement_step")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test invalid annotations
    if (!asthra_test_assert_bool_eq(context, is_ai_annotation("not_ai_annotation"), false, "Should not recognize non-AI annotation")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool_eq(context, is_ai_annotation("ai_unknown"), false, "Should not recognize unknown AI annotation")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool_eq(context, is_ai_annotation(NULL), false, "Should handle NULL annotation name")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool_eq(context, is_ai_annotation(""), false, "Should handle empty annotation name")) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("✅ AI annotation recognition tests passed\n");
    return ASTHRA_TEST_PASS;
}

// Test AI annotation type resolution
ASTHRA_TEST_DEFINE(test_ai_annotation_type_resolution, ASTHRA_TEST_SEVERITY_HIGH) {
    printf("Testing AI annotation type resolution...\n");
    
    if (!asthra_test_assert_int_eq(context, resolve_ai_annotation_type("ai_confidence"), AI_ANNOTATION_CONFIDENCE, 
                     "Should resolve ai_confidence type")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_int_eq(context, resolve_ai_annotation_type("ai_hypothesis"), AI_ANNOTATION_HYPOTHESIS, 
                     "Should resolve ai_hypothesis type")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_int_eq(context, resolve_ai_annotation_type("ai_todo"), AI_ANNOTATION_TODO, 
                     "Should resolve ai_todo type")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test invalid types
    if (!asthra_test_assert_int_eq(context, resolve_ai_annotation_type("invalid"), 0, "Should return 0 for invalid type")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_int_eq(context, resolve_ai_annotation_type(NULL), 0, "Should return 0 for NULL")) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("✅ AI annotation type resolution tests passed\n");
    return ASTHRA_TEST_PASS;
}

// Test context validation
ASTHRA_TEST_DEFINE(test_ai_annotation_context_validation, ASTHRA_TEST_SEVERITY_MEDIUM) {
    printf("Testing AI annotation context validation...\n");
    
    // Test function context
    if (!asthra_test_assert_bool(context, ai_annotation_valid_for_function(AI_ANNOTATION_CONFIDENCE), 
                "Confidence should be valid for functions")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, ai_annotation_valid_for_function(AI_ANNOTATION_HYPOTHESIS), 
                "Hypothesis should be valid for functions")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test statement context
    if (!asthra_test_assert_bool(context, ai_annotation_valid_for_statement(AI_ANNOTATION_CONFIDENCE), 
                "Confidence should be valid for statements")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test invalid statement contexts
    if (!asthra_test_assert_bool_eq(context, ai_annotation_valid_for_statement(AI_ANNOTATION_HYPOTHESIS), false,
                 "Hypothesis should not be valid for statements")) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("✅ AI annotation context validation tests passed\n");
    return ASTHRA_TEST_PASS;
}

// Test parameter validation
ASTHRA_TEST_DEFINE(test_ai_annotation_parameter_validation, ASTHRA_TEST_SEVERITY_MEDIUM) {
    printf("Testing AI annotation parameter validation...\n");
    
    // Create test parameters for confidence annotation
    AIAnnotationParam confidence_param = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"),
        .value = { .string_value = strdup("0.85") }
    };
    
    if (!asthra_test_assert_bool(context, validate_ai_confidence_annotation(&confidence_param, 1),
                "Should validate valid confidence parameter")) {
        free(confidence_param.name);
        free(confidence_param.value.string_value);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test invalid confidence (out of range)
    AIAnnotationParam invalid_confidence_param = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("confidence"), 
        .value = { .string_value = strdup("1.5") }
    };
    
    if (!asthra_test_assert_bool_eq(context, validate_ai_confidence_annotation(&invalid_confidence_param, 1), false,
                 "Should reject out-of-range confidence")) {
        free(confidence_param.name);
        free(confidence_param.value.string_value);
        free(invalid_confidence_param.name);
        free(invalid_confidence_param.value.string_value);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test hypothesis annotation
    AIAnnotationParam hypothesis_param = {
        .type = ANNOTATION_PARAM_STRING,
        .name = strdup("hypothesis"),
        .value = { .string_value = strdup("This function validates input") }
    };
    
    if (!asthra_test_assert_bool(context, validate_ai_hypothesis_annotation(&hypothesis_param, 1),
                "Should validate valid hypothesis parameter")) {
        free(confidence_param.name);
        free(confidence_param.value.string_value);
        free(invalid_confidence_param.name);
        free(invalid_confidence_param.value.string_value);
        free(hypothesis_param.name);
        free(hypothesis_param.value.string_value);
        return ASTHRA_TEST_FAIL;
    }
    
    // Cleanup
    free(confidence_param.name);
    free(confidence_param.value.string_value);
    free(invalid_confidence_param.name);
    free(invalid_confidence_param.value.string_value);
    free(hypothesis_param.name);
    free(hypothesis_param.value.string_value);
    
    printf("✅ AI annotation parameter validation tests passed\n");
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("Running AI Annotation Basic Tests...\n\n");
    
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        printf("Failed to create test statistics\n");
        return 1;
    }
    
    // Run tests
    AsthraTestFunction tests[] = {
        test_ai_annotation_recognition,
        test_ai_annotation_type_resolution,
        test_ai_annotation_context_validation,
        test_ai_annotation_parameter_validation
    };
    
    const AsthraTestMetadata metadata[] = {
        test_ai_annotation_recognition_metadata,
        test_ai_annotation_type_resolution_metadata,
        test_ai_annotation_context_validation_metadata,
        test_ai_annotation_parameter_validation_metadata
    };
    
    AsthraTestSuiteConfig config = asthra_test_suite_config_create("AI Annotation Basic Tests", 
                                                                  "Basic functionality tests for AI annotations");
    config.statistics = stats;
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, 4, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    if (result == ASTHRA_TEST_PASS) {
        printf("\n🎉 All AI annotation basic tests passed!\n");
        return 0;
    } else {
        printf("\n❌ Some AI annotation tests failed\n");
        return 1;
    }
} 
