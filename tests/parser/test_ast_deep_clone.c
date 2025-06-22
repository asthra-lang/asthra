/**
 * Asthra Programming Language Compiler
 * Test: AST Deep Cloning
 * 
 * Tests deep cloning functionality for AST nodes, particularly
 * for nodes containing ASTNodeList fields (struct literals, enums, etc.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "parser.h"
#include "ast.h"
#include "ast_node_creation.h"
#include "ast_node_list.h"

// Test deep cloning of struct literal with type args and field inits
static AsthraTestResult test_struct_literal_deep_clone(AsthraTestContext* context) {
    // Create a struct literal node
    ASTNode *original = ast_create_node(AST_STRUCT_LITERAL, (SourceLocation){0, 0});
    ASTHRA_TEST_ASSERT(context, original != NULL, "Failed to create struct literal node");
    
    original->data.struct_literal.struct_name = strdup("Point");
    
    // Add type arguments
    original->data.struct_literal.type_args = ast_node_list_create(2);
    ASTNode *type_arg1 = ast_create_node(AST_BASE_TYPE, (SourceLocation){0, 0});
    type_arg1->data.base_type.name = strdup("i32");
    ast_node_list_add(&original->data.struct_literal.type_args, type_arg1);
    
    // Add field initializations
    original->data.struct_literal.field_inits = ast_node_list_create(2);
    
    // Field init 1: x = 10
    ASTNode *field_init1 = ast_create_node(AST_ASSIGNMENT, (SourceLocation){0, 0});
    field_init1->data.assignment.target = ast_create_node(AST_IDENTIFIER, (SourceLocation){0, 0});
    field_init1->data.assignment.target->data.identifier.name = strdup("x");
    field_init1->data.assignment.value = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0, 0});
    field_init1->data.assignment.value->data.integer_literal.value = 10;
    ast_node_list_add(&original->data.struct_literal.field_inits, field_init1);
    
    // Field init 2: y = 20
    ASTNode *field_init2 = ast_create_node(AST_ASSIGNMENT, (SourceLocation){0, 0});
    field_init2->data.assignment.target = ast_create_node(AST_IDENTIFIER, (SourceLocation){0, 0});
    field_init2->data.assignment.target->data.identifier.name = strdup("y");
    field_init2->data.assignment.value = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0, 0});
    field_init2->data.assignment.value->data.integer_literal.value = 20;
    ast_node_list_add(&original->data.struct_literal.field_inits, field_init2);
    
    // Clone the node
    ASTNode *clone = ast_clone_node(original);
    ASTHRA_TEST_ASSERT(context, clone != NULL, "Failed to clone struct literal node");
    
    // Verify the clone
    asthra_test_assert_int_eq(context, clone->type, AST_STRUCT_LITERAL, "Clone type mismatch");
    asthra_test_assert_string_eq(context, clone->data.struct_literal.struct_name, "Point", "Struct name mismatch");
    
    // Verify type args are deep cloned
    ASTHRA_TEST_ASSERT(context, clone->data.struct_literal.type_args != NULL, "Type args not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.struct_literal.type_args != original->data.struct_literal.type_args, "Type args not deep cloned");
    asthra_test_assert_size_eq(context, ast_node_list_size(clone->data.struct_literal.type_args), (size_t)1, "Type args count mismatch");
    
    ASTNode *cloned_type_arg = ast_node_list_get(clone->data.struct_literal.type_args, 0);
    ASTHRA_TEST_ASSERT(context, cloned_type_arg != NULL, "Cloned type arg is null");
    ASTHRA_TEST_ASSERT(context, cloned_type_arg != type_arg1, "Type arg not deep cloned");
    asthra_test_assert_string_eq(context, cloned_type_arg->data.base_type.name, "i32", "Type arg name mismatch");
    
    // Verify field inits are deep cloned
    ASTHRA_TEST_ASSERT(context, clone->data.struct_literal.field_inits != NULL, "Field inits not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.struct_literal.field_inits != original->data.struct_literal.field_inits, "Field inits not deep cloned");
    asthra_test_assert_size_eq(context, ast_node_list_size(clone->data.struct_literal.field_inits), (size_t)2, "Field inits count mismatch");
    
    // Check first field init
    ASTNode *cloned_field1 = ast_node_list_get(clone->data.struct_literal.field_inits, 0);
    ASTHRA_TEST_ASSERT(context, cloned_field1 != NULL, "First cloned field is null");
    ASTHRA_TEST_ASSERT(context, cloned_field1 != field_init1, "First field not deep cloned");
    asthra_test_assert_int_eq(context, cloned_field1->type, AST_ASSIGNMENT, "First field type mismatch");
    asthra_test_assert_string_eq(context, cloned_field1->data.assignment.target->data.identifier.name, "x", "First field name mismatch");
    asthra_test_assert_long_eq(context, cloned_field1->data.assignment.value->data.integer_literal.value, (int64_t)10, "First field value mismatch");
    
    // Check second field init
    ASTNode *cloned_field2 = ast_node_list_get(clone->data.struct_literal.field_inits, 1);
    ASTHRA_TEST_ASSERT(context, cloned_field2 != NULL, "Second cloned field is null");
    ASTHRA_TEST_ASSERT(context, cloned_field2 != field_init2, "Second field not deep cloned");
    asthra_test_assert_int_eq(context, cloned_field2->type, AST_ASSIGNMENT, "Second field type mismatch");
    asthra_test_assert_string_eq(context, cloned_field2->data.assignment.target->data.identifier.name, "y", "Second field name mismatch");
    asthra_test_assert_long_eq(context, cloned_field2->data.assignment.value->data.integer_literal.value, (int64_t)20, "Second field value mismatch");
    
    // Clean up
    ast_free_node(original);
    ast_free_node(clone);
    
    return ASTHRA_TEST_PASS;
}

// Test deep cloning of enum declaration
static AsthraTestResult test_enum_decl_deep_clone(AsthraTestContext* context) {
    // Create an enum declaration node
    ASTNode *original = ast_create_node(AST_ENUM_DECL, (SourceLocation){0, 0});
    ASTHRA_TEST_ASSERT(context, original != NULL, "Failed to create enum declaration node");
    
    original->data.enum_decl.name = strdup("Option");
    original->data.enum_decl.visibility = VISIBILITY_PUBLIC;
    
    // Add type parameters
    original->data.enum_decl.type_params = ast_node_list_create(1);
    ASTNode *type_param = ast_create_node(AST_IDENTIFIER, (SourceLocation){0, 0});
    type_param->data.identifier.name = strdup("T");
    ast_node_list_add(&original->data.enum_decl.type_params, type_param);
    
    // Add variants
    original->data.enum_decl.variants = ast_node_list_create(2);
    
    // Variant 1: Some(T)
    ASTNode *variant1 = ast_create_node(AST_ENUM_VARIANT_DECL, (SourceLocation){0, 0});
    variant1->data.enum_variant_decl.name = strdup("Some");
    variant1->data.enum_variant_decl.associated_type = ast_create_node(AST_IDENTIFIER, (SourceLocation){0, 0});
    variant1->data.enum_variant_decl.associated_type->data.identifier.name = strdup("T");
    ast_node_list_add(&original->data.enum_decl.variants, variant1);
    
    // Variant 2: None
    ASTNode *variant2 = ast_create_node(AST_ENUM_VARIANT_DECL, (SourceLocation){0, 0});
    variant2->data.enum_variant_decl.name = strdup("None");
    ast_node_list_add(&original->data.enum_decl.variants, variant2);
    
    // Add annotations
    original->data.enum_decl.annotations = ast_node_list_create(1);
    ASTNode *annotation = ast_create_node(AST_OWNERSHIP_TAG, (SourceLocation){0, 0});
    annotation->data.ownership_tag.ownership = OWNERSHIP_GC;
    ast_node_list_add(&original->data.enum_decl.annotations, annotation);
    
    // Clone the node
    ASTNode *clone = ast_clone_node(original);
    ASTHRA_TEST_ASSERT(context, clone != NULL, "Failed to clone enum declaration node");
    
    // Verify the clone
    asthra_test_assert_int_eq(context, clone->type, AST_ENUM_DECL, "Clone type mismatch");
    asthra_test_assert_string_eq(context, clone->data.enum_decl.name, "Option", "Enum name mismatch");
    asthra_test_assert_int_eq(context, (int)clone->data.enum_decl.visibility, (int)VISIBILITY_PUBLIC, "Visibility mismatch");
    
    // Verify type params are deep cloned
    ASTHRA_TEST_ASSERT(context, clone->data.enum_decl.type_params != NULL, "Type params not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.enum_decl.type_params != original->data.enum_decl.type_params, "Type params not deep cloned");
    asthra_test_assert_size_eq(context, ast_node_list_size(clone->data.enum_decl.type_params), (size_t)1, "Type params count mismatch");
    
    // Verify variants are deep cloned
    ASTHRA_TEST_ASSERT(context, clone->data.enum_decl.variants != NULL, "Variants not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.enum_decl.variants != original->data.enum_decl.variants, "Variants not deep cloned");
    asthra_test_assert_size_eq(context, ast_node_list_size(clone->data.enum_decl.variants), (size_t)2, "Variants count mismatch");
    
    // Check first variant
    ASTNode *cloned_variant1 = ast_node_list_get(clone->data.enum_decl.variants, 0);
    ASTHRA_TEST_ASSERT(context, cloned_variant1 != NULL, "First cloned variant is null");
    ASTHRA_TEST_ASSERT(context, cloned_variant1 != variant1, "First variant not deep cloned");
    asthra_test_assert_string_eq(context, cloned_variant1->data.enum_variant_decl.name, "Some", "Variant name mismatch");
    ASTHRA_TEST_ASSERT(context, cloned_variant1->data.enum_variant_decl.associated_type != NULL, "Associated type not cloned");
    asthra_test_assert_string_eq(context, cloned_variant1->data.enum_variant_decl.associated_type->data.identifier.name, "T", "Associated type name mismatch");
    
    // Verify annotations are deep cloned
    ASTHRA_TEST_ASSERT(context, clone->data.enum_decl.annotations != NULL, "Annotations not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.enum_decl.annotations != original->data.enum_decl.annotations, "Annotations not deep cloned");
    asthra_test_assert_size_eq(context, ast_node_list_size(clone->data.enum_decl.annotations), (size_t)1, "Annotations count mismatch");
    
    // Clean up
    ast_free_node(original);
    ast_free_node(clone);
    
    return ASTHRA_TEST_PASS;
}

// Test deep cloning of function declaration
static AsthraTestResult test_function_decl_deep_clone(AsthraTestContext* context) {
    // Create a function declaration node
    ASTNode *original = ast_create_node(AST_FUNCTION_DECL, (SourceLocation){0, 0});
    ASTHRA_TEST_ASSERT(context, original != NULL, "Failed to create function declaration node");
    
    original->data.function_decl.name = strdup("calculate");
    original->data.function_decl.visibility = VISIBILITY_PUBLIC;
    
    // Add parameters
    original->data.function_decl.params = ast_node_list_create(2);
    
    ASTNode *param1 = ast_create_node(AST_PARAM_DECL, (SourceLocation){0, 0});
    param1->data.param_decl.name = strdup("x");
    param1->data.param_decl.type = ast_create_node(AST_BASE_TYPE, (SourceLocation){0, 0});
    param1->data.param_decl.type->data.base_type.name = strdup("i32");
    ast_node_list_add(&original->data.function_decl.params, param1);
    
    ASTNode *param2 = ast_create_node(AST_PARAM_DECL, (SourceLocation){0, 0});
    param2->data.param_decl.name = strdup("y");
    param2->data.param_decl.type = ast_create_node(AST_BASE_TYPE, (SourceLocation){0, 0});
    param2->data.param_decl.type->data.base_type.name = strdup("i32");
    ast_node_list_add(&original->data.function_decl.params, param2);
    
    // Set return type
    original->data.function_decl.return_type = ast_create_node(AST_BASE_TYPE, (SourceLocation){0, 0});
    original->data.function_decl.return_type->data.base_type.name = strdup("i32");
    
    // Set body
    original->data.function_decl.body = ast_create_node(AST_BLOCK, (SourceLocation){0, 0});
    original->data.function_decl.body->data.block.statements = ast_node_list_create(1);
    
    // Clone the node
    ASTNode *clone = ast_clone_node(original);
    ASTHRA_TEST_ASSERT(context, clone != NULL, "Failed to clone function declaration node");
    
    // Verify the clone
    asthra_test_assert_int_eq(context, clone->type, AST_FUNCTION_DECL, "Clone type mismatch");
    asthra_test_assert_string_eq(context, clone->data.function_decl.name, "calculate", "Function name mismatch");
    asthra_test_assert_int_eq(context, (int)clone->data.function_decl.visibility, (int)VISIBILITY_PUBLIC, "Visibility mismatch");
    
    // Verify params are deep cloned
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.params != NULL, "Params not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.params != original->data.function_decl.params, "Params not deep cloned");
    asthra_test_assert_size_eq(context, ast_node_list_size(clone->data.function_decl.params), (size_t)2, "Params count mismatch");
    
    // Verify return type is cloned
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.return_type != NULL, "Return type not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.return_type != original->data.function_decl.return_type, "Return type not deep cloned");
    asthra_test_assert_string_eq(context, clone->data.function_decl.return_type->data.base_type.name, "i32", "Return type name mismatch");
    
    // Verify body is cloned
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.body != NULL, "Body not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.body != original->data.function_decl.body, "Body not deep cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.body->data.block.statements != NULL, "Body statements not cloned");
    ASTHRA_TEST_ASSERT(context, clone->data.function_decl.body->data.block.statements != 
                         original->data.function_decl.body->data.block.statements, "Body statements not deep cloned");
    
    // Clean up
    ast_free_node(original);
    ast_free_node(clone);
    
    return ASTHRA_TEST_PASS;
}

// Test that modifying cloned lists doesn't affect original
static AsthraTestResult test_clone_independence(AsthraTestContext* context) {
    // Create an array literal
    ASTNode *original = ast_create_node(AST_ARRAY_LITERAL, (SourceLocation){0, 0});
    ASTHRA_TEST_ASSERT(context, original != NULL, "Failed to create array literal node");
    
    original->data.array_literal.elements = ast_node_list_create(2);
    
    ASTNode *elem1 = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0, 0});
    elem1->data.integer_literal.value = 1;
    ast_node_list_add(&original->data.array_literal.elements, elem1);
    
    ASTNode *elem2 = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0, 0});
    elem2->data.integer_literal.value = 2;
    ast_node_list_add(&original->data.array_literal.elements, elem2);
    
    // Clone the node
    ASTNode *clone = ast_clone_node(original);
    ASTHRA_TEST_ASSERT(context, clone != NULL, "Failed to clone array literal node");
    
    // Modify the clone by adding another element
    ASTNode *elem3 = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0, 0});
    elem3->data.integer_literal.value = 3;
    ast_node_list_add(&clone->data.array_literal.elements, elem3);
    
    // Verify original is unchanged
    asthra_test_assert_size_eq(context, ast_node_list_size(original->data.array_literal.elements), (size_t)2, "Original was modified");
    asthra_test_assert_size_eq(context, ast_node_list_size(clone->data.array_literal.elements), (size_t)3, "Clone was not modified");
    
    // Modify an element in the clone
    ASTNode *cloned_elem1 = ast_node_list_get(clone->data.array_literal.elements, 0);
    cloned_elem1->data.integer_literal.value = 100;
    
    // Verify original element is unchanged
    ASTNode *orig_elem1 = ast_node_list_get(original->data.array_literal.elements, 0);
    asthra_test_assert_long_eq(context, orig_elem1->data.integer_literal.value, (int64_t)1, "Original element was modified");
    asthra_test_assert_long_eq(context, cloned_elem1->data.integer_literal.value, (int64_t)100, "Clone element was not modified");
    
    // Clean up
    ast_free_node(original);
    ast_free_node(clone);
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE DEFINITION
// =============================================================================

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    // Initialize test framework
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }
    
    AsthraTestMetadata metadata = {
        .name = "AST Deep Clone Tests",
        .file = __FILE__,
        .line = __LINE__,
        .function = "main",
        .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
        .timeout_ns = 30000000000ULL,
        .skip = false,
        .skip_reason = NULL
    };
    
    AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        fprintf(stderr, "Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }
    
    printf("Running AST Deep Clone Tests...\n\n");
    
    // Run all tests
    AsthraTestResult result1 = test_struct_literal_deep_clone(context);
    AsthraTestResult result2 = test_enum_decl_deep_clone(context);
    AsthraTestResult result3 = test_function_decl_deep_clone(context);
    AsthraTestResult result4 = test_clone_independence(context);
    
    // Calculate results
    int passed = 0;
    int total = 4;
    
    if (result1 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_struct_literal_deep_clone\n");
        passed++;
    } else {
        printf("[FAIL] test_struct_literal_deep_clone\n");
    }
    
    if (result2 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_enum_decl_deep_clone\n");
        passed++;
    } else {
        printf("[FAIL] test_enum_decl_deep_clone\n");
    }
    
    if (result3 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_function_decl_deep_clone\n");
        passed++;
    } else {
        printf("[FAIL] test_function_decl_deep_clone\n");
    }
    
    if (result4 == ASTHRA_TEST_PASS) {
        printf("[PASS] test_clone_independence\n");
        passed++;
    } else {
        printf("[FAIL] test_clone_independence\n");
    }
    
    printf("\nTest Results: %d/%d passed\n", passed, total);
    
    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);
    return (passed == total) ? 0 : 1;
}