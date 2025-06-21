/**
 * Asthra Programming Language Compiler - Literal Parsing
 * Parsing of boolean, integer, float, string, and character literals
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "parser_ast_helpers.h"
#include <stdlib.h>
#include <string.h>

/**
 * Parse boolean literal (true or false)
 */
ASTNode *parse_bool_literal(Parser *parser, SourceLocation start_loc) {
    if (!parser) return NULL;
    
    bool value = false;
    if (match_token(parser, TOKEN_BOOL_TRUE)) {
        value = true;
        advance_token(parser);
    } else if (match_token(parser, TOKEN_BOOL_FALSE)) {
        value = false;
        advance_token(parser);
    } else {
        return NULL; // Not a boolean literal
    }
    
    ASTNode *node = ast_create_node(AST_BOOL_LITERAL, start_loc);
    if (!node) return NULL;
    
    node->data.bool_literal.value = value;
    return node;
}

/**
 * Parse integer literal
 */
ASTNode *parse_integer_literal(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_INTEGER)) return NULL;
    
    int64_t value = parser->current_token.data.integer.value;
    advance_token(parser);
    
    ASTNode *node = ast_create_node(AST_INTEGER_LITERAL, start_loc);
    if (!node) return NULL;
    
    node->data.integer_literal.value = value;
    return node;
}

/**
 * Parse float literal
 */
ASTNode *parse_float_literal(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_FLOAT)) return NULL;
    
    double value = parser->current_token.data.float_val.value;
    advance_token(parser);
    
    ASTNode *node = ast_create_node(AST_FLOAT_LITERAL, start_loc);
    if (!node) return NULL;
    
    node->data.float_literal.value = value;
    return node;
}

/**
 * Parse string literal
 */
ASTNode *parse_string_literal(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_STRING)) return NULL;
    
    char *value = strdup(parser->current_token.data.string.value);
    advance_token(parser);
    
    ASTNode *node = ast_create_node(AST_STRING_LITERAL, start_loc);
    if (!node) {
        free(value);
        return NULL;
    }
    
    node->data.string_literal.value = value;
    return node;
}

/**
 * Parse character literal
 */
ASTNode *parse_char_literal(Parser *parser, SourceLocation start_loc) {
    if (!parser || !match_token(parser, TOKEN_CHAR)) return NULL;
    
    int32_t value = parser->current_token.data.character.value;
    advance_token(parser);
    
    ASTNode *node = ast_create_node(AST_CHAR_LITERAL, start_loc);
    if (!node) return NULL;
    
    node->data.char_literal.value = value;
    return node;
}