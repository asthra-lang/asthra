/**
 * Asthra Programming Language Compiler
 * Generic Structs Testing - Common Utilities Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of shared test framework and utilities
 */

#include "generic_structs_test_common.h"

// =============================================================================
// TEST FRAMEWORK GLOBALS
// =============================================================================

size_t tests_run = 0;
size_t tests_passed = 0;

// =============================================================================
// VALIDATION HELPER FUNCTIONS IMPLEMENTATION
// =============================================================================

bool test_parse_success(const char *source, const char *description) {
    // Wrap the source in a package declaration if it doesn't have one
    char wrapped_source[4096];
    if (strncmp(source, "package", 7) != 0) {
        snprintf(wrapped_source, sizeof(wrapped_source), "package test;\n\n%s", source);
        source = wrapped_source;
    }
    
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) {
        printf("    ❌ Failed to create lexer for %s\n", description);
        return false;
    }
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        printf("    ❌ Failed to create parser for %s\n", description);
        return false;
    }
    
    ASTNode *node = parse_program(parser);
    bool success = (node != NULL);
    
    if (success) {
        printf("    ✅ Parse success: %s\n", description);
        ast_destroy_node(node);
    } else {
        printf("    ❌ Parse failed: %s\n", description);
        // Print parser errors if available
        if (parser->errors) {
            ParseError *error = parser->errors;
            while (error) {
                printf("       Error: %s at line %d, col %d\n", 
                       error->message, error->location.line, error->location.column);
                error = error->next;
            }
        }
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    return success;
}

bool test_parse_failure(const char *source, const char *description) {
    // Wrap the source in a package declaration if it doesn't have one
    char wrapped_source[4096];
    if (strncmp(source, "package", 7) != 0) {
        snprintf(wrapped_source, sizeof(wrapped_source), "package test;\n\n%s", source);
        source = wrapped_source;
    }
    
    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) return true; // Lexer failure is acceptable for invalid syntax
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return true; // Parser creation failure is acceptable
    }
    
    ASTNode *node = parse_program(parser);
    bool failed = (node == NULL);
    
    if (failed) {
        printf("    ✅ Parse correctly failed: %s\n", description);
    } else {
        printf("    ❌ Parse should have failed: %s\n", description);
        ast_destroy_node(node);
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    return failed;
}

bool test_semantic_success(const char *source, const char *description) {
    // Wrap the source in a package declaration if it doesn't have one
    char wrapped_source[4096];
    if (strncmp(source, "package", 7) != 0) {
        snprintf(wrapped_source, sizeof(wrapped_source), "package test;\n\n%s", source);
        source = wrapped_source;
    }
    
    // Parse first
Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer) return false;
    
Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return false;
    }
    
ASTNode *ast = parse_program(parser);
    if (!ast) {
        parser_destroy(parser);
        lexer_destroy(lexer);
        printf("    ❌ Parse failed for semantic test: %s\n", description);
        return false;
    }
    // Semantic analysis
SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("       Failed to create semantic analyzer\n");
        ast_destroy_node(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return false;
    }
    
bool success = semantic_analyze_program(analyzer, ast);
    if (success) {
        printf("    ✅ Semantic success: %s\n", description);
    } else {
        printf("    ❌ Semantic failed: %s\n", description);
        // Print semantic errors if available for better debugging
        if (analyzer->error_count > 0) {
            SemanticError *error = analyzer->errors;
            size_t i = 1;
            while (error) {
                printf("       Semantic Error %zu: %s at line %d, col %d\n", 
                       i, error->message, error->location.line, error->location.column);
                error = error->next;
                i++;
            }
        } else {
            printf("       No specific error messages reported\n");
        }
    }
    
    semantic_analyzer_destroy(analyzer);
    ast_destroy_node(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    return success;
}

void init_test_framework(void) {
    tests_run = 0;
    tests_passed = 0;
}

int print_test_summary(void) {
    printf("\n=============================================================================\n");
    printf("Validation Test Summary: %zu/%zu tests passed (%.1f%%)\n", 
           tests_passed, tests_run, 
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("=============================================================================\n");
    
    if (tests_passed == tests_run) {
        printf("🎉 All validation tests passed! Generic structs are robust and well-validated.\n");
        return 0;
    } else {
        printf("❌ Some validation tests failed. Check edge case handling.\n");
        return 1;
    }
} 
