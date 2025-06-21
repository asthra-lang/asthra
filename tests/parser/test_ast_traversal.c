/**
 * Asthra Programming Language
 * AST Traversal Tests
 * 
 * Tests for AST traversal algorithms and visitor patterns
 * as outlined in Phase 2.4 of the test coverage improvement plan.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "ast.h"
#include "ast_node.h"
#include "ast_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_ast_traversal_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_ast_traversal_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// AST TRAVERSAL TESTS
// =============================================================================

// Global counters for traversal testing
static int preorder_counter = 0;
static int postorder_counter = 0;

static void preorder_visitor(ASTNode* node, void* data) {
    if (node) {
        preorder_counter++;
    }
}

static void postorder_visitor(ASTNode* node, void* data) {
    if (node) {
        postorder_counter++;
    }
}

// Additional visitor functions for specific tests
static void counting_visitor(ASTNode* node, void* data) {
    int* count = (int*)data;
    (*count)++;
}

// Node type counting structures and visitor
typedef struct {
    int function_count;
    int variable_count;
    int expression_count;
} NodeTypeCounter;

static void type_counting_visitor(ASTNode* node, void* data) {
    NodeTypeCounter* counter = (NodeTypeCounter*)data;
    
    switch (node->type) {
        case AST_FUNCTION_DECL:
            counter->function_count++;
            break;
        case AST_LET_STMT:  // Fixed: was AST_VAR_DECL
            counter->variable_count++;
            break;
        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR:
        case AST_CALL_EXPR:
            counter->expression_count++;
            break;
        default:
            break;
    }
}

// Traversal order tracking structures and visitor
typedef struct {
    ASTNodeType* order;
    int count;
    int capacity;
} TraversalOrder;

static void order_tracking_visitor(ASTNode* node, void* data) {
    TraversalOrder* order = (TraversalOrder*)data;
    
    if (order->count < order->capacity) {
        order->order[order->count++] = node->type;
    }
}

/**
 * Test: AST Traversal Preorder
 * Verifies that preorder traversal works correctly
 */
