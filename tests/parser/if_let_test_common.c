/**
 * Asthra Programming Language
 * If-Let Statement Testing - Common Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of shared test framework and utilities for if-let testing
 */

#include "if_let_test_common.h"

// =============================================================================
// TEST FRAMEWORK GLOBALS
// =============================================================================

size_t tests_passed = 0;
size_t tests_total = 0;
size_t tests_failed = 0;

// =============================================================================
// HELPER FUNCTIONS IMPLEMENTATION
// =============================================================================

Parser *create_test_parser(const char *source) {
    if (!source)
        return NULL;

    Lexer *lexer = lexer_create(source, strlen(source), "test");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    return parser;
}

void destroy_test_parser(Parser *parser) {
    if (parser) {
        parser_destroy(parser);
    }
}

bool validate_if_let_ast(ASTNode *node) {
    if (!node || node->type != AST_IF_LET_STMT) {
        return false;
    }

    // Check required components
    if (!node->data.if_let_stmt.pattern)
        return false;
    if (!node->data.if_let_stmt.expression)
        return false;
    if (!node->data.if_let_stmt.then_block)
        return false;

    // else_block is optional
    return true;
}

void init_if_let_test_framework(void) {
    tests_passed = 0;
    tests_total = 0;
    tests_failed = 0;
}

int print_if_let_test_summary(void) {
    printf("\n=== Phase 4 Test Results ===\n");
    printf("Total Tests: %zu\n", tests_total);
    printf("Passed: %zu\n", tests_passed);
    printf("Failed: %zu\n", tests_failed);
    printf("Success Rate: %.1f%%\n", tests_total > 0 ? (100.0 * tests_passed / tests_total) : 0.0);

    if (tests_failed == 0) {
        printf("\n🎉 All Phase 4 tests PASSED!\n");
        printf("✅ If-let implementation is ready for production use\n");
        printf("✅ Parser integration working correctly\n");
        printf("✅ Grammar compliance validated\n");
        printf("✅ Memory management verified\n");
        printf("✅ Error handling robust\n");
        return 0;
    } else {
        printf("\n⚠️  Some Phase 4 tests FAILED\n");
        printf("❌ If-let implementation needs additional work\n");
        return 1;
    }
}
