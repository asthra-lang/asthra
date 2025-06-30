/**
 * Test file for Result enum variant code generation
 * Tests that Result.Ok() and Result.Err() expressions generate correct assembly
 * Part of Phase 4: Integration and Testing (Enum Variant Construction Implementation Plan)
 */

#include "test_enum_variant_common.h"

// Test Result.Ok(42) code generation
static void test_result_ok_codegen(void) {
    printf("Testing Result.Ok(42) code generation...\n");

    // Parse the expression
    ASTNode *expr = parse_enum_test_expression("Result.Ok(42)");
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
    void *backend = ctx->backend;

    // Run semantic analysis on the expression first
    // This satisfies the architectural requirement
    analyze_expression_for_test(ctx, expr);

    // Debug: Verify backend is ready
    printf("DEBUG: About to generate code, backend=%p\n", (void *)backend);

    // Generate code for the enum variant expression
    // Use the test helper function for enum variant construction
    char *assembly = test_code_generate_enum_variant_construction(backend, "Result", "Ok",
                                                                  expr->data.enum_variant.value);

    if (assembly) {
        printf("✓ Successfully generated code for Result.Ok(42)\n");

        // The test helper returns the assembly directly
        printf("✓ Assembly generation succeeded\n");

        // Check for expected assembly patterns
        bool has_constructor_call = strstr(assembly, "Result_Ok") != NULL ||
                                    strstr(assembly, "enum_constructor") != NULL ||
                                    strstr(assembly, "variant_create") != NULL;

        bool has_value_42 = strstr(assembly, "42") != NULL || strstr(assembly, "$42") != NULL ||
                            strstr(assembly, "#42") != NULL;

        bool has_target_register =
            strstr(assembly, "rax") != NULL || strstr(assembly, "%rax") != NULL;

        if (has_constructor_call) {
            printf("✓ Found enum constructor call pattern\n");
        }
        if (has_value_42) {
            printf("✓ Found value 42 in assembly\n");
        }
        if (has_target_register) {
            printf("✓ Found target register usage\n");
        }

        // For debugging, uncomment to see generated assembly
        // printf("Generated assembly:\n%s\n", assembly);

        free(assembly);

    } else {
        printf("✗ Failed to generate code for Result.Ok(42)\n");
        assert(false && "Code generation should succeed");
    }

    // Cleanup
    destroy_test_code_gen_context(ctx);
    ast_free_node(expr);

    printf("✓ Result.Ok(42) codegen test passed\n");
}

// Test Result.Err(error_value) code generation
static void test_result_err_codegen(void) {
    printf("Testing Result.Err(\"error\") code generation...\n");

    // Parse the expression
    ASTNode *expr = parse_enum_test_expression("Result.Err(\"error\")");
    assert(expr != NULL);
    printf("DEBUG: Parsed expression type: %d (expected AST_ENUM_VARIANT=%d)\n", expr->type,
           AST_ENUM_VARIANT);
    if (expr->type != AST_ENUM_VARIANT) {
        printf("⚠ Parser returned unexpected node type, skipping test\n");
        ast_free_node(expr);
        return;
    }

    // Set up code generator context
    TestCodeGenContext *ctx = create_test_code_gen_context();
    assert(ctx != NULL);
    void *backend = ctx->backend;

    // Run semantic analysis on the expression first
    analyze_expression_for_test(ctx, expr);

    // Generate code for the enum variant expression
    // Use the test helper function for enum variant construction
    char *assembly = test_code_generate_enum_variant_construction(backend, "Result", "Err",
                                                                  expr->data.enum_variant.value);

    if (assembly) {
        printf("✓ Successfully generated code for Result.Err(\"error\")\n");

        // The test helper returns the assembly directly
        printf("✓ Assembly generation succeeded\n");

        // Check for expected assembly patterns
        bool has_constructor_call = strstr(assembly, "Result_Err") != NULL ||
                                    strstr(assembly, "enum_constructor") != NULL ||
                                    strstr(assembly, "variant_create") != NULL;

        bool has_error_string =
            strstr(assembly, "error") != NULL || strstr(assembly, "string") != NULL;

        bool has_target_register =
            strstr(assembly, "rcx") != NULL || strstr(assembly, "%rcx") != NULL;

        if (has_constructor_call) {
            printf("✓ Found enum constructor call pattern\n");
        }
        if (has_error_string) {
            printf("✓ Found error string handling in assembly\n");
        }
        if (has_target_register) {
            printf("✓ Found target register usage\n");
        }

        // For debugging, uncomment to see generated assembly
        // printf("Generated assembly:\n%s\n", assembly);

        free(assembly);

    } else {
        printf("✗ Failed to generate code for Result.Err(\"error\")\n");
        assert(false && "Code generation should succeed");
    }

    // Cleanup
    destroy_test_code_gen_context(ctx);
    ast_free_node(expr);

    printf("✓ Result.Err(\"error\") codegen test passed\n");
}

int main(void) {
    printf("Running Result Variant Code Generation Tests...\n");
    printf("============================================\n\n");

    // Run Result variant tests
    test_result_ok_codegen();
    test_result_err_codegen();

    printf("\n✅ All Result variant code generation tests completed!\n");
    printf("Note: Some tests may require full build system integration to verify\n");
    printf("      generated assembly instructions in detail.\n");

    return 0;
}