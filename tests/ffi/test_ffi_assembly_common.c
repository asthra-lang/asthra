/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Common Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Common implementations and utilities for FFI assembly generator tests
 */

#include "test_ffi_assembly_common.h"
#include "type_info.h"

// Global test state
FFIAssemblyGenerator *g_generator = NULL;
size_t g_tests_run = 0;
size_t g_tests_passed = 0;

// Memory leak debug flag
bool g_ignore_leaks = false;

// =============================================================================
// MOCK IMPLEMENTATIONS FOR MISSING DEPENDENCIES
// =============================================================================

MockSemanticAnalyzer *mock_semantic_analyzer_create(void) {
    return calloc(1, sizeof(MockSemanticAnalyzer));
}

void mock_semantic_analyzer_destroy(MockSemanticAnalyzer *analyzer) {
    free(analyzer);
}

// Helper function to set up mock variables in the symbol table
bool setup_mock_variables(FFIAssemblyGenerator *generator) {
    if (!generator || !generator->base_generator) return false;
    
    // Create a mock type descriptor for testing
    TypeDescriptor *mock_type = calloc(1, sizeof(TypeDescriptor));
    if (!mock_type) return false;
    
    mock_type->category = TYPE_PRIMITIVE;
    mock_type->name = strdup("i32");
    mock_type->size = 4;
    mock_type->alignment = 4;
    
    // Add common test variables to the symbol table
    bool success = true;
    success &= add_local_variable(generator->base_generator, "result_value", mock_type, 4);
    success &= add_local_variable(generator->base_generator, "name", mock_type, 8);
    success &= add_local_variable(generator->base_generator, "value", mock_type, 4);
    success &= add_local_variable(generator->base_generator, "test_var", mock_type, 4);
    success &= add_local_variable(generator->base_generator, "object", mock_type, 8);
    
    if (!success) {
        free((char *)mock_type->name);
        free(mock_type);
    }
    
    return success;
}

// =============================================================================
// TEST HELPER FUNCTIONS
// =============================================================================

// Helper function to destroy mock TypeInfo
static void destroy_mock_type_info(TypeInfo *type_info) {
    if (type_info) {
        free(type_info->name);
        free(type_info);
    }
}

// Helper function to create mock TypeInfo for test identifiers
static TypeInfo *create_mock_type_info(const char *type_name) {
    TypeInfo *type_info = calloc(1, sizeof(TypeInfo));
    if (!type_info) return NULL;
    
    type_info->type_id = 1; // Mock type ID
    type_info->name = strdup(type_name ? type_name : "i32");
    type_info->category = TYPE_INFO_PRIMITIVE;
    type_info->size = 4;
    type_info->alignment = 4;
    
    // Set flags for a basic integer type
    type_info->flags.is_mutable = true;
    type_info->flags.is_owned = true;
    type_info->flags.is_ffi_compatible = true;
    type_info->flags.is_copyable = true;
    type_info->flags.is_movable = true;
    
    type_info->ownership = OWNERSHIP_INFO_STACK;
    
    // Set primitive type data
    type_info->data.primitive.kind = PRIMITIVE_INFO_I32;
    type_info->data.primitive.is_signed = true;
    type_info->data.primitive.is_integer = true;
    
    return type_info;
}

ASTNode *create_test_identifier(const char *name) {
    ASTNode *node = ast_create_node(AST_IDENTIFIER, (SourceLocation){0, 0, 0});
    if (node) {
        node->data.identifier.name = strdup(name);
        // Attach mock type information to satisfy architectural validation
        node->type_info = create_mock_type_info("i32");
    }
    return node;
}

// Custom cleanup function for test identifiers with TypeInfo
void free_test_identifier(ASTNode *node) {
    if (node && node->type == AST_IDENTIFIER) {
        // Clean up the attached TypeInfo
        if (node->type_info) {
            destroy_mock_type_info(node->type_info);
            node->type_info = NULL;
        }
        // Clean up the identifier name
        if (node->data.identifier.name) {
            free((char*)node->data.identifier.name);
            node->data.identifier.name = NULL;
        }
        // Free the node itself
        free(node);
    }
}

