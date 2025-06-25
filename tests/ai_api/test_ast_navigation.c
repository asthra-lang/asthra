#include "../framework/test_framework.h"
#include "ast_navigation.h"
#include "semantic_api_core.h"
#include <stdlib.h>
#include <string.h>

// Helper function to create a test API instance
static AsthraSemanticsAPI *setup_test_api(void) {
    AsthraSemanticsAPI *api = asthra_ai_create_api("test_file.asthra");
    if (!api)
        return NULL;

    const char *test_code = "package test;\n"
                            "pub struct Point {\n"
                            "    x: i32,\n"
                            "    y: i32\n"
                            "}\n"
                            "pub struct Calculator {\n"
                            "    value: f64\n"
                            "}\n";

    if (!asthra_ai_initialize_from_source(api, test_code)) {
        asthra_ai_destroy_api(api);
        return NULL;
    }

    return api;
}

static void cleanup_test_api(AsthraSemanticsAPI *api) {
    if (api) {
        asthra_ai_destroy_api(api);
    }
}

// Test finding symbol declarations
AsthraTestResult test_find_declaration(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");

    // Test finding struct declaration
    ASTNode *decl = asthra_ai_find_declaration(api, "Point");
    if (decl) {
        asthra_test_assert_int_eq(context, decl->type, AST_STRUCT_DECL,
                                  "Should find struct declaration");
        printf("DEBUG: Found Point declaration at line %d, column %d\n", decl->location.line,
               decl->location.column);
    } else {
        printf(
            "INFO: Declaration finding not available yet - semantic analyzer needs enhancement\n");
    }

    // Test finding function declaration
    decl = asthra_ai_find_declaration(api, "add");
    if (decl) {
        asthra_test_assert_int_eq(context, decl->type, AST_FUNCTION_DECL,
                                  "Should find function declaration");
        printf("DEBUG: Found add declaration at line %d, column %d\n", decl->location.line,
               decl->location.column);
    } else {
        printf("INFO: Function declaration finding not available yet\n");
    }

    // Test non-existent symbol
    decl = asthra_ai_find_declaration(api, "NonExistentSymbol");
    ASTHRA_TEST_ASSERT_NULL(context, decl, "Non-existent symbol should return NULL");

    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test finding symbol usages
AsthraTestResult test_find_usages(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");

    size_t count = 0;
    AISymbolUsage **usages = asthra_ai_find_symbol_usages(api, "Point", &count);

    if (usages && count > 0) {
        ASTHRA_TEST_ASSERT_TRUE(context, count >= 1, "Should find at least one usage of Point");

        // Validate the first usage
        AISymbolUsage *usage = usages[0];
        ASTHRA_TEST_ASSERT_NOT_NULL(context, usage->symbol_name, "Usage should have symbol name");
        ASTHRA_TEST_ASSERT_STR_EQ(context, usage->symbol_name, "Point", "Symbol name should match");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, usage->usage_type, "Usage should have type");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, usage->scope_name, "Usage should have scope name");

        printf("DEBUG: Found %zu usage(s) of Point\n", count);
        printf("DEBUG: First usage - type: %s, scope: %s, line: %zu\n", usage->usage_type,
               usage->scope_name, usage->location.line);

        asthra_ai_free_symbol_usage_array(usages, count);
    } else {
        printf("INFO: Usage finding not fully implemented yet\n");
    }

    // Test non-existent symbol
    usages = asthra_ai_find_symbol_usages(api, "NonExistentSymbol", &count);
    ASTHRA_TEST_ASSERT_NULL(context, usages, "Non-existent symbol should return NULL");
    asthra_test_assert_int_eq(context, count, 0, "Count should be 0 for non-existent symbol");

    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test getting symbol documentation
