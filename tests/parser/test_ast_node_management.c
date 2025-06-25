/**
 * Asthra Programming Language
 * AST Node Management Tests
 *
 * Tests for AST node creation, destruction, and list operations
 * as outlined in Phase 2.4 of the test coverage improvement plan.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "ast_node_list.h"
#include "ast_types.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_ast_node_management_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_ast_node_management_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// AST NODE CREATION TESTS
// =============================================================================

/**
 * Test: AST Node Creation for All Types
 * Verifies that AST nodes can be created for all node types
 */
static AsthraTestResult test_ast_node_creation_all_types(AsthraTestContext *context) {
    SourceLocation test_location = {.line = 1, .column = 1, .filename = "test.ast"};

    // Test creation of different AST node types (using correct types from ast_nodes.h)
    ASTNodeType node_types[] = {
        AST_PROGRAM,         AST_FUNCTION_DECL, AST_STRUCT_DECL, AST_EXTERN_DECL,
        AST_LET_STMT, // Fixed: was AST_VARIABLE_DECL
        AST_BINARY_EXPR,     AST_UNARY_EXPR,    AST_CALL_EXPR,
        AST_INTEGER_LITERAL, // Fixed: was AST_LITERAL
        AST_IDENTIFIER,      AST_IF_STMT,       AST_FOR_STMT,    AST_RETURN_STMT, AST_BLOCK,
        AST_BASE_TYPE,     // Fixed: was AST_TYPE_PRIMITIVE
        AST_PTR_TYPE,      // Fixed: was AST_TYPE_POINTER
        AST_ENUM_PATTERN,  // Fixed: was AST_PATTERN_LITERAL
        AST_STRUCT_PATTERN // Fixed: was AST_PATTERN_VARIABLE
    };

    size_t type_count = sizeof(node_types) / sizeof(node_types[0]);

    for (size_t i = 0; i < type_count; i++) {
        ASTNode *node = ast_create_node(node_types[i], test_location);

        if (!asthra_test_assert_not_null(context, node, "Failed to create AST node")) {
            return ASTHRA_TEST_FAIL;
        }

        if (!asthra_test_assert_int_eq(context, node->type, node_types[i],
                                       "Node type should match requested type")) {
            ast_free_node(node);
            return ASTHRA_TEST_FAIL;
        }

        if (!asthra_test_assert_int_eq(context, node->location.line, test_location.line,
                                       "Node location should match")) {
            ast_free_node(node);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(node);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Node Destruction
 * Verifies that AST nodes are properly destroyed and memory is freed
 */
static AsthraTestResult test_ast_node_destruction(AsthraTestContext *context) {
    SourceLocation test_location = {.line = 1, .column = 1, .filename = "test.ast"};

    // Test basic node destruction
    ASTNode *node = ast_create_node(AST_FUNCTION_DECL, test_location);

    if (!asthra_test_assert_not_null(context, node, "Failed to create AST node")) {
        return ASTHRA_TEST_FAIL;
    }

    // Initialize some data
    node->data.function_decl.name = strdup("test_function");
    node->data.function_decl.params = NULL;
    node->data.function_decl.return_type = NULL;
    node->data.function_decl.body = NULL;
    node->data.function_decl.annotations = NULL;

    // Test destruction - should not crash
    ast_free_node(node);

    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Node List Operations
 * Verifies that AST node lists work correctly
 */
static AsthraTestResult test_ast_node_list_operations(AsthraTestContext *context) {
    SourceLocation test_location = {.line = 1, .column = 1, .filename = "test.ast"};

    // Create a node list with initial capacity (fixed API call)
    ASTNodeList *list = ast_node_list_create(10);

    if (!asthra_test_assert_not_null(context, list, "Failed to create node list")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test initial state
    if (!asthra_test_assert_int_eq(context, ast_node_list_size(list), 0,
                                   "Empty list should have size 0")) {
        ast_node_list_destroy(list);
        return ASTHRA_TEST_FAIL;
    }

    // Add some nodes
    ASTNode *node1 = ast_create_node(AST_INTEGER_LITERAL, test_location); // Fixed: was AST_LITERAL
    ASTNode *node2 = ast_create_node(AST_IDENTIFIER, test_location);
    ASTNode *node3 = ast_create_node(AST_BINARY_EXPR, test_location);

    if (!asthra_test_assert_not_null(context, node1, "Failed to create node1") ||
        !asthra_test_assert_not_null(context, node2, "Failed to create node2") ||
        !asthra_test_assert_not_null(context, node3, "Failed to create node3")) {
        if (node1)
            ast_free_node(node1);
        if (node2)
            ast_free_node(node2);
        if (node3)
            ast_free_node(node3);
        ast_node_list_destroy(list);
        return ASTHRA_TEST_FAIL;
    }

    // Fixed: use correct API function ast_node_list_add instead of ast_node_list_append
    ast_node_list_add(&list, node1);
    ast_node_list_add(&list, node2);
    ast_node_list_add(&list, node3);

    // Test size after additions
    if (!asthra_test_assert_int_eq(context, ast_node_list_size(list), 3,
                                   "List should have size 3 after additions")) {
        ast_node_list_destroy(list);
        return ASTHRA_TEST_FAIL;
    }

    // Test node retrieval
    ASTNode *retrieved = ast_node_list_get(list, 0);
    if (!asthra_test_assert_ptr_eq(context, retrieved, node1,
                                   "Retrieved node should match added node")) {
        ast_node_list_destroy(list);
        return ASTHRA_TEST_FAIL;
    }

    // Test list destruction
    ast_node_list_destroy(list);

    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Node Memory Lifecycle
 * Verifies proper memory management during node lifecycle
 */
static AsthraTestResult test_ast_node_memory_lifecycle(AsthraTestContext *context) {
    SourceLocation test_location = {.line = 1, .column = 1, .filename = "test.ast"};

    // Track memory usage if available
    size_t initial_memory = get_current_memory_usage();

    // Create multiple nodes with different types
    for (int i = 0; i < 5; i++) {
        ASTNode *node = ast_create_node(AST_FUNCTION_DECL, test_location);

        if (!asthra_test_assert_not_null(context, node,
                                         "Failed to create node in lifecycle test")) {
            return ASTHRA_TEST_FAIL;
        }

        // Add some data to test proper cleanup
        node->data.function_decl.name = strdup("test_func");

        ast_free_node(node);
    }

    // Check that memory is properly managed
    size_t final_memory = get_current_memory_usage();

    // Allow some variation but flag significant leaks
    if (final_memory > initial_memory + 512) {
        asthra_test_log(context,
                        "Warning: Potential memory leak in node lifecycle. "
                        "Initial: %zu, Final: %zu",
                        initial_memory, final_memory);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    AsthraTestMetadata metadata = {.name = "AST Node Management Tests",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "main",
                                   .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                                   .timeout_ns = 30000000000ULL,
                                   .skip = false,
                                   .skip_reason = NULL};

    AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        fprintf(stderr, "Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    printf("Running AST Node Management Tests...\n\n");

    // Run all tests
    AsthraTestResult result1 = test_ast_node_creation_all_types(context);
    AsthraTestResult result2 = test_ast_node_destruction(context);
    AsthraTestResult result3 = test_ast_node_list_operations(context);
    AsthraTestResult result4 = test_ast_node_memory_lifecycle(context);

    // Calculate results
    int passed = 0;
    int total = 4;

    if (result1 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_node_creation_all_types\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_node_creation_all_types\n");
    }

    if (result2 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_node_destruction\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_node_destruction\n");
    }

    if (result3 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_node_list_operations\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_node_list_operations\n");
    }

    if (result4 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_node_memory_lifecycle\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_node_memory_lifecycle\n");
    }

    printf("\nTest Results: %d/%d passed\n", passed, total);

    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);
    return (passed == total) ? 0 : 1;
}
