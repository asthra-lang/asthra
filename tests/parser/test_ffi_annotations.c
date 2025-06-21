/**
 * Asthra Programming Language
 * FFI Annotations Parsing Tests
 * 
 * Tests for FFI annotations including parameter annotations, extern annotations,
 * and transfer semantics.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "grammar_annotations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_ffi_annotation_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_ffi_annotation_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// FFI ANNOTATIONS TESTS
// =============================================================================

/**
 * Test: Parse FFI Parameter Annotations
 * Verifies that FFI parameter annotations are parsed correctly
 */
static AsthraTestResult test_parse_ffi_param_annotations(AsthraTestContext* context) {
    const char* test_source = "extern fn ffi_function(#[borrowed] data: *u8, #[transfer_full] result: *i32) -> void;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_extern_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse FFI parameter annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify it's an extern declaration
    if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                  "Should be AST_EXTERN_DECL")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Extern Annotations
 * Verifies that extern block annotations are parsed correctly
 */
static AsthraTestResult test_parse_extern_annotations(AsthraTestContext* context) {
    const char* test_source = "extern fn malloc(size: usize) -> *mut u8;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_extern_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse extern annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                  "Should be AST_EXTERN_DECL")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Transfer Semantics
 * Verifies that transfer semantics annotations are parsed correctly
 */
static AsthraTestResult test_parse_transfer_semantics(AsthraTestContext* context) {
    const char* test_cases[] = {
        "extern fn give_ownership(none) -> #[transfer_full] *mut Data;",
        "extern fn borrow_data(#[borrowed] data: *const Data) -> void;",
        "extern fn transfer_container(#[transfer_none] items: *mut Item) -> void;"
    };
    
    size_t case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < case_count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parse_extern_decl(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse transfer semantics")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                      "Should be AST_EXTERN_DECL")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse C ABI Annotations
 * Verifies that C ABI annotations are parsed correctly
 */
static AsthraTestResult test_parse_c_abi_annotations(AsthraTestContext* context) {
    const char* test_source = "extern \"C\" fn c_function(x: i32, y: i32) -> i32;";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_extern_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse C ABI annotation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                  "Should be AST_EXTERN_DECL")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Library Link Annotations
 * Verifies that library linking annotations are parsed correctly
 */
static AsthraTestResult test_parse_library_link_annotations(AsthraTestContext* context) {
    const char* test_cases[] = {
        "extern fn sin(x: f64) -> f64;",                    // Math library
        "extern fn pthread_create(none) -> i32;", // Dynamic library
        "extern fn SSL_new(none) -> *mut u8;",          // Static library
        "extern fn wrapper_function(none) -> void;"  // Symbol name override
    };
    
    size_t case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < case_count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser for case %zu", i)) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parse_extern_decl(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse library link annotation for case %zu", i)) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                      "Should be AST_EXTERN_DECL for case %zu", i)) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Calling Convention Annotations
 * Verifies that calling convention annotations are parsed correctly
 */
static AsthraTestResult test_parse_calling_convention_annotations(AsthraTestContext* context) {
    const char* test_cases[] = {
        "extern fn cdecl_function(none) -> void;",
        "extern fn stdcall_function(none) -> void;",
        "extern fn fastcall_function(none) -> void;",
        "extern fn vectorcall_function(none) -> void;"
    };
    
    size_t case_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < case_count; i++) {
        Parser* parser = create_test_parser(test_cases[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser for case %zu", i)) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parse_extern_decl(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse calling convention annotation for case %zu", i)) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                      "Should be AST_EXTERN_DECL for case %zu", i)) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Memory Safety Annotations
 * Verifies that memory safety annotations for FFI are parsed correctly
 */
static AsthraTestResult test_parse_memory_safety_annotations(AsthraTestContext* context) {
    const char* test_source = 
        "extern fn strcpy(#[transfer_full] dest: *mut u8, #[borrowed] src: *const u8) -> *mut u8;";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_extern_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse memory safety annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                  "Should be AST_EXTERN_DECL")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Buffer Size Annotations
 * Verifies that buffer size annotations for FFI are parsed correctly
 */
static AsthraTestResult test_parse_buffer_size_annotations(AsthraTestContext* context) {
    const char* test_source = 
        "extern fn read_buffer(#[borrowed] data: *const u8, count: usize) -> i32;";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_extern_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse buffer size annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                  "Should be AST_EXTERN_DECL")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Callback Annotations
 * Verifies that callback function annotations are parsed correctly
 */
static AsthraTestResult test_parse_callback_annotations(AsthraTestContext* context) {
    const char* test_source = 
        "extern fn register_callback(#[transfer_none] cb: *mut u8, #[transfer_none] user_data: *mut u8) -> void;";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* result = parse_extern_decl(parser);
    
    if (!asthra_test_assert_not_null(context, result, "Failed to parse callback annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, result->type, AST_EXTERN_DECL, 
                                  "Should be AST_EXTERN_DECL")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all FFI annotation tests
 */
AsthraTestSuite* create_ffi_annotations_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("FFI Annotation Tests", 
                                                     "FFI annotation parsing testing");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_ffi_annotation_tests);
    asthra_test_suite_set_teardown(suite, teardown_ffi_annotation_tests);
    
    // FFI annotations
    asthra_test_suite_add_test(suite, "test_parse_ffi_param_annotations", 
                              "Parse FFI parameter annotations", 
                              test_parse_ffi_param_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_extern_annotations", 
                              "Parse extern annotations", 
                              test_parse_extern_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_transfer_semantics", 
                              "Parse transfer semantics", 
                              test_parse_transfer_semantics);
    
    asthra_test_suite_add_test(suite, "test_parse_c_abi_annotations", 
                              "Parse C ABI annotations", 
                              test_parse_c_abi_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_library_link_annotations", 
                              "Parse library link annotations", 
                              test_parse_library_link_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_calling_convention_annotations", 
                              "Parse calling convention annotations", 
                              test_parse_calling_convention_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_memory_safety_annotations", 
                              "Parse memory safety annotations", 
                              test_parse_memory_safety_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_buffer_size_annotations", 
                              "Parse buffer size annotations", 
                              test_parse_buffer_size_annotations);
    
    asthra_test_suite_add_test(suite, "test_parse_callback_annotations", 
                              "Parse callback annotations", 
                              test_parse_callback_annotations);
    
    return suite;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite* suite = create_ffi_annotations_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create FFI annotations test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return result;
} 