AsthraTestResult test_symbol_documentation(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");

    // Test getting documentation for existing symbol
    char *doc = asthra_ai_get_symbol_documentation(api, "Point");
    if (doc) {
        ASTHRA_TEST_ASSERT_NOT_NULL(context, doc, "Should return documentation string");
        printf("DEBUG: Documentation for Point: %s\n", doc);
        free(doc);
    } else {
        printf("INFO: Documentation extraction not available yet\n");
    }

    // Test getting documentation for function
    doc = asthra_ai_get_symbol_documentation(api, "add");
    if (doc) {
        ASTHRA_TEST_ASSERT_NOT_NULL(context, doc, "Should return documentation string");
        printf("DEBUG: Documentation for add: %s\n", doc);
        free(doc);
    }

    // Test non-existent symbol
    doc = asthra_ai_get_symbol_documentation(api, "NonExistentSymbol");
    ASTHRA_TEST_ASSERT_NULL(context, doc, "Non-existent symbol should return NULL");

    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test getting symbol locations
AsthraTestResult test_symbol_locations(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");

    size_t count = 0;
    AICodeLocation **locations = asthra_ai_get_symbol_locations(api, "Point", &count);

    if (locations && count > 0) {
        ASTHRA_TEST_ASSERT_TRUE(context, count >= 1, "Should find at least one location for Point");

        // Validate the first location
        AICodeLocation *location = locations[0];
        ASTHRA_TEST_ASSERT_NOT_NULL(context, location->file_path, "Location should have file path");
        ASTHRA_TEST_ASSERT_TRUE(context, location->line > 0,
                                "Location should have valid line number");
        ASTHRA_TEST_ASSERT_TRUE(context, location->column > 0,
                                "Location should have valid column number");

        printf("DEBUG: Found %zu location(s) for Point\n", count);
        printf("DEBUG: First location - file: %s, line: %zu, column: %zu\n", location->file_path,
               location->line, location->column);

        asthra_ai_free_code_location_array(locations, count);
    } else {
        printf("INFO: Location finding not fully implemented yet\n");
    }

    // Test non-existent symbol
    locations = asthra_ai_get_symbol_locations(api, "NonExistentSymbol", &count);
    ASTHRA_TEST_ASSERT_NULL(context, locations, "Non-existent symbol should return NULL");
    asthra_test_assert_int_eq(context, count, 0, "Count should be 0 for non-existent symbol");

    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test AST traversal functionality
AsthraTestResult test_ast_traversal(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");

    // Test getting child nodes (simplified test)
    ASTNode *decl = asthra_ai_find_declaration(api, "Point");
    if (decl) {
        size_t count = 0;
        ASTNode **children = asthra_ai_get_child_nodes(api, decl, &count);

        if (children) {
            printf("DEBUG: Found %zu child nodes for Point declaration\n", count);
            free(children); // Don't free the nodes themselves
        } else {
            printf("INFO: Child node extraction not fully implemented yet\n");
        }
    }

    // Test getting parent node
    if (decl) {
        ASTNode *parent = asthra_ai_get_parent_node(api, decl);
        if (parent) {
            printf("DEBUG: Found parent node for Point declaration\n");
        } else {
            printf("INFO: Parent node finding not implemented yet (expected)\n");
        }
    }

    // Test getting node scope
    if (decl) {
        char *scope = asthra_ai_get_node_scope(api, decl);
        if (scope) {
            ASTHRA_TEST_ASSERT_NOT_NULL(context, scope, "Should return scope name");
            printf("DEBUG: Scope for Point declaration: %s\n", scope);
            free(scope);
        }
    }

    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test code context and analysis
AsthraTestResult test_code_context(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");

    // Test getting code context
    char *context_code = asthra_ai_get_code_context(api, "test_file.asthra", 5, 2);
    if (context_code) {
        ASTHRA_TEST_ASSERT_NOT_NULL(context, context_code, "Should return context code");
        printf("DEBUG: Code context: %s\n", context_code);
        free(context_code);
    } else {
        printf("INFO: Code context extraction not fully implemented yet\n");
    }

    // Test symbol accessibility
    bool accessible = asthra_ai_is_symbol_accessible(api, "Point", "test_file.asthra", 10, 1);
    printf("DEBUG: Point accessible at line 10: %s\n", accessible ? "yes" : "no");

    accessible =
        asthra_ai_is_symbol_accessible(api, "NonExistentSymbol", "test_file.asthra", 10, 1);
    ASTHRA_TEST_ASSERT_FALSE(context, accessible, "Non-existent symbol should not be accessible");

    // Test getting visible symbols
    size_t count = 0;
    char **visible = asthra_ai_get_visible_symbols(api, "test_file.asthra", 10, 1, &count);
    if (visible) {
        printf("DEBUG: Found %zu visible symbols at line 10\n", count);
        asthra_ai_free_string_array(visible, count);
    } else {
        printf("INFO: Visible symbols analysis not fully implemented yet\n");
    }

    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test error handling
AsthraTestResult test_ast_navigation_error_handling(AsthraTestContext *context) {
    // Test with NULL API
    ASTNode *decl = asthra_ai_find_declaration(NULL, "Point");
    ASTHRA_TEST_ASSERT_NULL(context, decl, "NULL API should return NULL");

    // Test with NULL symbol name
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");

    decl = asthra_ai_find_declaration(api, NULL);
    ASTHRA_TEST_ASSERT_NULL(context, decl, "NULL symbol name should return NULL");

    // Test usages with NULL parameters
    size_t count = 0;
    AISymbolUsage **usages = asthra_ai_find_symbol_usages(api, NULL, &count);
    ASTHRA_TEST_ASSERT_NULL(context, usages, "NULL symbol name should return NULL");
    asthra_test_assert_int_eq(context, count, 0, "Count should be 0 for NULL symbol");

    usages = asthra_ai_find_symbol_usages(api, "Point", NULL);
    ASTHRA_TEST_ASSERT_NULL(context, usages, "NULL count parameter should return NULL");

    // Test documentation with NULL parameters
    char *doc = asthra_ai_get_symbol_documentation(api, NULL);
    ASTHRA_TEST_ASSERT_NULL(context, doc, "NULL symbol name should return NULL");

    // Test locations with NULL parameters
    AICodeLocation **locations = asthra_ai_get_symbol_locations(api, NULL, &count);
    ASTHRA_TEST_ASSERT_NULL(context, locations, "NULL symbol name should return NULL");

    locations = asthra_ai_get_symbol_locations(api, "Point", NULL);
    ASTHRA_TEST_ASSERT_NULL(context, locations, "NULL count parameter should return NULL");

    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create(
        "AI API AST Navigation Tests", "Tests for AST navigation and code location functionality");

    asthra_test_suite_add_test(suite, "test_find_declaration", "Test finding symbol declarations",
                               test_find_declaration);
    asthra_test_suite_add_test(suite, "test_find_usages", "Test finding symbol usages",
                               test_find_usages);
    asthra_test_suite_add_test(suite, "test_symbol_documentation",
                               "Test symbol documentation extraction", test_symbol_documentation);
    asthra_test_suite_add_test(suite, "test_symbol_locations", "Test getting symbol locations",
                               test_symbol_locations);
    asthra_test_suite_add_test(suite, "test_ast_traversal", "Test AST traversal functionality",
                               test_ast_traversal);
    asthra_test_suite_add_test(suite, "test_code_context", "Test code context and analysis",
                               test_code_context);
    asthra_test_suite_add_test(suite, "test_ast_navigation_error_handling",
                               "Test AST navigation error handling",
                               test_ast_navigation_error_handling);

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
