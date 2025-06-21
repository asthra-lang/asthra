#ifndef TEST_IMMUTABLE_BY_DEFAULT_COMMON_H
#define TEST_IMMUTABLE_BY_DEFAULT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// Include the necessary types
#include "../../src/codegen/code_generator_types.h"

// Use forward declarations that match the real types
typedef struct ASTNode ASTNode;
typedef struct Parser Parser;
typedef struct SemanticAnalyzer SemanticAnalyzer;
typedef struct CodeGenerator CodeGenerator;

// Function declarations - some are stubs, some use real compiler functions
Parser *parser_create(void);
void parser_destroy(Parser *parser);
ASTNode *parser_parse_string(Parser *parser, const char *source);
void ast_destroy(ASTNode *ast);

// These functions will be provided by the real compiler library
SemanticAnalyzer *semantic_analyzer_create(void);
void semantic_analyzer_destroy(SemanticAnalyzer *analyzer);
bool semantic_analyze_program(SemanticAnalyzer *analyzer, ASTNode *ast);

// Code generator functions - will be adapted to real compiler interface
CodeGenerator *code_generator_create(TargetArchitecture arch, CallingConvention conv);
void code_generator_destroy(CodeGenerator *generator);
bool code_generator_generate_program(CodeGenerator *generator, ASTNode *ast, FILE *output);

// =============================================================================
// SHARED DATA STRUCTURES
// =============================================================================

typedef struct {
    const char *asthra_code;
    const char *expected_c_pattern;
    bool should_compile;
    bool expect_optimization;
} IntegrationTestCase;

// =============================================================================
// SHARED UTILITY FUNCTIONS
// =============================================================================

/**
 * Compile and validate Asthra code against expected patterns
 * Returns true if compilation result matches expectations
 */
bool compile_and_validate_asthra_code(const char *source_code, const char *expected_pattern);

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

// Basic immutability tests
bool test_basic_immutable_variables(void);
bool test_mutable_variables_everywhere(void);
bool test_function_parameter_immutability(void);

// Integration tests
bool test_ffi_immutable_integration(void);
bool test_smart_optimization_integration(void);

// Real-world scenario tests
bool test_real_world_game_engine_scenario(void);
bool test_real_world_web_server_scenario(void);

// Performance and AI tests
bool test_performance_targets_validation(void);
bool test_ai_friendly_patterns(void);

#endif // TEST_IMMUTABLE_BY_DEFAULT_COMMON_H
