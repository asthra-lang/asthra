/**
 * Asthra Programming Language Compiler
 * Function Calling Convention Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test code generation for function calling conventions
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
// CALLING CONVENTION TESTS
// =============================================================================

/**
 * Test System V AMD64 calling convention
 */
AsthraTestResult test_generate_system_v_calling_convention(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test System V AMD64 calling convention (default)
    const char* source = "package test;\n\npub fn sysv_func(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) -> i32 { return 42; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse System V function")) {
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
    if (!asthra_test_assert_bool(context, result, "Failed to generate System V function code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that the calling convention is properly set
    if (!asthra_test_assert_bool(context, fixture->generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64,
                                "Calling convention should be System V AMD64")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test floating point parameter calling convention
 */
AsthraTestResult test_generate_float_calling_convention(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function with floating point parameters
    const char* source = "package test;\n\npub fn float_func(x: f64, y: f64, z: f64) -> f64 { return 3.14; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse floating point function")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate the whole program
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze float program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate floating point function code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test mixed integer and floating point parameters
 */
AsthraTestResult test_generate_mixed_calling_convention(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function with mixed integer and float parameters
    const char* source = "package test;\n\npub fn mixed_func(a: i32, x: f64, b: i32, y: f64) -> f64 { return 2.718; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse mixed parameter function")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze mixed parameter program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate mixed parameter function code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test stack parameter passing
 */
AsthraTestResult test_generate_stack_parameter_passing(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function with more than 6 integer parameters (some go on stack)
    const char* source = "package test;\n\npub fn many_int_params(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32, g: i32, h: i32, i: i32) -> i32 { return 99; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse function with stack parameters")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze stack parameter program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate function with stack parameters code")) {
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
 * Create test suite configuration for calling convention tests
 */
static AsthraTestSuiteConfig create_calling_conv_test_suite_config(const char* name,
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
 * Main test function for calling convention tests
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = create_calling_conv_test_suite_config(
        "Function Calling Convention Tests",
        "Test code generation for function calling conventions",
        stats
    );
    
    AsthraTestFunction tests[] = {
        test_generate_system_v_calling_convention,
        test_generate_float_calling_convention,
        test_generate_mixed_calling_convention,
        test_generate_stack_parameter_passing
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_generate_system_v_calling_convention", __FILE__, __LINE__, "test_generate_system_v_calling_convention", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_float_calling_convention", __FILE__, __LINE__, "test_generate_float_calling_convention", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_mixed_calling_convention", __FILE__, __LINE__, "test_generate_mixed_calling_convention", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_stack_parameter_passing", __FILE__, __LINE__, "test_generate_stack_parameter_passing", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}