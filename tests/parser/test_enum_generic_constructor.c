#include "../framework/test_framework.h"
#include "ast.h"
#include "parser.h"
#include <string.h>

// Test parsing of generic enum constructors like Option<i32>.Some(42)
TEST_CASE(test_enum_generic_constructor_with_value) {
    const char *code = "Option<i32>.Some(42)";

    Parser *parser = parser_create_from_string(code);
    ASSERT_NOT_NULL(parser);

    ASTNode *expr = parse_expr(parser);
    ASSERT_NOT_NULL(expr);

    // Should be an enum variant
    ASSERT_EQ(expr->type, AST_ENUM_VARIANT);
    ASSERT_STR_EQ(expr->data.enum_variant.enum_name, "Option");
    ASSERT_STR_EQ(expr->data.enum_variant.variant_name, "Some");

    // Should have a value (42)
    ASSERT_NOT_NULL(expr->data.enum_variant.value);
    ASSERT_EQ(expr->data.enum_variant.value->type, AST_INTEGER_LITERAL);
    ASSERT_EQ(expr->data.enum_variant.value->data.integer_literal.value, 42);

    ast_free_node(expr);
    parser_destroy(parser);
}

// Test parsing of generic enum constructors without value like Result<T, E>.Ok
TEST_CASE(test_enum_generic_constructor_no_value) {
    const char *code = "Result<String, Error>.Ok";

    Parser *parser = parser_create_from_string(code);
    ASSERT_NOT_NULL(parser);

    ASTNode *expr = parse_expr(parser);
    ASSERT_NOT_NULL(expr);

    // Should be an enum variant
    ASSERT_EQ(expr->type, AST_ENUM_VARIANT);
    ASSERT_STR_EQ(expr->data.enum_variant.enum_name, "Result");
    ASSERT_STR_EQ(expr->data.enum_variant.variant_name, "Ok");

    // In our implementation, when there's no value, we store type info in the value field
    // This is a temporary solution as noted in the TODO
    ASSERT_NOT_NULL(expr->data.enum_variant.value);
    ASSERT_EQ(expr->data.enum_variant.value->type, AST_ENUM_TYPE);

    ast_free_node(expr);
    parser_destroy(parser);
}

// Test parsing of complex generic enum constructor
TEST_CASE(test_enum_generic_constructor_complex) {
    const char *code = "Vec<Option<i32>>.Some(nested)";

    Parser *parser = parser_create_from_string(code);
    ASSERT_NOT_NULL(parser);

    ASTNode *expr = parse_expr(parser);
    ASSERT_NOT_NULL(expr);

    // Should be an enum variant
    ASSERT_EQ(expr->type, AST_ENUM_VARIANT);
    ASSERT_STR_EQ(expr->data.enum_variant.enum_name, "Vec");
    ASSERT_STR_EQ(expr->data.enum_variant.variant_name, "Some");

    // Should have a value (identifier 'nested')
    ASSERT_NOT_NULL(expr->data.enum_variant.value);
    ASSERT_EQ(expr->data.enum_variant.value->type, AST_IDENTIFIER);
    ASSERT_STR_EQ(expr->data.enum_variant.value->data.identifier.name, "nested");

    ast_free_node(expr);
    parser_destroy(parser);
}

// Test that regular enum constructors still work
TEST_CASE(test_enum_regular_constructor) {
    const char *code = "Color.Red";

    Parser *parser = parser_create_from_string(code);
    ASSERT_NOT_NULL(parser);

    ASTNode *expr = parse_expr(parser);
    ASSERT_NOT_NULL(expr);

    // Should be an enum variant
    ASSERT_EQ(expr->type, AST_ENUM_VARIANT);
    ASSERT_STR_EQ(expr->data.enum_variant.enum_name, "Color");
    ASSERT_STR_EQ(expr->data.enum_variant.variant_name, "Red");
    ASSERT_NULL(expr->data.enum_variant.value);

    ast_free_node(expr);
    parser_destroy(parser);
}

void register_enum_generic_constructor_tests(void) {
    REGISTER_TEST(test_enum_generic_constructor_with_value);
    REGISTER_TEST(test_enum_generic_constructor_no_value);
    REGISTER_TEST(test_enum_generic_constructor_complex);
    REGISTER_TEST(test_enum_regular_constructor);
}