/*
 * Common Header for Generic Structs Phase 5 Integration Tests
 * Shared definitions, types, and function declarations for end-to-end testing
 *
 * Part of test_generic_structs_phase5.c split (580 lines -> 6 focused modules)
 * Provides comprehensive generic structs integration test infrastructure
 */

#ifndef TEST_GENERIC_STRUCTS_PHASE5_COMMON_H
#define TEST_GENERIC_STRUCTS_PHASE5_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include all necessary headers
#include "ast.h"
#include "backend_interface.h"
#include "generic_instantiation.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"

// ============================================================================
// TEST FRAMEWORK MACROS
// ============================================================================

extern size_t tests_run;
extern size_t tests_passed;

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("  ✅ PASS: %s\n", message);                                                    \
        } else {                                                                                   \
            printf("  ❌ FAIL: %s\n", message);                                                    \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

#define TEST_SUCCESS()                                                                             \
    do {                                                                                           \
        printf("  🎉 Test completed successfully!\n");                                             \
        return true;                                                                               \
    } while (0)

// ============================================================================
// COMPILATION PIPELINE TYPES
// ============================================================================

/**
 * Complete compilation pipeline result
 */
typedef struct {
    bool success;
    ASTNode *ast;
    SemanticAnalyzer *analyzer;
    AsthraBackend *generator;
    char *c_code_output;
    char *error_message;
} CompilationResult;

// ============================================================================
// COMPILATION PIPELINE FUNCTIONS
// ============================================================================

/**
 * Perform complete compilation pipeline from source to C code
 */
CompilationResult *compile_source(const char *source);

/**
 * Clean up compilation result and free all resources
 */
void cleanup_compilation_result(CompilationResult *result);

// ============================================================================
// BASIC INTEGRATION TEST FUNCTIONS
// ============================================================================

/**
 * Test basic generic struct end-to-end compilation
 */
bool test_basic_generic_struct_e2e(void);

/**
 * Test multiple type parameters integration
 */
bool test_multiple_type_parameters_integration(void);

/**
 * Test nested generic types integration
 */
bool test_nested_generic_types_integration(void);

// ============================================================================
// ADVANCED INTEGRATION TEST FUNCTIONS
// ============================================================================

/**
 * Test generic struct with methods integration
 */
bool test_generic_struct_with_methods_integration(void);

/**
 * Test error handling for invalid generic usage
 */
bool test_error_handling_invalid_generic_usage(void);

/**
 * Test deduplication integration
 */
bool test_deduplication_integration(void);

// ============================================================================
// PERFORMANCE AND CONSISTENCY TEST FUNCTIONS
// ============================================================================

/**
 * Test performance with many instantiations
 */
bool test_performance_many_instantiations(void);

/**
 * Test type system consistency between generic enums and structs
 */
bool test_type_system_consistency(void);

#endif // TEST_GENERIC_STRUCTS_PHASE5_COMMON_H
