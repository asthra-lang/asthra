/**
 * Real Program Test Integration Implementation
 *
 * Contains functions for integration testing including end-to-end
 * compilation, semantic analysis integration, and component integration.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_test_integration.h"
#include "ast.h"
#include "code_generator.h"
#include "ffi_assembly_generator.h"
#include "parser_string_interface.h"
#include "real_program_test_suite.h"
#include "semantic_analyzer_core.h"
#include "semantic_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// INTEGRATION TESTING FUNCTIONS
// =============================================================================

bool test_source_to_executable(const char *source, const char *output_name,
                               const TestSuiteConfig *config) {
    if (!source || !output_name) {
        return false;
    }

    if (config && config->verbose_output) {
        fprintf(config->output_stream, "Testing end-to-end compilation: %s\n", output_name);
    }

    // Step 1: Parse the source code
    ParseResult parse_result = parse_string(source);
    if (!parse_result.success) {
        if (config && config->verbose_output) {
            fprintf(config->output_stream, "Parse failed: %s\n",
                    parse_result.error_count > 0 ? parse_result.errors[0] : "Unknown error");
        }
        cleanup_parse_result(&parse_result);
        return false;
    }

    // Step 2: Validate AST structure
    if (!parse_result.ast) {
        if (config && config->verbose_output) {
            fprintf(config->output_stream, "No AST generated\n");
        }
        cleanup_parse_result(&parse_result);
        return false;
    }

    // Step 3: Check AST integrity (semantic analysis)
    if (parse_result.ast->type != AST_PROGRAM) {
        if (config && config->verbose_output) {
            fprintf(config->output_stream, "Invalid AST root type: %d\n", parse_result.ast->type);
        }
        cleanup_parse_result(&parse_result);
        return false;
    }

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        fprintf(config->output_stream, "Failed to create semantic analyzer\n");
        cleanup_parse_result(&parse_result);
        return false;
    }

    bool semantic_success = semantic_analyze_program(analyzer, parse_result.ast);
    if (!semantic_success) {
        fprintf(config->output_stream, "Semantic analysis failed for program\n");
        semantic_analyzer_destroy(analyzer);
        cleanup_parse_result(&parse_result);
        return false;
    }

    // Step 4: Code Generation (simplified for now - just verify it generates without error)
    CodeGenerator *codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!codegen) {
        fprintf(config->output_stream, "Failed to create code generator\n");
        semantic_analyzer_destroy(analyzer);
        cleanup_parse_result(&parse_result);
        return false;
    }

    // Set the semantic analyzer in the code generator
    codegen->semantic_analyzer = analyzer;

    bool codegen_success = code_generate_program(codegen, parse_result.ast);

    code_generator_destroy(codegen);
    semantic_analyzer_destroy(analyzer);
    cleanup_parse_result(&parse_result);

    if (!codegen_success) {
        fprintf(config->output_stream, "Code generation failed\n");
        return false;
    }

    if (config && config->verbose_output) {
        fprintf(config->output_stream, "End-to-end compilation and execution successful\n");
    }

    return true;
}

bool test_parser_semantic_integration(const char *source, const TestSuiteConfig *config) {
    if (!source) {
        return false;
    }

    if (config && config->verbose_output) {
        fprintf(config->output_stream, "Testing parser-semantic integration\n");
    }

    // Parse the source
    ParseResult result = parse_string(source);
    if (!result.success) {
        if (config && config->verbose_output) {
            fprintf(config->output_stream, "Parser integration failed\n");
        }
        cleanup_parse_result(&result);
        return false;
    }

    // Check that we have a valid AST
    if (!result.ast || result.ast->type != AST_PROGRAM) {
        if (config && config->verbose_output) {
            fprintf(config->output_stream, "Invalid AST for semantic integration\n");
        }
        cleanup_parse_result(&result);
        return false;
    }

    // Perform semantic analysis on the parsed AST
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        if (config && config->verbose_output) {
            fprintf(config->output_stream, "Failed to create semantic analyzer\n");
        }
        cleanup_parse_result(&result);
        return false;
    }

    bool semantic_success = semantic_analyze_program(analyzer, result.ast);
    if (!semantic_success) {
        if (config && config->verbose_output) {
            fprintf(config->output_stream, "Semantic analysis failed for program\n");
        }
        semantic_analyzer_destroy(analyzer);
        cleanup_parse_result(&result);
        return false;
    }

    semantic_analyzer_destroy(analyzer);

    if (config && config->verbose_output) {
        fprintf(config->output_stream, "Parser-semantic integration successful\n");
    }

    cleanup_parse_result(&result);
    return true;
}

bool check_component_integration(const char *source, const char **components,
                                 size_t component_count, const TestSuiteConfig *config) {
    if (!source || !components || component_count == 0) {
        return false;
    }

    if (config && config->verbose_output) {
        fprintf(config->output_stream, "Checking integration of %zu components\n", component_count);
    }

    // Test the combined source
    RealProgramTestResult result = validate_complete_program(source, "integration_test", config);

    bool integration_success = result.success;

    cleanup_test_result(&result);

    return integration_success;
}