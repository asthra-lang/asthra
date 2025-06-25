/**
 * Asthra Programming Language Compiler - Core Statement Grammar Productions
 * Core statement parsing dispatcher with modular statement type parsers
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

// Include the modular statement parsing components
#include "grammar_statements_declarations.c"
#include "grammar_statements_simple.c"
#include "grammar_statements_blocks.c"
#include "grammar_statements_assignments.c"

// =============================================================================
// CORE STATEMENT PARSING
// =============================================================================

ASTNode *parse_statement(Parser *parser) {
    if (!parser)
        return NULL;

    // Try to parse assignment first by looking ahead
    if (check_token(parser, TOKEN_IDENTIFIER)) {
        // Look ahead to see if this might be an assignment
        Token next = peek_token(parser);
        if (next.type == TOKEN_ASSIGN) {
            // Parse as assignment - use parse_lvalue for left side according to grammar
            ASTNode *lvalue = parse_lvalue(parser);
            if (!lvalue)
                return NULL;

            SourceLocation assign_loc = parser->current_token.location;
            if (!expect_token(parser, TOKEN_ASSIGN)) {
                ast_free_node(lvalue);
                return NULL;
            }

            ASTNode *rvalue = parse_expr(parser);
            if (!rvalue) {
                ast_free_node(lvalue);
                return NULL;
            }

            if (!expect_token(parser, TOKEN_SEMICOLON)) {
                ast_free_node(lvalue);
                ast_free_node(rvalue);
                return NULL;
            }

            ASTNode *assign = ast_create_node(AST_ASSIGNMENT, assign_loc);
            if (!assign) {
                ast_free_node(lvalue);
                ast_free_node(rvalue);
                return NULL;
            }

            assign->data.assignment.target = lvalue;
            assign->data.assignment.value = rvalue;

            return assign;
        }
    }

    // Try other statement types
    if (match_token(parser, TOKEN_LET)) {
        return parse_var_decl(parser);
    } else if (match_token(parser, TOKEN_RETURN)) {
        return parse_return_stmt(parser);
    } else if (match_token(parser, TOKEN_IF)) {
        return parse_if_stmt(parser);
    } else if (match_token(parser, TOKEN_FOR)) {
        return parse_for_stmt(parser);
    } else if (match_token(parser, TOKEN_MATCH)) {
        return parse_match_stmt(parser);
    } else if (match_token(parser, TOKEN_SPAWN)) {
        return parse_spawn_stmt(parser);
    } else if (match_token(parser, TOKEN_SPAWN_WITH_HANDLE)) {
        return parse_spawn_stmt(parser);
    } else if (match_token(parser, TOKEN_UNSAFE)) {
        return parse_unsafe_block(parser);
    } else if (match_token(parser, TOKEN_BREAK)) {
        return parse_break_stmt(parser);
    } else if (match_token(parser, TOKEN_CONTINUE)) {
        return parse_continue_stmt(parser);
    } else if (match_token(parser, TOKEN_LEFT_BRACE)) {
        return parse_block(parser);
    } else {
        return parse_expr_stmt(parser);
    }
}

// parse_var_decl is now defined in grammar_statements_declarations.c

// parse_return_stmt is now defined in grammar_statements_simple.c

// parse_expr_stmt is now defined in grammar_statements_simple.c

// parse_assign_stmt is now defined in grammar_statements_simple.c

// parse_block is now defined in grammar_statements_blocks.c

// parse_lvalue is now defined in grammar_statements_assignments.c

// parse_unsafe_block is now defined in grammar_statements_blocks.c

// parse_break_stmt is now defined in grammar_statements_simple.c

// parse_continue_stmt is now defined in grammar_statements_simple.c
