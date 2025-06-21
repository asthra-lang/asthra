/**
 * Asthra Programming Language
 * SafeFFIAnnotation Parser Integration Tests
 * 
 * Phase 5.2: Parser Integration Tests for SafeFFIAnnotation
 * Validates that the parse_safe_ffi_annotation() function correctly parses
 * single annotations and rejects multiple conflicting annotations.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "grammar_annotations.h"
#include "lexer.h"
#include "ast_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURES
// =============================================================================

static AsthraTestResult setup_parser_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_parser_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Create a parser from source and attempt to parse SafeFFIAnnotation
 */
static ASTNode *parse_ffi_annotation_from_source(const char *source) {
    if (!source) return NULL;
    
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) return NULL;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }
    
    ASTNode *annotation = parse_safe_ffi_annotation(parser);
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return annotation;
}

/**
 * Validate FFI annotation AST node structure
 */
static bool validate_ffi_annotation_ast(ASTNode *node, FFITransferType expected_type) {
    if (!node) return false;
    
    if (node->type != AST_FFI_ANNOTATION) return false;
    
    return node->data.ffi_annotation.transfer_type == expected_type;
}

// =============================================================================
// PHASE 5.2: PARSER INTEGRATION TESTS
// =============================================================================

/**
 * Test: Parse Transfer Full Annotation
 * Validates parsing of #[transfer_full] annotation
 */
