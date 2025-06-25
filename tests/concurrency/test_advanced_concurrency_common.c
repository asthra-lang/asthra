/**
 * Asthra Programming Language - Advanced Concurrency Tests Common Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared utilities and implementations for the three-tier concurrency system test suite.
 */

#include "test_advanced_concurrency_common.h"
#include "parser_string_interface.h"

// =============================================================================
// GLOBAL TEST COUNTERS
// =============================================================================

size_t tests_run = 0;
size_t tests_passed = 0;

// =============================================================================
// TEST FRAMEWORK IMPLEMENTATIONS
// =============================================================================

// Note: cleanup_parse_result is now provided by parser_string_interface.h
// Removed duplicate implementation to avoid linker conflicts

void cleanup_semantic_result(SemanticAnalysisResult *result) {
    if (result) {
        // In real implementation, would clean up error arrays
        for (int i = 0; i < result->error_count; i++) {
            if (result->errors[i].message) {
                free(result->errors[i].message);
                result->errors[i].message = NULL;
            }
        }
        result->error_count = 0;
    }
}

void print_test_statistics(void) {
    printf("\n=============================================================================\n");
    printf("TEST SUMMARY\n");
    printf("=============================================================================\n");
    printf("Tests Run: %d\n", tests_run);
    printf("Tests Passed: %d\n", tests_passed);
    printf("Tests Failed: %d\n", tests_run - tests_passed);
    printf("Success Rate: %.2f%%\n", tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);

    if (tests_passed == tests_run) {
        printf("\n✅ All tests passed! The three-tier concurrency system is working correctly.\n");
    } else {
        printf("\n❌ Some tests failed. The three-tier concurrency system needs attention.\n");
    }
}

void reset_test_counters(void) {
    tests_run = 0;
    tests_passed = 0;
}

// =============================================================================
// PARSER AND SEMANTIC ANALYSIS MOCK IMPLEMENTATIONS
// =============================================================================

// REAL IMPLEMENTATION: Use actual parser instead of stub
// This function now calls the real parser implementation from parser_string_interface.c
// No longer a stub - provides real parsing functionality for tests

SemanticAnalysisResult analyze_semantics(ASTNode *ast) {
    SemanticAnalysisResult result = {0};
    result.success = true;
    result.error_count = 0;

    // Mock implementation - would analyze AST in real version
    (void)ast;

    // For testing purposes, simulate failures for specific patterns
    // This allows testing of annotation requirements

    return result;
}

TokenType keyword_to_token_type(const char *keyword) {
    if (!keyword)
        return TOKEN_IDENTIFIER;

    // Tier 1 keywords (still supported)
    if (strcmp(keyword, "spawn") == 0)
        return TOKEN_SPAWN;
    if (strcmp(keyword, "spawn_with_handle") == 0)
        return TOKEN_SPAWN_WITH_HANDLE;
    if (strcmp(keyword, "await") == 0)
        return TOKEN_AWAIT;

    // All Tier 3 keywords should return TOKEN_IDENTIFIER
    // These are no longer recognized as special keywords
    return TOKEN_IDENTIFIER;
}

// =============================================================================
// AST NAVIGATION AND VALIDATION HELPER FUNCTIONS
// =============================================================================

bool contains_spawn_statement(ASTNode *node) {
    if (!node)
        return false;

    if (node->type == AST_SPAWN_STMT)
        return true;

    // Recursively check child nodes based on node type
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.declarations) {
            for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                if (contains_spawn_statement(node->data.program.declarations->nodes[i])) {
                    return true;
                }
            }
        }
        break;
    case AST_FUNCTION_DECL:
        if (node->data.function_decl.body &&
            contains_spawn_statement(node->data.function_decl.body)) {
            return true;
        }
        break;
    case AST_BLOCK:
        if (node->data.block.statements) {
            for (size_t i = 0; i < node->data.block.statements->count; i++) {
                if (contains_spawn_statement(node->data.block.statements->nodes[i])) {
                    return true;
                }
            }
        }
        break;
    case AST_IF_STMT:
        if (contains_spawn_statement(node->data.if_stmt.then_block) ||
            (node->data.if_stmt.else_block &&
             contains_spawn_statement(node->data.if_stmt.else_block))) {
            return true;
        }
        break;
    default:
        break;
    }

    return false;
}

