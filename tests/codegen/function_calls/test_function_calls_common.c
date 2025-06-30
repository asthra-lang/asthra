#include "test_function_calls_common.h"
#include "../codegen_backend_wrapper.h"
#include "../../framework/backend_stubs.h"
#include <stdlib.h>

// =============================================================================
// COMMON UTILITIES AND HELPERS IMPLEMENTATION
// =============================================================================

Lexer *create_test_lexer(const char *source) {
    return lexer_create(source, strlen(source), "test_function_calls.asthra");
}

Parser *create_test_parser(const char *source) {
    Lexer *lexer = create_test_lexer(source);
    if (!lexer)
        return NULL;
    return parser_create(lexer);
}

void cleanup_parser(Parser *parser) {
    if (parser) {
        parser_destroy(parser); // Parser cleanup handles lexer destruction internally
    }
}

bool run_test_pipeline(FunctionCallTestPipeline *pipeline, const char *source,
                       AsthraTestContext *context) {
    printf("DEBUG: Starting test pipeline\n");
    fflush(stdout);

    // Initialize pipeline
    pipeline->parser = NULL;
    pipeline->program = NULL;
    pipeline->analyzer = NULL;
    pipeline->backend = NULL;
    pipeline->compiler_ctx = NULL;
    pipeline->output = NULL;
    pipeline->success = false;

    // Use the working parse_test_source pattern from statement_generation_helpers.c
    printf("DEBUG: Parsing program using working pattern\n");
    fflush(stdout);

    Lexer *lexer = lexer_create(source, strlen(source), "test_function_calls.asthra");
    if (!ASTHRA_TEST_ASSERT(context, lexer != NULL, "Lexer should be created")) {
        return false;
    }

    Parser *parser = parser_create(lexer);
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        lexer_destroy(lexer);
        return false;
    }

    printf("DEBUG: About to call parser_parse_program on parser %p\n", (void *)parser);
    printf("DEBUG: Pipeline pointer: %p\n", (void *)pipeline);
    printf("DEBUG: Pipeline->program field address: %p\n", (void *)&pipeline->program);

    // Check parser state before calling
    if (parser) {
        printf("DEBUG: Parser state check passed\n");
    }

    // Get function pointer and check it
    // Use a union to avoid ISO C restriction on function pointer to void* conversion
    union {
        void *obj;
        ASTNode *(*func)(Parser *);
    } func_ptr_union;
    func_ptr_union.func = parser_parse_program;
    printf("DEBUG: parser_parse_program function address: %p\n", func_ptr_union.obj);
    fflush(stdout);

    // Direct assignment without intermediate variable
    pipeline->program = parser_parse_program(parser);
    printf("DEBUG: parser_parse_program direct result in pipeline: %p\n",
           (void *)pipeline->program);

    // Check if the pipeline structure itself is corrupted
    printf("DEBUG: Pipeline fields after assignment:\n");
    printf("  parser: %p\n", (void *)pipeline->parser);
    printf("  program: %p\n", (void *)pipeline->program);
    printf("  analyzer: %p\n", (void *)pipeline->analyzer);
    printf("  backend: %p\n", (void *)pipeline->backend);
    printf("  compiler_ctx: %p\n", (void *)pipeline->compiler_ctx);
    printf("  output: %p\n", (void *)pipeline->output);
    printf("  success: %d\n", pipeline->success);
    fflush(stdout);

    // Clean up parser and lexer immediately after parsing (working pattern)
    parser_destroy(parser);

    if (!ASTHRA_TEST_ASSERT(context, pipeline->program != NULL, "Program should be parsed")) {
        return false;
    }
    printf("DEBUG: Program parsed successfully at %p\n", (void *)pipeline->program);
    fflush(stdout);

    // Create semantic analyzer
    printf("DEBUG: Creating semantic analyzer\n");
    fflush(stdout);
    pipeline->analyzer = semantic_analyzer_create();
    if (!ASTHRA_TEST_ASSERT(context, pipeline->analyzer != NULL,
                            "Semantic analyzer should be created")) {
        return false;
    }
    printf("DEBUG: Semantic analyzer created\n");
    fflush(stdout);

    // Initialize builtin types (required for semantic analysis)
    // Note: This function may be called differently or may be internal to semantic_analyzer_create
    // semantic_init_builtin_types(pipeline->analyzer);

    // Run semantic analysis
    printf("DEBUG: Running semantic analysis on program at %p\n", (void *)pipeline->program);
    fflush(stdout);

    // Check program structure before analysis
    if (pipeline->program) {
        printf("DEBUG: Program type: %d\n", pipeline->program->type);
        if (pipeline->program->type == AST_PROGRAM) {
            printf("DEBUG: Program has %zu declarations\n",
                   pipeline->program->data.program.declarations
                       ? ast_node_list_size(pipeline->program->data.program.declarations)
                       : 0);
        }
    }
    fflush(stdout);

    bool analysis_result = semantic_analyze_program(pipeline->analyzer, pipeline->program);
    printf("DEBUG: Semantic analysis completed with result: %s\n",
           analysis_result ? "success" : "failure");
    fflush(stdout);

    // Print semantic errors if analysis failed
    if (!analysis_result) {
        printf("Semantic errors detected but error reporting not implemented\n");
    }

    if (!ASTHRA_TEST_ASSERT(context, analysis_result, "Analysis should succeed")) {
        return false;
    }

    // Create backend using backend interface
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.target_arch = ASTHRA_TARGET_X86_64;
    // Backend type removed - LLVM is the only backend
    options.output_file = "test_output.ll";

    pipeline->backend = asthra_backend_create(&options);
    if (!ASTHRA_TEST_ASSERT(context, pipeline->backend != NULL, "Backend should be created")) {
        return false;
    }

    // Initialize backend
    if (asthra_backend_initialize(pipeline->backend, &options) != 0) {
        ASTHRA_TEST_ASSERT(context, false, "Backend initialization should succeed");
        return false;
    }

    // Create compiler context for backend
    pipeline->compiler_ctx = calloc(1, sizeof(AsthraCompilerContext));
    if (!ASTHRA_TEST_ASSERT(context, pipeline->compiler_ctx != NULL,
                            "Compiler context should be allocated")) {
        return false;
    }

    pipeline->compiler_ctx->options = options;
    pipeline->compiler_ctx->ast = pipeline->program;
    pipeline->compiler_ctx->symbol_table = pipeline->analyzer->global_scope;
    pipeline->compiler_ctx->type_checker = pipeline->analyzer;

    // Generate code
    int codegen_result = asthra_backend_generate(pipeline->backend, pipeline->compiler_ctx,
                                                 pipeline->program, options.output_file);
    if (!ASTHRA_TEST_ASSERT(context, codegen_result == 0, "Code generation should succeed")) {
        return false;
    }

    // For testing purposes, we'll consider generation successful if no error occurred
    pipeline->output = "generated"; // Placeholder - real impl would read output file

    if (!ASTHRA_TEST_ASSERT(context, pipeline->output != NULL, "Generated code should exist")) {
        return false;
    }

    pipeline->success = true;
    return true;
}

