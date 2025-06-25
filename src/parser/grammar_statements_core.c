/**
 * Asthra Programming Language Compiler - Core Statement Grammar Productions
 * Core statement parsing including variable declarations, returns, expressions, and blocks
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
        report_error(parser, "Expected variable name or pattern");
        return NULL;
    }

    char *var_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

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

ASTNode *parse_return_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_RETURN)) {
        return NULL;
    }

    // Always require expression
    ASTNode *expression = parse_expr(parser);
    if (!expression) {
        report_error(parser, "Expected expression after 'return' keyword. "
                             "Void returns must use 'return ();'");
        return NULL;
    }

    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        ast_free_node(expression);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_RETURN_STMT, start_loc);
    if (!node) {
        ast_free_node(expression);
        return NULL;
    }

    node->data.return_stmt.expression = expression; // Always non-NULL in current version

    return node;
}

ASTNode *parse_expr_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // First parse as expression
    ASTNode *expression = parse_expr(parser);
    if (!expression)
        return NULL;

    // Check if this is actually an assignment that we missed
    if (match_token(parser, TOKEN_ASSIGN)) {
        // This is an assignment, not an expression statement
        // The expression we parsed should be a valid lvalue
        advance_token(parser); // consume '='

        ASTNode *rvalue = parse_expr(parser);
        if (!rvalue) {
            ast_free_node(expression);
            return NULL;
        }

        if (!expect_token(parser, TOKEN_SEMICOLON)) {
            ast_free_node(expression);
            ast_free_node(rvalue);
            return NULL;
        }

        // Create assignment node
        ASTNode *assign = ast_create_node(AST_ASSIGNMENT, start_loc);
        if (!assign) {
            ast_free_node(expression);
            ast_free_node(rvalue);
            return NULL;
        }

        assign->data.assignment.target = expression; // Use the expression as lvalue
        assign->data.assignment.value = rvalue;

        return assign;
    }

    // Not an assignment, expect semicolon for expression statement
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        ast_free_node(expression);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_EXPR_STMT, start_loc);
    if (!node) {
        ast_free_node(expression);
        return NULL;
    }

    node->data.expr_stmt.expression = expression;

    return node;
}

ASTNode *parse_assign_stmt(Parser *parser) {
    // This function is now handled within parse_statement
    // keeping for interface compatibility
    return parse_statement(parser);
}

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

ASTNode *parse_lvalue(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Parse SimpleIdent
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected identifier in lvalue");
        return NULL;
    }

    char *name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    ASTNode *lvalue = ast_create_node(AST_IDENTIFIER, start_loc);
    if (!lvalue) {
        free(name);
        return NULL;
    }

    lvalue->data.identifier.name = name;

    // Parse LValueSuffix*
    while (true) {
        if (match_token(parser, TOKEN_DOT)) {
            // Field access: '.' SimpleIdent
            advance_token(parser);

            if (!match_token(parser, TOKEN_IDENTIFIER)) {
                report_error(parser, "Expected field name after '.'");
                ast_free_node(lvalue);
                return NULL;
            }

            char *field_name = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);

            ASTNode *field_access =
                ast_create_node(AST_FIELD_ACCESS, parser->current_token.location);
            if (!field_access) {
                free(field_name);
                ast_free_node(lvalue);
                return NULL;
            }

            field_access->data.field_access.object = lvalue;
            field_access->data.field_access.field_name = field_name;
            lvalue = field_access;

        } else if (match_token(parser, TOKEN_LEFT_BRACKET)) {
            // Array indexing: '[' Expr ']'
            advance_token(parser);

            ASTNode *index = parse_expr(parser);
            if (!index) {
                ast_free_node(lvalue);
                return NULL;
            }

            if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
                ast_free_node(index);
                ast_free_node(lvalue);
                return NULL;
            }

            ASTNode *array_access =
                ast_create_node(AST_INDEX_ACCESS, parser->current_token.location);
            if (!array_access) {
                ast_free_node(index);
                ast_free_node(lvalue);
                return NULL;
            }

            array_access->data.index_access.array = lvalue;
            array_access->data.index_access.index = index;
            lvalue = array_access;

        } else if (match_token(parser, TOKEN_MULTIPLY)) {
            // Pointer dereference: '*'
            advance_token(parser);

            ASTNode *deref = ast_create_node(AST_UNARY_EXPR, parser->current_token.location);
            if (!deref) {
                ast_free_node(lvalue);
                return NULL;
            }

            deref->data.unary_expr.operator= UNOP_DEREF;
            deref->data.unary_expr.operand = lvalue;
            lvalue = deref;

        } else {
            // No more suffixes
            break;
        }
    }

    return lvalue;
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

ASTNode *parse_break_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect 'break' keyword
    if (!expect_token(parser, TOKEN_BREAK)) {
        return NULL;
    }

    // Expect semicolon
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        report_error(parser, "Expected ';' after 'break' statement");
        return NULL;
    }

    // Create the break statement node
    ASTNode *node = ast_create_node(AST_BREAK_STMT, start_loc);
    if (!node) {
        return NULL;
    }

    return node;
}

ASTNode *parse_continue_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect 'continue' keyword
    if (!expect_token(parser, TOKEN_CONTINUE)) {
        return NULL;
    }

    // Expect semicolon
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        report_error(parser, "Expected ';' after 'continue' statement");
        return NULL;
    }

    // Create the continue statement node
    ASTNode *node = ast_create_node(AST_CONTINUE_STMT, start_loc);
    if (!node) {
        return NULL;
    }

    return node;
}
