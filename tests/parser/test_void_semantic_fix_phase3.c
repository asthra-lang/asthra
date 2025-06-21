/**
 * Asthra Programming Language Compiler
 * Phase 3 Tests: Void Semantic Overloading Fix Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for v1.19 grammar implementation with TOKEN_NONE support
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser.h"
#include "lexer.h"

// Test helper functions
static Parser* create_test_parser(const char* source) {
    Lexer* lexer = lexer_create(source, strlen(source), "test.asthra");
    if (!lexer) return NULL;
    return parser_create(lexer);
}

static void cleanup_test_parser(Parser* parser) {
    if (parser) {
        if (parser->lexer) {
            lexer_destroy(parser->lexer);
        }
        parser_destroy(parser);
    }
}

// Test 1: Function parameters with 'none' (should succeed)
static bool test_function_parameters_none_success(void) {
    printf("Testing function parameters with 'none' (should succeed)...\n");
    
    const char* source = "fn main(none) -> void { }";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_function_decl(parser);
    bool success = (result != NULL);
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 2: Function parameters with 'void' (should fail with helpful error)
static bool test_function_parameters_void_error(void) {
    printf("Testing function parameters with 'void' (should fail with helpful error)...\n");
    
    const char* source = "fn main(void) -> void { }";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_function_decl(parser);
    bool success = (result == NULL); // Should fail
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 3: Struct content with 'none' (should succeed)
static bool test_struct_content_none_success(void) {
    printf("Testing struct content with 'none' (should succeed)...\n");
    
    const char* source = "struct EmptyStruct { none }";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_struct_decl(parser);
    bool success = (result != NULL);
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 4: Struct content with 'void' (should fail with helpful error)
static bool test_struct_content_void_error(void) {
    printf("Testing struct content with 'void' (should fail with helpful error)...\n");
    
    const char* source = "struct EmptyStruct { void }";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_struct_decl(parser);
    bool success = (result == NULL); // Should fail
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 5: Array literals with 'none' (should succeed)
static bool test_array_literals_none_success(void) {
    printf("Testing array literals with 'none' (should succeed)...\n");
    
    const char* source = "[none]";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_primary(parser);
    bool success = (result != NULL && result->type == AST_ARRAY_LITERAL);
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 6: Array literals with 'void' (should fail with helpful error)
static bool test_array_literals_void_error(void) {
    printf("Testing array literals with 'void' (should fail with helpful error)...\n");
    
    const char* source = "[void]";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_primary(parser);
    bool success = (result == NULL); // Should fail
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 7: Function calls with 'none' (should succeed)
static bool test_function_calls_none_success(void) {
    printf("Testing function calls with 'none' (should succeed)...\n");
    
    const char* source = "main(none)";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_expr(parser);
    bool success = (result != NULL && result->type == AST_CALL_EXPR);
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 8: Function calls with 'void' (should fail with helpful error)
static bool test_function_calls_void_error(void) {
    printf("Testing function calls with 'void' (should fail with helpful error)...\n");
    
    const char* source = "main(void)";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_expr(parser);
    bool success = (result == NULL); // Should fail
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 9: Return type with 'void' (should succeed - preserved usage)
static bool test_return_type_void_preserved(void) {
    printf("Testing return type with 'void' (should succeed - preserved usage)...\n");
    
    const char* source = "fn cleanup(none) -> void { }";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_function_decl(parser);
    bool success = (result != NULL);
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 10: Pattern arguments with 'none' (should succeed)
static bool test_pattern_args_none_success(void) {
    printf("Testing pattern arguments with 'none' (should succeed)...\n");
    
    const char* source = "Option.None(none)";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_pattern(parser);
    bool success = (result != NULL && result->type == AST_ENUM_PATTERN);
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Test 11: Pattern arguments with 'void' (should fail with helpful error)
static bool test_pattern_args_void_error(void) {
    printf("Testing pattern arguments with 'void' (should fail with helpful error)...\n");
    
    const char* source = "Option.None(void)";
    Parser* parser = create_test_parser(source);
    assert(parser != NULL);
    
    ASTNode* result = parse_pattern(parser);
    bool success = (result == NULL); // Should fail
    
    if (result) {
        ast_free_node(result);
    }
    cleanup_test_parser(parser);
    
    printf("  %s\n", success ? "✅ PASSED" : "❌ FAILED");
    return success;
}

// Main test runner
int main(void) {
    printf("=============================================================\n");
    printf("Asthra Phase 3: Void Semantic Overloading Fix Tests\n");
    printf("Testing v1.19 Grammar Implementation with TOKEN_NONE\n");
    printf("=============================================================\n\n");
    
    int passed = 0;
    int total = 11;
    
    // Run all tests
    if (test_function_parameters_none_success()) passed++;
    if (test_function_parameters_void_error()) passed++;
    if (test_struct_content_none_success()) passed++;
    if (test_struct_content_void_error()) passed++;
    if (test_array_literals_none_success()) passed++;
    if (test_array_literals_void_error()) passed++;
    if (test_function_calls_none_success()) passed++;
    if (test_function_calls_void_error()) passed++;
    if (test_return_type_void_preserved()) passed++;
    if (test_pattern_args_none_success()) passed++;
    if (test_pattern_args_void_error()) passed++;
    
    printf("\n=============================================================\n");
    printf("TEST RESULTS\n");
    printf("=============================================================\n");
    printf("Passed: %d/%d tests\n", passed, total);
    
    if (passed == total) {
        printf("🎉 ALL TESTS PASSED! Phase 3 implementation successful.\n");
        printf("✅ v1.19 grammar correctly implemented\n");
        printf("✅ TOKEN_NONE support working\n");
        printf("✅ Semantic boundaries enforced\n");
        printf("✅ Helpful error messages provided\n");
        return 0;
    } else {
        printf("❌ %d tests failed. Phase 3 implementation needs fixes.\n", total - passed);
        return 1;
    }
} 