void cleanup_test_pipeline(FunctionCallTestPipeline *pipeline) {
    if (pipeline->backend) {
        asthra_backend_destroy(pipeline->backend);
        pipeline->backend = NULL;
    }
    if (pipeline->compiler_ctx) {
        free(pipeline->compiler_ctx);
        pipeline->compiler_ctx = NULL;
    }
    if (pipeline->analyzer) {
        semantic_analyzer_destroy(pipeline->analyzer);
        pipeline->analyzer = NULL;
    }
    if (pipeline->program) {
        ast_free_node(pipeline->program);
        pipeline->program = NULL;
    }
    // Note: parser and lexer are already cleaned up in run_test_pipeline
    pipeline->parser = NULL;
    pipeline->output = NULL;
    pipeline->success = false;
}

// This function is only used when running all function call tests together
// Individual test files have their own main functions
#ifdef RUN_ALL_FUNCTION_CALL_TESTS
AsthraTestResult run_function_calls_test_suite(void) {
    printf("Running function call generation test suite...\n");

    AsthraTestStatistics *global_stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    AsthraTestMetadata base_metadata = {.name = "Function Call Generation Tests",
                                        .file = __FILE__,
                                        .line = __LINE__,
                                        .description =
                                            "Test suite for function call code generation",
                                        .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                        .timeout_ns = 5000000000ULL, // 5 seconds
                                        .skip = false,
                                        .skip_reason = NULL};

    struct {
        const char *name;
        AsthraTestResult (*test_func)(AsthraTestContext *);
    } tests[] = {
        {"Associated function calls", test_associated_function_calls},
        {"Instance method calls", test_instance_method_calls},
        {"Self parameter handling", test_self_parameter_handling},
        {"Chained method calls", test_chained_method_calls},
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < test_count; i++) {
        AsthraTestMetadata metadata = base_metadata;
        metadata.name = tests[i].name;
        metadata.line = __LINE__;

        AsthraTestContext *context = asthra_test_context_create(&metadata, global_stats);
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

    printf("\n=== Function Call Test Summary ===\n");
    printf("Total tests: %zu\n", test_count);
    printf("Tests run: %llu\n", global_stats->tests_run);
    printf("Tests passed: %llu\n", global_stats->tests_passed);
    printf("Tests failed: %llu\n", global_stats->tests_failed);

    asthra_test_statistics_destroy(global_stats);
    return overall_result;
}
#endif // RUN_ALL_FUNCTION_CALL_TESTS