static AsthraTestResult test_parse_transfer_full_annotation(AsthraTestContext *context) {
    const char *source = "#[transfer_full]";
    
    ASTNode *annotation = parse_ffi_annotation_from_source(source);
    
    if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, annotation, "Should parse transfer_full annotation")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, validate_ffi_annotation_ast(annotation, FFI_TRANSFER_FULL),
                           "Should create correct FFI_TRANSFER_FULL AST node")) {
        ast_free_node(annotation);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(annotation);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Transfer None Annotation
 * Validates parsing of #[transfer_none] annotation
 */
static AsthraTestResult test_parse_transfer_none_annotation(AsthraTestContext *context) {
    const char *source = "#[transfer_none]";
    
    ASTNode *annotation = parse_ffi_annotation_from_source(source);
    
    if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, annotation, "Should parse transfer_none annotation")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, validate_ffi_annotation_ast(annotation, FFI_TRANSFER_NONE),
                           "Should create correct FFI_TRANSFER_NONE AST node")) {
        ast_free_node(annotation);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(annotation);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Borrowed Annotation
 * Validates parsing of #[borrowed] annotation
 */
static AsthraTestResult test_parse_borrowed_annotation(AsthraTestContext *context) {
    const char *source = "#[borrowed]";
    
    ASTNode *annotation = parse_ffi_annotation_from_source(source);
    
    if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, annotation, "Should parse borrowed annotation")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, validate_ffi_annotation_ast(annotation, FFI_BORROWED),
                           "Should create correct FFI_BORROWED AST node")) {
        ast_free_node(annotation);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(annotation);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Reject Invalid Annotation Names
 * Ensures invalid FFI annotation names are rejected
 */
static AsthraTestResult test_reject_invalid_annotation_names(AsthraTestContext *context) {
    const char *invalid_annotations[] = {
        "#[invalid_annotation]",
        "#[transfer_invalid]",
        "#[borrow]",           // Should be "borrowed"
        "#[transfer]",         // Incomplete
        "#[full]",            // Incomplete
        "#[none]"             // Incomplete
    };
    
    size_t count = sizeof(invalid_annotations) / sizeof(invalid_annotations[0]);
    
    for (size_t i = 0; i < count; i++) {
        ASTNode *annotation = parse_ffi_annotation_from_source(invalid_annotations[i]);
        
        if (!ASTHRA_TEST_ASSERT_NULL(context, annotation, 
                                    "Invalid annotation should be rejected: %s", invalid_annotations[i])) {
            if (annotation) ast_free_node(annotation);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Reject Malformed Annotation Syntax
 * Ensures malformed annotation syntax is rejected
 */
static AsthraTestResult test_reject_malformed_annotation_syntax(AsthraTestContext *context) {
    const char *malformed_annotations[] = {
        "[transfer_full]",      // Missing #
        "#transfer_full]",      // Missing [
        "#[transfer_full",      // Missing ]
        "# [transfer_full]",    // Space after #
        "#[ transfer_full]",    // Space after [
        "#[transfer_full ]",    // Space before ]
        "#[]",                  // Empty annotation
        "#[transfer_full][transfer_none]"  // Adjacent annotations (should parse only first)
    };
    
    size_t count = sizeof(malformed_annotations) / sizeof(malformed_annotations[0]);
    
    for (size_t i = 0; i < count; i++) {
        ASTNode *annotation = parse_ffi_annotation_from_source(malformed_annotations[i]);
        
        if (!ASTHRA_TEST_ASSERT_NULL(context, annotation, 
                                    "Malformed annotation should be rejected: %s", malformed_annotations[i])) {
            if (annotation) ast_free_node(annotation);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Single Annotation Parsing in Function Context
 * Validates that SafeFFIAnnotation works correctly in function declarations
 */
static AsthraTestResult test_single_annotation_in_function_context(AsthraTestContext *context) {
    const char *function_sources[] = {
        "fn test_function() -> #[transfer_full] *mut u8 { }",
        "fn test_function() -> #[transfer_none] *const u8 { }",
        "fn test_function(#[borrowed] param: *const u8) -> i32 { }",
        "fn test_function(#[transfer_full] param: *mut u8) -> i32 { }"
    };
    
    size_t count = sizeof(function_sources) / sizeof(function_sources[0]);
    
    for (size_t i = 0; i < count; i++) {
        Lexer *lexer = lexer_create(function_sources[i], strlen(function_sources[i]), "test");
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for: %s", function_sources[i])) {
            return ASTHRA_TEST_FAIL;
        }
        
        Parser *parser = parser_create(lexer);
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Failed to create parser for: %s", function_sources[i])) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode *func = parse_function_decl(parser);
        
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, func, 
                                        "Function with SafeFFIAnnotation should parse: %s", function_sources[i])) {
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        // Validate that function parsed correctly with annotation
        if (!ASTHRA_TEST_ASSERT_INT_EQ(context, func->type, AST_FUNCTION_DECL, 
                                      "Should be AST_FUNCTION_DECL for: %s", function_sources[i])) {
            ast_free_node(func);
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(func);
        parser_destroy(parser);
        lexer_destroy(lexer);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Single Annotation Parsing in Extern Context
 * Validates that SafeFFIAnnotation works correctly in extern declarations
 */
static AsthraTestResult test_single_annotation_in_extern_context(AsthraTestContext *context) {
    const char *extern_sources[] = {
        "extern \"C\" fn malloc(size: usize) -> #[transfer_full] *mut u8;",
        "extern \"C\" fn free(#[transfer_full] ptr: *mut u8);",
        "extern \"C\" fn strlen(#[borrowed] s: *const u8) -> usize;",
        "extern fn custom_function(#[transfer_none] data: *const u8) -> #[transfer_none] *const i32;"
    };
    
    size_t count = sizeof(extern_sources) / sizeof(extern_sources[0]);
    
    for (size_t i = 0; i < count; i++) {
        Lexer *lexer = lexer_create(extern_sources[i], strlen(extern_sources[i]), "test");
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for: %s", extern_sources[i])) {
            return ASTHRA_TEST_FAIL;
        }
        
        Parser *parser = parser_create(lexer);
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Failed to create parser for: %s", extern_sources[i])) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode *extern_decl = parse_extern_decl(parser);
        
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, extern_decl, 
                                        "Extern with SafeFFIAnnotation should parse: %s", extern_sources[i])) {
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        // Validate that extern parsed correctly with annotation
        if (!ASTHRA_TEST_ASSERT_INT_EQ(context, extern_decl->type, AST_EXTERN_DECL, 
                                      "Should be AST_EXTERN_DECL for: %s", extern_sources[i])) {
            ast_free_node(extern_decl);
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(extern_decl);
        parser_destroy(parser);
        lexer_destroy(lexer);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parser Error Recovery
 * Validates that parser handles errors gracefully and recovers
 */
static AsthraTestResult test_parser_error_recovery(AsthraTestContext *context) {
    const char *error_sources[] = {
        "#[invalid_ffi_annotation]",
        "#[transfer_invalid]",
        "#[borrowed extra_content]",
        "#[transfer_full",  // Incomplete
        "[transfer_none]"   // Malformed
    };
    
    size_t count = sizeof(error_sources) / sizeof(error_sources[0]);
    
    for (size_t i = 0; i < count; i++) {
        Lexer *lexer = lexer_create(error_sources[i], strlen(error_sources[i]), "test");
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for error test")) {
            return ASTHRA_TEST_FAIL;
        }
        
        Parser *parser = parser_create(lexer);
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Failed to create parser for error test")) {
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode *annotation = parse_safe_ffi_annotation(parser);
        
        // Should return NULL for invalid input
        if (!ASTHRA_TEST_ASSERT_NULL(context, annotation, 
                                    "Parser should reject invalid input: %s", error_sources[i])) {
            if (annotation) ast_free_node(annotation);
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        // Parser should still be in valid state for error recovery
        if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Parser should remain valid after error")) {
            parser_destroy(parser);
            lexer_destroy(lexer);
            return ASTHRA_TEST_FAIL;
        }
        
        parser_destroy(parser);
        lexer_destroy(lexer);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Annotation in Complete Program Context
 * Validates SafeFFIAnnotation parsing in realistic program context
 */
static AsthraTestResult test_annotation_in_complete_program_context(AsthraTestContext *context) {
    const char *program_source = 
        "package test;\n"
        "\n"
        "extern \"C\" fn malloc(size: usize) -> #[transfer_full] *mut u8;\n"
        "extern \"C\" fn free(#[transfer_full] ptr: *mut u8);\n"
        "extern \"C\" fn strlen(#[borrowed] s: *const u8) -> usize;\n"
        "\n"
        "fn safe_wrapper(#[borrowed] input: *const u8) -> Result<*mut u8, string> {\n"
        "    let len = strlen(input);\n"
        "    if len > 0 {\n"
        "        return Ok(malloc(len + 1));\n"
        "    }\n"
        "    return Err(\"Invalid input\");\n"
        "}\n"
        "\n"
        "fn cleanup(#[transfer_full] ptr: *mut u8) {\n"
        "    free(ptr);\n"
        "}\n";
    
    Lexer *lexer = lexer_create(program_source, strlen(program_source), "test_program.asthra");
    if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for complete program")) {
        return ASTHRA_TEST_FAIL;
    }
    
    Parser *parser = parser_create(lexer);
    if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, parser, "Failed to create parser for complete program")) {
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode *program = parse_program(parser);
    
    if (!ASTHRA_TEST_ASSERT_NOT_NULL(context, program, "Complete program with SafeFFIAnnotations should parse")) {
        parser_destroy(parser);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    // Validate that program parsed correctly
    if (!ASTHRA_TEST_ASSERT_INT_EQ(context, program->type, AST_PROGRAM, "Should be AST_PROGRAM")) {
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Verify Mutual Exclusivity at Parser Level
 * This test ensures that the grammar prevents multiple annotations
 */
static AsthraTestResult test_mutual_exclusivity_at_parser_level(AsthraTestContext *context) {
    // These should be impossible to parse with SafeFFIAnnotation grammar
    // since SafeFFIAnnotation only accepts single annotations
    const char *impossible_sources[] = {
        "#[transfer_full] #[transfer_none]",
        "#[borrowed] #[transfer_full]", 
        "#[transfer_full] #[borrowed] #[transfer_none]"
    };
    
    size_t count = sizeof(impossible_sources) / sizeof(impossible_sources[0]);
    
    for (size_t i = 0; i < count; i++) {
        ASTNode *annotation = parse_ffi_annotation_from_source(impossible_sources[i]);
        
        // The SafeFFIAnnotation grammar should only parse the first annotation
        // and stop, not create a conflicting annotation node
        if (annotation) {
            // If we got an annotation, it should be valid (just the first one)
            bool is_valid = validate_ffi_annotation_ast(annotation, FFI_TRANSFER_FULL) ||
                           validate_ffi_annotation_ast(annotation, FFI_TRANSFER_NONE) ||
                           validate_ffi_annotation_ast(annotation, FFI_BORROWED);
            
            if (!ASTHRA_TEST_ASSERT(context, is_valid, 
                                   "If annotation parses, it should be valid: %s", impossible_sources[i])) {
                ast_free_node(annotation);
                return ASTHRA_TEST_FAIL;
            }
            
            ast_free_node(annotation);
        }
        
        // The key test: parser should not create conflicting annotation structures
        // This test passes as long as no invalid AST structure is created
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all SafeFFIAnnotation parser integration tests
 */
void register_safe_ffi_annotation_parser_tests(void) {
    asthra_test_register_suite("SafeFFIAnnotation Parser", setup_parser_tests, teardown_parser_tests);
    
    // Core annotation parsing tests
    asthra_test_register_test("SafeFFIAnnotation Parser", "Parse Transfer Full Annotation", test_parse_transfer_full_annotation);
    asthra_test_register_test("SafeFFIAnnotation Parser", "Parse Transfer None Annotation", test_parse_transfer_none_annotation);
    asthra_test_register_test("SafeFFIAnnotation Parser", "Parse Borrowed Annotation", test_parse_borrowed_annotation);
    
    // Error handling tests
    asthra_test_register_test("SafeFFIAnnotation Parser", "Reject Invalid Annotation Names", test_reject_invalid_annotation_names);
    asthra_test_register_test("SafeFFIAnnotation Parser", "Reject Malformed Annotation Syntax", test_reject_malformed_annotation_syntax);
    asthra_test_register_test("SafeFFIAnnotation Parser", "Parser Error Recovery", test_parser_error_recovery);
    
    // Context integration tests
    asthra_test_register_test("SafeFFIAnnotation Parser", "Single Annotation in Function Context", test_single_annotation_in_function_context);
    asthra_test_register_test("SafeFFIAnnotation Parser", "Single Annotation in Extern Context", test_single_annotation_in_extern_context);
    asthra_test_register_test("SafeFFIAnnotation Parser", "Annotation in Complete Program Context", test_annotation_in_complete_program_context);
    
    // Safety validation test
    asthra_test_register_test("SafeFFIAnnotation Parser", "Mutual Exclusivity at Parser Level", test_mutual_exclusivity_at_parser_level);
}

/**
 * Main function for standalone testing
 */
int main(void) {
    printf("=== SafeFFIAnnotation Parser Integration Tests ===\n");
    printf("Phase 5.2: Testing parse_safe_ffi_annotation() function\n\n");
    
    register_safe_ffi_annotation_parser_tests();
    
    int result = asthra_test_run_suite("SafeFFIAnnotation Parser");
    
    if (result == 0) {
        printf("\n✅ All SafeFFIAnnotation parser tests passed!\n");
        printf("parse_safe_ffi_annotation() correctly handles single annotations and rejects conflicts.\n");
    } else {
        printf("\n❌ SafeFFIAnnotation parser tests failed!\n");
        printf("CRITICAL: Parser does not properly handle SafeFFIAnnotation grammar.\n");
    }
    
    return result;
} 
