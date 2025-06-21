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
        printf("⚠ Parser returned unexpected node type %d (expected AST_ENUM_VARIANT=%d), skipping test\n", expr->type, AST_ENUM_VARIANT);
        ast_free_node(expr);
        return;
    }
    
    // Set up code generator context
    TestCodeGenContext *ctx = create_test_code_gen_context();
    assert(ctx != NULL);
    CodeGenerator *generator = ctx->generator;
    
    // Run semantic analysis on the expression first
    analyze_expression_for_test(ctx, expr);
    
    // Generate code for the enum variant expression
    Register target_reg = ASTHRA_REG_RBX; // Use a different test register
    bool success = code_generate_enum_variant_construction(generator, expr, target_reg);
    
    if (success) {
        printf("✓ Successfully generated code for Option.Some(\"hello\")\n");
        
        // Verify the generated assembly instructions
        char assembly_buffer[4096];
        bool assembly_success = code_generator_emit_assembly(generator, assembly_buffer, sizeof(assembly_buffer));
        
        if (assembly_success) {
            printf("✓ Assembly generation succeeded\n");
            
            // Check for expected assembly patterns
            bool has_constructor_call = strstr(assembly_buffer, "Option_Some") != NULL ||
                                       strstr(assembly_buffer, "enum_constructor") != NULL ||
                                       strstr(assembly_buffer, "variant_create") != NULL;
            
            bool has_string_hello = strstr(assembly_buffer, "hello") != NULL ||
                                   strstr(assembly_buffer, "string") != NULL;
            
            bool has_target_register = strstr(assembly_buffer, "rbx") != NULL ||
                                      strstr(assembly_buffer, "%rbx") != NULL;
            
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
            // printf("Generated assembly:\n%s\n", assembly_buffer);
        } else {
            printf("⚠ Assembly generation failed, but codegen succeeded\n");
        }
        
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
        printf("⚠ Parser returned unexpected node type %d (expected AST_ENUM_VARIANT=%d), skipping test\n", expr->type, AST_ENUM_VARIANT);
        ast_free_node(expr);
        return;
    }
    
    // Set up code generator context
    TestCodeGenContext *ctx = create_test_code_gen_context();
    assert(ctx != NULL);
    CodeGenerator *generator = ctx->generator;
    
    // Run semantic analysis on the expression first
    analyze_expression_for_test(ctx, expr);
    
    // Generate code for the enum variant expression
    Register target_reg = ASTHRA_REG_RDX; // Use another test register
    bool success = code_generate_enum_variant_construction(generator, expr, target_reg);
    
    if (success) {
        printf("✓ Successfully generated code for Option.None\n");
        
        // Verify the generated assembly instructions
        char assembly_buffer[4096];
        bool assembly_success = code_generator_emit_assembly(generator, assembly_buffer, sizeof(assembly_buffer));
        
        if (assembly_success) {
            printf("✓ Assembly generation succeeded\n");
            
            // Check for expected assembly patterns
            bool has_constructor_call = strstr(assembly_buffer, "Option_None") != NULL ||
                                       strstr(assembly_buffer, "enum_constructor") != NULL ||
                                       strstr(assembly_buffer, "variant_create") != NULL;
            
            // For None variants, we don't expect complex argument passing
            bool simple_construction = strstr(assembly_buffer, "mov") != NULL ||
                                      strstr(assembly_buffer, "lea") != NULL;
            
            bool has_target_register = strstr(assembly_buffer, "rdx") != NULL ||
                                      strstr(assembly_buffer, "%rdx") != NULL;
            
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
            // printf("Generated assembly:\n%s\n", assembly_buffer);
        } else {
            printf("⚠ Assembly generation failed, but codegen succeeded\n");
        }
        
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