/**
 * Asthra Programming Language Compiler - Core Pattern Grammar Implementation
 * Main pattern parsing logic
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_patterns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for functions in other pattern modules
extern ASTNode *parse_enum_pattern_impl(Parser *parser, char *name, SourceLocation start_loc);
extern ASTNode *parse_struct_pattern_impl(Parser *parser, char *name, SourceLocation start_loc);

// =============================================================================
// CORE PATTERN PARSING
// =============================================================================

ASTNode *parse_pattern(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

#ifdef DEBUG_PARSER
    fprintf(stderr, "parse_pattern: current token type=%d\n", parser->current_token.type);
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        fprintf(stderr, "parse_pattern: identifier='%s'\n",
                parser->current_token.data.identifier.name);
    }
#endif

    // Handle 'mut' modifier for mutable bindings in patterns
    bool is_mutable = false;
    if (match_token(parser, TOKEN_MUT)) {
        is_mutable = true;
        advance_token(parser);
        // Update start location after consuming 'mut'
        start_loc = parser->current_token.location;
    }

    // Handle enum patterns like Result.Ok(value) and Option.Some(value) and struct patterns like
    // Point { x, y } Also handle Result and Option keywords for enum patterns
    if (match_token(parser, TOKEN_IDENTIFIER) || match_token(parser, TOKEN_RESULT) ||
        match_token(parser, TOKEN_OPTION)) {
        char *name;
        if (parser->current_token.type == TOKEN_RESULT) {
            name = strdup("Result");
        } else if (parser->current_token.type == TOKEN_OPTION) {
            name = strdup("Option");
        } else {
            name = strdup(parser->current_token.data.identifier.name);
        }

        advance_token(parser);

        if (match_token(parser, TOKEN_DOT)) {
            // This is an enum pattern: EnumName.Variant(binding)
            return parse_enum_pattern_impl(parser, name, start_loc);
        } else if (match_token(parser, TOKEN_LESS_THAN) || match_token(parser, TOKEN_LEFT_BRACE)) {
            // This is a struct pattern: StructName<T> { field1, field2 } or StructName { field1,
            // field2 }
            return parse_struct_pattern_impl(parser, name, start_loc);
        } else if (match_token(parser, TOKEN_LEFT_PAREN)) {
            // This looks like a function call pattern, which is not supported
            // Users must use qualified syntax like Option.Some(x)
            report_error(parser, "Unqualified enum variant patterns are not allowed. Use qualified "
                                 "syntax like Option.Some(x) or Option.None");
            free(name);
            return NULL;
        } else {
            // Check for wildcard pattern
            if (strcmp(name, "_") == 0) {
                free(name);
                ASTNode *node = ast_create_node(AST_WILDCARD_PATTERN, start_loc);
                if (!node)
                    return NULL;
                return node;
            }

            // Simple identifier pattern
            ASTNode *node = ast_create_node(AST_IDENTIFIER, start_loc);
            if (!node) {
                free(name);
                return NULL;
            }

            node->data.identifier.name = name;
            // Note: For patterns, mutability is stored differently than regular identifiers
            // This might need to be handled at a higher level in the AST
            // For now, we'll just parse it correctly
            return node;
        }
    }

    // Note: According to grammar line 105, StructPattern requires SimpleIdent
    // StructPattern <- SimpleIdent TypeArgs? '{' FieldPattern (',' FieldPattern)* '}'
    // So we don't allow anonymous struct patterns like { x, y } here

    // Handle tuple patterns: (pattern1, pattern2, ...)
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);

        // Check for unit pattern ()
        if (match_token(parser, TOKEN_RIGHT_PAREN)) {
            advance_token(parser);
            // Unit pattern - treat as wildcard or special case
            ASTNode *node = ast_create_node(AST_WILDCARD_PATTERN, start_loc);
            if (!node)
                return NULL;
            return node;
        }

        // Parse tuple pattern
        ASTNodeList *patterns = ast_node_list_create(2);
        if (!patterns)
            return NULL;

        // Parse first pattern
        ASTNode *first_pattern = parse_pattern(parser);
        if (!first_pattern) {
            ast_node_list_destroy(patterns);
            return NULL;
        }

        // Check for comma - if no comma, it's just a parenthesized pattern
        if (!match_token(parser, TOKEN_COMMA)) {
            // Just a parenthesized pattern
            if (expect_token(parser, TOKEN_RIGHT_PAREN)) {
                ast_node_list_destroy(patterns);
                return first_pattern;
            } else {
                ast_free_node(first_pattern);
                ast_node_list_destroy(patterns);
                return NULL;
            }
        }

        // We have a comma, so this is a tuple pattern
        ast_node_list_add(&patterns, first_pattern);
        advance_token(parser); // consume comma

        // Parse remaining patterns
        do {
            ASTNode *pattern = parse_pattern(parser);
            if (!pattern) {
                // Clean up
                for (size_t i = 0; i < patterns->count; i++) {
                    ast_free_node(patterns->nodes[i]);
                }
                ast_node_list_destroy(patterns);
                return NULL;
            }

            ast_node_list_add(&patterns, pattern);

            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser) && !match_token(parser, TOKEN_RIGHT_PAREN));

        // Expect closing paren
        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            // Clean up
            for (size_t i = 0; i < patterns->count; i++) {
                ast_free_node(patterns->nodes[i]);
            }
            ast_node_list_destroy(patterns);
            return NULL;
        }

        // Create tuple pattern node
        ASTNode *node = ast_create_node(AST_TUPLE_PATTERN, start_loc);
        if (!node) {
            // Clean up
            for (size_t i = 0; i < patterns->count; i++) {
                ast_free_node(patterns->nodes[i]);
            }
            ast_node_list_destroy(patterns);
            return NULL;
        }

        node->data.tuple_pattern.patterns = patterns;
        return node;
    }

    // Note: Wildcard patterns are now handled in the identifier pattern section above

    // Handle literals and other patterns
    if (match_token(parser, TOKEN_INTEGER)) {
        int64_t value = parser->current_token.data.integer.value;
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_INTEGER_LITERAL, start_loc);
        if (!node)
            return NULL;

        node->data.integer_literal.value = value;
        node->flags.is_constant_expr = true; // Integer literals are compile-time constants
        return node;
    }

    if (match_token(parser, TOKEN_STRING)) {
        char *value = strdup(parser->current_token.data.string.value);
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_STRING_LITERAL, start_loc);
        if (!node) {
            free(value);
            return NULL;
        }

        node->data.string_literal.value = value;
        node->flags.is_constant_expr = true; // String literals are compile-time constants
        return node;
    }

    if (match_token(parser, TOKEN_BOOL_TRUE)) {
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_BOOL_LITERAL, start_loc);
        if (!node)
            return NULL;

        node->data.bool_literal.value = true;
        node->flags.is_constant_expr = true; // Boolean literals are compile-time constants
        return node;
    }

    if (match_token(parser, TOKEN_BOOL_FALSE)) {
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_BOOL_LITERAL, start_loc);
        if (!node)
            return NULL;

        node->data.bool_literal.value = false;
        node->flags.is_constant_expr = true; // Boolean literals are compile-time constants
        return node;
    }

    report_error(parser, "Expected pattern");
    return NULL;
}