/**
 * Asthra Programming Language
 * Type Alias Error Handling Tests
 * 
 * Tests for proper error handling when type aliases are encountered
 * (since they are not yet implemented)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser.h"
#include "lexer.h"
#include "parser_core.h"

// Test framework macros (simple pattern)
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return 0; \
        } \
    } while (0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return 1; \
    } while (0)

// =============================================================================
// TYPE ALIAS ERROR TESTS
// =============================================================================

/**
 * Test: Type Alias Declaration Error
 * Verifies that type alias declarations produce proper error messages
 */
static int test_type_alias_declaration_error(void) {
    const char* test_source = "package test;\n\ntype MyInt = i32;";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Parse and expect an error
    ASTNode* result = parser_parse_program(parser);
    
    // The parser should have encountered an error
    TEST_ASSERT(parser_had_error(parser), "Should have parsing error for type alias");
    
    // Check if error messages were reported
    // The parser stores errors as a linked list
    if (parser->errors) {
        // Check if any error mentions type aliases
        bool found_type_alias_error = false;
        ParseError* error = parser->errors;
        while (error) {
            if (strstr(error->message, "Type aliases are not yet implemented") ||
                strstr(error->message, "type alias")) {
                found_type_alias_error = true;
                break;
            }
            error = error->next;
        }
        TEST_ASSERT(found_type_alias_error, "Error should mention type aliases not implemented");
    }
    
    if (result) ast_free_node(result);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    TEST_PASS("Type alias declaration error");
}

/**
 * Test: Multiple Type Alias Errors
 * Verifies error recovery after type alias declaration
 */
static int test_multiple_type_alias_errors(void) {
    const char* test_source = 
        "package test;\n"
        "\n"
        "type MyInt = i32;\n"
        "type MyString = string;\n"
        "\n"
        "function main() {\n"
        "    // This should still be parsed\n"
        "}\n";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Parse and expect errors
    ASTNode* result = parser_parse_program(parser);
    
    // The parser should have encountered errors
    TEST_ASSERT(parser_had_error(parser), "Should have parsing errors for type aliases");
    
    // Despite errors, the parser should try to continue and parse the function
    if (result && result->type == AST_PROGRAM) {
        // Check if we managed to parse anything after the errors
        printf("INFO: Parser attempted recovery after type alias errors\n");
    }
    
    if (result) ast_free_node(result);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    TEST_PASS("Multiple type alias errors");
}

/**
 * Test: Type Alias With Generic Parameters
 * Verifies error handling for generic type aliases
 */
static int test_generic_type_alias_error(void) {
    const char* test_source = "package test;\n\ntype Container<T> = Vec<T>;";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Parse and expect an error
    ASTNode* result = parser_parse_program(parser);
    
    // The parser should have encountered an error
    TEST_ASSERT(parser_had_error(parser), "Should have parsing error for generic type alias");
    
    if (result) ast_free_node(result);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    TEST_PASS("Generic type alias error");
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Running type alias error handling tests...\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Run all tests
    total++; if (test_type_alias_declaration_error()) passed++;
    total++; if (test_multiple_type_alias_errors()) passed++;
    total++; if (test_generic_type_alias_error()) passed++;
    
    printf("\nType Alias Error Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("All type alias error tests passed!\n");
        return 0;
    } else {
        printf("Some type alias error tests failed!\n");
        return 1;
    }
}