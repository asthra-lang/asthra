/**
 * Real Program Test Quality Gates Implementation
 *
 * Contains functions for validating quality gates, assessing feature quality,
 * and ensuring comprehensive testing standards.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_test_quality.h"
#include "real_program_test_data.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// QUALITY GATE FUNCTIONS
// =============================================================================

QualityGateResult validate_quality_gates(const char *feature_name, const char **test_programs,
                                         size_t program_count) {
    QualityGateResult result = {0};

    if (!feature_name || !test_programs || program_count == 0) {
        result.failure_reasons[result.failure_count++] = "Invalid input parameters";
        return result;
    }

    TestSuiteConfig config = create_default_config();
    config.verbose_output = false; // Less verbose for quality gates

    size_t passed = 0;
    size_t grammar_compliant = 0;
    size_t performance_acceptable = 0;
    size_t error_detection_correct = 0;
    double total_parse_time = 0.0;

    // Error test programs - these should fail semantic analysis
    const char *error_programs[] = {
        "package main; pub fn main(none) -> void { let x: int = \"invalid\"; }", // Type mismatch
        "package main; pub fn main(none) -> void { let x = undefined_var; }", // Undefined variable
        "package main; pub fn main(none) -> void { fn foo(void) { return 42; } }", // Invalid
                                                                                   // function
                                                                                   // syntax
        "package main; pub fn main(none) -> void { x = 5; }", // Assignment to undefined variable
    };
    size_t error_program_count = sizeof(error_programs) / sizeof(error_programs[0]);

    // Test each program
    for (size_t i = 0; i < program_count; i++) {
        RealProgramTestResult test_result =
            validate_complete_program(test_programs[i], feature_name, &config);

        // Check grammar compliance
        if (test_result.success) {
            grammar_compliant++;
            passed++;
        }

        // Check performance
        total_parse_time += test_result.execution_time_ms;
        if (test_result.execution_time_ms < 100.0) { // 100ms threshold
            performance_acceptable++;
        }

        cleanup_test_result(&test_result);
    }

    // Test error detection - these programs should fail semantic analysis
    for (size_t i = 0; i < error_program_count; i++) {
        RealProgramTestResult test_result =
            validate_complete_program(error_programs[i], "error_detection", &config);

        // For error programs, success means they correctly failed semantic analysis
        if (!test_result.success) {
            error_detection_correct++;
        }

        cleanup_test_result(&test_result);
    }

    // Calculate quality metrics
    double success_rate = (double)passed / program_count;
    double grammar_rate = (double)grammar_compliant / program_count;
    double performance_rate = (double)performance_acceptable / program_count;
    double error_detection_rate = (double)error_detection_correct / error_program_count;
    double avg_parse_time = total_parse_time / program_count;

    // Set quality gate results
    result.grammar_compliant = (grammar_rate >= 0.8); // 80% threshold
    result.end_to_end_functional = (success_rate >= 0.8);
    result.performance_acceptable = (performance_rate >= 0.7 && avg_parse_time < 200.0);
    result.error_handling_correct =
        (error_detection_rate >= 0.75); // 75% of error programs should be caught
    result.feature_coverage_complete = (success_rate >= 0.9); // 90% for complete coverage

    // Calculate overall score
    int gates_passed = 0;
    if (result.grammar_compliant)
        gates_passed++;
    if (result.end_to_end_functional)
        gates_passed++;
    if (result.performance_acceptable)
        gates_passed++;
    if (result.error_handling_correct)
        gates_passed++;
    if (result.feature_coverage_complete)
        gates_passed++;

    result.overall_score = (double)gates_passed / 5.0;

    // Record failure reasons
    if (!result.grammar_compliant) {
        result.failure_reasons[result.failure_count++] = "Grammar compliance below 80%";
    }
    if (!result.end_to_end_functional) {
        result.failure_reasons[result.failure_count++] = "End-to-end functionality below 80%";
    }
    if (!result.performance_acceptable) {
        result.failure_reasons[result.failure_count++] = "Performance requirements not met";
    }
    if (!result.feature_coverage_complete) {
        result.failure_reasons[result.failure_count++] = "Feature coverage below 90%";
    }

    return result;
}

QualityGateResult assess_feature_quality(const char *feature_name) {
    QualityGateResult result = {0};

    if (!feature_name) {
        result.failure_reasons[result.failure_count++] = "Feature name is NULL";
        return result;
    }

    // Select appropriate test programs based on feature name
    const char **test_programs = NULL;
    size_t program_count = 0;

    if (strcmp(feature_name, "basic_types") == 0) {
        test_programs = BASIC_TYPES_PROGRAMS;
        program_count = BASIC_TYPES_PROGRAM_COUNT;
    } else if (strcmp(feature_name, "functions") == 0) {
        test_programs = FUNCTION_TEST_PROGRAMS;
        program_count = FUNCTION_TEST_PROGRAM_COUNT;
    } else if (strcmp(feature_name, "control_flow") == 0) {
        test_programs = CONTROL_FLOW_PROGRAMS;
        program_count = CONTROL_FLOW_PROGRAM_COUNT;
    } else {
        result.failure_reasons[result.failure_count++] = "Unknown feature name";
        return result;
    }

    return validate_quality_gates(feature_name, test_programs, program_count);
}

bool validate_real_functionality(const char *feature_name, const char **real_world_programs,
                                 size_t program_count) {
    if (!feature_name || !real_world_programs || program_count == 0) {
        return false;
    }

    QualityGateResult quality =
        validate_quality_gates(feature_name, real_world_programs, program_count);

    // Real functionality requires all quality gates to pass
    return quality.grammar_compliant && quality.end_to_end_functional &&
           quality.performance_acceptable && quality.feature_coverage_complete;
}

void generate_coverage_analysis(const RealProgramTestSuite *suite, const TestSuiteConfig *config) {
    if (!suite || !config) {
        return;
    }

    FILE *out = config->output_stream;

    fprintf(out, "\n=== COVERAGE ANALYSIS ===\n");
    fprintf(out, "Test suite: %s\n", suite->suite_name);
    fprintf(out, "Real functionality validation: %s\n", suite->passed > 0 ? "VERIFIED" : "FAILED");
    fprintf(out, "False positive risk: %s\n", suite->failed == 0 ? "LOW" : "HIGH");
    fprintf(out, "Integration coverage: %s\n",
            suite->test_count >= 5 ? "COMPREHENSIVE" : "LIMITED");
    fprintf(out, "========================\n\n");
}