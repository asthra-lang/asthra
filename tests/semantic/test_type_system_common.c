/**
 * Asthra Programming Language
 * Type System Test Common Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common utilities and fixtures for type system tests
 */

#include "test_type_system_common.h"
#include "../framework/semantic_test_utils.h"
#include "../framework/parser_test_utils.h"

// =============================================================================
// SHARED TEST FIXTURES
// =============================================================================

AsthraTestResult setup_type_checking_test(AsthraTestContext* context) {
    // Generic setup that can be used by all type system tests
    // For now, just return success as individual tests handle their own setup
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

AsthraTestResult teardown_type_checking_test(AsthraTestContext* context) {
    // Generic teardown that can be used by all type system tests
    // For now, just return success as individual tests handle their own cleanup
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

SemanticAnalyzer* create_test_semantic_analyzer(void) {
    SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        return NULL;
    }

    // Built-in types are initialized by setup_semantic_analyzer
    return analyzer;
}

void destroy_test_semantic_analyzer(SemanticAnalyzer* analyzer) {
    if (analyzer) {
        destroy_semantic_analyzer(analyzer);
    }
}

ASTNode* parse_test_source_code(const char* source, const char* filename) {
    return parse_test_source(source, filename);
}

bool analyze_test_ast_node(SemanticAnalyzer* analyzer, ASTNode* ast) {
    return analyze_test_ast(analyzer, ast);
} 
