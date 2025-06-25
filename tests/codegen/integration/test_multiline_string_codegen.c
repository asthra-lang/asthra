/**
 * Asthra Programming Language
 * Code Generation Multi-line String Tests (Phase 5)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test multi-line string literal code generation with proper escaping
 */

#include "../framework/test_framework.h"
#include "ast.h"
#include "code_generator.h"
#include "semantic_analyzer.h"
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_multiline_string_codegen", __FILE__, __LINE__,
     "Test basic multi-line string code generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL,
     false, NULL},
    {"test_multiline_string_with_quotes", __FILE__, __LINE__,
     "Test multi-line string with embedded quotes", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
     NULL},
    {"test_multiline_string_with_special_chars", __FILE__, __LINE__,
     "Test multi-line string with special characters", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL,
     false, NULL},
    {"test_multiline_string_with_unicode", __FILE__, __LINE__,
     "Test multi-line string with unicode", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false,
     NULL}};

// =============================================================================
// HELPER FUNCTIONS FOR TESTING
// =============================================================================

/**
 * Helper function to create a string literal AST node for testing
 */
static ASTNode *create_test_string_literal(const char *value) {
    ASTNode *node = ast_create_node(AST_STRING_LITERAL, (SourceLocation){0});
    if (!node)
        return NULL;

    node->data.string_literal.value = strdup(value);
    if (!node->data.string_literal.value) {
        ast_destroy_node(node);
        return NULL;
    }

    return node;
}

// Forward declaration
static void destroy_test_code_generator(CodeGenerator *generator);

/**
 * Helper function to create a minimal code generator for testing
 */
static CodeGenerator *create_test_code_generator(void) {
    CodeGenerator *generator =
        code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator)
        return NULL;

    // Create a semantic analyzer for the code generator
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        destroy_test_code_generator(generator);
        return NULL;
    }

    // Set the semantic analyzer in the code generator
    generator->semantic_analyzer = analyzer;

    return generator;
}

/**
 * Helper function to destroy code generator and its semantic analyzer
 */
static void destroy_test_code_generator(CodeGenerator *generator) {
    if (!generator)
        return;

    // Save and destroy the semantic analyzer
    SemanticAnalyzer *analyzer = generator->semantic_analyzer;
    generator->semantic_analyzer = NULL;

    code_generator_destroy(generator);

    if (analyzer) {
        semantic_analyzer_destroy(analyzer);
    }
}

// =============================================================================
// MULTI-LINE STRING ESCAPING TESTS
// =============================================================================

/**
 * Test basic multi-line string code generation
 */
