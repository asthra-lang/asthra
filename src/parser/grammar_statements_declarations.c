/**
 * Asthra Programming Language Compiler - Declaration Statement Grammar Productions
 * Variable declarations with pattern destructuring support
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include "grammar_statements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// DECLARATION STATEMENT PARSING
// =============================================================================

ASTNode *parse_var_decl(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_LET)) {
        return NULL;
    }

    // Check for optional mut keyword (immutable by default)
    bool is_mutable = false;
    if (match_token(parser, TOKEN_MUT)) {
        is_mutable = true;
        advance_token(parser);
    }

    // Check if we have a pattern (tuple) or simple identifier
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        // This is a tuple pattern destructuring
        // Parse as pattern
        ASTNode *pattern = parse_pattern(parser);
        if (!pattern) {
            return NULL;
        }

        // Type annotation now REQUIRED
        if (!expect_token(parser, TOKEN_COLON)) {
            report_error(parser,
                         "Expected ':' after pattern in declaration. "
                         "Type annotations are mandatory - use 'let (x, y): Type = value;'");
            ast_free_node(pattern);
            return NULL;
        }

        ASTNode *type = parse_type(parser);
        if (!type) {
            report_error(parser, "Expected type annotation after ':'");
            ast_free_node(pattern);
            return NULL;
        }

        ASTNode *initializer = NULL;
        if (match_token(parser, TOKEN_ASSIGN)) {
            advance_token(parser);
            initializer = parse_expr(parser);
            if (!initializer) {
                ast_free_node(pattern);
                ast_free_node(type);
                return NULL;
            }
        }

        if (!expect_token(parser, TOKEN_SEMICOLON)) {
            ast_free_node(pattern);
            ast_free_node(type);
            if (initializer)
                ast_free_node(initializer);
            return NULL;
        }

        // For now, we'll create an if-let statement to handle pattern matching
        // This is a workaround until we have proper pattern-based let statements
        ASTNode *node = ast_create_node(AST_IF_LET_STMT, start_loc);
        if (!node) {
            ast_free_node(pattern);
            ast_free_node(type);
            if (initializer)
                ast_free_node(initializer);
            return NULL;
        }

        // Create a block with no statements for the then block
        ASTNode *then_block = ast_create_node(AST_BLOCK, start_loc);
        if (!then_block) {
            ast_free_node(pattern);
            ast_free_node(type);
            if (initializer)
                ast_free_node(initializer);
            ast_free_node(node);
            return NULL;
        }
        then_block->data.block.statements = NULL;

        node->data.if_let_stmt.pattern = pattern;
        node->data.if_let_stmt.expression = initializer;
        node->data.if_let_stmt.then_block = then_block;
        node->data.if_let_stmt.else_block = NULL;

        // Note: This is a temporary implementation.
        // TODO: Create proper AST_LET_PATTERN_STMT node type

        return node;
    }

    // Regular identifier-based let statement
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        // Check if this is a reserved keyword being used as a variable name
        TokenType current_type = parser->current_token.type;
        if (current_type == TOKEN_FN || current_type == TOKEN_LET || current_type == TOKEN_CONST ||
            current_type == TOKEN_MUT || current_type == TOKEN_IF || current_type == TOKEN_ELSE ||
            current_type == TOKEN_ENUM || current_type == TOKEN_MATCH ||
            current_type == TOKEN_RETURN || current_type == TOKEN_STRUCT ||
            current_type == TOKEN_EXTERN || current_type == TOKEN_SPAWN ||
            current_type == TOKEN_UNSAFE || current_type == TOKEN_PACKAGE ||
            current_type == TOKEN_IMPORT || current_type == TOKEN_FOR || current_type == TOKEN_IN ||
            current_type == TOKEN_AWAIT || current_type == TOKEN_BREAK ||
            current_type == TOKEN_CONTINUE || current_type == TOKEN_SIZEOF ||
            current_type == TOKEN_PUB || current_type == TOKEN_PRIV || current_type == TOKEN_IMPL ||
            current_type == TOKEN_SELF || current_type == TOKEN_VOID ||
            current_type == TOKEN_NONE || current_type == TOKEN_AS ||
            current_type == TOKEN_BOOL_TRUE || current_type == TOKEN_BOOL_FALSE ||
            current_type == TOKEN_RESULT || current_type == TOKEN_OPTION ||
            current_type == TOKEN_TASKHANDLE || current_type == TOKEN_SPAWN_WITH_HANDLE) {
            report_error(parser, "Cannot use reserved keyword as variable name");
        } else {
            report_error(parser, "Expected variable name or pattern");
        }
        return NULL;
    }

    char *var_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    // Check if this might be missing type annotation
    if (match_token(parser, TOKEN_ASSIGN)) {
        // Missing type annotation - report clear error
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg),
                 "Missing type annotation in variable declaration. "
                 "Type annotations are mandatory - use 'let %s: Type = value;'",
                 var_name);
        report_error(parser, error_msg);
        free(var_name);
        return NULL;
    }

    // Type annotation now REQUIRED
    if (!expect_token(parser, TOKEN_COLON)) {
        report_error(parser, "Expected ':' after variable name in declaration. "
                             "Type annotations are mandatory - use 'let name: Type = value;'");
        free(var_name);
        return NULL;
    }

    ASTNode *type = parse_type(parser);
    if (!type) {
        report_error(parser, "Expected type annotation after ':'");
        free(var_name);
        return NULL;
    }

    // OwnershipTag is optional and comes after the type
    ASTNodeList *annotations = NULL;
    if (is_annotation_start(parser)) {
        // Parse annotations after type
        annotations = parse_annotation_list(parser);

        // Validate that only ownership annotations are allowed on variables
        if (annotations) {
            for (size_t i = 0; i < annotations->count; i++) {
                ASTNode *ann = annotations->nodes[i];
                if (ann->type != AST_OWNERSHIP_TAG) {
                    report_error(parser, "Only ownership annotations are allowed on variables");
                    // Don't free annotations here, they'll be freed with the node
                }
            }
        }
    }

    ASTNode *initializer = NULL;
    if (match_token(parser, TOKEN_ASSIGN)) {
        advance_token(parser);
        initializer = parse_expr(parser);
        if (!initializer) {
            free(var_name);
            ast_free_node(type);
            if (annotations)
                ast_node_list_destroy(annotations);
            return NULL;
        }
    }

    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        free(var_name);
        ast_free_node(type);
        if (initializer)
            ast_free_node(initializer);
        if (annotations)
            ast_node_list_destroy(annotations);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_LET_STMT, start_loc);
    if (!node) {
        free(var_name);
        ast_free_node(type);
        if (initializer)
            ast_free_node(initializer);
        if (annotations)
            ast_node_list_destroy(annotations);
        return NULL;
    }

    node->data.let_stmt.name = var_name;
    node->data.let_stmt.type = type; // Always non-NULL in v1.15+
    node->data.let_stmt.initializer = initializer;
    node->data.let_stmt.is_mutable = is_mutable;   // Set mutability flag
    node->data.let_stmt.annotations = annotations; // Ownership tags

    return node;
}