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
    if (!lexer)
        return NULL;

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

// Test helper function to create a minimal backend context
TestCodeGenContext *create_test_code_gen_context(void) {
    TestCodeGenContext *ctx = calloc(1, sizeof(TestCodeGenContext));
    if (!ctx) {
        return NULL;
    }

    AsthraCompilerOptions options = {.target_arch = ASTHRA_TARGET_X86_64,
                                     .opt_level = ASTHRA_OPT_NONE,
                                     .output_file = "test_output.ll",
                                     .debug_info = true,
                                     .verbose = false};
    ctx->backend = asthra_backend_create(&options);
    if (!ctx->backend) {
        free(ctx);
        return NULL;
    }

    // Create and set a semantic analyzer to satisfy architectural requirements
    ctx->analyzer = semantic_analyzer_create();
    if (!ctx->analyzer) {
        asthra_backend_destroy(ctx->backend);
        free(ctx);
        return NULL;
    }

    // Debug: Created backend and analyzer
    printf("DEBUG: Created backend %p and analyzer %p\n", (void *)ctx->backend,
           (void *)ctx->analyzer);

    return ctx;
}

// Test helper function to destroy backend context
void destroy_test_code_gen_context(TestCodeGenContext *ctx) {
    if (!ctx)
        return;

    if (ctx->backend) {
        asthra_backend_destroy(ctx->backend);
    }
    if (ctx->analyzer) {
        semantic_analyzer_destroy(ctx->analyzer);
    }
    free(ctx);
}

// Test helper function to run semantic analysis on an expression
void analyze_expression_for_test(TestCodeGenContext *ctx, ASTNode *expr) {
    if (!ctx || !ctx->analyzer || !expr)
        return;

    // Run semantic analysis on the expression
    // For test purposes, we don't fail if semantic analysis fails
    bool semantic_success = semantic_analyze_expression(ctx->analyzer, expr);
    if (!semantic_success) {
        printf("⚠ Warning: Semantic analysis failed, but continuing with test\n");
    }
}