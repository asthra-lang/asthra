/**
 * Asthra Programming Language
 * Generic Constraint Error Handling Tests
 * 
 * Tests for proper error handling when generic type constraints are encountered
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
// GENERIC CONSTRAINT ERROR TESTS
// =============================================================================

/**
 * Test: Struct With Generic Constraint Error
 * Verifies that structs with generic constraints produce proper error messages
 */
static int test_struct_generic_constraint_error(void) {
    const char* test_source = 
        "package test;\n"
        "\n"
        "pub struct Container<T: Comparable> {\n"
        "    value: T\n"
        "}\n";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Parse and expect an error
    ASTNode* result = parser_parse_program(parser);
    
    // The parser should have encountered an error
    TEST_ASSERT(parser_had_error(parser), "Should have parsing error for generic constraint");
    
    // Check if error messages were reported
    if (parser->errors) {
        // Check if any error mentions constraints
        bool found_constraint_error = false;
        ParseError* error = parser->errors;
        while (error) {
            if (strstr(error->message, "Type parameter constraints are not supported") ||
                strstr(error->message, "constraint")) {
                found_constraint_error = true;
                break;
            }
            error = error->next;
        }
        TEST_ASSERT(found_constraint_error, "Error should mention constraints not supported");
    }
    
    if (result) ast_free_node(result);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    TEST_PASS("Struct generic constraint error");
}

/**
 * Test: Enum With Generic Constraint Error
 * Verifies that enums with generic constraints produce proper error messages
 */
static int test_enum_generic_constraint_error(void) {
    const char* test_source = 
        "package test;\n"
        "\n"
        "pub enum Result<T: Display, E: Error> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Parse and expect an error
    ASTNode* result = parser_parse_program(parser);
    
    // The parser should have encountered an error
    TEST_ASSERT(parser_had_error(parser), "Should have parsing error for enum generic constraint");
    
    if (result) ast_free_node(result);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    TEST_PASS("Enum generic constraint error");
}

/**
 * Test: Multiple Type Parameters With Constraints
 * Verifies error handling for multiple constrained type parameters
 */
static int test_multiple_constraints_error(void) {
    const char* test_source = 
        "package test;\n"
        "\n"
        "pub struct Map<K: Hash + Eq, V: Clone> {\n"
        "    // Implementation\n"
        "}\n";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Parse and expect an error
    ASTNode* result = parser_parse_program(parser);
    
    // The parser should have encountered an error on the first constraint
    TEST_ASSERT(parser_had_error(parser), "Should have parsing error for multiple constraints");
    
    if (result) ast_free_node(result);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    TEST_PASS("Multiple constraints error");
}

/**
 * Test: Valid Generic Without Constraints
 * Verifies that generics without constraints still work correctly
 */
static int test_valid_generic_without_constraint(void) {
    const char* test_source = 
        "package test;\n"
        "\n"
        "pub struct Container<T> {\n"
        "    value: T\n"
        "}\n";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // This should parse successfully
    ASTNode* result = parser_parse_program(parser);
    
    TEST_ASSERT(result != NULL, "Should parse generic without constraint");
    TEST_ASSERT(!parser_had_error(parser), "Should parse without errors");
    
    ast_free_node(result);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    TEST_PASS("Valid generic without constraint");
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Running generic constraint error handling tests...\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Run all tests
    total++; if (test_struct_generic_constraint_error()) passed++;
    total++; if (test_enum_generic_constraint_error()) passed++;
    total++; if (test_multiple_constraints_error()) passed++;
    total++; if (test_valid_generic_without_constraint()) passed++;
    
    printf("\nGeneric Constraint Error Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("All generic constraint error tests passed!\n");
        return 0;
    } else {
        printf("Some generic constraint error tests failed!\n");
        return 1;
    }
}