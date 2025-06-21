/**
 * Basic pub/impl/self Parser Tests
 * 
 * This module tests fundamental parsing functionality for pub, impl, and self
 * language constructs including struct declarations, impl blocks, and self parameters.
 * 
 * Test Coverage:
 * - Public struct parsing
 * - Impl block parsing  
 * - Self parameter parsing
 * - Associated function call parsing
 * 
 * Copyright (c) 2024 Asthra Project
 */

#include "test_pub_impl_self_common.h"

// =============================================================================
// BASIC PARSING TESTS
// =============================================================================

static AsthraTestResult test_parse_pub_struct(AsthraTestContext* context) {
    const char* source = "pub struct Point { pub x: f64, y: f64 }";
    Parser* parser = create_test_parser(source);
    
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* struct_decl = parser_parse_struct_declaration(parser);
    
    if (!ASTHRA_TEST_ASSERT(context, struct_decl != NULL, "Struct declaration should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, struct_decl->type == AST_STRUCT_DECL, "Node should be AST_STRUCT_DECL")) {
        ast_free_node(struct_decl);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check visibility
    if (!ASTHRA_TEST_ASSERT(context, struct_decl->data.struct_decl.visibility == VISIBILITY_PUBLIC, 
                           "Struct should have public visibility")) {
        ast_free_node(struct_decl);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check struct name
    if (!ASTHRA_TEST_ASSERT(context, strcmp(struct_decl->data.struct_decl.name, "Point") == 0, 
                           "Struct name should be 'Point'")) {
        ast_free_node(struct_decl);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(struct_decl);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_parse_impl_block(AsthraTestContext* context) {
    const char* source = 
        "impl Point {\n"
        "    pub fn new(x: f64, y: f64) -> Point { void }\n"
        "    priv fn distance(self) -> f64 { void }\n"
        "}";
    
    Parser* parser = create_test_parser(source);
    
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* impl_block = parser_parse_impl_declaration(parser);
    
    if (!ASTHRA_TEST_ASSERT(context, impl_block != NULL, "Impl block should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, impl_block->type == AST_IMPL_BLOCK, "Node should be AST_IMPL_BLOCK")) {
        ast_free_node(impl_block);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check struct name
    if (!ASTHRA_TEST_ASSERT(context, strcmp(impl_block->data.impl_block.struct_name, "Point") == 0, 
                           "Impl block should be for 'Point'")) {
        ast_free_node(impl_block);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check methods count
    if (!ASTHRA_TEST_ASSERT(context, impl_block->data.impl_block.methods != NULL, 
                           "Impl block should have methods")) {
        ast_free_node(impl_block);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t method_count = ast_node_list_size(impl_block->data.impl_block.methods);
    if (!ASTHRA_TEST_ASSERT(context, method_count == 2, "Impl block should have 2 methods")) {
        ast_free_node(impl_block);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(impl_block);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_parse_self_parameter(AsthraTestContext* context) {
    const char* source = "priv fn distance(self, other: Point) -> f64 { void }";
    Parser* parser = create_test_parser(source);
    
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* method = parser_parse_function_declaration(parser);
    
    if (!ASTHRA_TEST_ASSERT(context, method != NULL, "Method should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, method->type == AST_METHOD_DECL, "Node should be AST_METHOD_DECL")) {
        ast_free_node(method);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check parameters
    if (!ASTHRA_TEST_ASSERT(context, method->data.method_decl.parameters != NULL, 
                           "Method should have parameters")) {
        ast_free_node(method);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t param_count = ast_node_list_size(method->data.method_decl.parameters);
    if (!ASTHRA_TEST_ASSERT(context, param_count == 2, "Method should have 2 parameters")) {
        ast_free_node(method);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check first parameter is self
    ASTNode* first_param = ast_node_list_get(method->data.method_decl.parameters, 0);
    if (!ASTHRA_TEST_ASSERT(context, first_param->data.parameter.is_self, 
                           "First parameter should be self")) {
        ast_free_node(method);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(method);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_parse_associated_function_call(AsthraTestContext* context) {
    const char* source = "Point::new(3.0, 4.0)";
    Parser* parser = create_test_parser(source);
    
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* call_expr = parser_parse_expression(parser);
    
    if (!ASTHRA_TEST_ASSERT(context, call_expr != NULL, "Associated function call should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, call_expr->type == AST_ASSOCIATED_FUNC_CALL, 
                           "Node should be AST_ASSOCIATED_FUNC_CALL")) {
        ast_free_node(call_expr);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check struct name and function name
    if (!ASTHRA_TEST_ASSERT(context, strcmp(call_expr->data.associated_func_call.struct_name, "Point") == 0, 
                           "Struct name should be 'Point'")) {
        ast_free_node(call_expr);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, strcmp(call_expr->data.associated_func_call.function_name, "new") == 0, 
                           "Function name should be 'new'")) {
        ast_free_node(call_expr);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(call_expr);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST EXECUTION
// =============================================================================

int main(void) {
    printf("Basic pub/impl/self Parser Tests\n");
    printf("================================\n\n");
    
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;
    
    struct {
        const char* name;
        AsthraTestResult (*test_func)(AsthraTestContext*);
    } tests[] = {
        {"Parse pub struct", test_parse_pub_struct},
        {"Parse impl block", test_parse_impl_block},
        {"Parse self parameter", test_parse_self_parameter},
        {"Parse associated function call", test_parse_associated_function_call},
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    for (size_t i = 0; i < test_count; i++) {
        AsthraTestMetadata metadata = pub_impl_self_base_metadata;
        metadata.name = tests[i].name;
        metadata.line = __LINE__;
        
        AsthraTestContext* context = asthra_test_context_create(&metadata, stats);
        if (!context) {
            printf("❌ Failed to create test context for '%s'\n", tests[i].name);
            overall_result = ASTHRA_TEST_FAIL;
            continue;
        }
        
        asthra_test_context_start(context);
        AsthraTestResult result = tests[i].test_func(context);
        asthra_test_context_end(context, result);
        
        if (result == ASTHRA_TEST_PASS) {
            printf("✅ %s: PASS\n", tests[i].name);
        } else {
            printf("❌ %s: FAIL", tests[i].name);
            if (context->error_message) {
                printf(" - %s", context->error_message);
            }
            printf("\n");
            overall_result = ASTHRA_TEST_FAIL;
        }
        
        asthra_test_context_destroy(context);
    }
    
    printf("\n=== Basic Parser Test Summary ===\n");
    printf("Total tests: %zu\n", test_count);
    printf("Assertions checked: %llu\n", stats->assertions_checked);
    printf("Assertions failed: %llu\n", stats->assertions_failed);
    
    asthra_test_statistics_destroy(stats);
    
    if (overall_result == ASTHRA_TEST_PASS) {
        printf("✅ All basic parser tests passed!\n");
        return 0;
    } else {
        printf("❌ Some basic parser tests failed!\n");
        return 1;
    }
} 
