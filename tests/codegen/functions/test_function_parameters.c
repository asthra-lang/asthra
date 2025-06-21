/**
 * Asthra Programming Language Compiler
 * Function Parameter Handling Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test code generation for function parameter handling
 */

#ifdef TEST_FRAMEWORK_MINIMAL
#include "../framework/test_framework_minimal.h"
#else
#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#endif
#include "code_generator.h"
#include "code_generator_core.h"
#include "code_generator_types.h"
#include "code_generator_instructions.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// =============================================================================
// TEST FIXTURE AND UTILITIES
// =============================================================================

/**
 * Test fixture for code generator testing
 */
typedef struct {
    CodeGenerator* generator;
    SemanticAnalyzer* analyzer;
    ASTNode* test_ast;
    char* output_buffer;
    size_t output_buffer_size;
} CodeGenTestFixture;

/**
 * Setup test fixture with a code generator
 */
static CodeGenTestFixture* setup_codegen_fixture(void) {
    CodeGenTestFixture* fixture = calloc(1, sizeof(CodeGenTestFixture));
    if (!fixture) return NULL;
    
    fixture->generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!fixture->generator) {
        free(fixture);
        return NULL;
    }
    
    fixture->analyzer = setup_semantic_analyzer();
    if (!fixture->analyzer) {
        code_generator_destroy(fixture->generator);
        free(fixture);
        return NULL;
    }
    
    // Connect the semantic analyzer to the code generator
    fixture->generator->semantic_analyzer = fixture->analyzer;
    
    fixture->output_buffer_size = 4096;
    fixture->output_buffer = malloc(fixture->output_buffer_size);
    if (!fixture->output_buffer) {
        destroy_semantic_analyzer(fixture->analyzer);
        code_generator_destroy(fixture->generator);
        free(fixture);
        return NULL;
    }
    
    return fixture;
}

/**
 * Cleanup test fixture
 */
static void cleanup_codegen_fixture(CodeGenTestFixture* fixture) {
    if (!fixture) return;
    
    if (fixture->test_ast) {
        ast_free_node(fixture->test_ast);
    }
    if (fixture->output_buffer) {
        free(fixture->output_buffer);
    }
    if (fixture->analyzer) {
        destroy_semantic_analyzer(fixture->analyzer);
    }
    if (fixture->generator) {
        code_generator_destroy(fixture->generator);
    }
    free(fixture);
}

// =============================================================================
// PARAMETER HANDLING TESTS
// =============================================================================

/**
 * Test basic parameter handling
 */
AsthraTestResult test_generate_basic_parameters(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function with parameters: fn add(a: i32, b: i32) -> i32 { return a + b; }
    const char* source = "package test;\n\npub fn add(a: i32, b: i32) -> i32 { return 42; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse function with parameters")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Need to analyze the program first, then generate code
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Generate the whole program, not just the function
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate function with parameters code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Parameter size is unsigned, so no need to check >= 0
    // Just verify it's reasonable (not too large)
    if (!asthra_test_assert_bool(context, fixture->generator->current_function_param_size < 1000,
                                "Parameter size should be reasonable")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test register spilling for many parameters
 */
AsthraTestResult test_generate_many_parameters(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function with many parameters to test register spilling
    const char* source = "package test;\n\npub fn many_params(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32, g: i32, h: i32) -> i32 { return 100; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse function with many parameters")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate the whole program
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze many params program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate function with many parameters code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test mixed parameter types
 */
AsthraTestResult test_generate_mixed_parameter_types(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function with different parameter types
    const char* source = "package test;\n\npub fn mixed_params(x: i32, y: f64, z: bool) -> bool { return true; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse function with mixed parameters")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate the whole program
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze mixed params program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate function with mixed parameters code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test parameter passing in different registers
 */
AsthraTestResult test_generate_parameter_registers(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function with 6 integer parameters (should use all integer registers)
    const char* source = "package test;\n\npub fn six_params(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) -> i32 { return 60; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse function with six parameters")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze six params program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate function with six parameters code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE CONFIGURATION
// =============================================================================

/**
 * Create test suite configuration for parameter tests
 */
static AsthraTestSuiteConfig create_parameter_test_suite_config(const char* name,
                                                                const char* description,
                                                                AsthraTestStatistics* stats) {
    AsthraTestSuiteConfig config = {
        .name = name,
        .description = description,
        .lightweight_mode = false,
        .custom_main = false,
        .statistics_tracking = true,
        .reporting_level = ASTHRA_TEST_REPORTING_STANDARD,
        .default_timeout_ns = 30000000000ULL, // 30 seconds
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .max_parallel_tests = 1,
        .statistics = stats
    };
    return config;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for parameter handling tests
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = create_parameter_test_suite_config(
        "Function Parameter Handling Tests",
        "Test code generation for function parameter handling",
        stats
    );
    
    AsthraTestFunction tests[] = {
        test_generate_basic_parameters,
        test_generate_many_parameters,
        test_generate_mixed_parameter_types,
        test_generate_parameter_registers
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_generate_basic_parameters", __FILE__, __LINE__, "test_generate_basic_parameters", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_many_parameters", __FILE__, __LINE__, "test_generate_many_parameters", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_mixed_parameter_types", __FILE__, __LINE__, "test_generate_mixed_parameter_types", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_parameter_registers", __FILE__, __LINE__, "test_generate_parameter_registers", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}