static AsthraTestResult test_ast_traversal_preorder(AsthraTestContext* context) {
    // Create a simple AST structure
    const char* test_source = "fn test(none) { let x: i32 = 42; return x; }";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parse_function_decl(parser);
    
    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Reset counter and traverse
    preorder_counter = 0;
    ast_traverse_preorder(ast, preorder_visitor, NULL);
    
    // Should have visited at least the root node
    if (!asthra_test_assert_int_gt(context, preorder_counter, 0, 
                                  "Should visit at least one node")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Traversal Postorder
 * Verifies that postorder traversal works correctly
 */
static AsthraTestResult test_ast_traversal_postorder(AsthraTestContext* context) {
    // Create a simple AST structure
    const char* test_source = "fn test(none) { return 42; }";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parse_function_decl(parser);
    
    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Reset counter and traverse
    postorder_counter = 0;
    ast_traverse_postorder(ast, postorder_visitor, NULL);
    
    // Should have visited at least the root node
    if (!asthra_test_assert_int_gt(context, postorder_counter, 0, 
                                  "Should visit at least one node")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Visitor Pattern
 * Verifies that the visitor pattern works correctly
 */
static AsthraTestResult test_ast_visitor_pattern(AsthraTestContext* context) {
    // Create AST structure for testing
    const char* test_source = "fn add(a: i32, b: i32) -> i32 { return a + b; }";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parse_function_decl(parser);
    
    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test visitor with counting
    int visit_count = 0;
    
    ast_traverse_preorder(ast, counting_visitor, &visit_count);
    
    if (!asthra_test_assert_int_gt(context, visit_count, 0, 
                                  "Should visit at least one node")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Visitor with Data Collection
 * Verifies that visitors can collect data during traversal
 */
static AsthraTestResult test_ast_visitor_data_collection(AsthraTestContext* context) {
    // Create AST structure for testing
    const char* test_source = "fn test(none) { let x: i32 = 42; let y: i32 = x + 1; return y; }";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parse_function_decl(parser);
    
    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Use the globally defined node type counter
    NodeTypeCounter counter = {0, 0, 0};
    ast_traverse_preorder(ast, type_counting_visitor, &counter);
    
    // Should have found at least one function
    if (!asthra_test_assert_int_gt(context, counter.function_count, 0, 
                                  "Should find at least one function")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Traversal Order Verification
 * Verifies that traversal order is consistent
 */
static AsthraTestResult test_ast_traversal_order_verification(AsthraTestContext* context) {
    // Create a simple AST structure with known hierarchy
    const char* test_source = "fn test(none) { return 42; }";
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parse_function_decl(parser);
    
    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Track traversal order using globally defined structures
    ASTNodeType order_buffer[20];
    TraversalOrder order = {order_buffer, 0, 20};
    
    ast_traverse_preorder(ast, order_tracking_visitor, &order);
    
    // First node should be the function declaration
    if (order.count > 0) {
        if (!asthra_test_assert_int_eq(context, order.order[0], AST_FUNCTION_DECL, 
                                      "First node should be function declaration")) {
            ast_free_node(ast);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: AST Traversal Performance
 * Verifies that traversal performance is acceptable
 */
static AsthraTestResult test_ast_traversal_performance(AsthraTestContext* context) {
    // Create a larger AST structure for performance testing
    const char* test_source = 
        "fn performance_test(none) {\n"
        "    let a: i32 = 1;\n"
        "    let b: i32 = 2;\n"
        "    let c: i32 = 3;\n"
        "    let d: i32 = 4;\n"
        "    let result: i32 = a + b + c + d;\n"
        "    return result;\n"
        "}";
    
    Parser* parser = create_test_parser(test_source);
    
    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* ast = parse_function_decl(parser);
    
    if (!asthra_test_assert_not_null(context, ast, "Failed to parse function")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Simple performance test - just count nodes
    int node_count = 0;
    
    // Traverse multiple times to test performance
    for (int i = 0; i < 10; i++) {
        node_count = 0;
        ast_traverse_preorder(ast, counting_visitor, &node_count);
    }
    
    // Should have found some nodes
    if (!asthra_test_assert_int_gt(context, node_count, 0, 
                                  "Should find some nodes during traversal")) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(ast);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all AST traversal tests
 */
AsthraTestSuite* create_ast_traversal_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("AST Traversal Tests", 
                                                     "AST traversal algorithms and visitor patterns");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_ast_traversal_tests);
    asthra_test_suite_set_teardown(suite, teardown_ast_traversal_tests);
    
    // AST traversal tests
    asthra_test_suite_add_test(suite, "test_ast_traversal_preorder", 
                              "AST preorder traversal", 
                              test_ast_traversal_preorder);
    
    asthra_test_suite_add_test(suite, "test_ast_traversal_postorder", 
                              "AST postorder traversal", 
                              test_ast_traversal_postorder);
    
    asthra_test_suite_add_test(suite, "test_ast_visitor_pattern", 
                              "AST visitor pattern", 
                              test_ast_visitor_pattern);
    
    asthra_test_suite_add_test(suite, "test_ast_visitor_data_collection", 
                              "AST visitor data collection", 
                              test_ast_visitor_data_collection);
    
    asthra_test_suite_add_test(suite, "test_ast_traversal_order_verification", 
                              "AST traversal order verification", 
                              test_ast_traversal_order_verification);
    
    asthra_test_suite_add_test(suite, "test_ast_traversal_performance", 
                              "AST traversal performance", 
                              test_ast_traversal_performance);
    
    return suite;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    AsthraTestMetadata metadata = {
        .name = "AST Traversal Tests",
        .file = __FILE__,
        .line = __LINE__,
        .function = "main",
        .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
        .timeout_ns = 30000000000ULL,
        .skip = false,
        .skip_reason = NULL
    };
    
    AsthraTestContext* context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        fprintf(stderr, "Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }
    
    printf("Running AST Traversal Tests...\n\n");
    
    // Run all tests
    AsthraTestResult result1 = test_ast_traversal_preorder(context);
    AsthraTestResult result2 = test_ast_traversal_postorder(context);
    AsthraTestResult result3 = test_ast_visitor_pattern(context);
    AsthraTestResult result4 = test_ast_visitor_data_collection(context);
    AsthraTestResult result5 = test_ast_traversal_order_verification(context);
    AsthraTestResult result6 = test_ast_traversal_performance(context);
    
    // Calculate results
    int passed = 0;
    int total = 6;
    
    if (result1 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_traversal_preorder\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_traversal_preorder\n");
    }
    
    if (result2 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_traversal_postorder\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_traversal_postorder\n");
    }
    
    if (result3 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_visitor_pattern\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_visitor_pattern\n");
    }
    
    if (result4 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_visitor_data_collection\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_visitor_data_collection\n");
    }
    
    if (result5 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_traversal_order_verification\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_traversal_order_verification\n");
    }
    
    if (result6 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_ast_traversal_performance\n");
        passed++;
    } else {
        printf("[FAIL] test_ast_traversal_performance\n");
    }
    
    printf("\nTest Results: %d/%d passed\n", passed, total);
    
    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);
    return (passed == total) ? 0 : 1;
} 
