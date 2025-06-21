/**
 * Common implementation for enum variant code generation test utilities
 * Part of Phase 4: Integration and Testing (Enum Variant Construction Implementation Plan)
 */

#include "test_enum_variant_common.h"

// Test helper function to create a lexer from source code
Lexer *create_test_lexer(const char *source) {
    return lexer_create(source, strlen(source), "test.asthra");
}

// Test helper function to parse an expression from source
ASTNode *parse_enum_test_expression(const char *source) {
    Lexer *lexer = create_test_lexer(source);
    if (!lexer) return NULL;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }
    
    ASTNode *expr = parser_parse_expression(parser);
    
    if (!expr || parser_had_error(parser)) {
        printf("Parser errors:\n");
        const ParseError *errors = parser_get_errors(parser);
        if (errors && errors[0].message) {
            printf("  %s:%d:%d: %s\n", 
                   errors[0].location.filename ? errors[0].location.filename : "unknown",
                   errors[0].location.line, errors[0].location.column, errors[0].message);
        }
        if (expr) {
            ast_free_node(expr);
            expr = NULL;
        }
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return expr;
}

// Test helper function to create a minimal code generator context
TestCodeGenContext *create_test_code_gen_context(void) {
    TestCodeGenContext *ctx = calloc(1, sizeof(TestCodeGenContext));
    if (!ctx) {
        return NULL;
    }
    
    ctx->generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!ctx->generator) {
        free(ctx);
        return NULL;
    }
    
    // Create and set a semantic analyzer to satisfy architectural requirements
    ctx->analyzer = semantic_analyzer_create();
    if (!ctx->analyzer) {
        code_generator_destroy(ctx->generator);
        free(ctx);
        return NULL;
    }
    
    // Set the semantic analyzer on the code generator
    code_generator_set_semantic_analyzer(ctx->generator, ctx->analyzer);
    
    // Debug: Verify the analyzer was set
    printf("DEBUG: Set semantic analyzer %p on generator %p\n", (void*)ctx->analyzer, (void*)ctx->generator);
    
    return ctx;
}

// Test helper function to destroy code generator context
void destroy_test_code_gen_context(TestCodeGenContext *ctx) {
    if (!ctx) return;
    
    if (ctx->generator) {
        code_generator_destroy(ctx->generator);
    }
    if (ctx->analyzer) {
        semantic_analyzer_destroy(ctx->analyzer);
    }
    free(ctx);
}

// Test helper function to run semantic analysis on an expression
void analyze_expression_for_test(TestCodeGenContext *ctx, ASTNode *expr) {
    if (!ctx || !ctx->analyzer || !expr) return;
    
    // Run semantic analysis on the expression
    // For test purposes, we don't fail if semantic analysis fails
    bool semantic_success = semantic_analyze_expression(ctx->analyzer, expr);
    if (!semantic_success) {
        printf("⚠ Warning: Semantic analysis failed, but continuing with test\n");
    }
}