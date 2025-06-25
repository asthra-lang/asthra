/**
 * Asthra Programming Language Compiler - Block Statement Grammar Productions
 * Block and unsafe block parsing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_statements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// BLOCK STATEMENT PARSING
// =============================================================================

ASTNode *parse_block(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        return NULL;
    }

    ASTNode **statements = NULL;
    size_t statement_count = 0;
    size_t statement_capacity = 8;

    statements = malloc(statement_capacity * sizeof(ASTNode *));
    if (!statements)
        return NULL;

    while (!match_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser)) {
        ASTNode *stmt = parse_statement(parser);
        if (!stmt) {
            if (!parser->config.allow_incomplete_parse) {
                for (size_t i = 0; i < statement_count; i++) {
                    ast_free_node(statements[i]);
                }
                free(statements);
                return NULL;
            }
            synchronize(parser);
            continue;
        }

        if (statement_count >= statement_capacity) {
            statement_capacity *= 2;
            statements = realloc(statements, statement_capacity * sizeof(ASTNode *));
            if (!statements) {
                ast_free_node(stmt);
                return NULL;
            }
        }

        statements[statement_count++] = stmt;
    }

    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        for (size_t i = 0; i < statement_count; i++) {
            ast_free_node(statements[i]);
        }
        free(statements);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_BLOCK, start_loc);
    if (!node) {
        for (size_t i = 0; i < statement_count; i++) {
            ast_free_node(statements[i]);
        }
        free(statements);
        return NULL;
    }

    // Convert statements array to ASTNodeList
    if (statements && statement_count > 0) {
        node->data.block.statements = ast_node_list_create(statement_count);
        if (node->data.block.statements) {
            for (size_t i = 0; i < statement_count; i++) {
                ast_node_list_add(&node->data.block.statements, statements[i]);
            }
        }
        free(statements);
    } else {
        node->data.block.statements = NULL;
    }

    return node;
}

ASTNode *parse_unsafe_block(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect 'unsafe' keyword
    if (!expect_token(parser, TOKEN_UNSAFE)) {
        return NULL;
    }

    // Parse the block that follows
    ASTNode *block = parse_block(parser);
    if (!block) {
        report_error(parser, "Expected block after 'unsafe' keyword");
        return NULL;
    }

    // Create the unsafe block node
    ASTNode *node = ast_create_node(AST_UNSAFE_BLOCK, start_loc);
    if (!node) {
        ast_free_node(block);
        return NULL;
    }

    node->data.unsafe_block.block = block;

    return node;
}