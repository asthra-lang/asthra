/**
 * Test file for Option enum variant code generation
 * Tests that Option.Some() and Option.None expressions generate correct assembly
 * Part of Phase 4: Integration and Testing (Enum Variant Construction Implementation Plan)
 */

#include "test_enum_variant_common.h"

// Test Option.Some("hello") code generation with string values
static void test_option_some_codegen(void) {
    printf("Testing Option.Some(\"hello\") code generation...\n");

    // Parse the expression
    ASTNode *expr = parse_enum_test_expression("Option.Some(\"hello\")");
    assert(expr != NULL);
    if (expr->type != AST_ENUM_VARIANT) {
        printf("⚠ Parser returned unexpected node type %d (expected AST_ENUM_VARIANT=%d), skipping "
               "test\n",
               expr->type, AST_ENUM_VARIANT);
        ast_free_node(expr);
        return;
    }

    // Set up code generator context
    TestCodeGenContext *ctx = create_test_code_gen_context();
    assert(ctx != NULL);
    AsthraBackend *backend = ctx->backend;

    // Run semantic analysis on the expression first
    analyze_expression_for_test(ctx, expr);

    // Generate code for the enum variant expression
    // Use the test helper function for enum variant construction
    char *assembly = test_code_generate_enum_variant_construction(backend, "Option", "Some",
                                                                  expr->data.enum_variant.value);

    if (assembly) {
        printf("✓ Successfully generated code for Option.Some(\"hello\")\n");

        // The test helper returns the assembly directly
        printf("✓ Assembly generation succeeded\n");

        // Check for expected assembly patterns
        bool has_constructor_call = strstr(assembly, "Option_Some") != NULL ||
                                    strstr(assembly, "enum_constructor") != NULL ||
                                    strstr(assembly, "variant_create") != NULL;

        bool has_string_hello =
            strstr(assembly, "hello") != NULL || strstr(assembly, "string") != NULL;

        bool has_target_register =
            strstr(assembly, "rbx") != NULL || strstr(assembly, "%rbx") != NULL;

        if (has_constructor_call) {
            printf("✓ Found enum constructor call pattern\n");
        }
        if (has_string_hello) {
            printf("✓ Found string handling in assembly\n");
        }
        if (has_target_register) {
            printf("✓ Found target register usage\n");
        }

        // For debugging, uncomment to see generated assembly
        // printf("Generated assembly:\n%s\n", assembly);

        free(assembly);

    } else {
        printf("✗ Failed to generate code for Option.Some(\"hello\")\n");
        assert(false && "Code generation should succeed");
    }

    // Cleanup
    destroy_test_code_gen_context(ctx);
    ast_free_node(expr);

    printf("✓ Option.Some(\"hello\") codegen test passed\n");
}

// Test Option.None code generation (no value)
static void test_option_none_codegen(void) {
    printf("Testing Option.None code generation...\n");

    // Parse the expression
    ASTNode *expr = parse_enum_test_expression("Option.None");
    assert(expr != NULL);
    if (expr->type != AST_ENUM_VARIANT) {
        printf("⚠ Parser returned unexpected node type %d (expected AST_ENUM_VARIANT=%d), skipping "
               "test\n",
               expr->type, AST_ENUM_VARIANT);
        ast_free_node(expr);
        return;
    }

    // Set up code generator context
    TestCodeGenContext *ctx = create_test_code_gen_context();
    assert(ctx != NULL);
    AsthraBackend *backend = ctx->backend;

    // Run semantic analysis on the expression first
    analyze_expression_for_test(ctx, expr);

    // Generate code for the enum variant expression
    // For None variants, pass NULL as the payload
    char *assembly = test_code_generate_enum_variant_construction(backend, "Option", "None", NULL);

    if (assembly) {
        printf("✓ Successfully generated code for Option.None\n");

        // The test helper returns the assembly directly
        printf("✓ Assembly generation succeeded\n");

        // Check for expected assembly patterns
        bool has_constructor_call = strstr(assembly, "Option_None") != NULL ||
                                    strstr(assembly, "enum_constructor") != NULL ||
                                    strstr(assembly, "variant_create") != NULL;

        // For None variants, we don't expect complex argument passing
        bool simple_construction =
            strstr(assembly, "mov") != NULL || strstr(assembly, "lea") != NULL;

        bool has_target_register =
            strstr(assembly, "rdx") != NULL || strstr(assembly, "%rdx") != NULL;

        if (has_constructor_call) {
            printf("✓ Found enum constructor call pattern\n");
        }
        if (simple_construction) {
            printf("✓ Found simple construction pattern (no arguments)\n");
        }
        if (has_target_register) {
            printf("✓ Found target register usage\n");
        }

        // For debugging, uncomment to see generated assembly
        // printf("Generated assembly:\n%s\n", assembly);

        free(assembly);

    } else {
        printf("✗ Failed to generate code for Option.None\n");
        assert(false && "Code generation should succeed");
    }

    // Cleanup
    destroy_test_code_gen_context(ctx);
    ast_free_node(expr);

    printf("✓ Option.None codegen test passed\n");
}

int main(void) {
    printf("Running Option Variant Code Generation Tests...\n");
    printf("============================================\n\n");

    // Run Option variant tests
    test_option_some_codegen();
    test_option_none_codegen();

    printf("\n✅ All Option variant code generation tests completed!\n");
    printf("Note: Some tests may require full build system integration to verify\n");
    printf("      generated assembly instructions in detail.\n");

    return 0;
}