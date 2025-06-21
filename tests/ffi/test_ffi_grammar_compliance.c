/**
 * Asthra Programming Language
 * FFI Grammar Compliance Tests
 * 
 * Phase 5.1: Grammar Compliance Tests for SafeFFIAnnotation
 * Validates that the SafeFFIAnnotation grammar properly rejects dangerous patterns
 * and accepts valid patterns, ensuring AI generation safety.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "grammar_annotations.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURES
// =============================================================================

static AsthraTestResult setup_grammar_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_grammar_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Test if a source string with SafeFFIAnnotation parses successfully
 */
static bool test_safe_ffi_annotation_parsing(const char *source) {
    if (!source) return false;
    
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) return false;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return false;
    }
    
    ASTNode *annotation = parse_safe_ffi_annotation(parser);
    bool success = (annotation != NULL);
    
    if (annotation) {
        ast_free_node(annotation);
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return success;
}

/**
 * Test if a function declaration with FFI annotations parses successfully
 */
static bool test_function_decl_with_annotation(const char *source) {
    if (!source) return false;
    
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) return false;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return false;
    }
    
    ASTNode *func = parse_function_decl(parser);
    bool success = (func != NULL);
    
    if (func) {
        ast_free_node(func);
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return success;
}

/**
 * Test if an extern declaration with FFI annotations parses successfully
 */
static bool test_extern_decl_with_annotation(const char *source) {
    if (!source) return false;
    
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) return false;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return false;
    }
    
    ASTNode *extern_decl = parse_extern_decl(parser);
    bool success = (extern_decl != NULL);
    
    if (extern_decl) {
        ast_free_node(extern_decl);
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return success;
}

// =============================================================================
// PHASE 5.1: GRAMMAR COMPLIANCE TESTS
// =============================================================================

/**
 * Test: Valid Single FFI Annotations
 * Ensures that valid single SafeFFIAnnotations parse correctly
 */
