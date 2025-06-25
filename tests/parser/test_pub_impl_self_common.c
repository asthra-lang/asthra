/**
 * Common Implementation for pub/impl/self Parser Tests
 *
 * This file provides shared utility functions and implementations
 * for the modular pub/impl/self parser test suite.
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "test_pub_impl_self_common.h"

// =============================================================================
// SHARED UTILITY IMPLEMENTATIONS
// =============================================================================

Lexer *create_test_lexer(const char *source) {
    return lexer_create(source, strlen(source), "test_pub_impl_self_parsing.asthra");
}

Parser *create_test_parser(const char *source) {
    Lexer *lexer = create_test_lexer(source);
    if (!lexer)
        return NULL;
    return parser_create(lexer);
}

void cleanup_parser(Parser *parser) {
    if (parser) {
        Lexer *lexer = parser_get_lexer(parser);
        parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
    }
}

// Base test metadata for all pub/impl/self tests
AsthraTestMetadata pub_impl_self_base_metadata = {.name = "pub/impl/self Parser Tests",
                                                  .file = __FILE__,
                                                  .line = __LINE__,
                                                  .function = __func__,
                                                  .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                                  .timeout_ns = 3000000000ULL, // 3 seconds
                                                  .skip = false,
                                                  .skip_reason = NULL};