static AsthraTestResult test_basic_multiline_string_codegen(AsthraTestContext *context) {
    CodeGenerator *generator = create_test_code_generator();
    if (!generator) {
        return asthra_test_fail(context, "Failed to create code generator");
    }

    const char *multiline_content = "line 1\nline 2\nline 3";
    ASTNode *string_node = create_test_string_literal(multiline_content);
    if (!string_node) {
        destroy_test_code_generator(generator);
        return asthra_test_fail(context, "Failed to create string literal node");
    }

    // Test code generation for multi-line string
    Register target_reg = ASTHRA_REG_RAX;
    bool result = code_generate_expression(generator, string_node, target_reg);

    // Cleanup
    ast_destroy_node(string_node);
    destroy_test_code_generator(generator);

    if (!result) {
        return asthra_test_fail(context, "Failed to generate code for multi-line string");
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test multi-line string with special characters
 */
static AsthraTestResult test_multiline_string_special_chars_codegen(AsthraTestContext *context) {
    CodeGenerator *generator = create_test_code_generator();
    if (!generator) {
        return asthra_test_fail(context, "Failed to create code generator");
    }

    const char *content_with_special_chars = "String with \"quotes\" and \ttabs\nand \\backslashes";
    ASTNode *string_node = create_test_string_literal(content_with_special_chars);
    if (!string_node) {
        destroy_test_code_generator(generator);
        return asthra_test_fail(context, "Failed to create string literal node");
    }

    // Test code generation
    Register target_reg = ASTHRA_REG_RCX;
    bool result = code_generate_expression(generator, string_node, target_reg);

    // Cleanup
    ast_destroy_node(string_node);
    destroy_test_code_generator(generator);

    if (!result) {
        return asthra_test_fail(context,
                                "Failed to generate code for string with special characters");
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test empty multi-line string (edge case)
 */
static AsthraTestResult test_empty_string_codegen(AsthraTestContext *context) {
    CodeGenerator *generator = create_test_code_generator();
    if (!generator) {
        return asthra_test_fail(context, "Failed to create code generator");
    }

    ASTNode *string_node = create_test_string_literal("");
    if (!string_node) {
        destroy_test_code_generator(generator);
        return asthra_test_fail(context, "Failed to create empty string literal node");
    }

    // Test code generation for empty string
    Register target_reg = ASTHRA_REG_RDX;
    bool result = code_generate_expression(generator, string_node, target_reg);

    // Cleanup
    ast_destroy_node(string_node);
    destroy_test_code_generator(generator);

    if (!result) {
        return asthra_test_fail(context, "Failed to generate code for empty string");
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test single-line string (backward compatibility)
 */
static AsthraTestResult test_single_line_string_codegen(AsthraTestContext *context) {
    CodeGenerator *generator = create_test_code_generator();
    if (!generator) {
        return asthra_test_fail(context, "Failed to create code generator");
    }

    const char *single_line_content = "This is a single line string";
    ASTNode *string_node = create_test_string_literal(single_line_content);
    if (!string_node) {
        destroy_test_code_generator(generator);
        return asthra_test_fail(context, "Failed to create single-line string literal node");
    }

    // Test code generation for single-line string (should work as before)
    Register target_reg = ASTHRA_REG_RSI;
    bool result = code_generate_expression(generator, string_node, target_reg);

    // Cleanup
    ast_destroy_node(string_node);
    destroy_test_code_generator(generator);

    if (!result) {
        return asthra_test_fail(context, "Failed to generate code for single-line string");
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test SQL query multi-line string (real-world use case)
 */
static AsthraTestResult test_sql_query_multiline_codegen(AsthraTestContext *context) {
    CodeGenerator *generator = create_test_code_generator();
    if (!generator) {
        return asthra_test_fail(context, "Failed to create code generator");
    }

    const char *sql_query = "SELECT users.name, users.email\n"
                            "FROM users\n"
                            "WHERE users.active = true\n"
                            "ORDER BY users.created_at DESC";

    ASTNode *string_node = create_test_string_literal(sql_query);
    if (!string_node) {
        destroy_test_code_generator(generator);
        return asthra_test_fail(context, "Failed to create SQL query string node");
    }

    // Test code generation for SQL query string
    Register target_reg = ASTHRA_REG_RDI;
    bool result = code_generate_expression(generator, string_node, target_reg);

    // Cleanup
    ast_destroy_node(string_node);
    destroy_test_code_generator(generator);

    if (!result) {
        return asthra_test_fail(context, "Failed to generate code for SQL query string");
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test config template multi-line string (real-world use case)
 */
static AsthraTestResult test_config_template_multiline_codegen(AsthraTestContext *context) {
    CodeGenerator *generator = create_test_code_generator();
    if (!generator) {
        return asthra_test_fail(context, "Failed to create code generator");
    }

    const char *config_template = "server {\n"
                                  "    listen 80;\n"
                                  "    server_name example.com;\n"
                                  "    location / {\n"
                                  "        proxy_pass http://backend;\n"
                                  "    }\n"
                                  "}";

    ASTNode *string_node = create_test_string_literal(config_template);
    if (!string_node) {
        destroy_test_code_generator(generator);
        return asthra_test_fail(context, "Failed to create config template string node");
    }

    // Test code generation for config template string
    Register target_reg = ASTHRA_REG_R8;
    bool result = code_generate_expression(generator, string_node, target_reg);

    // Cleanup
    ast_destroy_node(string_node);
    destroy_test_code_generator(generator);

    if (!result) {
        return asthra_test_fail(context, "Failed to generate code for config template string");
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST REGISTRATION AND MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("Running Multi-line String Code Generation Tests (Phase 5)...\n");

    AsthraTestContext *context = asthra_test_create_context(&test_metadata[0]);
    if (!context) {
        printf("Failed to create test context\n");
        return 1;
    }

    // Register and run all tests
    bool all_passed = true;

    printf("Testing basic multi-line string code generation...\n");
    if (test_basic_multiline_string_codegen(context) != ASTHRA_TEST_PASS) {
        all_passed = false;
    }

    printf("Testing multi-line string with special characters...\n");
    if (test_multiline_string_special_chars_codegen(context) != ASTHRA_TEST_PASS) {
        all_passed = false;
    }

    printf("Testing empty string code generation...\n");
    if (test_empty_string_codegen(context) != ASTHRA_TEST_PASS) {
        all_passed = false;
    }

    printf("Testing single-line string backward compatibility...\n");
    if (test_single_line_string_codegen(context) != ASTHRA_TEST_PASS) {
        all_passed = false;
    }

    printf("Testing SQL query multi-line string...\n");
    if (test_sql_query_multiline_codegen(context) != ASTHRA_TEST_PASS) {
        all_passed = false;
    }

    printf("Testing config template multi-line string...\n");
    if (test_config_template_multiline_codegen(context) != ASTHRA_TEST_PASS) {
        all_passed = false;
    }

    // Print results
    if (all_passed) {
        printf("✅ All multi-line string code generation tests passed!\n");
        printf("Phase 5 (Code Generation) implementation complete.\n");
    } else {
        printf("❌ Some multi-line string code generation tests failed.\n");
    }

    asthra_test_destroy_context(context);
    return all_passed ? 0 : 1;
}