static AsthraTestResult test_valid_single_ffi_annotations(AsthraTestContext *context) {
    const char *valid_annotations[] = {
        "#[transfer_full]",
        "#[transfer_none]",
        "#[borrowed]"
    };
    
    size_t count = sizeof(valid_annotations) / sizeof(valid_annotations[0]);
    
    for (size_t i = 0; i < count; i++) {
        bool parsed = test_safe_ffi_annotation_parsing(valid_annotations[i]);
        if (!ASTHRA_TEST_ASSERT(context, parsed, 
                               "Valid SafeFFIAnnotation should parse: %s", valid_annotations[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Invalid Multiple FFI Annotations (CRITICAL SAFETY TEST)
 * Ensures that multiple conflicting FFI annotations are rejected
 * This is the core fix - preventing dangerous AI-generated patterns
 */
static AsthraTestResult test_invalid_multiple_ffi_annotations(AsthraTestContext *context) {
    // These patterns MUST be rejected by SafeFFIAnnotation grammar
    const char *dangerous_patterns[] = {
        "#[transfer_full] #[transfer_none]",
        "#[transfer_full] #[borrowed]", 
        "#[transfer_none] #[borrowed]",
        "#[transfer_full] #[transfer_none] #[borrowed]",
        "#[borrowed] #[transfer_full]",
        "#[transfer_none] #[transfer_full]"
    };
    
    size_t count = sizeof(dangerous_patterns) / sizeof(dangerous_patterns[0]);
    
    for (size_t i = 0; i < count; i++) {
        bool parsed = test_safe_ffi_annotation_parsing(dangerous_patterns[i]);
        if (!ASTHRA_TEST_ASSERT(context, !parsed, 
                               "Dangerous multiple FFI annotations MUST be rejected: %s", dangerous_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Valid Function Declarations with Single FFI Annotations
 * Tests the grammar rule: FunctionDecl <- 'fn' SimpleIdent '(' ParamList? ')' '->' SafeFFIAnnotation? Type Block
 */
static AsthraTestResult test_valid_function_declarations(AsthraTestContext *context) {
    const char *valid_functions[] = {
        "fn safe_malloc() -> #[transfer_full] *mut u8 { }",
        "fn get_const_ptr() -> #[transfer_none] *const u8 { }",
        "fn safe_function(#[borrowed] input: *const u8) -> i32 { }",
        "fn complex_function(#[transfer_full] ptr: *mut u8, #[borrowed] data: *const u8) -> #[transfer_none] *const i32 { }"
    };
    
    size_t count = sizeof(valid_functions) / sizeof(valid_functions[0]);
    
    for (size_t i = 0; i < count; i++) {
        bool parsed = test_function_decl_with_annotation(valid_functions[i]);
        if (!ASTHRA_TEST_ASSERT(context, parsed, 
                               "Valid function with SafeFFIAnnotation should parse: %s", valid_functions[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Invalid Function Declarations with Multiple FFI Annotations
 * Ensures dangerous function declaration patterns are rejected
 */
static AsthraTestResult test_invalid_function_declarations(AsthraTestContext *context) {
    const char *dangerous_functions[] = {
        "fn dangerous_malloc() -> #[transfer_full] #[transfer_none] *mut u8 { }",
        "fn dangerous_function(#[borrowed] #[transfer_full] ptr: *mut u8) -> i32 { }",
        "fn very_dangerous() -> #[transfer_full] #[transfer_none] #[borrowed] *mut u8 { }"
    };
    
    size_t count = sizeof(dangerous_functions) / sizeof(dangerous_functions[0]);
    
    for (size_t i = 0; i < count; i++) {
        bool parsed = test_function_decl_with_annotation(dangerous_functions[i]);
        if (!ASTHRA_TEST_ASSERT(context, !parsed, 
                               "Dangerous function with multiple FFI annotations MUST be rejected: %s", dangerous_functions[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Valid Extern Declarations with Single FFI Annotations  
 * Tests the grammar rule: ExternDecl <- 'extern' STRING? 'fn' SimpleIdent '(' ExternParamList? ')' '->' SafeFFIAnnotation? Type ';'
 */
static AsthraTestResult test_valid_extern_declarations(AsthraTestContext *context) {
    const char *valid_externs[] = {
        "extern \"C\" fn malloc(size: usize) -> #[transfer_full] *mut u8;",
        "extern \"C\" fn free(#[transfer_full] ptr: *mut u8);",
        "extern \"C\" fn strlen(#[borrowed] s: *const u8) -> usize;",
        "extern fn custom_function(#[transfer_none] data: *const u8) -> #[transfer_none] *const i32;"
    };
    
    size_t count = sizeof(valid_externs) / sizeof(valid_externs[0]);
    
    for (size_t i = 0; i < count; i++) {
        bool parsed = test_extern_decl_with_annotation(valid_externs[i]);
        if (!ASTHRA_TEST_ASSERT(context, parsed, 
                               "Valid extern with SafeFFIAnnotation should parse: %s", valid_externs[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Invalid Extern Declarations with Multiple FFI Annotations
 * Ensures dangerous extern declaration patterns are rejected
 */
static AsthraTestResult test_invalid_extern_declarations(AsthraTestContext *context) {
    const char *dangerous_externs[] = {
        "extern \"C\" fn dangerous_malloc() -> #[transfer_full] #[transfer_none] *mut u8;",
        "extern \"C\" fn dangerous_free(#[transfer_full] #[borrowed] ptr: *mut u8);",
        "extern fn very_dangerous(#[transfer_full] #[transfer_none] #[borrowed] data: *mut u8);"
    };
    
    size_t count = sizeof(dangerous_externs) / sizeof(dangerous_externs[0]);
    
    for (size_t i = 0; i < count; i++) {
        bool parsed = test_extern_decl_with_annotation(dangerous_externs[i]);
        if (!ASTHRA_TEST_ASSERT(context, !parsed, 
                               "Dangerous extern with multiple FFI annotations MUST be rejected: %s", dangerous_externs[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parameter Grammar Compliance
 * Tests the grammar rules: Param <- SafeFFIAnnotation? SimpleIdent ':' Type
 *                          ExternParam <- SafeFFIAnnotation? SimpleIdent ':' Type
 */
static AsthraTestResult test_parameter_grammar_compliance(AsthraTestContext *context) {
    // Valid parameter patterns - single annotation or none
    const char *valid_params[] = {
        "fn test(param: i32) -> i32 { }",                           // No annotation
        "fn test(#[borrowed] param: *const u8) -> i32 { }",        // Single annotation
        "fn test(#[transfer_full] param: *mut u8) -> i32 { }",     // Single annotation
        "fn test(#[transfer_none] param: *const u8) -> i32 { }"    // Single annotation
    };
    
    size_t valid_count = sizeof(valid_params) / sizeof(valid_params[0]);
    
    for (size_t i = 0; i < valid_count; i++) {
        bool parsed = test_function_decl_with_annotation(valid_params[i]);
        if (!ASTHRA_TEST_ASSERT(context, parsed, 
                               "Valid parameter pattern should parse: %s", valid_params[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Invalid parameter patterns - multiple annotations  
    const char *invalid_params[] = {
        "fn test(#[borrowed] #[transfer_full] param: *mut u8) -> i32 { }",
        "fn test(#[transfer_full] #[transfer_none] param: *mut u8) -> i32 { }",
        "fn test(#[transfer_none] #[borrowed] param: *const u8) -> i32 { }"
    };
    
    size_t invalid_count = sizeof(invalid_params) / sizeof(invalid_params[0]);
    
    for (size_t i = 0; i < invalid_count; i++) {
        bool parsed = test_function_decl_with_annotation(invalid_params[i]);
        if (!ASTHRA_TEST_ASSERT(context, !parsed, 
                               "Invalid parameter pattern MUST be rejected: %s", invalid_params[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AI Generation Safety Validation
 * Ensures that patterns AI models commonly generate are properly handled
 */
static AsthraTestResult test_ai_generation_safety(AsthraTestContext *context) {
    // Common AI-generated dangerous patterns that MUST be rejected
    const char *ai_dangerous_patterns[] = {
        // AI might try to be "extra safe" with multiple annotations
        "extern \"C\" fn malloc() -> #[transfer_full] #[transfer_none] *mut u8;",
        
        // AI might copy patterns and add conflicting annotations
        "fn wrapper(#[borrowed] #[transfer_full] ptr: *mut u8) -> i32 { }",
        
        // AI might not understand mutual exclusivity
        "extern fn complex(#[transfer_full] #[transfer_none] #[borrowed] data: *mut u8);",
        
        // AI might generate redundant "safety" annotations
        "fn safe_function() -> #[transfer_full] #[transfer_full] *mut u8 { }"
    };
    
    size_t count = sizeof(ai_dangerous_patterns) / sizeof(ai_dangerous_patterns[0]);
    
    for (size_t i = 0; i < count; i++) {
        // These patterns should fail at the parsing stage, not semantic analysis
        bool parsed = test_function_decl_with_annotation(ai_dangerous_patterns[i]) || 
                     test_extern_decl_with_annotation(ai_dangerous_patterns[i]);
        
        if (!ASTHRA_TEST_ASSERT(context, !parsed, 
                               "AI-generated dangerous pattern MUST be rejected at parse time: %s", 
                               ai_dangerous_patterns[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Backward Compatibility
 * Ensures that valid existing FFI code continues to work
 */
static AsthraTestResult test_backward_compatibility(AsthraTestContext *context) {
    const char *existing_valid_code[] = {
        // Simple extern declarations without annotations (should work)
        "extern \"C\" fn simple_function() -> i32;",
        "extern \"C\" fn function_with_params(x: i32, y: i32) -> i32;",
        
        // Functions without annotations (should work)
        "fn regular_function(x: i32) -> i32 { }",
        "fn pointer_function(ptr: *mut u8) -> *const u8 { }",
        
        // Single annotation patterns (should work)
        "extern \"C\" fn malloc(size: usize) -> #[transfer_full] *mut u8;",
        "fn safe_wrapper(#[borrowed] input: *const u8) -> i32 { }"
    };
    
    size_t count = sizeof(existing_valid_code) / sizeof(existing_valid_code[0]);
    
    for (size_t i = 0; i < count; i++) {
        bool parsed = test_function_decl_with_annotation(existing_valid_code[i]) || 
                     test_extern_decl_with_annotation(existing_valid_code[i]);
        
        if (!ASTHRA_TEST_ASSERT(context, parsed, 
                               "Existing valid FFI code should continue to work: %s", existing_valid_code[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all FFI grammar compliance tests
 */
void register_ffi_grammar_compliance_tests(void) {
    asthra_test_register_suite("FFI Grammar Compliance", setup_grammar_tests, teardown_grammar_tests);
    
    // Core grammar compliance tests
    asthra_test_register_test("FFI Grammar Compliance", "Valid Single FFI Annotations", test_valid_single_ffi_annotations);
    asthra_test_register_test("FFI Grammar Compliance", "Invalid Multiple FFI Annotations", test_invalid_multiple_ffi_annotations);
    
    // Function and extern declaration tests
    asthra_test_register_test("FFI Grammar Compliance", "Valid Function Declarations", test_valid_function_declarations);
    asthra_test_register_test("FFI Grammar Compliance", "Invalid Function Declarations", test_invalid_function_declarations);
    asthra_test_register_test("FFI Grammar Compliance", "Valid Extern Declarations", test_valid_extern_declarations);
    asthra_test_register_test("FFI Grammar Compliance", "Invalid Extern Declarations", test_invalid_extern_declarations);
    
    // Parameter and AI safety tests
    asthra_test_register_test("FFI Grammar Compliance", "Parameter Grammar Compliance", test_parameter_grammar_compliance);
    asthra_test_register_test("FFI Grammar Compliance", "AI Generation Safety", test_ai_generation_safety);
    
    // Compatibility test
    asthra_test_register_test("FFI Grammar Compliance", "Backward Compatibility", test_backward_compatibility);
}

/**
 * Main function for standalone testing
 */
int main(void) {
    printf("=== FFI Grammar Compliance Tests ===\n");
    printf("Phase 5.1: Testing SafeFFIAnnotation grammar compliance\n\n");
    
    register_ffi_grammar_compliance_tests();
    
    int result = asthra_test_run_suite("FFI Grammar Compliance");
    
    if (result == 0) {
        printf("\n✅ All FFI grammar compliance tests passed!\n");
        printf("SafeFFIAnnotation grammar successfully prevents dangerous patterns.\n");
    } else {
        printf("\n❌ FFI grammar compliance tests failed!\n");
        printf("CRITICAL: Grammar does not properly prevent dangerous FFI annotation patterns.\n");
    }
    
    return result;
} 