ASTNode *create_test_integer_literal(int64_t value) {
    ASTNode *node = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0, 0, 0});
    if (node) {
        node->data.integer_literal.value = value;
    }
    return node;
}

ASTNode *create_test_string_literal(const char *value) {
    ASTNode *node = ast_create_node(AST_STRING_LITERAL, (SourceLocation){0, 0, 0});
    if (node) {
        node->data.string_literal.value = strdup(value);
    }
    return node;
}

ASTNode *create_test_call_expr(const char *function_name, ASTNodeList *args) {
    ASTNode *node = ast_create_node(AST_CALL_EXPR, (SourceLocation){0, 0, 0});
    if (node) {
        node->data.call_expr.function = create_test_identifier(function_name);
        node->data.call_expr.args = args;
    }
    return node;
}

ASTNode *create_test_spawn_stmt(const char *function_name, ASTNodeList *args) {
    ASTNode *node = ast_create_node(AST_SPAWN_STMT, (SourceLocation){0, 0, 0});
    if (node) {
        node->data.spawn_stmt.function_name = strdup(function_name);
        node->data.spawn_stmt.args = args;
    }
    return node;
}

ASTNode *create_test_match_stmt(ASTNode *expression, ASTNodeList *arms) {
    ASTNode *node = ast_create_node(AST_MATCH_STMT, (SourceLocation){0, 0, 0});
    if (node) {
        node->data.match_stmt.expression = expression;
        node->data.match_stmt.arms = arms;
    }
    return node;
}

ASTNode *create_test_unsafe_block(ASTNode *block) {
    ASTNode *node = ast_create_node(AST_UNSAFE_BLOCK, (SourceLocation){0, 0, 0});
    if (node) {
        node->data.unsafe_block.block = block;
    }
    return node;
}

// Test runner function
void run_test(test_function_t test_func) {
    g_tests_run++;
    bool result = test_func();
    if (result) g_tests_passed++;
}

// Setup and teardown functions for test suite
bool setup_test_suite(void) {
    // Create the FFI assembly generator
    g_generator = ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!g_generator) {
        fprintf(stderr, "ERROR: Failed to create FFI assembly generator\n");
        return false;
    }
    
    // Set up semantic analyzer on the base generator
    if (g_generator->base_generator) {
        SemanticAnalyzer* analyzer = setup_semantic_analyzer();
        if (analyzer) {
            g_generator->base_generator->semantic_analyzer = analyzer;
        }
    }
    
    // Initialize mock variables
    if (!setup_mock_variables(g_generator)) {
        fprintf(stderr, "ERROR: Failed to setup mock variables\n");
        ffi_assembly_generator_destroy(g_generator);
        g_generator = NULL;
        return false;
    }
    
    g_tests_run = 0;
    g_tests_passed = 0;
    
    return true;
}

void teardown_test_suite(void) {
    if (g_generator) {
        // Clean up semantic analyzer first
        if (g_generator->base_generator && g_generator->base_generator->semantic_analyzer) {
            destroy_semantic_analyzer(g_generator->base_generator->semantic_analyzer);
            g_generator->base_generator->semantic_analyzer = NULL;
        }
        ffi_assembly_generator_destroy(g_generator);
        g_generator = NULL;
    }
    
    printf("\n==================================================\n");
    printf("FFI Assembly Generator Test Results\n");
    printf("==================================================\n");
    printf("Tests run: %zu\n", g_tests_run);
    printf("Tests passed: %zu\n", g_tests_passed);
    printf("Success rate: %.1f%%\n\n", (float)g_tests_passed / (float)g_tests_run * 100.0f);
    
    if (g_tests_passed == g_tests_run) {
        printf("🎉 All tests passed!\n");
    } else {
        printf("❌ Some tests failed!\n");
    }
} 
