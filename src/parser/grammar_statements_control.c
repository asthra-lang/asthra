/**
 * Asthra Programming Language Compiler - Control Flow Statement Grammar Productions
 * Control flow statement parsing including if, for, and match statements
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_statements.h"
#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// CONTROL FLOW STATEMENTS
// =============================================================================

ASTNode *parse_if_stmt(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!expect_token(parser, TOKEN_IF)) {
        return NULL;
    }
    
    // Check for 'if let' pattern by peeking ahead
    if (match_token(parser, TOKEN_LET)) {
        // We found 'if let', so we need to parse this as an if-let statement
        // But we already consumed the 'if' token, so we need to reconstruct the parsing
        // Let's parse the pattern, expression, and blocks directly here
        
        // Advance past the 'let' token
        advance_token(parser);
        
        // Parse pattern
        ASTNode *pattern = parse_pattern(parser);
        if (!pattern) {
            report_error(parser, "Expected pattern after 'let' in if-let statement");
            return NULL;
        }
        
        // Expect '=' 
        if (!expect_token(parser, TOKEN_ASSIGN)) {
            ast_free_node(pattern);
            return NULL;
        }
        
        // Parse expression
        ASTNode *expression = parse_expr(parser);
        if (!expression) {
            ast_free_node(pattern);
            return NULL;
        }
        
        // Parse then block
        ASTNode *then_block = parse_block(parser);
        if (!then_block) {
            ast_free_node(pattern);
            ast_free_node(expression);
            return NULL;
        }
        
        // Parse optional else block
        ASTNode *else_block = NULL;
        if (match_token(parser, TOKEN_ELSE)) {
            advance_token(parser);
            else_block = parse_block(parser);
            if (!else_block) {
                ast_free_node(pattern);
                ast_free_node(expression);
                ast_free_node(then_block);
                return NULL;
            }
        }
        
        // Create if-let AST node
        ASTNode *node = ast_create_node(AST_IF_LET_STMT, start_loc);
        if (!node) {
            ast_free_node(pattern);
            ast_free_node(expression);
            ast_free_node(then_block);
            if (else_block) ast_free_node(else_block);
            return NULL;
        }
        
        node->data.if_let_stmt.pattern = pattern;
        node->data.if_let_stmt.expression = expression;
        node->data.if_let_stmt.then_block = then_block;
        node->data.if_let_stmt.else_block = else_block;
        
        return node;
    }
    
    // Continue with regular if statement parsing
    ASTNode *condition = parse_expr(parser);
    if (!condition) return NULL;
    
    ASTNode *then_block = parse_block(parser);
    if (!then_block) {
        ast_free_node(condition);
        return NULL;
    }
    
    ASTNode *else_block = NULL;
    if (match_token(parser, TOKEN_ELSE)) {
        advance_token(parser);
        
        if (match_token(parser, TOKEN_IF)) {
            // Parse "else if" as nested if statement
            else_block = parse_if_stmt(parser);
        } else {
            // Parse "else" block
            else_block = parse_block(parser);
        }
        
        if (!else_block) {
            ast_free_node(condition);
            ast_free_node(then_block);
            return NULL;
        }
    }
    
    ASTNode *node = ast_create_node(AST_IF_STMT, start_loc);
    if (!node) {
        ast_free_node(condition);
        ast_free_node(then_block);
        if (else_block) ast_free_node(else_block);
        return NULL;
    }
    
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_block = then_block;
    node->data.if_stmt.else_block = else_block;
    
    return node;
}

ASTNode *parse_if_let_stmt(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Expect 'if' 'let' sequence
    if (!expect_token(parser, TOKEN_IF)) return NULL;
    if (!expect_token(parser, TOKEN_LET)) return NULL;
    
    // Parse pattern
    ASTNode *pattern = parse_pattern(parser);
    if (!pattern) return NULL;
    
    // Expect '=' 
    if (!expect_token(parser, TOKEN_ASSIGN)) {
        ast_free_node(pattern);
        return NULL;
    }
    
    // Parse expression
    ASTNode *expression = parse_expr(parser);
    if (!expression) {
        ast_free_node(pattern);
        return NULL;
    }
    
    // Parse then block
    ASTNode *then_block = parse_block(parser);
    if (!then_block) {
        ast_free_node(pattern);
        ast_free_node(expression);
        return NULL;
    }
    
    // Parse optional else block
    ASTNode *else_block = NULL;
    if (match_token(parser, TOKEN_ELSE)) {
        advance_token(parser);
        else_block = parse_block(parser);
        if (!else_block) {
            ast_free_node(pattern);
            ast_free_node(expression);
            ast_free_node(then_block);
            return NULL;
        }
    }
    
    // Create if-let AST node
    ASTNode *node = ast_create_node(AST_IF_LET_STMT, start_loc);
    if (!node) {
        ast_free_node(pattern);
        ast_free_node(expression);
        ast_free_node(then_block);
        if (else_block) ast_free_node(else_block);
        return NULL;
    }
    
    node->data.if_let_stmt.pattern = pattern;
    node->data.if_let_stmt.expression = expression;
    node->data.if_let_stmt.then_block = then_block;
    node->data.if_let_stmt.else_block = else_block;
    
    return node;
}

ASTNode *parse_for_stmt(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!expect_token(parser, TOKEN_FOR)) {
        return NULL;
    }
    
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected loop variable name");
        return NULL;
    }
    
    char *variable = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    if (!expect_token(parser, TOKEN_IN)) {
        free(variable);
        return NULL;
    }
    
    ASTNode *iterable = parse_expr(parser);
    if (!iterable) {
        free(variable);
        return NULL;
    }
    
    ASTNode *body = parse_block(parser);
    if (!body) {
        free(variable);
        ast_free_node(iterable);
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_FOR_STMT, start_loc);
    if (!node) {
        free(variable);
        ast_free_node(iterable);
        ast_free_node(body);
        return NULL;
    }
    
    node->data.for_stmt.variable = variable;
    node->data.for_stmt.iterable = iterable;
    node->data.for_stmt.body = body;
    
    return node;
}

ASTNode *parse_match_stmt(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    if (!expect_token(parser, TOKEN_MATCH)) {
        return NULL;
    }
    
    ASTNode *expression = parse_expr(parser);
    if (!expression) return NULL;
    
    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        ast_free_node(expression);
        return NULL;
    }
    
    ASTNode **arms = NULL;
    size_t arm_count = 0;
    size_t arm_capacity = 4;
    
    arms = malloc(arm_capacity * sizeof(ASTNode*));
    if (!arms) {
        ast_free_node(expression);
        return NULL;
    }
    
    while (!match_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser)) {
        ASTNode *arm = parse_match_arm(parser);
        if (!arm) {
            for (size_t i = 0; i < arm_count; i++) {
                ast_free_node(arms[i]);
            }
            free(arms);
            ast_free_node(expression);
            return NULL;
        }
        
        if (arm_count >= arm_capacity) {
            arm_capacity *= 2;
            arms = realloc(arms, arm_capacity * sizeof(ASTNode*));
            if (!arms) {
                ast_free_node(arm);
                ast_free_node(expression);
                return NULL;
            }
        }
        
        arms[arm_count++] = arm;
        
        // Handle optional comma separators between match arms
        if (match_token(parser, TOKEN_COMMA)) {
            advance_token(parser);
            // After comma, we must have another match arm (no trailing comma)
            if (match_token(parser, TOKEN_RIGHT_BRACE)) {
                report_error(parser, "Trailing comma not allowed in match arms");
                for (size_t i = 0; i < arm_count; i++) {
                    ast_free_node(arms[i]);
                }
                free(arms);
                ast_free_node(expression);
                return NULL;
            }
            // Continue parsing the next match arm
        } else if (!match_token(parser, TOKEN_RIGHT_BRACE)) {
            // If no comma and not at end, we still continue (match arms can be separated by nothing)
            // This maintains compatibility with grammar while supporting comma separators
        }
    }
    
    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        for (size_t i = 0; i < arm_count; i++) {
            ast_free_node(arms[i]);
        }
        free(arms);
        ast_free_node(expression);
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_MATCH_STMT, start_loc);
    if (!node) {
        for (size_t i = 0; i < arm_count; i++) {
            ast_free_node(arms[i]);
        }
        free(arms);
        ast_free_node(expression);
        return NULL;
    }
    
    node->data.match_stmt.expression = expression;
    
    // Convert arms array to ASTNodeList
    if (arms && arm_count > 0) {
        node->data.match_stmt.arms = ast_node_list_create(arm_count);
        if (node->data.match_stmt.arms) {
            for (size_t i = 0; i < arm_count; i++) {
                ast_node_list_add(&node->data.match_stmt.arms, arms[i]);
            }
        }
        free(arms);
    } else {
        node->data.match_stmt.arms = NULL;
    }
    
    return node;
}

ASTNode *parse_match_arm(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    ASTNode *pattern = parse_pattern(parser);
    if (!pattern) {
        return NULL;
    }
    
    if (!expect_token(parser, TOKEN_FAT_ARROW)) {
        ast_free_node(pattern);
        return NULL;
    }
    
    ASTNode *body;
    if (match_token(parser, TOKEN_LEFT_BRACE)) {
        body = parse_block(parser);
    } else {
        body = parse_expr(parser);
        // Make semicolon optional for match arm expression bodies
        if (body && match_token(parser, TOKEN_SEMICOLON)) {
            advance_token(parser);  // consume optional semicolon
        }
    }
    
    if (!body) {
        ast_free_node(pattern);
        return NULL;
    }
    
    ASTNode *node = ast_create_node(AST_MATCH_ARM, start_loc);
    if (!node) {
        ast_free_node(pattern);
        ast_free_node(body);
        return NULL;
    }
    
    node->data.match_arm.pattern = pattern;
    node->data.match_arm.body = body;
    
    return node;
} 
