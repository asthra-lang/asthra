/**
 * Advanced pub/impl/self Parser Tests
 * 
 * This module tests advanced parsing functionality for pub, impl, and self
 * language constructs including nested impl blocks and visibility modifiers.
 * 
 * Test Coverage:
 * - Nested impl blocks with complex structures
 * - Visibility modifier parsing and validation
 * - Complex method declarations
 * - Program-level parsing with multiple declarations
 * 
 * Copyright (c) 2024 Asthra Project
 */

#include "test_pub_impl_self_common.h"

// =============================================================================
// ADVANCED PARSING TESTS
// =============================================================================

static AsthraTestResult test_parse_nested_impl_blocks(AsthraTestContext* context) {
    const char* source = 
        "priv struct Point { x: f64, y: f64 }\n"
        "impl Point {\n"
        "    pub fn new(x: f64, y: f64) -> Point {\n"
        "        return Point { x: x, y: y };\n"
        "    }\n"
        "    \n"
        "    priv fn distance(self) -> f64 {\n"
        "        return sqrt(self.x * self.x + self.y * self.y);\n"
        "    }\n"
        "    \n"
        "    priv fn magnitude(self) -> f64 {\n"
        "        return self.distance();\n"
        "    }\n"
        "}";
    
    Parser* parser = create_test_parser(source);
    
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* program = parser_parse_program(parser);
    
    if (!ASTHRA_TEST_ASSERT(context, program != NULL, "Program should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, program->type == AST_PROGRAM, "Node should be AST_PROGRAM")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that we have both struct and impl declarations
    if (!ASTHRA_TEST_ASSERT(context, program->data.program.declarations != NULL, 
                           "Program should have declarations")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    size_t decl_count = ast_node_list_size(program->data.program.declarations);
    if (!ASTHRA_TEST_ASSERT(context, decl_count == 2, "Program should have 2 declarations (struct + impl)")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_parse_visibility_modifiers(AsthraTestContext* context) {
    const char* source = 
        "pub struct Point {\n"
        "    pub x: f64,\n"
        "    priv y: f64\n"  // explicit private
        "}\n"
        "impl Point {\n"
        "    pub fn new(none) -> Point { }\n"     // public method
        "    priv fn internal_helper(none) -> f64 { 0.0 }\n"  // private method
        "}";
    
    Parser* parser = create_test_parser(source);
    
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* program = parser_parse_program(parser);
    
    if (!ASTHRA_TEST_ASSERT(context, program != NULL, "Program should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Validate struct visibility
    ASTNode* struct_decl = ast_node_list_get(program->data.program.declarations, 0);
    if (!ASTHRA_TEST_ASSERT(context, struct_decl->data.struct_decl.visibility == VISIBILITY_PUBLIC,
                           "Struct should be public")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    // Validate field visibilities
    ASTNode* first_field = ast_node_list_get(struct_decl->data.struct_decl.fields, 0);
    if (!ASTHRA_TEST_ASSERT(context, first_field->data.field_decl.visibility == VISIBILITY_PUBLIC,
                           "First field should be public")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ASTNode* second_field = ast_node_list_get(struct_decl->data.struct_decl.fields, 1);
    if (!ASTHRA_TEST_ASSERT(context, second_field->data.field_decl.visibility == VISIBILITY_PRIVATE,
                           "Second field should be private")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }
    
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST EXECUTION
// =============================================================================

int main(void) {
    printf("Advanced pub/impl/self Parser Tests\n");
    printf("===================================\n\n");
    
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;
    
    struct {
        const char* name;
        AsthraTestResult (*test_func)(AsthraTestContext*);
    } tests[] = {
        {"Parse nested impl blocks", test_parse_nested_impl_blocks},
        {"Parse visibility modifiers", test_parse_visibility_modifiers},
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
    
    printf("\n=== Advanced Parser Test Summary ===\n");
    printf("Total tests: %zu\n", test_count);
    printf("Assertions checked: %llu\n", stats->assertions_checked);
    printf("Assertions failed: %llu\n", stats->assertions_failed);
    
    asthra_test_statistics_destroy(stats);
    
    if (overall_result == ASTHRA_TEST_PASS) {
        printf("✅ All advanced parser tests passed!\n");
        return 0;
    } else {
        printf("❌ Some advanced parser tests failed!\n");
        return 1;
    }
} 
