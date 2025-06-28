/**
 * Common utilities for enum variant integration tests
 * Shared helper functions and types
 */

#ifndef TEST_ENUM_INTEGRATION_COMMON_H
#define TEST_ENUM_INTEGRATION_COMMON_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the necessary headers for full integration testing
#include "ast.h"
#include "backend_interface.h"
#include "backend_test_wrapper.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_core.h"

// Test helper function to create a lexer from source code
static inline Lexer *create_test_lexer(const char *source) {
    return lexer_create(source, strlen(source), "test.asthra");
}

// Test helper function to parse a complete program
static inline ASTNode *parse_test_program(const char *source) {
    Lexer *lexer = create_test_lexer(source);
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    ASTNode *program = parser_parse_program(parser);

    if (parser->had_error) {
        printf("Parser errors:\n");
        const ParseError *error = parser->errors;
        while (error) {
            printf("  %s:%d:%d: %s\n",
                   error->location.filename ? error->location.filename : "unknown",
                   error->location.line, error->location.column, error->message);
            error = error->next;
        }
        if (program) {
            ast_free_node(program);
            program = NULL;
        }
    }

    parser_destroy(parser);
    lexer_destroy(lexer);

    return program;
}

#endif // TEST_ENUM_INTEGRATION_COMMON_H