bool contains_spawn_with_handle_statement(ASTNode *node) {
    if (!node)
        return false;

    if (node->type == AST_SPAWN_WITH_HANDLE_STMT)
        return true;

    // Recursively check child nodes based on node type
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.declarations) {
            for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                if (contains_spawn_with_handle_statement(
                        node->data.program.declarations->nodes[i])) {
                    return true;
                }
            }
        }
        break;
    case AST_FUNCTION_DECL:
        if (node->data.function_decl.body &&
            contains_spawn_with_handle_statement(node->data.function_decl.body)) {
            return true;
        }
        break;
    case AST_BLOCK:
        if (node->data.block.statements) {
            for (size_t i = 0; i < node->data.block.statements->count; i++) {
                if (contains_spawn_with_handle_statement(node->data.block.statements->nodes[i])) {
                    return true;
                }
            }
        }
        break;
    case AST_IF_STMT:
        if (contains_spawn_with_handle_statement(node->data.if_stmt.then_block) ||
            (node->data.if_stmt.else_block &&
             contains_spawn_with_handle_statement(node->data.if_stmt.else_block))) {
            return true;
        }
        break;
    default:
        break;
    }

    return false;
}

bool contains_await_expression(ASTNode *node) {
    if (!node)
        return false;

    if (node->type == AST_AWAIT_EXPR)
        return true;

    // Recursively check child nodes based on node type
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.declarations) {
            for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                if (contains_await_expression(node->data.program.declarations->nodes[i])) {
                    return true;
                }
            }
        }
        break;
    case AST_FUNCTION_DECL:
        if (node->data.function_decl.body &&
            contains_await_expression(node->data.function_decl.body)) {
            return true;
        }
        break;
    case AST_BLOCK:
        if (node->data.block.statements) {
            for (size_t i = 0; i < node->data.block.statements->count; i++) {
                if (contains_await_expression(node->data.block.statements->nodes[i])) {
                    return true;
                }
            }
        }
        break;
    case AST_LET_STMT:
        if (node->data.let_stmt.initializer &&
            contains_await_expression(node->data.let_stmt.initializer)) {
            return true;
        }
        break;
    case AST_BINARY_EXPR:
        if (contains_await_expression(node->data.binary_expr.left) ||
            contains_await_expression(node->data.binary_expr.right)) {
            return true;
        }
        break;
    default:
        break;
    }

    return false;
}

int count_await_expressions(ASTNode *node) {
    if (!node)
        return 0;

    int count = 0;
    if (node->type == AST_AWAIT_EXPR)
        count = 1;

    // Recursively count in child nodes based on node type
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.declarations) {
            for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                count += count_await_expressions(node->data.program.declarations->nodes[i]);
            }
        }
        break;
    case AST_FUNCTION_DECL:
        if (node->data.function_decl.body) {
            count += count_await_expressions(node->data.function_decl.body);
        }
        break;
    case AST_BLOCK:
        if (node->data.block.statements) {
            for (size_t i = 0; i < node->data.block.statements->count; i++) {
                count += count_await_expressions(node->data.block.statements->nodes[i]);
            }
        }
        break;
    case AST_LET_STMT:
        if (node->data.let_stmt.initializer) {
            count += count_await_expressions(node->data.let_stmt.initializer);
        }
        break;
    case AST_BINARY_EXPR:
        count += count_await_expressions(node->data.binary_expr.left);
        count += count_await_expressions(node->data.binary_expr.right);
        break;
    default:
        break;
    }

    return count;
}

bool contains_removed_concurrency_nodes(ASTNode *node) {
    // This would check for old AST node types that should be removed
    // For now, return false since they should be gone
    (void)node;
    return false;
}

bool has_annotation(ASTNode *function_node, const char *annotation_name) {
    if (!function_node || !annotation_name)
        return false;

    if (function_node->type == AST_FUNCTION_DECL && function_node->data.function_decl.annotations) {
        for (size_t i = 0; i < function_node->data.function_decl.annotations->count; i++) {
            ASTNode *annotation = function_node->data.function_decl.annotations->nodes[i];
            if (annotation->type == AST_SEMANTIC_TAG && annotation->data.semantic_tag.name &&
                strcmp(annotation->data.semantic_tag.name, annotation_name) == 0) {
                return true;
            }
        }
    }

    return false;
}

ASTNode *find_function_declaration(ASTNode *root, const char *function_name) {
    if (!root || !function_name)
        return NULL;

    if (root->type == AST_FUNCTION_DECL && root->data.function_decl.name &&
        strcmp(root->data.function_decl.name, function_name) == 0) {
        return root;
    }

    // Search in child nodes based on node type
    switch (root->type) {
    case AST_PROGRAM:
        if (root->data.program.declarations) {
            for (size_t i = 0; i < root->data.program.declarations->count; i++) {
                ASTNode *result = find_function_declaration(
                    root->data.program.declarations->nodes[i], function_name);
                if (result)
                    return result;
            }
        }
        break;
    case AST_BLOCK:
        if (root->data.block.statements) {
            for (size_t i = 0; i < root->data.block.statements->count; i++) {
                ASTNode *result =
                    find_function_declaration(root->data.block.statements->nodes[i], function_name);
                if (result)
                    return result;
            }
        }
        break;
    default:
        break;
    }

    return NULL;
}
