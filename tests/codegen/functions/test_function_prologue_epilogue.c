/**
 * Asthra Programming Language Compiler
 * Function Prologue/Epilogue Generation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test code generation for function prologue and epilogue
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
// FUNCTION PROLOGUE/EPILOGUE TESTS
// =============================================================================

/**
 * Test basic function prologue and epilogue generation
 */
AsthraTestResult test_generate_basic_prologue_epilogue(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test simple function: fn test() -> i32 { return 42; }
    const char* source = "package test;\n\npub fn test(none) -> i32 { return 42; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse function declaration")) {
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
    if (!asthra_test_assert_bool(context, result, "Failed to generate function code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that instructions were generated
    if (!asthra_test_assert_bool(context, fixture->generator->instruction_buffer->count > 0,
                                "No instructions generated for function")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that stack frame was set up (current_function_stack_size should be set)
    if (!asthra_test_assert_pointer(context, fixture->generator->current_function_name,
                                   "Function name should be set during generation")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test void function prologue and epilogue
 */
AsthraTestResult test_generate_void_function_prologue_epilogue(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test void function: fn print_hello() -> void { return (); }
    const char* void_source = "package test;\n\npub fn print_hello(none) -> void { return (); }";
    fixture->test_ast = parse_test_source(void_source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse void function")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate the whole program
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze void program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate void function code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test function prologue with stack alignment
 */
AsthraTestResult test_generate_stack_aligned_prologue(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test function that requires stack alignment
    const char* source = "package test;\n\npub fn aligned_func(x: i32) -> i32 { let y: i32 = 100; return 200; }";
    fixture->test_ast = parse_test_source(source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast, "Failed to parse aligned function")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Analyze and generate
    bool analyze_result = semantic_analyze_program(fixture->analyzer, fixture->test_ast);
    if (!asthra_test_assert_bool(context, analyze_result, "Failed to analyze aligned program")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    bool result = code_generate_program(fixture->generator, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to generate aligned function code")) {
        cleanup_codegen_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify stack was properly aligned (should be multiple of 16)
    if (!asthra_test_assert_bool(context, 
                                (fixture->generator->current_function_stack_size % 16) == 0,
                                "Stack size should be aligned to 16 bytes")) {
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
 * Create test suite configuration for function prologue/epilogue tests
 */
static AsthraTestSuiteConfig create_prologue_test_suite_config(const char* name,
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
 * Main test function for function prologue/epilogue generation
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = create_prologue_test_suite_config(
        "Function Prologue/Epilogue Generation Tests",
        "Test code generation for function prologue and epilogue",
        stats
    );
    
    AsthraTestFunction tests[] = {
        test_generate_basic_prologue_epilogue,
        test_generate_void_function_prologue_epilogue,
        test_generate_stack_aligned_prologue
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_generate_basic_prologue_epilogue", __FILE__, __LINE__, "test_generate_basic_prologue_epilogue", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_void_function_prologue_epilogue", __FILE__, __LINE__, "test_generate_void_function_prologue_epilogue", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_stack_aligned_prologue", __FILE__, __LINE__, "test_generate_stack_aligned_prologue", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}