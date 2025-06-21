/**
 * Asthra Programming Language
 * Integration Test Stubs
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Stub implementations for integration tests to resolve missing symbols
 * while maintaining compatibility with existing runtime functions.
 */

#include "asthra_runtime.h"
#include "asthra_ffi_memory.h"
#include "../tests/framework/test_framework.h"
#include <string.h>
#include <stdio.h>
#include "asthra_integration_stubs.h"

// =============================================================================
// RUNTIME FUNCTION STUBS
// =============================================================================

/**
 * Register C thread with Asthra runtime
 * Delegates to existing asthra_thread_register function
 */
AsthraResult Asthra_register_c_thread(void) {
    // Delegate to existing runtime function
    asthra_thread_register();
    
    // Return success result
    AsthraResult result = { .tag = ASTHRA_RESULT_OK };
    return result;
}

/**
 * Create error result with FFI signature
 * Matches the signature in asthra_ffi_memory.h
 */
AsthraFFIResult Asthra_result_err(int error_code, const char* error_message, 
                                 const char* error_source, void* error_context) {
    AsthraFFIResult result;
    result.tag = ASTHRA_FFI_RESULT_ERR;
    result.data.err.error_code = error_code;
    
    // Safely copy error message
    if (error_message) {
        strncpy(result.data.err.error_message, error_message, 255);
        result.data.err.error_message[255] = '\0';
    } else {
        strcpy(result.data.err.error_message, "Unknown error");
    }
    
    result.data.err.error_source = error_source;
    result.data.err.error_context = error_context;
    
    return result;
}

/**
 * Unregister C thread from Asthra runtime
 * Delegates to existing asthra_thread_unregister function
 */
void Asthra_unregister_c_thread(void) {
    // Delegate to existing runtime function
    asthra_thread_unregister();
}

// =============================================================================
// TEST FRAMEWORK FUNCTION STUBS
// =============================================================================

/**
 * Initialize test context
 * Provides basic initialization for integration tests
 */
void asthra_test_context_init(AsthraTestContext* context) {
    if (!context) {
        return;
    }
    
    // Initialize context structure
    memset(context, 0, sizeof(AsthraTestContext));
    context->result = ASTHRA_TEST_PASS;
    
    // Initialize metadata with default values
    context->metadata.name = "integration_test";
    context->metadata.file = __FILE__;
    context->metadata.line = __LINE__;
    context->metadata.function = "asthra_test_context_init";
    context->metadata.severity = ASTHRA_TEST_SEVERITY_MEDIUM;
    context->metadata.timeout_ns = 30000000000ULL; // 30 seconds
    context->metadata.skip = false;
    context->metadata.skip_reason = NULL;
    
    // Initialize timing
    context->start_time_ns = asthra_test_get_time_ns();
    context->end_time_ns = 0;
    context->duration_ns = 0;
    context->error_message = NULL;
    context->error_message_allocated = false;
    context->assertions_in_test = 0;
    context->global_stats = NULL;
}

// =============================================================================
// PARSER FUNCTION STUBS
// =============================================================================

/**
 * Create lexer stub
 * Returns dummy pointer for basic functionality
 */
Lexer* lexer_create_stub(const char* source) {
    (void)source; // Suppress unused parameter warning
    
    // Return dummy pointer to indicate "success"
    // Integration tests will use this for basic compilation testing
    return (Lexer*)0x1;
}

/**
 * Create parser stub
 * Returns dummy pointer for basic functionality
 */
Parser* parser_create_stub(Lexer* lexer) {
    (void)lexer; // Suppress unused parameter warning
    
    // Return dummy pointer to indicate "success"
    return (Parser*)0x2;
}

/**
 * Destroy parser stub
 * No-op for stub implementation
 */
void parser_destroy_stub(Parser* parser) {
    (void)parser; // Suppress unused parameter warning
    // No-op for stub implementation
}

/**
 * Parse program stub
 * Returns dummy AST pointer for basic functionality
 */
ASTNode* parser_parse_program_stub(Parser* parser) {
    (void)parser; // Suppress unused parameter warning
    
    // Return dummy AST pointer to indicate "success"
    return (ASTNode*)0x3;
}

// =============================================================================
// SEMANTIC ANALYSIS FUNCTION STUBS
// =============================================================================

/**
 * Analyze program semantically stub
 * Returns success for basic functionality
 */
int semantic_analyze_program_stub(SemanticAnalyzer* analyzer, ASTNode* ast) {
    (void)analyzer; // Suppress unused parameter warning
    (void)ast;      // Suppress unused parameter warning
    
    // Return 0 to indicate "success"
    return 0;
}

/**
 * Create semantic analyzer stub
 * Returns dummy pointer for basic functionality
 */
SemanticAnalyzer* semantic_analyzer_create_stub(void) {
    // Return dummy pointer to indicate "success"
    return (SemanticAnalyzer*)0x4;
}

/**
 * Destroy semantic analyzer stub
 * No-op for stub implementation
 */
void semantic_analyzer_destroy_stub(SemanticAnalyzer* analyzer) {
    (void)analyzer; // Suppress unused parameter warning
    // No-op for stub implementation
} 
