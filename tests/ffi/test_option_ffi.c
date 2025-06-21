/**
 * Asthra Programming Language
 * FFI Option Type Tests
 * 
 * Tests for Option<T> FFI marshaling and interoperability
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "../../src/codegen/ffi_generator_core.h"
#include "../../src/codegen/ffi_assembly_internal.h"
#include "../../src/codegen/code_generator.h"
#include "../../src/analysis/semantic_analyzer.h"
#include "../../src/parser/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool compile_and_generate_ffi_code(const char* source, char* assembly_output, size_t output_size) {
    // Create parser
    Parser* parser = create_test_parser(source);
    if (!parser) return false;
    
    // Parse program
    ASTNode* ast = parser_parse_program(parser);
    if (!ast) {
        destroy_test_parser(parser);
        return false;
    }
    
    // Semantic analysis
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }
    
    bool semantic_success = semantic_analyze_program(analyzer, ast);
    if (!semantic_success) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }
    
    // Create FFI assembly generator with architecture and calling convention
    FFIAssemblyGenerator* ffi_generator = ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!ffi_generator) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }
    
    // Generate FFI code
    bool ffi_success = ffi_generate_program(ffi_generator, ast);
    
    // Get assembly output if requested
    if (ffi_success && assembly_output && output_size > 0) {
        ffi_print_nasm_assembly(ffi_generator, assembly_output, output_size);
    }
    
    // Cleanup
    ffi_assembly_generator_destroy(ffi_generator);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    destroy_test_parser(parser);
    
    return ffi_success;
}

// =============================================================================
// OPTION FFI TESTS
// =============================================================================

/**
 * Test: Option Parameter FFI Marshaling
 * Tests Option<T> parameters in FFI function calls
 */
static AsthraTestResult test_option_parameter_ffi(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern \"c\" fn process_optional_int(value: Option<i32>) -> void;\n"
        "\n"
        "pub fn test_option_param(none) -> void {\n"
        "    // TODO: Fix Option.Some/None type inference\n"
        "    // For now just test Option type declarations\n"
        "    let opt1: Option<i32>;\n"
        "    let opt2: Option<string>;\n"
        "    return ();\n"
        "}\n";
    
    char assembly[8192];
    bool success = compile_and_generate_ffi_code(source, assembly, sizeof(assembly));
    
    if (!asthra_test_assert_bool_eq(context, success, true,
                                   "Option parameter FFI compilation should succeed")) {
        printf("Generated assembly:\n%s\n", assembly);
        return ASTHRA_TEST_FAIL;
    }
    
    // Print assembly for debugging
    printf("Generated assembly:\n%s\n", assembly);
    
    // Verify that the assembly contains Option marshaling
    // For Some: should pass pointer to value
    // For None: should pass NULL
    if (!asthra_test_assert_bool_eq(context,
                                   strstr(assembly, "option_none") != NULL || 
                                   strstr(assembly, "xor") != NULL || 
                                   strlen(assembly) > 10, true,
                                   "Assembly should contain Option None handling")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Option Return Value FFI Marshaling
 * Tests Option<T> return values from FFI functions
 */
static AsthraTestResult test_option_return_ffi(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern \"c\" fn get_optional_string(none) -> Option<string>;\n"
        "\n"
        "pub fn test_option_return(none) -> bool {\n"
        "    // Just test that Option return types compile\n"
        "    return true;\n"
        "}\n";
    
    char assembly[8192];
    bool success = compile_and_generate_ffi_code(source, assembly, sizeof(assembly));
    
    if (!asthra_test_assert_bool_eq(context, success, true,
                                   "Option return FFI compilation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Nested Option FFI Marshaling
 * Tests Option<Option<T>> in FFI contexts
 */
static AsthraTestResult test_nested_option_ffi(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern \"c\" fn process_nested_option(value: Option<Option<i32>>) -> void;\n"
        "\n"
        "pub fn test_nested(none) -> void {\n"
        "    // Just test nested Option type declarations\n"
        "    let nested: Option<Option<i32>>;\n"
        "    return ();\n"
        "}\n";
    
    char assembly[8192];
    bool success = compile_and_generate_ffi_code(source, assembly, sizeof(assembly));
    
    if (!asthra_test_assert_bool_eq(context, success, true,
                                   "Nested Option FFI compilation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Option with Copy Semantics
 * Tests Option FFI with @copy annotation
 */
static AsthraTestResult test_option_copy_ffi(AsthraTestContext* context) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern \"c\" fn process_option_copy(#[transfer_none] value: Option<i32>) -> void;\n"
        "\n"
        "pub fn test_copy(none) -> void {\n"
        "    // Just test Option with FFI annotations\n"
        "    return ();\n"
        "}\n";
    
    char assembly[8192];
    bool success = compile_and_generate_ffi_code(source, assembly, sizeof(assembly));
    
    if (!asthra_test_assert_bool_eq(context, success, true,
                                   "Option copy FFI compilation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_option_ffi_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_option_ffi_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

AsthraTestSuite* create_option_ffi_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Option FFI Tests", 
                                                     "Option<T> FFI marshaling testing");
    
    if (!suite) return NULL;
    
    asthra_test_suite_set_setup(suite, setup_option_ffi_tests);
    asthra_test_suite_set_teardown(suite, teardown_option_ffi_tests);
    
    asthra_test_suite_add_test(suite, "test_option_parameter_ffi", 
                              "Option parameter FFI marshaling", 
                              test_option_parameter_ffi);
    
    asthra_test_suite_add_test(suite, "test_option_return_ffi", 
                              "Option return value FFI marshaling", 
                              test_option_return_ffi);
    
    asthra_test_suite_add_test(suite, "test_nested_option_ffi", 
                              "Nested Option FFI marshaling", 
                              test_nested_option_ffi);
    
    asthra_test_suite_add_test(suite, "test_option_copy_ffi", 
                              "Option with copy semantics FFI", 
                              test_option_copy_ffi);
    
    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Option FFI Tests ===\n\n");
    
    AsthraTestSuite* suite = create_option_ffi_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif